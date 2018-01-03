// 
// 
// 

#include "MS_Communication.h"

#define I_AM_MASTER
#define DESTINATION_ADDRESS 0x01

void MS_CommunicationClass::init()
{
	Wire.begin();
	I2CDataTransfer.init(MASTER);
	_beepIdx = I2CDataTransfer.RequestDataGroup.AddAddress<boolean>();			//Beep command
	_loginStateIdx = I2CDataTransfer.RequestDataGroup.AddAddress<LoginState>();		//Login state from slave
	_unlockCmdIdx = I2CDataTransfer.RequestDataGroup.AddAddress<boolean>();			//Unlock command from slave
	_lockCmdIdx = I2CDataTransfer.RequestDataGroup.AddAddress<boolean>();			//lock command from slave

	_hourIdx = I2CDataTransfer.WriteDataGroup.AddAddress<uint8_t>();	//hour
	_minIdx = I2CDataTransfer.WriteDataGroup.AddAddress<uint8_t>();		//minute
	_secondIdx = I2CDataTransfer.WriteDataGroup.AddAddress<uint8_t>();	//second
	_dayIdx = I2CDataTransfer.WriteDataGroup.AddAddress<uint8_t>();		//day
	_monthIdx = I2CDataTransfer.WriteDataGroup.AddAddress<uint8_t>();	//month
	_yearIdx = I2CDataTransfer.WriteDataGroup.AddAddress<uint16_t>();	//year

	_lockStateIdx = I2CDataTransfer.WriteDataGroup.AddAddress<LockState>();	//Lock state

	rtc.begin();
}

void MS_CommunicationClass::Execute()
{
	/*uint32_t t = micros();*/
	static uint32_t tmr = millis();
	if (tmr > millis())
		tmr = millis();
	if (millis() - tmr > 100)
	{
		tmr = millis();
		I2CDataTransfer.Request(DESTINATION_ADDRESS);
		if (I2CDataTransfer.RequestDataGroup.GetValue<boolean>(_beepIdx))	//Beep command from slave
		{
			MatrixKeyToLEDs.Beep();
		}
		if (I2CDataTransfer.RequestDataGroup.GetValue<boolean>(_unlockCmdIdx))	//unlock command from slave
		{
			LOGLNF("SLAVE IS REQUESTING TO UNLOCK");
			MatrixKeyToLEDs.Unlock();
		}
		if (I2CDataTransfer.RequestDataGroup.GetValue<boolean>(_lockCmdIdx))	//lock command from slave
		{
			LOGLNF("SLAVE IS REQUESTING TO LOCK");
			MatrixKeyToLEDs.Lock();
		}

		MatrixKeyToLEDs.pLoginState = I2CDataTransfer.RequestDataGroup.GetValue<LoginState>(_loginStateIdx);
		

		now = rtc.now();
		I2CDataTransfer.WriteDataGroup.SetValue<uint8_t>(_hourIdx, now.hour());
		I2CDataTransfer.WriteDataGroup.SetValue<uint8_t>(_minIdx, now.minute());
		I2CDataTransfer.WriteDataGroup.SetValue<uint8_t>(_secondIdx, now.second());
		I2CDataTransfer.WriteDataGroup.SetValue<uint8_t>(_dayIdx, now.date());
		I2CDataTransfer.WriteDataGroup.SetValue<uint8_t>(_monthIdx, now.month());
		I2CDataTransfer.WriteDataGroup.SetValue<uint16_t>(_yearIdx, now.year());

		I2CDataTransfer.WriteDataGroup.SetValue<LockState>(_lockStateIdx, MatrixKeyToLEDs.GetLockState());

		I2CDataTransfer.Write(DESTINATION_ADDRESS);
		/*t = micros() - t;
		LOGLN(t);*/
	}
}


MS_CommunicationClass MS;

