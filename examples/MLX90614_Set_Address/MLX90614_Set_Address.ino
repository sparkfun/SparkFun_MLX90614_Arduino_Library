/****************************************************************************** 
MLX90614_Set_Address.ino
Configure an MLX90614 to use a new I2C address.

This example demonstrates the setAddress function.
If an MLX90614 is configured for the default address (0x5A),
it'll change the address to a new address (0x5B).
If the Arduino can communicate with an MLX90614 on the new
address, it'll print the device's ID registers.

Note: for the new address to take effect, you need to cycle
power to the MLX90614.

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

const byte oldAddress = 0x5A;
const byte newAddress = 0x5B;

void setup() 
{
  Serial.begin(9600); // Initialize Serial to log output
  Serial.println("Press a key to begin");
  while (!Serial.available()) ;
  
  therm.begin(oldAddress); // Try using the old address first
  
  byte address;
  if (!therm.readID()) // Try reading the ID registers
  {
    // If the read fails, try re-initializing with the
    // new address. Maybe we've already set it.
    therm.begin(newAddress);
    
    if (therm.readID()) // Read from the ID registers
    { // If the read succeeded, print the ID:
      Serial.println("Communicated with new address.");
      Serial.println("ID: 0x" + 
                     String(therm.getIDH(), HEX) +
                     String(therm.getIDL(), HEX));
    }
    else
    {
      Serial.println("Failed to communicate with either address.");      
    }
  }
  else
  {
    // If the read suceeds, change the address to something
    // new.
    if (!therm.setAddress(newAddress))
    {
      Serial.println("Failed to set new address.");
    }
    else
    {
      Serial.println("Set the address to 0x" + String(newAddress, HEX));
      Serial.println("Cycle power to try it out.");
    }
  }
}

void loop() 
{

}
