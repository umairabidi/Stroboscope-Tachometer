#include <Arduino.h>
#include <SPI.h>
#include "LedControl.h"


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
#define MULTIPLIER_BUTTON_PIN 7

#define IRE1 PD5
#define IRE2 PD6
#define CLOCKWISE 1
#define COUNTERCLOCKWISE -1

unsigned long prevTime = 0;
unsigned long prevTime_button = 0;
unsigned long prevTime_print = 0;
double speedRPM = 0;
int resolution_i = 1;

double resolutions[5] = {0.1, 1, 10, 100, 1000};
char printBuf[30] = {0};

void ButtonBounce();
int button_pressed = 0;

LedControl lc = LedControl(12,11,10,1);
// IRE1 is on digital pin 5	which is PD5 or PCINT21
// IRE2 is on digital pin 6 which is PD6 or PCINT22
int IRE_Direction = 0;
volatile int interrupt_flag = 0;
ISR (PCINT2_vect){	// all PCINT23~16 share the same vector
	interrupt_flag = 1;
	// The interrupt flag is the PCIF2 bit in PCIFR but we don't need this
}

void setup(){
    lc.shutdown(0,false);
    lc.setIntensity(0,8);
    lc.clearDisplay(0);

	// Pin Setup
	DDRD &= ~((1<<DDD5)|(1<<DDD6));	// Input

	// PCINT Setup
	PCICR |= (1<<PCIE2);					// General enable (for PCINT23~16)
	//PCMSK2 |= (1<<PCINT21)|(1<<PCINT22);	// Pin-specific enable
	PCMSK2 |= (1<<PCINT21);	// Pin-specific enable

	// Global interrupt enable
	//SREG |= (1<<I);	// In the past I have had issues with this line
	sei();
}

void loop(){
	if (interrupt_flag == 1){
		IRE_Direction = ((PIND & (1<<IRE1)) <<1)^(PIND & (1<<IRE2))?CLOCKWISE:COUNTERCLOCKWISE;
		// Based on truth table for pin state vs direction (IRE1 XOR IRE2 = CW)
		speedRPM += resolutions[resolution_i]*IRE_Direction;
		Serial.print("Changed to ");
		interrupt_flag = 0;
	}

	if ((millis() - prevTime_button >= 100) && button_pressed){
		resolution_i = (resolution_i++)%5+1;
		prevTime_button = millis();
    }

    if(millis() - prevTime_print >= 250){
		sprintf(printBuf, "%lf\n",speedRPM);
		Serial.print(printBuf);
		prevTime_print = millis();
	}

}


void ButtonBounce(){

	static int Button_f = 0;
	Button_f += !digitalRead(MULTIPLIER_BUTTON_PIN);

	if (Button_f && digitalRead(MULTIPLIER_BUTTON_PIN)){
		button_pressed = 1;
		Button_f = 0;
	}
	else {
		button_pressed = 0;
	}
}
