// 
// 
// 

#include "SDCardHandler.h"

boolean SDCardHandlerClass::init(uint8_t cs_pin)
{
	return SD.begin(cs_pin);
}

boolean SDCardHandlerClass::WriteUserLog(String * filePath, String * log)
{
	File userLogFile = SD.open(*filePath, FILE_WRITE);
	if (!userLogFile)
	{
		//Try one more time
		delay(500); 
		userLogFile = SD.open(*filePath, FILE_WRITE);
	}
	if (userLogFile) {
		userLogFile.println(*log);
		userLogFile.close();
		return true;
	}
	return false;;
}


SDCardHandlerClass SDCardHandler;

