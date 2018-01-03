// RFIDTagReader.h

#ifndef _RFIDTAGREADER_h
#define _RFIDTAGREADER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <MFRC522.h>

class RFIDTagReaderClass
{
 protected:
	MFRC522 * rfid = nullptr; // Instance of the class
	MFRC522::MIFARE_Key key;
	byte nuidPICC[4];
	String toString(byte *buffer, byte bufferSize);

 public:
	void init(uint8_t SS_PIN, uint8_t RST_PIN);
	boolean IsNewCardPresent();
	String ReadCardSerial();
};

extern RFIDTagReaderClass RFIDTagReader;

#endif

