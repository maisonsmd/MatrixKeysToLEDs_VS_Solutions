// 
// 
// 

#include "I2CDataTransfer.h"

void I2CDataTransferClass::init(Role role, uint8_t address = 0, void(*onRequestFunction)() = nullptr, void(*onReceiveFunction)(uint8_t sender, uint8_t groupID) = nullptr)
{
	_role = role;
	_I2CAddress = address;
	_onReceiveFunction = onReceiveFunction;
	_onRequestFunction = onRequestFunction;

	ReceiveDataGroup.SetGroupID(DEFAULT_GROUP);
	WriteDataGroup.SetGroupID(DEFAULT_GROUP);
	RequestDataGroup.SetGroupID(DEFAULT_GROUP);

	if (_role = Role::MASTER)
	{
		_I2CAddress = 0;
		Wire.begin();
	}
	else
	{
		Wire.begin(_I2CAddress);
		Wire.onReceive(OnReceiveEvent);
		Wire.onRequest(OnRequestEvent);
	}
}

void I2CDataTransferClass::Request(uint8_t address)
{
	Wire.requestFrom(address, uint8_t(PACKAGE_SIZE));
	uint8_t arr[PACKAGE_SIZE];
	uint8_t index = 0;
	memset(arr, 0, sizeof(arr));
	boolean isResponsed = false;
	while (Wire.available())
	{
		isResponsed = true;
		if (index < sizeof(arr))
			arr[index] = Wire.read();
		index++;
	}
	if (!isResponsed)
	{
		//Serial.println("Response nothing");
		return;
	}
	uint8_t responser = arr[0];
	uint8_t groupId = arr[1];
	//LOGLN(String(responser) + " responsed groupId = " + String(groupId));
	/*for (uint8_t i = 0; i < sizeof(arr); i++)
	{
		Serial.print(arr[i], HEX);
		Serial.print(',');
	}
	Serial.println();*/
	if (groupId == DEFAULT_GROUP)
	{
		RequestDataGroup.SetBytes(arr + 2);
		//LOGLN(RequestDataGroup.GetValue<boolean>(0));
	}
	else
	{
		DataTransferer.GetDataGroup(groupId)->SetBytes(arr + 2);
		//Serial.println(DataTransferer.GetDataGroup(groupId)->GetValue<float>(0));
	}
}

void I2CDataTransferClass::ResponseRequest()
{
	ResponseRequest(RequestDataGroup);
}

void I2CDataTransferClass::ResponseRequest(DataGroupClass dataGroup)
{
	Wire.write(_I2CAddress);
	Wire.write(dataGroup.GetGroupID());
	for (uint8_t i = 0; i < PACKAGE_SIZE - 2; i++)
		Wire.write(dataGroup.GetBytes()[i]);
}

void I2CDataTransferClass::Write(uint8_t address, DataGroupClass dataGroup)
{
	Wire.beginTransmission(address);
	Wire.write(I2CDataTransfer._I2CAddress);
	Wire.write(dataGroup.GetGroupID());
	//Serial.println();
	for (uint8_t i = 0; i < PACKAGE_SIZE - 2; i++)
	{
		Wire.write(dataGroup.GetBytes()[i]);
		//Serial.print(dataGroup.GetBytes()[i], HEX);
		//Serial.print(',');
	}
	//Serial.println();
	Wire.endTransmission();
}
void I2CDataTransferClass::Write(uint8_t address, uint8_t dataGroupId)
{
	//Serial.println(dataGroupId); 
	//Serial.println( DataTransferer.GetDataGroup(dataGroupId)->GetGroupID());
	Write(address, *DataTransferer.GetDataGroup(dataGroupId));
}
void I2CDataTransferClass::Write(uint8_t address)
{
	Write(address, WriteDataGroup);
}
//
//void I2CDataTransferClass::SetRequestDataGroup(uint8_t groupId)
//{
//	_requestDataGroupIndex = groupId;
//}
//
//void I2CDataTransferClass::SetReceiveDataGroup(uint8_t groupId)
//{
//	_receiveDataGroupIndex = groupId;
//}
//
//void I2CDataTransferClass::SetWriteDataGroup(uint8_t groupId)
//{
//	_writeDataGroupIndex = groupId;
//}

void I2CDataTransferClass::OnReceiveEvent(int howMany)
{
	uint8_t arr[PACKAGE_SIZE];
	uint8_t index = 0;
	while (Wire.available())
	{
		if (index < sizeof(arr))
			arr[index] = Wire.read();
		//Serial.print(arr[index], HEX);
		//Serial.print(',');
		index++;
	}
	//Serial.println();
	uint8_t sender = arr[0];
	uint8_t groupIdx = arr[1];
	if (groupIdx == DEFAULT_GROUP)
		I2CDataTransfer.ReceiveDataGroup.SetBytes(arr + 2);
	else
		DataTransferer.GetDataGroup(groupIdx)->SetBytes(arr + 2);
	//Serial.println(DataTransferer.GetDataGroup(groupIdx)->GetValue<float>(0));

	//uint8_t id = I2CDataTransfer._dataGroup[I2CDataTransfer._receiveDataGroupIndex].
	if (I2CDataTransfer._onReceiveFunction != nullptr)
		I2CDataTransfer._onReceiveFunction(sender, groupIdx);
}

void I2CDataTransferClass::OnRequestEvent()
{
	if (I2CDataTransfer._onRequestFunction != nullptr)
		I2CDataTransfer._onRequestFunction();
}


I2CDataTransferClass I2CDataTransfer;
