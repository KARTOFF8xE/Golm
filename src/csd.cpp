#pragma once

#include <Arduino.h>
#include "config.cpp"

class CSD {
  private:
    int pin;
    int vel;
    int dir;

  public:
    CSD(int pin) {
      this->pin = pin;
      this->vel = 0;
      this->dir = -1;
    }

    void setDirection(int direction) {
      if (direction != clockwise && direction != counterClockwise) return;

      this->dir = direction;
      analogWrite(dacPin, 0);
      digitalWrite(clockwise, HIGH);
      digitalWrite(counterClockwise, HIGH);
      digitalWrite(this->dir, LOW);
    }

    void incVel() {
      if (this->vel < 255) {
        analogWrite(dacPin, ++this->vel);
      }
    }

    void decVel() {
      if (this->vel > 0) {
        analogWrite(dacPin, --this->vel);
      }
    }

    void setVel(int32_t newVel) {
      newVel = constrain(newVel, 0, 255);
      while (this->vel != newVel) {
        if (this->vel > newVel) decVel();
        else incVel();
        delay(1);
      }
    }
};