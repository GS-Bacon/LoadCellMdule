#include <M5Stack.h>
#include "HX711.h"


const int DT_PIN = 36;
const int SCK_PIN = 26;

HX711 scale;

void setup() {
	Serial.begin(115200);
	Serial.println("start");
	scale.begin(DT_PIN, SCK_PIN);

	Serial.print("read:");
	Serial.println(scale.read());

	scale.set_scale();
	scale.tare();

	Serial.print("calibrating...");
	delay(5000);
	Serial.println(scale.get_units(50));

	scale.set_scale(-20.54);
	scale.tare();

	Serial.print("read (calibrated):");
	Serial.println(scale.get_units(10));
}


void loop() {
	Serial.println(scale.get_units(10), 1);

	scale.power_down();
	delay(500);
	scale.power_up();
}