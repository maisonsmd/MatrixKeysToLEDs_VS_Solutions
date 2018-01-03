// 
// 
// 

#include "SM_Communication.h"

#define I_AM_SLAVE
#define MY_ADDRESS 0x01

void SM_CommunicationClass::OnRequestFunction()
{
	//LOGLN("YOU ARE REQUESTED AT " + String(millis()));
	I2CDataTransfer.RequestDataGroup.SetValue<LoginState>(SM._loginStateIdx, SM._loginState);
	I2CDataTransfer.ResponseRequest();
	SM.Debeep();
	I2CDataTransfer.RequestDataGroup.SetValue<boolean>(SM._unlockCmdIdx, false);
	I2CDataTransfer.RequestDataGroup.SetValue<boolean>(SM._lockCmdIdx, false);
}

void SM_CommunicationClass::OnWriteFunction(uint8_t sender, uint8_t grID)
{
	//LOGLN(String(sender) + " WROTE GROUP " + String(grID));
	//LOGLN(I2CDataTransfer.ReceiveDataGroup.GetValue<boolean>(SM._isLockedIdx));

	SM.dt.hour = I2CDataTransfer.ReceiveDataGroup.GetValue<uint8_t>(SM._hourIdx);
	SM.dt.minute = I2CDataTransfer.ReceiveDataGroup.GetValue<uint8_t>(SM._minIdx);
	SM.dt.second = I2CDataTransfer.ReceiveDataGroup.GetValue<uint8_t>(SM._secondIdx);
	SM.dt.day = I2CDataTransfer.ReceiveDataGroup.GetValue<uint8_t>(SM._dayIdx);
	SM.dt.month = I2CDataTransfer.ReceiveDataGroup.GetValue<uint8_t>(SM._monthIdx);
	SM.dt.year = I2CDataTransfer.ReceiveDataGroup.GetValue<uint16_t>(SM._yearIdx);

	SM._lockState = I2CDataTransfer.ReceiveDataGroup.GetValue<LockState>(SM._lockStateIdx);

	//switch (SM._lockState)
	//{
	//case LockState::LOCKED:
	//	LOGLN("LOCKED");
	//	break;
	//case LockState::SHORT_LOCKED:
	//	LOGLN("SHORT_LOCKED");
	//	break;
	//case LockState::UNLOCKED:
	//	LOGLN("UNLOCKED");
	//	break;
	//default:
	//	break;
	//}
	//LOGLN(SM.dt.toString());
}

void SM_CommunicationClass::init()
{
	I2CDataTransfer.init(SLAVE, MY_ADDRESS, OnRequestFunction, OnWriteFunction);
	_beepIdx = I2CDataTransfer.RequestDataGroup.AddAddress<boolean>();	//isTaggeg, true when a tag is tagged
	_loginStateIdx = I2CDataTransfer.RequestDataGroup.AddAddress<LoginState>();//Login state
	_unlockCmdIdx = I2CDataTransfer.RequestDataGroup.AddAddress<boolean>();			//Unlock command from slave
	_lockCmdIdx = I2CDataTransfer.RequestDataGroup.AddAddress<boolean>();			//lock command from slave

	_hourIdx = I2CDataTransfer.ReceiveDataGroup.AddAddress<uint8_t>();	//hour
	_minIdx = I2CDataTransfer.ReceiveDataGroup.AddAddress<uint8_t>();	//minute
	_secondIdx = I2CDataTransfer.ReceiveDataGroup.AddAddress<uint8_t>();	//second
	_dayIdx = I2CDataTransfer.ReceiveDataGroup.AddAddress<uint8_t>();	//day
	_monthIdx = I2CDataTransfer.ReceiveDataGroup.AddAddress<uint8_t>();	//month
	_yearIdx = I2CDataTransfer.ReceiveDataGroup.AddAddress<uint16_t>();	//year

	_lockStateIdx = I2CDataTransfer.ReceiveDataGroup.AddAddress<LockState>();
}

void SM_CommunicationClass::Execute()
{

}

void SM_CommunicationClass::Beep()
{
	I2CDataTransfer.RequestDataGroup.SetValue<boolean>(_beepIdx, true);
}

void SM_CommunicationClass::Debeep()
{
	I2CDataTransfer.RequestDataGroup.SetValue<boolean>(_beepIdx, false);
}

void SM_CommunicationClass::UnlockMaster()
{
	I2CDataTransfer.RequestDataGroup.SetValue<boolean>(_unlockCmdIdx, true);
}

void SM_CommunicationClass::LockMaster()
{
	I2CDataTransfer.RequestDataGroup.SetValue<boolean>(_lockCmdIdx, true);
}




SM_CommunicationClass SM;

