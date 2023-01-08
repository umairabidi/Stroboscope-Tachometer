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

#define LED_PIN 7
#define MULTIPLIER_BUTTON_PIN A5

#define IRE1 PD5
#define IRE2 PD6
#define CLOCKWISE 1
#define COUNTERCLOCKWISE -1
#define LED_1 A1
#define LED_2 A1
#define LED_3 A1
#define LED_4 A1
#define LED_5 A1

int multiplier_LEDs[5] = {LED_1, LED_2, LED_3, LED_4, LED_5};


unsigned long prevTime = 0;
unsigned long prevTime_button = 0;
unsigned long prevTime_print = 0;
long speedRPM = 0;
int resolution_i = 1;
long resolutions[5] = {1, 5, 50, 500, 5000};

char printBuf[30] = {0};



int button_pressed = 0;

void ButtonBounce();
void displayNum(long value);

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
	Serial.begin(115200);
	
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
	ButtonBounce();
	if ((millis() - prevTime_button >= 250) && button_pressed){
		resolution_i = (resolution_i++)%5+1;
		prevTime_button = millis();
		digitalWrite(multiplier_LEDs[0], LOW);
		digitalWrite(multiplier_LEDs[1], LOW);
		digitalWrite(multiplier_LEDs[2], LOW);
		digitalWrite(multiplier_LEDs[3], LOW);
		digitalWrite(multiplier_LEDs[4], LOW);
		digitalWrite(multiplier_LEDs[resolution_i-1], HIGH);
		
		Serial.print("Resolution is ");
		Serial.println(resolutions[resolution_i-1]);
    }
    
	if (interrupt_flag == 1){
		IRE_Direction = ((PIND & (1<<IRE1)) <<1)^(PIND & (1<<IRE2))?CLOCKWISE:COUNTERCLOCKWISE;
		// Based on truth table for pin state vs direction (IRE1 XOR IRE2 = CW)
		speedRPM += resolutions[resolution_i-1]*IRE_Direction;
		if (speedRPM <= 1){
			speedRPM = 1;
		}
		interrupt_flag = 0;
	}

	

    if(millis() - prevTime_print >= 150){
//    	char s[8] = {0};
//    	dtostrf(speedRPM,8,1,s);
//		sprintf(printBuf,"%s rpm\n",s);
//		Serial.print(printBuf);
		
		prevTime_print = millis();
		Serial.println(speedRPM);
		displayNum(speedRPM);
	}

}


void ButtonBounce(){
	static long Button_f = 0;
	Button_f += !digitalRead(MULTIPLIER_BUTTON_PIN);

	if (Button_f >= 60000){
		speedRPM = 1;
	}
	else if (Button_f && digitalRead(MULTIPLIER_BUTTON_PIN)){
		Serial.print("button_f");
		Serial.println(Button_f);
		button_pressed = 1;
		Button_f = 0;
	}
	else {
		button_pressed = 0;
	}
	
}

void displayNum(long value){
	bool blank = true;
	int digits[8] = {0};
	for (int i=7; i>=0; i--){
		digits[i] = value%10;
		value /= 10;
	}
	for (int i=0; i<=5; i++){
		if (blank && !digits[i]){
			lc.setChar(0,7-i,' ',false);
		}
		else {
			lc.setDigit(0,7-i,digits[i],false);
			blank = false;
		}
		lc.setDigit(0,1,digits[6],true);
		lc.setDigit(0,0,digits[7],false);
		value /= 10;
	}
}
