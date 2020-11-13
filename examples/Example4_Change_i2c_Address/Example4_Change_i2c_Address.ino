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
#include <SparkFunMLX90614.h> //Click here to get the library: http://librarymanager/All#Qwiic_IR_Thermometer by SparkFun

IRTherm therm; // Create an IRTherm object to interact with throughout

void setup() 
{
  Serial.begin(115200); // Initialize Serial to log output
  Wire.begin(); // Join I2C bus

  // check if qwiic IR thermometer will acknowledge over I2C
  if (therm.begin() == false) {
    Serial.println("Qwiic IR thermometer did not acknowledge! Running I2C scanner.");
  }

  else {
    Serial.println("Device acknowledged!");

    Serial.println();
    Serial.println("Enter a new I2C address for the Qwiic IR Thermometer to use!");
    Serial.println("Don't use the 0x prefix. For instance, if you wanted to ");
    Serial.println("change the address to 0x5B, you would type 5B in the serial");
    Serial.println("input bar and press enter.");
    Serial.println();
    Serial.println("One more thing! Make sure your line ending is set to 'Both NL & CR'");
    Serial.println("in the Serial Monitor.");
    Serial.println();
    Serial.println("Note, for the new I2C address to take effect you need to power cycle");
    Serial.println("the MLX90614.");

    // Wait for serial data to be available
    while (Serial.available() == 0);

    if (Serial.available()) {
      uint8_t newAddress = 0;
      String stringBuffer = Serial.readString();
      char charBuffer[10];
      stringBuffer.toCharArray(charBuffer, 10);
      uint8_t success = sscanf(charBuffer, "%x", &newAddress);

      if (success) {
        if (newAddress > 0x08 && newAddress < 0x77) {
          Serial.println("Character received and device address is valid!");
          Serial.print("Attempting to set device address to 0x");
          Serial.println(newAddress, HEX);

          if (therm.setAddress(newAddress) == true) {
            Serial.println("Device address set succeeded!");
          }
          else {
            Serial.println("Device address set failed!");
          }

          delay(100); // give the hardware time to do whatever configurations it needs

          if (therm.isConnected()) {
            Serial.println("Device acknowledged on new I2C address!");
          } else {
            Serial.println("Device did not acknowledge on new I2C address!");
          }
        }
        else {
          Serial.println("Address out of range! Try an address between 0x08 and 0x77");
        }
      }
      else {
        Serial.println("Invalid text, try again.");
      }
    }
    delay(100);
  }
//  byte address;
//  if (!therm.readID()) // Try reading the ID registers
//  {
//    // If the read fails, try re-initializing with the
//    // new address. Maybe we've already set it.
//    therm.begin(newAddress);
//    
//    if (therm.readID()) // Read from the ID registers
//    { // If the read succeeded, print the ID:
//      Serial.println("Communicated with new address.");
//      Serial.println("ID: 0x" + 
//                     String(therm.getIDH(), HEX) +
//                     String(therm.getIDL(), HEX));
//    }
//    else
//    {
//      Serial.println("Failed to communicate with either address.");      
//    }
//  }
//  else
//  {
//    // If the read suceeds, change the address to something
//    // new.
//    if (!therm.setAddress(newAddress))
//    {
//      Serial.println("Failed to set new address.");
//    }
//    else
//    {
//      Serial.println("Set the address to 0x" + String(newAddress, HEX));
//      Serial.println("Cycle power to try it out.");
//    }
//  }
}

void loop() 
{
  // If no I2C device found or Qwiic IR thermometer is set to new address,
  // scan for available I2C devices
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++) {
    // The i2c_scanner uses the return value of the Write.endTransmission()
    // to see if a device ddi acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");

      nDevices++;
    }
    else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  delay(5000);  // Wait 5 seconds for next scan
}
