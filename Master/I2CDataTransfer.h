// I2CDataTransfer.h

#ifndef _I2CDATATRANSFER_h
#define _I2CDATATRANSFER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "DataTransferer.h"
#include <Wire.h>

//#define ENABLE_LOG

#ifdef ENABLE_LOG
#define BEGIN(x)	Serial.begin(x)
#define LOG(x)		Serial.print(x)
#define LOGF(x)		Serial.print(F(x))
#define LOGLN(x)	Serial.println(x)
#define LOGLNF(x)	Serial.println(F(x))
#else
#define BEGIN(x)
#define LOG(x)
#define LOGF(x)
#define LOGLN(x)
#define LOGLNF(x)
#endif // ENABLE_LOG

#define PACKAGE_SIZE 32
#define DEFAULT_GROUP 255

enum Role {
	MASTER,
	SLAVE
};

class I2CDataTransferClass : public DataTransfererClass
{
protected:
	uint8_t _I2CAddress = 0;
	Role _role = Role::SLAVE;
	/*uint8_t _requestDataGroupIndex = 0;
	uint8_t _receiveDataGroupIndex = 0;
	uint8_t _writeDataGroupIndex = 0;*/

	void(*_onRequestFunction)() = nullptr;
	void(*_onReceiveFunction)(uint8_t sender, uint8_t groupID) = nullptr;
public:
	DataGroupClass RequestDataGroup;
	DataGroupClass ReceiveDataGroup;
	DataGroupClass WriteDataGroup;

	void init(Role role, uint8_t address = 0, void(*onRequestFunction)() = nullptr, void(*onReceiveFunction)(uint8_t sender, uint8_t groupID) = nullptr);
	void Request(uint8_t address);
	void ResponseRequest();
	void ResponseRequest(DataGroupClass dataGroup);
	void Write(uint8_t address, DataGroupClass dataGroup);
	void Write(uint8_t address, uint8_t dataGroupId);
	void Write(uint8_t address);

	/*void SetRequestDataGroup(uint8_t groupId);
	void SetReceiveDataGroup(uint8_t groupId);
	void SetWriteDataGroup(uint8_t groupId);*/

	static void OnReceiveEvent(int howMany);
	static void OnRequestEvent();
};
extern I2CDataTransferClass I2CDataTransfer;

#endif

