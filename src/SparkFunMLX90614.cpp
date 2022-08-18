/******************************************************************************
SparkFunMLX90614.cpp
Source file for the SparkFun IR Thermometer Library

Jim Lindblom @ SparkFun Electronics
October 23, 2015
https://github.com/sparkfun/SparkFun_MLX90614_Arduino_Library

This file defines the SMBus hardware interface(s) for the MLX90614 IR thermometer
and abstracts temperature measurments and other features of the MLX90614

Development environment specifics:
Arduino 1.6.5
SparkFun IR Thermometer Evaluation Board - MLX90614
******************************************************************************/

#include <SparkFunMLX90614.h>

IRTherm::IRTherm()
{
	// Set initial values for all private member variables
	_deviceAddress = 0;
	_defaultUnit = TEMP_C;
	_rawObject = 0;
	_rawAmbient = 0;
	_rawObject2 = 0;
	_rawMax = 0;
	_rawMin = 0;
}

bool IRTherm::begin(uint8_t address, TwoWire &wirePort)
{
	_deviceAddress = address; // Store the address in a private member
	_i2cPort = &wirePort;

	return (isConnected());
}

bool IRTherm::isConnected()
{
	_i2cPort->beginTransmission(_deviceAddress);
	if (_i2cPort->endTransmission() == 0)
		return true;
	return false;
}

void IRTherm::setUnit(temperature_units unit)
{
	_defaultUnit = unit; // Store the unit into a private member
}

bool IRTherm::read()
{
	// read both the object and ambient temperature values
	if (readObject() && readAmbient())
	{
		// If the reads succeeded, return success
		return true;
	}
	return false; // Else return fail
}

bool IRTherm::readRange()
{
	// Read both minimum and maximum values from EEPROM
	if (readMin() && readMax())
	{
		// If the read succeeded, return success
		return true;
	}
	return false; // Else return fail
}

float IRTherm::ambient(void)
{
	// Return the calculated ambient temperature
	return calcTemperature(_rawAmbient);
}

float IRTherm::object(void)
{
	// Return the calculated object temperature
	return calcTemperature(_rawObject);
}

float IRTherm::minimum(void)
{
	// Return the calculated minimum temperature
	return calcTemperature(_rawMin);
}

float IRTherm::maximum(void)
{
	// Return the calculated maximum temperature
	return calcTemperature(_rawMax);
}

bool IRTherm::readObject()
{
	int16_t rawObj;
	// Read from the TOBJ1 register, store into the rawObj variable
	if (I2CReadWord(MLX90614_REGISTER_TOBJ1, &rawObj))
	{
		// If the read succeeded
		if (rawObj & 0x8000) // If there was a flag error
		{
			return 0; // Return fail
		}
		// Store the object temperature into the class variable
		_rawObject = rawObj;
		return true;
	}
	return false;
}

bool IRTherm::readObject2()
{
	int16_t rawObj;
	// Read from the TOBJ2 register, store into the rawObj variable
	if (I2CReadWord(MLX90614_REGISTER_TOBJ2, &rawObj))
	{
		// If the read succeeded
		if (rawObj & 0x8000) // If there was a flag error
		{
			return 0; // Return fail
		}
		// Store the object2 temperature into the class variable
		_rawObject2 = rawObj;
		return true;
	}
	return false;
}

bool IRTherm::readAmbient()
{
	int16_t rawAmb;
	// Read from the TA register, store value in rawAmb
	if (I2CReadWord(MLX90614_REGISTER_TA, &rawAmb))
	{
		// If the read succeeds, store the read value
		_rawAmbient = rawAmb; // return success
		return true;
	}
	return false; // else return fail
}

bool IRTherm::readMax()
{
	int16_t toMax;
	// Read from the TOMax EEPROM address, store value in toMax
	if (I2CReadWord(MLX90614_REGISTER_TOMAX, &toMax))
	{
		_rawMax = toMax;
		return true;
	}
	return false;
}

bool IRTherm::readMin()
{
	int16_t toMin;
	// Read from the TOMin EEPROM address, store value in toMax
	if (I2CReadWord(MLX90614_REGISTER_TOMIN, &toMin))
	{
		_rawMin = toMin;
		return true;
	}
	return false;
}

uint8_t IRTherm::setMax(float maxTemp)
{
	// Convert the unit-ed value to a raw ADC value:
	int16_t rawMax = calcRawTemp(maxTemp);
	// Write that value to the TOMAX EEPROM address:
	return writeMLXEEPROM(MLX90614_REGISTER_TOMAX, rawMax);
}

uint8_t IRTherm::setMin(float minTemp)
{
	// Convert the unit-ed value to a raw ADC value:
	int16_t rawMin = calcRawTemp(minTemp);
	// Write that value to the TOMIN EEPROM address:
	return writeMLXEEPROM(MLX90614_REGISTER_TOMIN, rawMin);
}

uint8_t IRTherm::setEmissivity(float emis)
{
	// Make sure emissivity is between 0.1 and 1.0
	if ((emis > 1.0) || (emis < 0.1))
		return 0; // Return fail if not
	// Calculate the raw 16-bit value:
	uint16_t ke = uint16_t(65535.0 * emis);
	ke = constrain(ke, 0x2000, 0xFFFF);

	// Write that value to the ke register
	return writeMLXEEPROM(MLX90614_REGISTER_KE, (int16_t)ke);
}

float IRTherm::readEmissivity()
{
	int16_t ke;
	if (I2CReadWord(MLX90614_REGISTER_KE, &ke))
	{
		// If we successfully read from the ke register
		// calculate the emissivity between 0.1 and 1.0:
		return (((float)((uint16_t)ke)) / 65535.0);
	}
	return 0; // Else return fail
}

uint8_t IRTherm::readAddress()
{
	int16_t tempAdd;
	// Read from the 7-bit I2C address EEPROM storage address:
	if (I2CReadWord(MLX90614_REGISTER_ADDRESS, &tempAdd))
	{
		// If read succeeded, return the address:
		return (uint8_t) tempAdd;
	}
	return 0; // Else return fail
}

bool IRTherm::setAddress(uint8_t newAdd)
{
	int16_t tempAdd;
	// Make sure the address is within the proper range:
	if ((newAdd >= 0x80) || (newAdd == 0x00))
		return 0; // Return fail if out of range
	// Read from the I2C address address first:
	if (I2CReadWord(MLX90614_REGISTER_ADDRESS, &tempAdd))
	{
		tempAdd &= 0xFF00; // Mask out the address (MSB is junk?)
		tempAdd |= newAdd; // Add the new address

		// Write the new addres back to EEPROM:
		return writeMLXEEPROM(MLX90614_REGISTER_ADDRESS, tempAdd);
	}
	return 0;
}

uint8_t IRTherm::readID()
{
	for (int i=0; i<4; i++)
	{
		int16_t temp = 0;
		// Read from all four ID registers, beginning at the first:
		if (!I2CReadWord(MLX90614_REGISTER_ID0 + i, &temp))
			return 0;
		// If the read succeeded, store the ID into the id array:
		id[i] = (uint16_t)temp;
	}
	return 1;
}

uint32_t IRTherm::getIDH()
{
	// Return the upper 32 bits of the ID
	return ((uint32_t)id[3] << 16) | id[2];
}

uint32_t IRTherm::getIDL()
{
	// Return the lower 32 bits of the ID
	return ((uint32_t)id[1] << 16) | id[0];
}

void IRTherm::sleep()
{
	// Calculate a crc8 value.
	// Bits sent: _deviceAddress (shifted left 1) + 0xFF
	uint8_t crc = crc8(0, (_deviceAddress << 1));
	crc = crc8(crc, MLX90614_REGISTER_SLEEP);

	// Manually send the sleep command:
	_i2cPort->beginTransmission(_deviceAddress);
	_i2cPort->write(MLX90614_REGISTER_SLEEP);
	_i2cPort->write(crc);
	_i2cPort->endTransmission(true);

	// Set the SCL pin LOW, and SDA pin HIGH (should be pulled up)
	pinMode(SCL, OUTPUT);
	digitalWrite(SCL, LOW);
	pinMode(SDA, INPUT);
}

void IRTherm::wake()
{
	// Wake operation from datasheet
	_i2cPort->endTransmission(true); // stop i2c bus transmission BEFORE sending wake up request
	pinMode(SCL, INPUT); // SCL high
	pinMode(SDA, OUTPUT);
	digitalWrite(SDA, LOW); // SDA low
	delay(50); // delay at least 33ms
	pinMode(SDA, INPUT); // SDA high
	delay(250);
	// PWM to SMBus mode:
	pinMode(SCL, OUTPUT);
	digitalWrite(SCL, LOW); // SCL low
	delay(10); // Delay at least 1.44ms
	pinMode(SCL, INPUT); // SCL high
	_i2cPort->beginTransmission(_deviceAddress); // reactivate i2c bus transmission AFTER sending wake up request
}

int16_t IRTherm::calcRawTemp(float calcTemp)
{
	int16_t rawTemp; // Value to eventually be returned

	if (_defaultUnit == TEMP_RAW)
	{
		// If unit is set to raw, just return that:
		rawTemp = (int16_t) calcTemp;
	}
	else
	{
		float tempFloat;
		// First convert each temperature to Kelvin:
		if (_defaultUnit == TEMP_F)
		{
			// Convert from farenheit to Kelvin
			tempFloat = (calcTemp - 32.0) * 5.0 / 9.0 + 273.15;
		}
		else if (_defaultUnit == TEMP_C)
		{
			tempFloat = calcTemp + 273.15;
		}
		else if (_defaultUnit == TEMP_K)
		{
			tempFloat = calcTemp;
		}
		// Then multiply by 0.02 degK / bit
		tempFloat *= 50.0;
		rawTemp = (int16_t) tempFloat;
	}
	return rawTemp;
}

float IRTherm::calcTemperature(int16_t rawTemp)
{
	float retTemp;

	if (_defaultUnit == TEMP_RAW)
	{
		retTemp = (float) rawTemp;
	}
	else
	{
		retTemp = float(rawTemp) * 0.02;
		if (_defaultUnit != TEMP_K)
		{
			retTemp -= 273.15;
			if (_defaultUnit == TEMP_F)
			{
				retTemp = retTemp * 9.0 / 5.0 + 32.0;
			}
		}
	}

	return retTemp;
}

bool IRTherm::I2CReadWord(uint8_t reg, int16_t * dest)
{
	_i2cPort->beginTransmission(_deviceAddress);
	_i2cPort->write(reg);

	_i2cPort->endTransmission(false); // Send restart
	_i2cPort->requestFrom(_deviceAddress, (uint8_t) 3);

	uint8_t lsb = _i2cPort->read();
	uint8_t msb = _i2cPort->read();
	uint8_t pec = _i2cPort->read();

	uint8_t crc = crc8(0, (_deviceAddress << 1));
	crc = crc8(crc, reg);
	crc = crc8(crc, (_deviceAddress << 1) + 1);
	crc = crc8(crc, lsb);
	crc = crc8(crc, msb);

	if (crc == pec)
	{
		*dest = (msb << 8) | lsb;
		return true;
	}
	else
	{
		return false;;
	}
}

bool IRTherm::writeMLXEEPROM(uint8_t reg, int16_t data)
{
	// Clear out EEPROM first:
	if (I2CWriteWord(reg, 0) != 0)
		return 0; // If the write failed, return 0
	delay(10); // Delay tErase at least 5 ms

	uint8_t i2cRet = I2CWriteWord(reg, data);
	delay(10); // Delay tWrite at least 5 ms

	if (i2cRet == 0)
		return true;
	else
		return false;
}

uint8_t IRTherm::I2CWriteWord(uint8_t reg, int16_t data)
{
	uint8_t crc;
	uint8_t lsb = data & 0x00FF;
	uint8_t msb = (data >> 8);

	crc = crc8(0, (_deviceAddress << 1));
	crc = crc8(crc, reg);
	crc = crc8(crc, lsb);
	crc = crc8(crc, msb);

	_i2cPort->beginTransmission(_deviceAddress);
	_i2cPort->write(reg);
	_i2cPort->write(lsb);
	_i2cPort->write(msb);
	_i2cPort->write(crc);
	return _i2cPort->endTransmission(true);
}

uint8_t IRTherm::crc8 (uint8_t inCrc, uint8_t inData)
{
	uint8_t i;
	uint8_t data;
	data = inCrc ^ inData;
	for ( i = 0; i < 8; i++ )
	{
		if (( data & 0x80 ) != 0 )
		{
			data <<= 1;
			data ^= 0x07;
		}
		else
		{
			data <<= 1;
		}
	}
	return data;
}
