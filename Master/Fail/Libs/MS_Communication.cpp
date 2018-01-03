// 
// 
// 

#include "MS_Communication.h"

#define I_AM_MASTER
#define DESTINATION_ADDRESS 0x01

void MS_CommunicationClass::init()
{
	I2CDataTransfer.init(MASTER);
	I2CDataTransfer.RequestDataGroup.AddAddress<boolean>();

	_isLockedIdx = I2CDataTransfer.WriteDataGroup.AddAddress<boolean>();	//isLocked
	_hourIdx = I2CDataTransfer.WriteDataGroup.AddAddress<uint8_t>();	//hour
	_minIdx = I2CDataTransfer.WriteDataGroup.AddAddress<uint8_t>();		//minute
	_secondIdx = I2CDataTransfer.WriteDataGroup.AddAddress<uint8_t>();	//second
	_dayIdx = I2CDataTransfer.WriteDataGroup.AddAddress<uint8_t>();		//day
	_monthIdx = I2CDataTransfer.WriteDataGroup.AddAddress<uint8_t>();	//month
	_yearIdx = I2CDataTransfer.WriteDataGroup.AddAddress<uint16_t>();	//year

	rtc.begin();

}

void MS_CommunicationClass::Execute()
{
	uint32_t t = micros();
	static uint32_t tmr = millis();
	if (tmr > millis())
		tmr = millis();
	if (millis() - tmr > 500)
	{
		tmr = millis();
		I2CDataTransfer.Request(DESTINATION_ADDRESS);
		DateTime now = rtc.now();
		I2CDataTransfer.WriteDataGroup.SetValue<boolean>(_isLockedIdx, MatrixKeyToLEDs.IsLocked());
		I2CDataTransfer.WriteDataGroup.SetValue<uint8_t>(_hourIdx, now.hour());
		I2CDataTransfer.WriteDataGroup.SetValue<uint8_t>(_minIdx, now.minute());
		I2CDataTransfer.WriteDataGroup.SetValue<uint8_t>(_secondIdx, now.second());
		I2CDataTransfer.WriteDataGroup.SetValue<uint8_t>(_dayIdx, now.date());
		I2CDataTransfer.WriteDataGroup.SetValue<uint8_t>(_monthIdx, now.month());
		I2CDataTransfer.WriteDataGroup.SetValue<uint16_t>(_yearIdx, now.year());

		I2CDataTransfer.Write(DESTINATION_ADDRESS);
		t = micros() - t;
		LOGLN(t);
	}
}


MS_CommunicationClass MS;

