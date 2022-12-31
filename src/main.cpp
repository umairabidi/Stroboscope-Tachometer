#include <Arduino.h>
#include <SPI.h>


/*
Wiring
Display Pin | Arduino Pin Number
Vcc         | Vcc
GND         | GND
DIN         | 11
CS          | 10
CLK         | 13
*/

#define LED_PIN 5


void setup() {
    pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
}
