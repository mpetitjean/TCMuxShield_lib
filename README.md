# TCMuxShield (KTA-259) Arduino library

## About 

This library allows to read temperature from thermocouples on a [KTA-259](https://oceancontrols.com.au/kta-259.html) Arduino shield from Ocean Controls.

It includes NIST thermocouple voltage linearization.

## Usage

An example code is provided. Create a `TCMuxShield` object by specifying the Chip Select (CS) pin number (hardware default on the shield is 9).

Use the method `readTemperature` to get the temperature of a given thermocouple.


