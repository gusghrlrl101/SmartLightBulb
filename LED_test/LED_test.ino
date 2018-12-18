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
  
  for(int i = 0; i < NUMPIXELS; i++){
     pixels.setPixelColor(i, pixels.Color(255, 255, 255));
  }
  delay(100);
}

int r = 0, g = 0, b = 0;

int mymode = 1;
int toggle = 0;
int change = 0;

const int MODE_NUM = 3;

BLYNK_WRITE(V0){
  r = param[0].asInt();
  g = param[1].asInt();
  b = param[2].asInt();
}

BLYNK_WRITE(V1){
  toggle = param.asInt();
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

void loop() {
  Blynk.run();
  changeMode();

  if(mymode == 2){
    for(int i = 0; i < NUMPIXELS; i++){
       pixels.setPixelColor(i, pixels.Color(r, g, b));
    }
  }
  
  pixels.show();
}
