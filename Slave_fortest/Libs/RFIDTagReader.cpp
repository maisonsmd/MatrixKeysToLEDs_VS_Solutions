// 
// 
// 

#include "RFIDTagReader.h"

String RFIDTagReaderClass::toString(byte * buffer, byte bufferSize)
{
	char hex[16] = { '0','1','2' ,'3' ,'4' ,'5' ,'6' ,'7' ,'8' ,'9' ,'A' ,'B' ,'C' ,'D','E','F' };
	String str = "";
	for (byte i = 0; i < bufferSize; i++) {
		str += String(hex[(buffer[i] >> 0x04) & 0x0F]);
		str += String(hex[buffer[i] & 0x0F]);
	}
	return str;
}

void RFIDTagReaderClass::init(uint8_t SS_PIN, uint8_t RST_PIN)
{
	rfid = new MFRC522(SS_PIN, RST_PIN);
	rfid->PCD_Init();
	for (byte i = 0; i < 6; i++) {
		key.keyByte[i] = 0xFF;
	}
}

boolean RFIDTagReaderClass::IsNewCardPresent()
{
	return rfid->PICC_IsNewCardPresent();
}

String RFIDTagReaderClass::ReadCardSerial()
{
	String tagID = "";
	if (!rfid->PICC_ReadCardSerial())
		return "";

	//Serial.print(F("PICC type: "));
	MFRC522::PICC_Type piccType = rfid->PICC_GetType(rfid->uid.sak);
	//Serial.println(rfid->PICC_GetTypeName(piccType));

	// Check is the PICC of Classic MIFARE type
	if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
		piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
		piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
		//Serial.println(F("Your tag is not of type MIFARE Classic."));
		return "";
	}

	if (rfid->uid.uidByte[0] != nuidPICC[0] ||
		rfid->uid.uidByte[1] != nuidPICC[1] ||
		rfid->uid.uidByte[2] != nuidPICC[2] ||
		rfid->uid.uidByte[3] != nuidPICC[3] || 1) {
		//Serial.println(F("A new card has been detected."));

		// Store NUID into nuidPICC array
		for (byte i = 0; i < 4; i++) {
			nuidPICC[i] = rfid->uid.uidByte[i];
		}

		//Serial.println(F("The NUID tag is:"));
		//Serial.print(F("In hex: "));
		tagID = toString(rfid->uid.uidByte, rfid->uid.size);
		//Serial.println(tagID);
		//Serial.println();
	}
	//else
		//Serial.println(F("Card read previously."));

	// Halt PICC
	rfid->PICC_HaltA();

	// Stop encryption on PCD
	rfid->PCD_StopCrypto1();
	return tagID;
}


RFIDTagReaderClass RFIDTagReader;

