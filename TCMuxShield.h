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
#ifndef _TCMUXSHIELD_H
#define _TCMUXSHIELD_H

// Pins
#define MUX_A0 4
#define MUX_A1 5
#define MUX_A2 6
#define MUX_EN 7

// MAX31855 error codes
#define TC_OK   0
#define TC_OC   1
#define TC_SCG  2
#define TC_SCV  4

// MAX31855 register bits
#define MAX31855_SCV_BIT            2
#define MAX31855_SCG_BIT            1
#define MAX31855_OC_BIT             0
#define MAX31855_TCDATA_OFFSET      18
#define MAX31855_INTDATA_OFFSET     4
#define MAX31855_14B_SIGN_OFFSET    13
#define MAX31855_12B_SIGN_OFFSET    11

class TCMuxShield {
    
    public:
        // Constructor
        TCMuxShield(int _CS);
        
        // Methods
        int readTemperature(int tc_n);

        // Attributes
        float temperature;
    
    private:
        // Methods
        int getTCData(void);
        void activateMux(int tc_n);
        void disableMux(void);
        float linearizeTC(float tc_temp, float int_temp);

        // Attributes
        int PIN_CS;
};

#endif