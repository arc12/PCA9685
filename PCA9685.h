/*
 * PCA9685.h
 *
 * Created: 3/15/2014 9:36:08 PM
 *  Author: Adam Cooper
 */
/* 
 * ***Made available using the The MIT License (MIT)***
 * Copyright (c) 2014, Adam Cooper
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the Software), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */

#ifndef PCA9685_H
#define PCA9685_H

#include <Arduino.h>
#include <I2CUtils.h>

// I2C Address for "all call" (7 bit form)
#define PCA9685_ALLCALLADR 0x70 //!< 0xE0 default "all call" I2C write address (7bit => 0x70)


/// \cond registers
//this section of #defines hidden from doxygen
/* Register Addresses
a - mode and chip level control*/
#define PCA9685_REG_MODE1 0x00
#define PCA9685_REG_MODE2 0x01
#define PCA9685_REG_PRESCALE 0xFE //!< Sets PLL frequency. Not used.

/* Register Addresses
b - LED registers, computed according to LED index (zero-based) */
#define PCA9685_REG_LED(led) ((led * 4) + 6)
#define PCA9685_REG_RGB(led) (led * 12 + 6)

/* Register Addresses
c - All-LED control */
#define PCA9685_REG_ALLLED 0xFA
/// \endcond

/* MODE1 bits */
#define PCA9685_RESTART    0x80  //!< Restart logic. Default disabled. Not used.
#define PCA9685_EXTCLK    0x40  //!< External clock. Default disabled. Not used.
#define PCA9685_AI    0x20 //!< Register auto-increment. Enabled by initialise()
#define PCA9685_SLEEP    0x10 //!< Sleep (osc off). Device is sleeping on POR, brought out of sleep by initialise()
#define PCA9685_SUB1    0x08 //!< Sub-address 1 enable. Default disabled. Not used.
#define PCA9685_SUB2    0x04 //!< Sub-address 2 enable. Default disabled. Not used.
#define PCA9685_SUB3    0x02 //!< Sub-address 3 enable. Default disabled. Not used.
#define PCA9685_ALLCALL    0x01 //!< Respond to all-call. Default enabled. Not used.

/* MODE2 bits. These are not actually used at present */
#define PCA9685_INVRT    0x10
#define PCA9685_OCH    0x08
#define PCA9685_OUTDRV    0x04
#define PCA9685_OUTNE1    0x02
#define PCA9685_OUTNE0    0x01

/* The sequence of register values to send to assert/de-assert LED outputs */
#define PCA9685_LEDREG_ON {0x00, 0x10, 0x00, 0x00}
#define PCA9685_LEDREG_OFF {0x00, 0x00, 0x00, 0x10}

/** @name PCA9685_DRIVE_*
 MODE2 register settings for various stereotype LED driving situations. Note that the /OE pin is not under the control of this library. */
//!@{
#define PCA9685_DRIVE_PMOS 0x15 //!< P-type MOSFET. Outputs 1 if /OE=1
#define PCA9685_DRIVE_NMOS 0x04 //!< N-type MOSFET. Outputs 0 if /OE=1
#define PCA9685_DRIVE_DIRECT 0x13 //!< Direct LED connection (common anode to VDD). Outputs hi-Z if /OE=1
//!@}

/*! Doxygen documentation about class PCA9685
 @brief This is specifically for driving LEDs using the PCA9685. It is unlikely to be suitable for driving servos.
 Various features of the PCA9685 are not needed at present so support is not implemented for:
 a) sub-addresses (any aspect); b) software reset I2C address; c) changing the all-call address;
 d) enabling "restart"; e) use of external clock; f) disabling all=call; g) change always on STOP (not ACK);
 h) PLL frequency (PRE-SCALE register) is left at POR default and is unchangable */
class PCA9685{
public:
  /** @name Constructor, initialiser and sleep control */
  //!@{
  /*! Basic constructor. You must call initialise() before use. */
  PCA9685();
  
  /*! Initalise the device.
  @param driveMode The mode of driving the outputs. Use a value from PCA9685_DRIVE_*
  @param i2cAddr The hardware-set I2C address. Optional: if omitted, the default all-call address will be used, which would be a mistake if there is >1 PCA9685 on the I2C bus
  @returns true if the device appears to be connected. */
  bool initialise(uint8_t driveMode, uint8_t i2cAddr = PCA9685_ALLCALLADR);
  
  /*! Send device to sleep or waken. Device will be awake after initialise()
  @param toSleep true to put the device to sleep */
  void sleep(bool toSleep);
  //!@}

  /** @name Boolean Control 
  @brief Switching is effectively instantaneous - OCH flag in MODE2 register always 0 (change on I2C STOP)*/
  //!@{
  /*! Turn all LEDs on. */
  void AllOn();
  
  /*! Turn all LEDs off. An alternative might be to control via the /OE pin */
  void AllOff();
  
  /*! Turn LEDs on or off according to a bit pattern.
  @param bitPattern a 1 turns LED on and a 0 turns it off, bit 0 acts on LED 0 */
  void ByBits(uint16_t bitPattern);
  
  /*! Turn LEDs on according to a mask.
  @param onMask A 1 turns the LED on, while 0 leads to no change. Bit 0 acts on LED 0 */
  void OnByMask(uint16_t onMask);

  
  /*! Turn LEDs off according to a mask.
  @param onMask A 1 turns the LED off, while 0 leads to no change. Bit 0 acts on LED 0 */
  void OffByMask(uint16_t offMask);
  
  //!@}

/** @name LED brightness/PWM control.
@brief The PCA9685 allows for the turn-on and turn-off times of each LED to be controlled.
Turn-on time is phased according to the LED so that switch-on current pulses are staggered.
All brightness values in range 0-255 rather than the 12-bit value the PCA9685 actually receives. */
  //!@{
  /*! Set brightness for a single LED. */
  void SetBrightness(uint8_t brightness, uint8_t led);
  
    /*! Set brightness for all LEDs.
	All will have the same turn-on time so it may be better to use SetBrightnesses with a brightness vector
	containing identical values for all LEDS since that will cause the turn-on times to be staggered.*/
    void SetBrightnessAll(uint8_t brightness);
  
  /*! Set brightness for multiple LEDs.
  @param brightness a fixed length vector of LED brightnesses. A window may be chosen using firstLed and numLeds args.
  @param firstLed the 0-based index of the first LED to change brightness of (and index in brightness[])
  @param numLeds Length of the sequence of LEDS from firstLed that will be altered */
  void SetBrightnesses(uint8_t brightness[16], uint8_t firstLed = 0, uint8_t numLeds = 16);
   //!@}

/** @name RGB LED brightness/PWM control.
@brief These methods assume that RGB leds are controlled and maps the PCA9685 LEDs in order RGB.
5 RGB LEDs may be controlled; the 16th output is not available.
Turn-on phasing as for normal LED brightness/PWM control.
RGB brightness values in range 0-255 rather than the 12-bit value the PCA9685 actually receives. */
//!@{
/*! Set brightness for a RGB LED. */
void SetRGB(uint8_t rgbValues[3], uint8_t rgbLed);
//!@}


private:
  //private member variables
  I2CUtils _i2c;
  //private methods
};



#endif //include guard