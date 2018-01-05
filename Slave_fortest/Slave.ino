/*
TO DO:
-	Move function ScanUsers() to class SDCardHandler
-	Maybe the SD card lose function when wait for a long period of time
*/

#include <SPI.h>
#include "Libs/SDCardHandler.h"
#include "Libs/RFIDTagReader.h"
#include "Libs/UserManager.h"
#include "Libs/SM_Communication.h"

#define RFID_SS_PIN 49
#define RFID_RST_PIN 48
#define SD_SS_PIN 47

#define ERROR_INDICATOR	13

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

#define NO_USER 255
uint8_t loggedinUserID = NO_USER;

String LOG_PATH = "log.txt";
String USERS_PATH = "users.txt";

void setup()
{
	BEGIN(115200);
	LOGLN("HELLO");
	pinMode(ERROR_INDICATOR, OUTPUT);

	SPI.begin();
	RFIDTagReader.init(RFID_SS_PIN, RFID_RST_PIN);
	SM.init();
	UserManager.init();
	if (!SDCardHandler.init(SD_SS_PIN))
	{
		LOGLNF("Init SD card OK!");
	}
	else {
		LOGLNF("Init SD card failed!");
		LOGLNF("Code can't run without SD card!!! Stuck here!");

		digitalWrite(ERROR_INDICATOR, HIGH);
		while (1);
	}
	if (!ScanUsers())
	{
		digitalWrite(ERROR_INDICATOR, HIGH);
		while (1);
	}

	UserManager.AddUser("Nguyen Van A", "3695EBA0");
	UserManager.AddUser("Pham Van B", "86B777A1");
	SM.LockMaster();
}

void loop()
{
	SM.Execute();

	if (RFIDTagReader.IsNewCardPresent())
	{
		SM.Beep();

		String tagKey = RFIDTagReader.ReadCardSerial();
		//LOGLN("Scanned: " + tagKey);
		boolean detected = false;
		if (tagKey.length() == 8)
		{
			for (uint8_t i = 0; i < UserManager.GetUsersCount(); i++)
			{
				if (UserManager.GetUser(i)->tagID.equals(tagKey))
				{
					detected = true;
					LOG("<" + UserManager.GetUser(i)->name + ">");
					LOGLNF(" was detected!");
					ProcessLoginStatus(UserManager.GetUser(i)->ID);
					break;
				}
			}
			if (!detected)
			{
				LOG(tagKey);
				LOGLNF(" is not a valid user!");
			}
		}
		else
		{
			LOGLNF("your tag is not supported!");
			//LOGLN(tagKey);
		}
	}
}

void ProcessLoginStatus(uint8_t userID)
{
	String logStr = "";
	if (UserManager.GetUser(userID)->loginStatus == LOGGED_IN)
	{
		//This user already logged in
		if (SM._lockState == LockState::LOCKED)	//not "short lock"
		{
			LOGLNF("LOGGED IN BEFORE, LOCKED, NOW UNLOCKING...");
			SM.UnlockMaster();
		}
		else
		{
			UserManager.GetUser(userID)->loginStatus = LOGGED_OUT;
			logStr = "[" + SM.dt.toString() + "] <" + UserManager.GetUser(userID)->name + "> LOGGED OUT\n";
			LOGLN(logStr);
			if (!SDCardHandler.WriteUserLog(&LOG_PATH, &logStr))
				LOGLNF("LOG TO SD CARD FAILED");
			loggedinUserID = NO_USER;
			//user logged out, lock the key
			LOGLNF("LOCKING MASTER...");
			SM.LockMaster();
		}
	}
	else if (loggedinUserID != NO_USER)
	{
		//diferent user is logging in
		switch (SM._lockState)
		{
		case LockState::UNLOCKED:
			//do nothing as customer's requirement
			LOGLNF("ANOTHER USER LOGGED IN!");
			break;
		case LockState::SHORT_LOCKED:
			//do nothing as customer's requirement
			LOGLNF("ANOTHER USER LOGGED IN!");
			break;
		case LockState::LOCKED:
			//Kick the current user and then login!
			logStr = "[" + SM.dt.toString() + "] <" + UserManager.GetUser(userID)->name
				+ "> KICKED <" + UserManager.GetUser(loggedinUserID)->name
				+ "> AND LOGGED IN\n";
			LOGLN(logStr);
			if (!SDCardHandler.WriteUserLog(&LOG_PATH, &logStr))
				LOGLNF("LOG TO SD CARD FAILED");
			UserManager.GetUser(userID)->loginStatus = LOGGED_IN;
			UserManager.GetUser(loggedinUserID)->loginStatus = LOGGED_OUT;
			loggedinUserID = userID;
			//New user logged in, unlock to use immediately
			SM.UnlockMaster();
			break;
		}
	}
	else
	{
		//no one has logged in yet
		logStr = "[" + SM.dt.toString() + "] <" + UserManager.GetUser(userID)->name
			+ "> LOGGED IN\n";
		LOGLN(logStr);
		if (!SDCardHandler.WriteUserLog(&LOG_PATH, &logStr))
			LOGLNF("LOG TO SD CARD FAILED");
		UserManager.GetUser(userID)->loginStatus = LOGGED_IN;
		loggedinUserID = userID;
		//New user logged in, unlock to use immediately
		SM.UnlockMaster();
	}

	if (loggedinUserID == NO_USER)
		SM._loginState = LoginState::LOGGED_OUT;
	else
		SM._loginState = LoginState::LOGGED_IN;
}

boolean ScanUsers()
{
	return true;
	File dataFile = SD.open(USERS_PATH);
	const uint16_t MAX_READ_LENGTH = 1000;

	LOGLNF("SCANNING USERS: ");
	if (!dataFile)
	{
		LOGLNF("CAN'T OPEN FILE, MAY BE IT DOESN'T EXITS OR CORRUPTED!");
		return false;
	}

	uint16_t charIdx = 0;
	String readData = "";
	while ((dataFile.available()) && (charIdx++ < MAX_READ_LENGTH))
	{
		char currentChar = char(dataFile.read());
		readData += currentChar;

		if (currentChar == ';')	//end of 1 user field
		{
			//LOGLN("READ: \"" + readData + "\"");
			String username = readData.substring(readData.indexOf("<") + 1, readData.indexOf(">"));
			readData = readData.substring(readData.indexOf(","), readData.lastIndexOf(">"));
			String tagID = readData.substring(readData.indexOf("<") + 1, readData.indexOf(">"));

			LOGLN("GOT USER: \"" + username + "\" , \"" + tagID + "\"");

			UserManager.AddUser(username, tagID);

			readData = "";
		}
	}
	LOGLNF("<END OF FILE>");
	dataFile.close();

	return true;
}