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
char ssid[] = "U+Net1B3B";
char pass[] = "1000003762";

const int MODE_NUM = 3;
int mymode = 1;
int toggle = 0;
int change = 0;
int power = 0;

// mode 1
int cds = 0;

// mode 2
int r = 255, g = 255, b = 255;
int brightness = 255;

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


void setup() {
  Serial.begin(9600);
  delay(100);

  Blynk.begin(auth, ssid, pass);
  delay(100);

#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixels.begin();
  delay(100);
}


void loop() {
  Blynk.run();

  int temp = digitalRead(D1);
  Serial.println(temp);
  
  cds = analogRead(A0);
  int cds_brightness = get_cds_brightness();
  
  if(power == 1){
    changeMode();

    // auto
    if(mymode == 1){
      for(int i = 0; i < NUMPIXELS; i++){
         pixels.setPixelColor(i, pixels.Color(r, g, b));
         pixels.setBrightness(cds_brightness);
      }
    }
    // select
    else if(mymode == 2){
      for(int i = 0; i < NUMPIXELS; i++){
         pixels.setPixelColor(i, pixels.Color(r, g, b));
         pixels.setBrightness(brightness);
      }
    }
    // random
    else if(mymode == 3){
      for(int i = 0; i < NUMPIXELS; i++){
         pixels.setPixelColor(i, pixels.Color(random(0, 256), random(0,256), random(0, 256)));
         pixels.setBrightness(random(10, 256));
      }
    }
  } else{
    for(int i = 0; i < NUMPIXELS; i++)
       pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
  
  pixels.show();
}
