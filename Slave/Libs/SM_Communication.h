// SM_Communication.h

#ifndef _SM_COMMUNICATION_h
#define _SM_COMMUNICATION_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "I2CDataTransfer.h"
#include "UserManager.h"

#define ENABLE_LOG

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

enum LockState
{
	SHORT_LOCKED,
	LOCKED,
	UNLOCKED,
};

class DateTime
{
public:
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t day;
	uint8_t month;
	uint16_t year;

	String toString() {
		String str = "";
		if (day < 10)
			str += "0";
		str += String(day) + "/";

		if (month < 10)
			str += "0";
		str += String(month) + "/";

		str += String(year) + "-";

		if (hour < 10)
			str += "0";
		str += String(hour) + ":";
		if (minute < 10)
			str += "0";
		str += String(minute) + ":";
		if (second < 10)
			str += "0";
		str += String(second);
		return str;
	}

};

class SM_CommunicationClass
{
protected:
	static void OnRequestFunction();
	static void OnWriteFunction(uint8_t sender, uint8_t grID);

	uint8_t _hourIdx;
	uint8_t _minIdx;
	uint8_t _secondIdx;
	uint8_t _dayIdx;
	uint8_t _monthIdx;
	uint8_t _yearIdx;

	uint8_t _beepIdx;
	uint8_t _loginStateIdx;
	uint8_t _lockStateIdx;
	uint8_t _unlockCmdIdx;
	uint8_t _lockCmdIdx;

public:
	DateTime dt;

	LockState _lockState;
	LoginState _loginState = LoginState::LOGGED_OUT;

	void init();
	void Execute();
	void Beep();
	void Debeep();
	void UnlockMaster(); 
	void LockMaster();
};

extern SM_CommunicationClass SM;

#endif

