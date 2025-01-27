#include <Arduino.h>

const int dacPin = A0;
const int clockwise = 12;
const int counterClockwise = 13;

void setup() {
  Serial.begin(9600);

  pinMode(clockwise, OUTPUT);
  pinMode(counterClockwise, OUTPUT);
  pinMode(dacPin, OUTPUT);

  digitalWrite(clockwise, HIGH);
  digitalWrite(counterClockwise, HIGH);
  analogWrite(dacPin, 0);

  
  Serial.println("Pins set up.");
  delay(5000);
}

void loop() {
  Serial.println("Run.");
  digitalWrite(clockwise, LOW);
  for (int i = 0; i <= 255; i++){
    analogWrite(dacPin, i);
    printf("i: %d\n", i);
    delay(5);
  }
  for (int i = 255; i >= 0; i--){
    analogWrite(dacPin, i);
    printf("i: %d\n", i);
    delay(5);
  }
  analogWrite(dacPin, 0);
  digitalWrite(clockwise, HIGH);
  delay(2000);
}