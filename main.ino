/***************************************************************************************************

TCMuxShield library example code.
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

TCMuxShield tc = TCMuxShield(9);

// the setup function runs once when you press reset or power the board
void setup()
{
    // initialize Serial
    Serial.begin(9600);
    while(!Serial) {;}
}

// the loop function runs over and over again forever
void loop()
{
    // Measure all thermocouple inputs, and display temperatures and error codes over serial
    for (char i = 1; i < 9; ++i)
    {
        int err_code = tc.readTemperature(i);
        Serial.print("-------------------\nThermocouple ");
        Serial.println(i, DEC);
        Serial.print("Temperature = ");
        Serial.println(tc.temperature);
        Serial.print("Error Code = ");
        Serial.println(err_code);
    }
    
    delay(5000);
}
