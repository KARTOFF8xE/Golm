#include <Arduino.h>
#include <seesaw_neopixel.h>
#include "Adafruit_seesaw.h"

#include "config.cpp"
#include "csd.cpp"

Adafruit_seesaw ss;
seesaw_NeoPixel sspixel = seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800);
int32_t encoder_position;

CSD* csd;

void setupRotaryEncoder() {
  Serial.println("Looking for seesaw!");
  if (!ss.begin(SEESAW_ADDR) || !sspixel.begin(SEESAW_ADDR)) {
    Serial.println("Couldn't find seesaw!");
    while(1) delay(10);
  }
  Serial.println("Seesaw started");

  uint32_t version = ((ss.getVersion() >> 16) & 0xFFFF);
  Serial.print("Seesaw Version: ");
  Serial.println(version);
  
  if (version != 4991) {
    Serial.println("Wrong firmware!");
    while(1) delay(10);
  }

  sspixel.setBrightness(20);
  sspixel.show();

  ss.pinMode(SS_SWITCH, INPUT_PULLUP);
  encoder_position = ss.getEncoderPosition();
  Serial.println("Turning on interrupts");
  ss.setGPIOInterrupts((uint32_t)1 << SS_SWITCH, 1);
  ss.enableEncoderInterrupt();
}

void setupCSD() {
  pinMode(clockwise, OUTPUT);
  pinMode(counterClockwise, OUTPUT);
  pinMode(dacPin, OUTPUT);

  csd = new CSD(dacPin);
  csd->setDirection(clockwise);
  csd->setVel(0);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  setupRotaryEncoder();
  setupCSD();
}

void loop() {
  int32_t new_position = ss.getEncoderPosition();

  if (encoder_position != new_position) {
    Serial.print("Encoder Pos: ");
    Serial.println(new_position);
    csd->setVel(new_position);
    encoder_position = new_position;
  }
  delay(10);
}
