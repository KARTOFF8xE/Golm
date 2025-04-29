#pragma once

#include <Arduino.h>

#include <seesaw_neopixel.h>
#include "Adafruit_seesaw.h"

#include "config.cpp"


enum State {
  INITIALSETUP,
  CALIBRATE,
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
    int initial_speed, speed;
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
      this->initial_speed = -1;
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

    void incSpeed(int increment) {
      if (this->speed < 255) {
        this->speed += increment;
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
        else incSpeed(accSpeed);
        delay(1);
      }
    }

    int getSpeed() {
      return speed;
    }

    void initialSetup(Adafruit_seesaw ss) {
      ss.setEncoderPosition(0);
      this->encoder_then = ss.getEncoderPosition();
      this->encoder_now = ss.getEncoderPosition();
      this->t_then = 0;
      this->t_now = 0;
      this->setSpeed(0);
    }

    bool calibrate(Adafruit_seesaw ss) {
      this->t_now = millis();
      if (this->t_now - this->t_then >= 1000) {
        this->incSpeed(calibrationSpeed);
        this->t_then = this->t_now;
      }

      if (ss.getEncoderPosition() == 0) return true;
      
      this->initial_speed = this->speed;
      Serial.print("Set initial speed: ");
      Serial.println(this->initial_speed);
      
      return false;
    }

    void setup(Adafruit_seesaw ss) {
      ss.setEncoderPosition(0);
      this->encoder_then = ss.getEncoderPosition();
      this->encoder_now = ss.getEncoderPosition();
      this->t_then = 0;
      this->t_now = 0;
      this->setSpeed(this->initial_speed);
    }

    // Takes vel in km/h and rotary-sensor
    bool accelerateToVel(float vel, Adafruit_seesaw ss) {
        double vel_current = 0.0;
        if (this->encoder_now != this->encoder_then) {
          this->t_then = this->t_now;
          this->t_now = millis();
          vel_current = (abs(this->encoder_now - this->encoder_then) * 3.14 * 10) / (t_now - t_then);
          Serial.print("current vel: ");
          Serial.println(vel_current);
          Serial.print("enc_now: ");
          Serial.println(encoder_now);
          Serial.print("encoder diff: ");
          Serial.println(this->encoder_now - this->encoder_then);
          Serial.print("t_now: ");
          Serial.println(t_now);
          Serial.print("t_then: ");
          Serial.println(t_then);
          Serial.print("vel: ");
          Serial.println(vel);
          if (vel_current * 3.6 >= vel || this->speed >= 256) {
            Serial.println("Done Accelerating");
            return true;
          }

          this->encoder_then = this->encoder_now;
          this->incSpeed(accSpeed);
        }
        this->encoder_now = ss.getEncoderPosition();

      return false;
    }

    // Takes speed
    bool accelerateToSpeed(int goalSpeed) {
      Serial.print("acc. speed: ");
      Serial.println(speed);
      // Serial.println("accelerateToSpeed");
      // Serial.print(this->speed);
      // Serial.print(" : ");
      // Serial.print(goalSpeed);
      if (this->speed >= goalSpeed || this->speed >= 256) {
        // Serial.println(" DOOOOOOOOOOOOOOOOOOOOOOOOONE");
        return true;
      }
      this->t_now = millis();
      if (this->t_now-this->t_then > 50)
      {
        this->incSpeed(accSpeed);
        this->t_then = this->t_now;
      }
      // Serial.println(" accelerated");
      return false;
    }

    bool drive(Adafruit_seesaw ss) {
      return (abs(ss.getEncoderPosition()) >= abs(200));
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