#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#define PIN            14
#define NUMPIXELS      12
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
int delayval = 500; // delay for half a second

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
char auth[] = "1084182bd58f49f5817670c4d51d183b";
char ssid[] = "HyunHoPhone";
char pass[] = "01089050241";

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#define WLAN_SSID "HyunHoPhone"
#define WLAN_PASS "01089050241"
#define MQTT_SERVER "172.20.10.11"
#define MQTT_PORT 1883
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);
Adafruit_MQTT_Subscribe rasp_out = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME "/rasp"); 

void MQTT_connect() { 
 int8_t ret; 
 // Stop if already connected. 
 if (mqtt.connected()) { 
   return; 
 } 
 Serial.print("Connecting to MQTT... "); 
 uint8_t retries = 3; 
 while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected 
      Serial.println(mqtt.connectErrorString(ret)); 
      Serial.println("Retrying MQTT connection in 5 seconds..."); 
      mqtt.disconnect(); 
      delay(5000);  // wait 5 seconds 
      retries--; 
      if (retries == 0) { 
        // basically die and wait for WDT to reset me 
        while (1); 
      } 
 } 
 Serial.println("MQTT Connected!"); 
}  

const int MODE_NUM = 3;
int mymode = 1;
int toggle = 0;
int change = 0;
int power = 0;

// mode 1
int cds = 0;
int pow2 = 0;
int mode1_nochange = 0;

// mode 2
int r = 255, g = 255, b = 255;
int rr = 255, gg = 255, bb = 255;
int brightness = 255;

int first = 1;

// RGB read
BLYNK_WRITE(V0){
  r = param[0].asInt();
  g = param[1].asInt();
  b = param[2].asInt();
}

// mode change read
BLYNK_WRITE(V1){
  toggle = param.asInt();
}

// power read
BLYNK_WRITE(V3){
  power = param.asInt();
}

// brightness read
BLYNK_WRITE(V4){
  brightness = param.asInt();
}

// brightness read
BLYNK_WRITE(V5){
  mode1_nochange = param.asInt();
}

void changeMode(){
  if(change == 0){
    if(toggle == 1)
      change = 1;
  } else{
    if(toggle == 0){
      change = 0;
      if (mymode++ == MODE_NUM)
        mymode = 1;
    }
  }
  Blynk.virtualWrite(V2, mymode);
}

// change cds_brighness
int get_cds_brightness(){
  int cds_brightness = 0;
  if(200 < cds)
    cds_brightness = 255;
  else if(50 < cds)
    cds_brightness = 100;
  else
    cds_brightness = 2;
  return cds_brightness;
}

String getRasp(){
  String str1;
  Adafruit_MQTT_Subscribe *subscription; 
   while ((subscription = mqtt.readSubscription())) { 
     if (subscription == &rasp_out) { 
       str1 = (char *)rasp_out.lastread; 
     } 
   }

   return str1;
}

///////////setup////////////////////////

void setup() {
  Serial.begin(9600);
  delay(100);
  
  Blynk.begin(auth, ssid, pass);
  delay(100);

 WiFi.begin(WLAN_SSID, WLAN_PASS); 
 while (WiFi.status() != WL_CONNECTED) { 
   delay(500); 
   Serial.print("."); 
 } 
  mqtt.subscribe(&rasp_out);
  
  pinMode(D3, OUTPUT);

#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixels.begin();
  delay(100);
}


////////////////loop/////////////////////

void loop() { 
  Blynk.run();
  MQTT_connect();
  if(first == 1){
    Serial.println(mqtt.connected());
    first = 0;
    power = 1;
    Blynk.virtualWrite(3, HIGH);
  } 
  
  
  int pow = digitalRead(D6);
  if (pow2 != pow && pow == 1){
    if(power == 1){
      power = 0;
      Blynk.virtualWrite(3, LOW);
    }
    else{
      power = 1;
      Blynk.virtualWrite(3, HIGH);
    }
  }
  pow2 = pow;

  int moving = digitalRead(D1);
  if (moving == 1){
    power = 1;
    Blynk.virtualWrite(3, HIGH);
  }

  int alerm = digitalRead(D2);

  cds = analogRead(A0);
  int cds_brightness = get_cds_brightness();
  
  if(power == 1){
    digitalWrite(D3, HIGH);
    changeMode();

    if(alerm == 0){
      // auto
      if(mymode == 1){
        String s = getRasp();
        int xx = s.indexOf(' ');
        int yy = s.lastIndexOf(' ');

        if(mode1_nochange==0){
          if(xx != -1 && yy != -1){
            rr = s.substring(0, xx).toInt();
            gg = s.substring(xx, yy).toInt();
            bb = s.substring(yy).toInt();
          }
        }
        
        pixels.setBrightness(cds_brightness);
        for(int i = 0; i < NUMPIXELS; i++)
           pixels.setPixelColor(i, pixels.Color(rr, gg, bb));
      }
      // select
      else if(mymode == 2){
        pixels.setBrightness(brightness);
        for(int i = 0; i < NUMPIXELS; i++)
           pixels.setPixelColor(i, pixels.Color(r, g, b));
      }
      // random
      else if(mymode == 3){
        pixels.setBrightness(random(10, 256));
        for(int i = 0; i < NUMPIXELS; i++)
           pixels.setPixelColor(i, pixels.Color(random(0, 256), random(0,256), random(0, 256)));
      }
    }
    // alerm
    else{
      pixels.setBrightness(255);
        
      for(int i = 0; i < NUMPIXELS; i++)
         pixels.setPixelColor(i, pixels.Color(255, 0, 0));
      pixels.show();
      delay(500);
      
      for(int i = 0; i < NUMPIXELS; i++)
         pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      pixels.show();
      delay(500);
    }
  }
  // power off
  else{
    digitalWrite(D3, LOW);
    for(int i = 0; i < NUMPIXELS; i++)
       pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
  
   Serial.print("mode: ");
   Serial.print(mymode);
   Serial.print(", power: ");
   Serial.print(power);
   Serial.print(", alerm: ");
   Serial.print(alerm);
   Serial.print(", moving: ");
   Serial.print(moving);
   Serial.print(", cds: ");
   Serial.print(cds);
   Serial.print(", r: ");
   Serial.print(r);
   Serial.print(", g: ");
   Serial.print(g);
   Serial.print(", b: ");
   Serial.print(b);
   Serial.print(", rr: ");
   Serial.print(rr);
   Serial.print(", gg: ");
   Serial.print(gg);
   Serial.print(", bb: ");
   Serial.println(bb);
   
  pixels.show();
  delay(100);
}
