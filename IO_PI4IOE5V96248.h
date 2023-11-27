/*
 *  © 2021, Neil McKechnie. All rights reserved.
 *  © 2023, Ross Scanlon. All rights reserved.
 *
 *  This file is part of DCC++EX API
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  It is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CommandStation.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef io_PI4IOE5V96248_h
#define io_PI4IOE5V96248_h

#include "IO_GPIOBase.h"
#include "FSH.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * IODevice subclass for PI4IOE5V96248 16-bit I/O expander.
 */

//class PI4IOE5V96248 : public GPIOBase<uint16_t> {
class PI4IOE5V96248 : public GPIOBase<uint64_t> {
public:
  static void create(VPIN vpin, uint8_t nPins, I2CAddress i2cAddress, int interruptPin=-1) {
    // need to check both addresses for overlap here eg vpin to (vpin + npins/2) and vpin + (nPins / 2) + 1 to vpin +nPins
//    if (checkNoOverlap(vpin, nPins, i2cAddress)) new PI4IOE5V96248(vpin, nPins, i2cAddress, interruptPin);
    if ( checkNoOverlap( vpin, nPins, i2cAddress )  && checkNoOverlap( vpin, 0, ( i2cAddress + 1 ) ) ) {
       new PI4IOE5V96248(vpin, nPins, i2cAddress, interruptPin);
    }
  }

private:
  // Constructor
  PI4IOE5V96248(VPIN vpin, uint8_t nPins, I2CAddress i2cAddress, int interruptPin=-1)
    : GPIOBase<uint64_t>((FSH *)F("PI4IOE5V96248"), vpin, nPins, i2cAddress, interruptPin)
  {
    requestBlock.setRequestParams(_I2CAddress, inputBuffer, sizeof(inputBuffer),
      outputBuffer, sizeof(outputBuffer));
    outputBuffer[0] = 0xFF;
    outputBuffer[1] = 0xFF;
    outputBuffer[2] = 0xFF;
    outputBuffer[3] = 0xFF;
    outputBuffer[4] = 0xFF;
    outputBuffer[5] = 0xFF;
  }

  void _writeGpioPort() override {
    I2CManager.write(_I2CAddress, outputBuffer, 6);
  }

/*
  void _writePullups() override {
    // Set pullups only for in-use pins.  This prevents pullup being set for a pin that
    //  is intended for use as an output but hasn't been written to yet.
    uint16_t temp = _portPullup & _portInUse;
    I2CManager.write(_I2CAddress, 3, REG_GPPUA, temp, temp>>8);
  }
*/

/*
  void _writePortModes() override {
    // Write 0 to IODIR for in-use pins that are outputs, 1 for others.
    uint16_t temp = ~(_portMode & _portInUse);
    I2CManager.write(_I2CAddress, 3, REG_IODIRA, temp, temp>>8);
    // Enable interrupt for in-use pins which are inputs (_portMode=0)
    temp = ~_portMode & _portInUse;
    I2CManager.write(_I2CAddress, 3, REG_INTCONA, 0x00, 0x00);
    I2CManager.write(_I2CAddress, 3, REG_GPINTENA, temp, temp>>8);
  }
*/

  void _readGpioPort(bool immediate) override {
    if (immediate) {
/*
      uint8_t buffer[2];
      I2CManager.read(_I2CAddress, buffer, 2, 1, REG_GPIOA);
      _portInputState = ((uint16_t)buffer[1]<<8) | buffer[0] | _portMode;
*/
      I2CManager.read(_I2CAddress, inputBuffer, 6);

    } else {
      // Queue new request
      requestBlock.wait(); // Wait for preceding operation to complete
      // Issue new request to read GPIO register
      I2CManager.queueRequest(&requestBlock);
    }
  }

  // This function is invoked when an I/O operation on the requestBlock completes.
  void _processCompletion(uint8_t status) override {
/*
    if (status == I2C_STATUS_OK)
      _portInputState = (((uint16_t)inputBuffer[1]<<8) | inputBuffer[0]) | _portMode;
    else
      _portInputState = 0xffff;
*/
    if (status == I2C_STATUS_OK)
      _portInputState = ((uint64_t) inputBuffer[5]<<48) |(uint64_t) inputBuffer[4]<<32 |(uint64_t) inputBuffer[3]<<24 |(uint64_t) inputBuffer[2]<<16 |(uint64_t) inputBuffer[1]<<8 |(uint64_t) inputBuffer[0] | _portMode;
    else
      _portInputState = 0xffffffff;
  }

  void _setupDevice() override {
/*
    // IOCON is set MIRROR=1, ODR=1 (open drain shared interrupt pin)
    I2CManager.write(_I2CAddress, 2, REG_IOCON, 0x44);
    _writePortModes();
    _writePullups();
    _writeGpioPort();
*/
  }

  uint8_t inputBuffer[6];
  uint8_t outputBuffer[6];


};

#endif
