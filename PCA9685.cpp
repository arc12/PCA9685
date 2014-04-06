/*
* PCA9685.cpp
*
* Created: 3/15/2014 9:36:08 PM
*  Author: Adam Cooper
*/
/*
* ***Made available using the The MIT License (MIT)***
* Copyright (c) 2014, Adam Cooper
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the �Software�), to
* deal in the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED �AS IS�, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE. */

#include <avr/io.h>

#include "PCA9685.h"

PCA9685::PCA9685(void){
	_i2c = I2CUtils();
}

bool PCA9685::initialise(uint8_t driveMode, uint8_t i2cAddr){
	_i2c.setDeviceAddress(i2cAddr);
	//set the drive mode
	_i2c.writemem(PCA9685_REG_MODE2, driveMode & 0x17);
	//Set register auto-increment (assumed by all setter methods), and bring out of sleep mode.
	_i2c.writemem(PCA9685_REG_MODE1, PCA9685_AI | PCA9685_ALLCALL);
	//check it is connected
	uint8_t mode1Value;
	_i2c.readmem(PCA9685_REG_MODE1, 1, &mode1Value);
	return (mode1Value == (PCA9685_AI | PCA9685_ALLCALL));
}

void PCA9685::sleep(bool toSleep){
	uint8_t mode1Value;
	_i2c.readmem(PCA9685_REG_MODE1, 1, &mode1Value);
	if(toSleep) {
		mode1Value |= PCA9685_SLEEP;
		}else{
		mode1Value &= ~PCA9685_SLEEP;
	}
	_i2c.writemem(PCA9685_REG_MODE1,mode1Value);
}

void PCA9685::AllOn(){
	uint8_t regValues[4]={0x00, 0x10, 0x00, 0x00} ;
	_i2c.writememBatch(PCA9685_REG_ALLLED, regValues,4);
}

void PCA9685::AllOff(){
	uint8_t regValues[4]={0x00, 0x00, 0x00, 0x10} ;
	_i2c.writememBatch(PCA9685_REG_ALLLED, regValues,4);
}

void PCA9685::ByBits(uint16_t bitPattern){
	uint8_t reg;
	uint8_t regValuesOn[4] = PCA9685_LEDREG_ON;
	uint8_t regValuesOff[4] = PCA9685_LEDREG_OFF;
	for(uint8_t i=0;i<16;i++){
		reg = PCA9685_REG_LED(i);
		if(bitPattern&0x0001){
			//on
			_i2c.writememBatch(reg, regValuesOn,4);
			}else{
			//off
			_i2c.writememBatch(reg, regValuesOff,4);
		}
		//get ready to check the next bit
		bitPattern = bitPattern>>1;
	}
}

void PCA9685::OnByMask(uint16_t onMask){
	uint8_t reg;
	uint8_t regValuesOn[4] = PCA9685_LEDREG_ON;
	for(uint8_t i=0;i<16;i++){
		reg = PCA9685_REG_LED(i);
		if(onMask&0x0001){
			//on
			_i2c.writememBatch(reg, regValuesOn,4);
		}
		//get ready to check the next bit
		onMask = onMask>>1;
	}
}

void PCA9685::OffByMask(uint16_t offMask){
	uint8_t reg;
	uint8_t regValuesOff[4] = PCA9685_LEDREG_OFF;
	for(uint8_t i=0;i<16;i++){
		reg = PCA9685_REG_LED(i);
		if(offMask&0x0001){
			//off
			_i2c.writememBatch(reg, regValuesOff,4);
		}
		//get ready to check the next bit
		offMask = offMask>>1;
	}
}

void PCA9685::SetBrightness(uint8_t brightness, uint8_t led){
	uint8_t regValues[4]={0x00, 0x00, 0x00, 0x00};
	regValues[1]=led;//on time H byte, to get staggered turn-on
	regValues[3]=0x0F & (led+(brightness>>4));//off time H byte, remembering to offset for the on time.
	regValues[2]=brightness<<4;//off time L byte. no offset because the L byte for on time is always 0
	_i2c.writememBatch(PCA9685_REG_LED(led), regValues,4);
}

void PCA9685::SetBrightnessAll(uint8_t brightness){
	uint8_t offL = brightness<<4;
	uint8_t offH = brightness>>4;
	uint8_t regValues[4] = {0x00, 0x00, offL, offH};
	_i2c.writememBatch(PCA9685_REG_ALLLED, regValues, 4);
}

void PCA9685::SetBrightnesses(uint8_t brightness[16], uint8_t firstLed, uint8_t numLeds){
	uint8_t lastLed = (firstLed + numLeds-1)%16;
	for(uint8_t i=firstLed;i<lastLed;i++){
		SetBrightness(brightness[i],i);
	}
}

void PCA9685::SetRGB(uint8_t rgbValues[3], uint8_t rgbLed){
	uint8_t led=3*rgbLed;
	SetBrightness(rgbValues[0],led);
	led++;
	SetBrightness(rgbValues[1],led);
	led++;
	SetBrightness(rgbValues[2],led);
	led++;
}