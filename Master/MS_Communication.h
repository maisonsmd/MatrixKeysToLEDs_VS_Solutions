// MS_Communication.h

#ifndef _MS_COMMUNICATION_h
#define _MS_COMMUNICATION_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

//#include <Wire.h>
#include "I2CDataTransfer.h"
#include "MatrixKeyToLEDs.h"
#include "Sodaq_DS3231.h"

class MS_CommunicationClass
{
protected:
	DateTime now; //get the current date-time

	uint8_t _hourIdx;
	uint8_t _minIdx;
	uint8_t _secondIdx;
	uint8_t _dayIdx;
	uint8_t _monthIdx;
	uint8_t _yearIdx;

	uint8_t _lockStateIdx;
	uint8_t _beepIdx;
	uint8_t _loginStateIdx;
	uint8_t _unlockCmdIdx;
	uint8_t _lockCmdIdx;
public:
	void init();
	void Execute();
};

extern MS_CommunicationClass MS;

#endif

