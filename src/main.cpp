#include <Arduino.h>
#include <HX711.h>

// Scale Settings
const int SCALE_DOUT_PIN = D2;
const int SCALE_SCK_PIN = D3;
HX711 scale(SCALE_DOUT_PIN, SCALE_SCK_PIN);
void setup() {
  Serial.begin(115200);
  scale.set_scale();// <- set here calibration factor!!!
  scale.tare();
}
void loop() {
  float weight = scale.get_units(1);
  Serial.println(String(weight, 2));
}
// Test
