// MS_Communication.h

#ifndef _MS_COMMUNICATION_h
#define _MS_COMMUNICATION_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "I2CDataTransfer.h"
#include "MatrixKeyToLEDs.h"
#include "Sodaq_DS3231.h"

class MS_CommunicationClass
{
protected:
	DateTime now = rtc.now(); //get the current date-time

	uint8_t _isLockedIdx;
	uint8_t _hourIdx;
	uint8_t _minIdx;
	uint8_t _secondIdx;
	uint8_t _dayIdx;
	uint8_t _monthIdx;
	uint8_t _yearIdx;
public:
	void init();
	void Execute();
};

extern MS_CommunicationClass MS;

#endif

