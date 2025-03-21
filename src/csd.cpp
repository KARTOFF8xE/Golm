#pragma once

#include <Arduino.h>

#include <seesaw_neopixel.h>
#include "Adafruit_seesaw.h"

#include "config.cpp"


enum State {
  SETUP,
  STARTUP,
  ACCELERATING,
  DRIVING,
  BREAKING,
  TOGGLE,
  SLEEP,
};

class CSD {
  private:
    int pin;
    int speed;
    int dir;
    int t_then, t_now;
    int encoder_then, encoder_now;

  public:
    State state;

    CSD(int pin) {
      this->pin = pin;
      this->speed = 0;
      this->dir = -1;
      this->state = SLEEP;
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
        this->speed += accSpeed;
        if (this->speed > 255) { this->speed = 255; }
        analogWrite(dacPin, this->speed);
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

    void setup(Adafruit_seesaw ss) {
      // Serial.println("setup");
      ss.setEncoderPosition(0);
      this->encoder_then = ss.getEncoderPosition();
      this->encoder_now = ss.getEncoderPosition();
      this->t_then = 0;
      this->t_now = 0;
      this->setSpeed(0);
    }

    // returns if already turning
    bool startup(Adafruit_seesaw ss) {
      // Serial.println("startup");
      this->t_now = millis();
      if (this->t_now - this->t_then >= 50) {
        this->incSpeed();
        this->t_then = this->t_now;
      }
      delay(10);
      return (this->encoder_then != ss.getEncoderPosition()) ? true : false;
    }

    // Takes vel in km/h and rotary-sensor
    bool accelerateToVel(float vel, Adafruit_seesaw ss) {
      // Serial.println("accelerateToVel");
        double vel_current = 0.0;
        if (this->encoder_now != this->encoder_then) {
          this->t_then = this->t_now;
          this->t_now = millis();
          vel_current = (abs(this->encoder_now - this->encoder_then) * 3.14 * 10) / (t_now - t_then);

          if (vel_current * 3.6 >= vel || this->speed >= 256) return true;

          this->encoder_then = this->encoder_now;
          this->incSpeed();
        }
        this->encoder_now = ss.getEncoderPosition();

        return false;
    }

    bool drive(Adafruit_seesaw ss) {
      return (abs(ss.getEncoderPosition()) >= abs(32));
    }

    bool breakSpeed() {
      for (speed; speed >= 0; speed--) {
          this->decSpeed();
          delay(10);
      }
      return true;
    }

    void toggle() {
      if (this->dir == clockwise) this->setDirection(counterClockwise);
      else this->setDirection(clockwise);
    }
};