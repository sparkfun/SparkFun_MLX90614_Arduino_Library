/****************************************************************************** 
MLX90614_Get_ID.ino
Print ID register values stored in the MLX90614

This example reads from the MLX90614's ID registers, and 
prints out the 64-byte value to the serial monitor.

Hardware Hookup (if you're not using the eval board):
MLX90614 ------------- Arduino
  VDD ------------------ 3.3V
  VSS ------------------ GND
  SDA ------------------ SDA (A4 on older boards)
  SCL ------------------ SCL (A5 on older boards)
  
Jim Lindblom @ SparkFun Electronics
October 23, 2015
https://github.com/sparkfun/SparkFun_MLX90614_Arduino_Library

Development environment specifics:
Arduino 1.6.5
SparkFun IR Thermometer Evaluation Board - MLX90614
******************************************************************************/

#include <Wire.h> // I2C library, required for MLX90614
#include <SparkFunMLX90614.h> // SparkFunMLX90614 Arduino library

IRTherm therm; // Create an IRTherm object to interact with throughout

void setup() 
{
  Serial.begin(9600); // Initialize Serial to log output
  therm.begin(); // Initialize the MLX90614
  
  if (therm.readID()) // Read from the ID registers
  { // If the read succeeded, print the ID:
    Serial.println("ID: 0x" + 
                   String(therm.getIDH(), HEX) +
                   String(therm.getIDL(), HEX));
  }
}

void loop() 
{

}
