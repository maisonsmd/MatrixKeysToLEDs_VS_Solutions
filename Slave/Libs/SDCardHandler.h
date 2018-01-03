// SDCardHandler.h

#ifndef _SDCARDHANDLER_h
#define _SDCARDHANDLER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <SD.h>

class SDCardHandlerClass
{
 protected:
	String userLogPath;
 public:
	boolean init(uint8_t cs_pin);
	boolean WriteUserLog(String * filePath, String * log);
};

extern SDCardHandlerClass SDCardHandler;

#endif

