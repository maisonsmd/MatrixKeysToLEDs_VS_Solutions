// MatrixKeyToLEDs.h
/*
*	CHANGE LOG:
-	Go back to DirectIO
-	Go back to 2D Arrays instead of READ/WRITE 1D Arrays
*/

#ifndef _MATRIXKEYTOLEDS_h
#define _MATRIXKEYTOLEDS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

//#include <TimerThree.h>
#include <SPI.h>
#include <DirectIO.h>

//For Arduino UNO
/*#define LATCH_PIN 8
#define LOAD_PIN 9
#define BUZZER_PIN 10*/

//For Arduino Mega 2560
#define LATCH_PIN	53
#define LOAD_PIN	49
#define BUZZER_PIN	48

#define MAX_GROUPS	10
#define MAX_GR_MEMBERS	10

#define LATCH_PORT	PORTB
#define LOAD_PORT	PORTL
#define LATCH_BIT	0
#define LOAD_BIT	0

#define SCAN_INTERVAL_US	4000
#define SCAN_INTERVAL_MS	4

#define ROWS		8		//Max value. If use more, change the algorithm bach to 2D array (much slower!!)
#define COLUMNS		8

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
#define obj		MatrixKeyToLEDs

//Bit manipulation
#define READ_BIT(BYTE,BIT)		((BYTE >> BIT) & 0x01)
#define SET_BIT(BYTE,BIT)		BYTE |= 0x01 << BIT
#define CLEAR_BIT(BYTE,BIT)		BYTE &= ~(0x01 << BIT)
#define WRITE_BIT(BYTE,BIT,STATE)	BYTE ^= (-STATE ^ BYTE) & (0x01 << BIT)
#define TOGGLE_BIT(BYTE,BIT)		BYTE ^= 0x01 << BIT

#define NO_PIN		255

//ButtonLogic
#define	INACTIVE	0x00	//onRelease
#define	ACTIVE		0x01	//onClick

//ToggleStyle
#define CLICK_TOGGLE	0x00
#define PRESS_AND_HOLD	0x01

//ButtonState
#define	RELEASED	INACTIVE
#define HOLDING		ACTIVE

//ReactMoment
#define	ON_CLICK	0x01
#define	ON_RELEASE	0x00

//BuzzerType
#define	SUPPLY_FREQ	0x00
#define	SUPPLY_VOLTAGE	0x01

//LockAgent
#define USE_LOCK_SWITCH	0x00
#define USE_RFID_TAG	0x01

//Matrix7219Code
#define	OP_NOOP		0
#define	OP_DIGIT0	1
#define	OP_DIGIT1	2
#define	OP_DIGIT2	3
#define	OP_DIGIT3	4
#define	OP_DIGIT4	5
#define	OP_DIGIT5	6
#define	OP_DIGIT6	7
#define	OP_DIGIT7	8
#define	OP_DECODEMODE	9
#define	OP_INTENSITY	10
#define	OP_SCANLIMIT	11
#define	OP_SHUTDOWN	12
#define	OP_DISPLAYTEST	15

class MatrixKeyToLEDsClass
{
protected:
	Output<LATCH_PIN> LATCH;
	Output<LOAD_PIN> LOAD;

	uint8_t pShiftInData;
	uint8_t pShiftOutData595[1];
	uint8_t pShiftOutData7219[2];
	uint8_t pStatusByte[ROWS];

	uint8_t pButtonStateArray[ROWS][COLUMNS];
	uint32_t pButtonTimerArray[ROWS][COLUMNS];
	uint8_t pLEDStateArray[ROWS][COLUMNS];

	uint8_t pCurrentButtonColumnIndex = 0;
	uint8_t pCurrentButtonRowIndex = 0;
	uint8_t pDebounceInterval = 75;		//Equal to computer keyboard
	uint16_t pToneFrequency = 1244;
	uint32_t pToneDuration = 30;
	uint8_t pBuzzerActiveLogicLevel = HIGH;

	uint8_t pBuzzerType = SUPPLY_VOLTAGE;
	boolean pIsBuzzerOn = false;
	uint32_t pBuzzerTimer = 0;

	uint8_t pToggleStyle[ROWS][COLUMNS];
	uint8_t pReactMoment = ON_CLICK;

	uint8_t pLockSourse = USE_LOCK_SWITCH;
	boolean pLockState = false;
	uint8_t pLockSwitchPin = NO_PIN;
	uint8_t pLockSwitchActiveLogicLevel = LOW;

	uint8_t ppShortLockButtonPin = NO_PIN;
	uint8_t ppShortLockButtonPinActiveLogicLevel = LOW;

	uint8_t pLockStateIndicatorPin = NO_PIN;

	uint8_t pTempOutputPin = NO_PIN;
	uint8_t pTempOutputKeyIndex;
	uint32_t pTempOutputInitTmr = 0;
	uint32_t pTempOutputIntervalTmr = 0;
	uint32_t pTempOutputInitTime = 0;	//wait to ON
	uint32_t pTempOutputInterval = 100;	//wait to OFF
	boolean isWaitingToTurnTempOutputOn = false;

	struct KeyGroup {
		volatile uint8_t size = 0;
		volatile uint8_t member[MAX_GR_MEMBERS];
	} pKeyGroup[MAX_GROUPS];
	uint8_t pKeyGroupsCount = 0;				//Number of groups
	/*uint8_t pKeyGroupMembersCount[MAX_GROUPS];		//Size of each group
	uint8_t pKeyGroup[MAX_GROUPS][MAX_GR_MEMBERS];		//Members*/

	boolean pScanGroup(uint8_t row, uint8_t column, uint8_t state, uint8_t toggleStyle);
	void pReact(uint8_t row, uint8_t column, uint8_t state);
	void pShiftOut(void);
	void pShiftIn(void);
	void pBeep(void);
	void pTransfer7219(uint8_t opcode, uint8_t data);

	uint8_t toIndex(uint8_t row, uint8_t col);
	void toRowCol(uint8_t index, uint8_t * row, uint8_t * col);
	boolean pIsLockSwitchActivated(void);
	void pReactTopShortLockButton(void);
	void pReactToLockSwitch(void);
	void pHandleTempOutput(void);
	void pFlashTempOutput(uint8_t keyIndex);
public:
	static void Execute(void);
	void init(void);
	void SetLEDsIntensity(uint8_t intensity);
	void ClearAll(void);
	void SetAll(void);
	void SetLed(uint8_t row, uint8_t column, uint8_t state);
	void SetLed(uint8_t index, uint8_t state);

	void SetToneStyle(uint16_t freq, uint32_t duration);

	void SetKeyToggleStyle(uint8_t row, uint8_t column, uint8_t toggleStyle);
	void SetKeyToggleStyle(uint8_t index, uint8_t toggleStyle);
	void SetRowToggleStyle(uint8_t row, uint8_t toggleStyle);
	void SetColumnToggleStyle(uint8_t column, uint8_t toggleStyle);
	void SetKeysToggleStyle(uint8_t toggleStyle);
	void SetKeysToggleStyle(uint8_t * key, uint8_t size, uint8_t toggleStyle);

	void SetReactMoment(uint8_t reactMoment);
	void SetDebounceInterval(uint32_t ms);
	void SetBuzzerType(uint8_t buzzerType);
	void SetBuzzerActiveLogicLevel(uint8_t level);

	void AddKeyGroup(uint8_t * btnArray, uint8_t size);

	void Lock(void);
	void Unlock(void);

	boolean IsLocked(void);

	void SetLockSource(uint8_t ls);

	void SetLockSwitchPin(uint8_t pin);
	void SetLockSwitchActiveLogicLevel(uint8_t level);

	void SetShortLockButtonPin(uint8_t pin);
	void SetShortLockButtonActiveLogicLevel(uint8_t level);

	void SetLockStateIndicatorPin(uint8_t pin);
	void SetTempOutput(uint8_t pin, uint8_t keyIndex,uint32_t initTime, uint32_t interval = 100);
	
	void GetButtonsState(uint8_t(*buttonArray)[ROWS][COLUMNS]);
	void GetButtonsState(uint8_t(*buttonArray)[ROWS*COLUMNS]);
	uint8_t GetButtonState(uint8_t buttonNumber);
	uint8_t GetButtonState(uint8_t row, uint8_t column);

	void GetLEDsState(uint8_t(*LEDArray)[ROWS][COLUMNS]);
	void GetLEDsState(uint8_t(*LEDArray)[ROWS*COLUMNS]);
	uint8_t GetLEDState(uint8_t LEDNumber);
	uint8_t GetLEDState(uint8_t row, uint8_t column);
};

extern MatrixKeyToLEDsClass MatrixKeyToLEDs;

#endif

