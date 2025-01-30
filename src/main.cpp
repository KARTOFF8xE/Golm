#include <Arduino.h>

#include <seesaw_neopixel.h>
#include "Adafruit_seesaw.h"

// Pins and stuff for rotary encoder
#define SS_SWITCH        24
#define SS_NEOPIX        6
#define SEESAW_ADDR          0x36
Adafruit_seesaw ss;
seesaw_NeoPixel sspixel = seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800);
int32_t encoder_position;

// Pins and stuff for CSD
#define dacPin            A0
#define clockwise         12
#define counterClockwise  13

class CSD {
  private:
    int pin;
    int vel;
    int dir;

  public:
    CSD(int pin) {
      pin = pin;
      vel = 0;
      dir = -1;

      setDirection(dir);
    }

    void setDirection(int direction) {
      dir = direction;
      analogWrite(dacPin, 0);
      digitalWrite(clockwise, HIGH);
      digitalWrite(counterClockwise, HIGH);
      if (direction < 0) { return; }
      digitalWrite(direction, LOW);
    }

    void incVel() {
      if (vel < 255) {
        analogWrite(dacPin, ++vel);
      }
    }

    void decVel() {
      if (vel > 0) {
        analogWrite(dacPin, --vel);
      }
    }

    void setVel(int32_t newVel) {
      if (newVel > 255) {
        newVel = 255;
      } else if (newVel < 0) {
        newVel = 0;
      }

      if (vel > newVel) {
        while (vel >= newVel) {
          decVel();
          delay(1);
        }
        return;
      }
      if (vel < newVel) {
        while (vel <= newVel) {
          incVel();
          delay(1);
        }
        return;
      }
    }
};

CSD csd(dacPin);

// setupRotaryEncoder sets up the rotary encoder
void setupRotaryEncoder() {
  // search for seesaw rotary encoder
  Serial.println("Looking for seesaw!");
  if (! ss.begin(SEESAW_ADDR) || ! sspixel.begin(SEESAW_ADDR)) {
    Serial.println("Couldn't find seesaw on default address");
    while(1) delay(10);
  }
  Serial.println("seesaw started");

  // lookup for correct Firmware
  uint32_t version = ((ss.getVersion() >> 16) & 0xFFFF);
  if (version  != 4991){
    Serial.print("Wrong firmware loaded? ");
    Serial.println(version);
    while(1) delay(10);
  }
  Serial.println("Found Product 4991");

  // set LED brightness
  sspixel.setBrightness(20);
  sspixel.show();
  
  // use a pin for the built in encoder switch
  ss.pinMode(SS_SWITCH, INPUT_PULLUP);

  // get starting position
  encoder_position = ss.getEncoderPosition();

  Serial.println("Turning on interrupts");
  delay(10);
  ss.setGPIOInterrupts((uint32_t)1 << SS_SWITCH, 1);
  ss.enableEncoderInterrupt();
}


// setupCSD sets up the cordless screwdriver
void setupCSD() {
  pinMode(clockwise, OUTPUT);
  pinMode(counterClockwise, OUTPUT);
  pinMode(dacPin, OUTPUT);
  csd.setDirection(-1);
  csd.setVel(0);
}

void setup() {
  // start Serial
  Serial.begin(115200);
  while (!Serial) delay(10);

  // setup components
  setupRotaryEncoder();
  setupCSD();

  csd.setDirection(clockwise);
}

void loop() {
  int32_t new_position = ss.getEncoderPosition();
  // did we move around?
  if (encoder_position != new_position) {
    csd.setVel(new_position);
    encoder_position = new_position;      // and save for next round
  }
  // CSD_setVel(100);


  // don't overwhelm serial port
  delay(1);
}