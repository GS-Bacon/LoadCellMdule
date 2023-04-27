#include <M5Stack.h>
#include "HX711.h"

const int DT_PIN=36;
const int SCK_PIN=26;

long offset;
String jobname;

HX711 scale;

void setup(){
  M5.begin();
  M5.Power.begin();
  M5.Lcd.begin();
  Serial.begin(115200);
  scale.begin(DT_PIN,SCK_PIN);
  }

void loop(){

  if (scale.is_ready()) {
  } else {
    M5.Lcd.println("HX711 not found.");
  }

  delay(1000);

}

long GetOffSetValue(int n){
}