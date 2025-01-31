#pragma once

#include <Arduino.h>

#include <seesaw_neopixel.h>
#include "Adafruit_seesaw.h"

#include "config.cpp"

class CSD {
  private:
    int pin;
    int speed;
    int dir;

  public:
    CSD(int pin) {
      this->pin = pin;
      this->speed = 0;
      this->dir = -1;
    }

    void setDirection(int direction) {
      if (direction != clockwise && direction != counterClockwise) return;

      this->breakSpeed();
      this->dir = direction;
      // analogWrite(dacPin, this->speed);
      digitalWrite(clockwise, HIGH);
      digitalWrite(counterClockwise, HIGH);
      digitalWrite(this->dir, LOW);
    }

    void incSpeed() {
      if (this->speed < 255) {
        analogWrite(dacPin, ++this->speed);
      }
    }

    void decSpeed() {
      if (this->speed > 0) {
        analogWrite(dacPin, --this->speed);
      }
    }

    void setSpeed(int32_t newSpeed) {
      newSpeed = constrain(newSpeed, 0, 255);
      while (this->speed != newSpeed) {
        if (this->speed > newSpeed) decSpeed();
        else incSpeed();
        delay(1);
      }
    }

    int getSpeed() {
      return speed;
    }

    // Takes vel in km/h and rotary-sensor
    void accelerateToVel(float vel, Adafruit_seesaw ss) {
        vel = vel / 3.6;
        ss.setEncoderPosition(0);
        int rotary_now, rotary_then = ss.getEncoderPosition();
        int t_now, t_then = 0;
        do {
            this->incSpeed();
            delay(50);
            t_now = millis();
        } while(rotary_then == ss.getEncoderPosition());
        // if (this->minSpeed == 0) {
        //   this->minSpeed = this->speed - 5;
        // }


        double vel_current = 0.0;
        rotary_now = ss.getEncoderPosition();
        rotary_then = rotary_now;
        do {
            if (rotary_now != rotary_then) {
                t_then = t_now;
                t_now = millis();
            
                this->incSpeed();

                vel_current = (abs(rotary_now - rotary_then) * 3.14 * 10) / ((t_now - t_then));
                rotary_then = rotary_now;
            }
            rotary_now = ss.getEncoderPosition();
        } while (vel_current <= vel && this->speed < 256);
    }

    void breakSpeed() {
        for (speed; speed >= 0; speed--) {
            this->decSpeed();
            delay(10);
        }
    }
};