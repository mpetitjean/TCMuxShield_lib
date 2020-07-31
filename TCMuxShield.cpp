/***************************************************************************************************

Arduino library for the TCMuxShield from Ocean Controls using the MAX31855K the ADG608
Written by Mathieu Petitjean - mathieu@petitjean.cc

This program is free software: you can redistribute it and/or modify it under the terms of the
GNU General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

***************************************************************************************************/

#include "TCMuxShield.h"

#include <SPI.h>

TCMuxShield::TCMuxShield(int _CS)
{
    // Chip Select can be D8, D9 or D10. Hardware default is 9
    PIN_CS = _CS;
    pinMode(PIN_CS, OUTPUT);
    digitalWrite(PIN_CS, HIGH);

    // Multiplexer
    pinMode(MUX_A0, OUTPUT);    
    pinMode(MUX_A1, OUTPUT);
    pinMode(MUX_A2, OUTPUT);
    pinMode(MUX_EN, OUTPUT);
    digitalWrite(MUX_A0, LOW);
    digitalWrite(MUX_A1, LOW);
    digitalWrite(MUX_A2, LOW);
    digitalWrite(MUX_EN, LOW);

    // SPI
    SPI.begin();
}

/**************************************************************************/
/*!
    @brief  Reads temperature of a given thermocouple of the shield and writes in the temperature object attritube
    @param tc_n Thermocouple number (1 to 8)
    @return Error code: 0 if OK, 1 if Open Circuit, 2 if Short to GND, 3 if Short to VCC
*/
/**************************************************************************/
int TCMuxShield::readTemperature(int tc_n)
{
    activateMux(tc_n);

    int err_code = getTCData();

    disableMux();

    return err_code;
}

/**************************************************************************/
/*!
    @brief  Reads raw SPI data from the sensor and extracts temperature
    @return Error code: 0 if OK, 1 if Open Circuit, 2 if Short to GND, 3 if Short to VCC
*/
/**************************************************************************/
int TCMuxShield::getTCData(void)
{   
    // Get 32-bit SPI data
    unsigned long raw_data = 0;
    digitalWrite(PIN_CS, LOW);
    // The MAX31855 needs at least 0.1us between the fall of CS and the output enable
    delayMicroseconds(1);
    for (int i = 0; i < 4; i++)
    {
        raw_data = (raw_data << 8) + SPI.transfer(0x00);
    }
    digitalWrite(PIN_CS, HIGH);

    // First check if some of the fault bits are set. If yes, stop and return the error code.
    // Else, read temperature
    if (raw_data & 0b111)
    {   
        temperature = 0;
        // Open Circuit bit
        if (raw_data & (1 << MAX31855_OC_BIT))
            return TC_OC;

        // Short to Ground bit
        if (raw_data & (1 << MAX31855_SCG_BIT))
            return TC_SCG;

        // Short to VCC bit
        if (raw_data & (1 << MAX31855_SCV_BIT))
            return TC_SCV;
    }

    // Extract tc temperature
    unsigned long tc_data = raw_data >> MAX31855_TCDATA_OFFSET;
    
    // If the sign bit is set, the value is negative. Take 2's complement
    if (tc_data >> MAX31855_14B_SIGN_OFFSET)
        tc_data -= 16384;
    // Scale down because MSB is 2^-2
    float tc_temp = (tc_data/4);

    // Extract internal temperature
    unsigned long int_data = (raw_data >> MAX31855_INTDATA_OFFSET) & 0xFFF;
    // If the sign bit is set, the value is negative. Take 2's complement
    if (int_data >> MAX31855_12B_SIGN_OFFSET)
        int_data -= 4096;
    // Scale down because MSB is 2^-4
    float int_temp = (int_data/16);

    temperature = linearizeTC(tc_temp, int_temp);

    return TC_OK;
}

/**************************************************************************/
/*!
    @brief  Activate the multiplexer pins corresponding to the given thermocouple number
    @param tc_n Thermocouple number (1 to 8)
*/
/**************************************************************************/
void TCMuxShield::activateMux(int tc_n)
{
    /* ADG608 Truth Table
    |A2 | A1 | A0 | ON SWITCH

    | 0 | 0  |  0 |     1
    | 0 | 0  |  1 |     2
    | 0 | 1  |  0 |     3
    | 0 | 1  |  1 |     4
    | 1 | 0  |  0 |     5
    | 1 | 0  |  1 |     6
    | 1 | 1  |  0 |     7 
    | 1 | 1  |  1 |     8  */

    // Enable pin
    digitalWrite(MUX_EN, HIGH);

    // Input select. TC1 on pin1 and so on
    tc_n -= 1;
    char a2 = (tc_n & 0b100) >> 2;
    char a1 = (tc_n & 0b10) >> 1;
    char a0 = tc_n & 0b1;

    digitalWrite(MUX_A0, a0);
    digitalWrite(MUX_A1, a1);
    digitalWrite(MUX_A2, a2);

    // Stabilise signals
    delay(150);
}

/**************************************************************************/
/*!
    @brief  Disable the multiplexer
*/
/**************************************************************************/
void TCMuxShield::disableMux(void)
{
    digitalWrite(MUX_A0, LOW);
    digitalWrite(MUX_A1, LOW);
    digitalWrite(MUX_A2, LOW);
    digitalWrite(MUX_EN, LOW);
}

/**************************************************************************/
/*!
    @brief Linearizes type K thermocouple reading based on NIST table. Original code by heypete from
    the Adafruit forums. See https://learn.adafruit.com/calibrating-sensors/maxim-31855-linearization
    @param tc_temp Temperature value of the thermocouple
    @param int_temp Temperature value of the sensor chip for compensation
    @return Linearized temperature
*/
/**************************************************************************/
float TCMuxShield::linearizeTC(float tc_temp, float int_temp)
{
    // https://learn.adafruit.com/calibrating-sensors/maxim-31855-linearization
    float thermocoupleVoltage = (tc_temp - int_temp) * 0.041276;
   
   // MAX31855 cold junction voltage reading in mV
   float coldJunctionVoltage = -0.176004136860E-01 +
        0.389212049750E-01  * int_temp +
        0.185587700320E-04  * pow(int_temp, 2.0) +
        -0.994575928740E-07 * pow(int_temp, 3.0) +
        0.318409457190E-09  * pow(int_temp, 4.0) +
        -0.560728448890E-12 * pow(int_temp, 5.0) +
        0.560750590590E-15  * pow(int_temp, 6.0) +
        -0.320207200030E-18 * pow(int_temp, 7.0) +
        0.971511471520E-22  * pow(int_temp, 8.0) +
        -0.121047212750E-25 * pow(int_temp, 9.0) +
        0.118597600000E+00  * exp(-0.118343200000E-03 * pow((int_temp-0.126968600000E+03), 2.0));
                        
   // cold junction voltage + thermocouple voltage         
   float voltageSum = thermocoupleVoltage + coldJunctionVoltage;
   
   // calculate corrected temperature reading based on coefficients for 3 different ranges   
   float b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
   if(thermocoupleVoltage < 0)
   {
        b0 = 0.0000000E+00;
        b1 = 2.5173462E+01;
        b2 = -1.1662878E+00;
        b3 = -1.0833638E+00;
        b4 = -8.9773540E-01;
        b5 = -3.7342377E-01;
        b6 = -8.6632643E-02;
        b7 = -1.0450598E-02;
        b8 = -5.1920577E-04;
        b9 = 0.0000000E+00;
   }
   
   else if (thermocoupleVoltage < 20.644)
   {
        b0 = 0.000000E+00;
        b1 = 2.508355E+01;
        b2 = 7.860106E-02;
        b3 = -2.503131E-01;
        b4 = 8.315270E-02;
        b5 = -1.228034E-02;
        b6 = 9.804036E-04;
        b7 = -4.413030E-05;
        b8 = 1.057734E-06;
        b9 = -1.052755E-08;
   }
   
   else if (thermocoupleVoltage < 54.886)
   {
        b0 = -1.318058E+02;
        b1 = 4.830222E+01;
        b2 = -1.646031E+00;
        b3 = 5.464731E-02;
        b4 = -9.650715E-04;
        b5 = 8.802193E-06;
        b6 = -3.110810E-08;
        b7 = 0.000000E+00;
        b8 = 0.000000E+00;
        b9 = 0.000000E+00;
   }
   
   else {
        // Out of range, this should not happen
        return 0;
   }
   
   return b0 + 
      b1 * voltageSum +
      b2 * pow(voltageSum, 2.0) +
      b3 * pow(voltageSum, 3.0) +
      b4 * pow(voltageSum, 4.0) +
      b5 * pow(voltageSum, 5.0) +
      b6 * pow(voltageSum, 6.0) +
      b7 * pow(voltageSum, 7.0) +
      b8 * pow(voltageSum, 8.0) +
      b9 * pow(voltageSum, 9.0);
}