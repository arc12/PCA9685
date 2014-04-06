/*
* Copyright 2012 Adam Cooper, derived from the work of Elco Jacobs.
* See http://www.elcojacobs.com/easy-to-use-atmel-studio-project-for-arduino-and-programming-the-arduino-from-python/
*
* This is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ShiftPWM is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Arduino.h>
#include <PCA9685.cpp>
#include <I2CUtils.cpp>
#include <Wire.cpp>
#include <twi.c>

// Function prototypes go here (telling the compiler these functions exist).
void setup();
void loop();
void doTest();

/*
* Place a direct direct copy of sketch (*.ino or *.pde) beneath this comment block.... except that you:
* 1: might have to add some function prototypes to the top, because the Arduino environment does this automatically but Atmel Studio does not.
* 2: might have to change the #include to the .cpp rather than .h (check which one includes the other. Arduino IDE seems able to work it out!)
*/

PCA9685 pwm = PCA9685();
const uint8_t OEpin = 4; // negative logic output enable
const uint8_t buttonPin=3; //push button to cycle through test cases
uint8_t testId=0;//test case number

void setup(){
	Serial.begin(9600);
	Serial.println("Begin");
	pinMode(OEpin, OUTPUT);
	pinMode(buttonPin, INPUT_PULLUP);	
	digitalWrite(OEpin, HIGH);//disable outputs
	//initialise for direct LED drive (with current limit resistor), common anode, using the all-call I2C address.
	pwm.initialise(PCA9685_DRIVE_DIRECT);
	//pwm.AllOff();
	digitalWrite(OEpin, LOW);//enable outputs
}

void loop(){	
	if(!digitalRead(buttonPin)){
		Serial.print("Test ");
		Serial.println(testId);
		doTest();
		testId++;
		testId = testId %9;
		delay(500);
	}
}

void doTest(){
	switch (testId)
	{
		case 0:
		{
			Serial.println("blink all 5 times");
			for(uint8_t i=0;i<5;i++){
				pwm.AllOn();
				delay(300);
				pwm.AllOff();
				delay(300);
			}
			break;
		}
		
		case 1:
		{
			//bit pattern on/off, use a bit shift to make a moving dot, from led 0-15
			Serial.println("Moving Dot");
			uint16_t bits=0x1;
			while(bits>0){
				pwm.ByBits(bits);
				bits = bits <<1;
				delay(300);
			}
		}
		break;
		
		case 2:
		{
			//bit pattern on, use a bit shift to make a growing line, from led 0-15
			Serial.println("Growing bar");
			uint16_t bits=0x1;
			pwm.AllOff();
			while(bits>0){
				pwm.OnByMask(bits);
				bits = bits <<1;
				delay(300);
			}
		}
		break;
		
		case 3:
		{
			//bit pattern of, use a bit shift to make a shrinking line,
			Serial.println("Shrinking bar");
			uint16_t bits=0x1;
			pwm.AllOn();
			while(bits>0){
				pwm.OffByMask(bits);
				bits = bits <<1;
				delay(300);
			}
		}
		break;

		case 4:
		{
			//set the leds to have progressively increasing brightness from led0 to led15
			Serial.println("LEDs progressively brighten");
			for (uint8_t i=0; i<16; i++){
				pwm.SetBrightness(i<<4, i);
			}
		}
		break;


		case 5:
		{
			//make all LEDS increase and decrease bri9ghtness over a few seconds
			Serial.println("All brighten then dim in time");
			for (uint8_t i=0; i<255; i++){
				pwm.SetBrightnessAll(i);
				delay(10);
			}
			for (uint8_t i=255; i>0; i--){
				pwm.SetBrightnessAll(i);
				delay(10);
			}
		}
		break;

		case 6:
		{
			//alternating brightness, a pause then a change for the middle 8
			Serial.println("Alternate brightness then switch Nos. 4-8");
			uint8_t brightness1[16]={0x40, 0xC0,0x40, 0xC0,0x40, 0xC0,0x40, 0xC0,0x40, 0xC0,0x40, 0xC0,0x40, 0xC0,0x40, 0xC0};
			uint8_t brightness2[16]={0xC0,0x40, 0xC0,0x40, 0xC0,0x40, 0xC0,0x40, 0xC0,0x40, 0xC0,0x40, 0xC0,0x40, 0xC0, 0x40};
			pwm.SetBrightnesses(brightness1);
			delay(1000);
			pwm.SetBrightnesses(brightness2,4,8);
		}
		break;

		case 7:
		{
			//cycle through colours if a RGB LED is attached, otherwise cycle brightness of lowest 3 LEDs
			Serial.println("RGB cycling");
			uint8_t rgb[3] = {0xFF, 0x00, 0x00};
				pwm.AllOff();
			for(uint8_t i=0; i<255; i++){
				rgb[0]--;
				rgb[1]++;
				pwm.SetRGB(rgb,0);
				delay(10);
			}
			for(uint8_t i=0; i<255; i++){
				rgb[1]--;
				rgb[2]++;
				pwm.SetRGB(rgb,0);
				delay(10);
			}
			for(uint8_t i=0; i<255; i++){
				rgb[2]--;
				rgb[0]++;
				pwm.SetRGB(rgb,0);
				delay(10);
			}
			
		}
		break;
		
		case 8:
			Serial.println("Sleep, pause, and re-awake");
			pwm.sleep(true);
			delay(1000);
			pwm.sleep(false);
			break;
		
		default:
		testId=0;
	}
}