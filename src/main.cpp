#include <M5Stack.h>
#include "HX711.h"

const int DT_PIN=36;
const int SCK_PIN=26;
const int DISPLAY_X=320;
const int DISPLAY_Y=240;

String jobname;
long GetOffSetValue(int n);

HX711 scale;

void setup(){
  M5.begin();
  M5.Power.begin();
  M5.Lcd.begin();
  Serial.begin(115200);
  scale.begin(DT_PIN,SCK_PIN);
  }

void loop(){
  M5.Lcd.println(jobname);

  delay(1000);

}

long GetOffSetValue(int n){
  long offset;
  for(size_t i=0;i<n;i++){
  offset+=scale.read_average(10);
  jobname=("Wait for"+String(n-i)+"sec");
}
jobname=("offset value="+String(offset/n));
return offset;

}

int TextCentering(int pixel,int size,int textlen){
  return (DISPLAY_X-(pixel*size*textlen))/2;
}