// MatrixKeyToLEDs.h



#ifndef _MATRIXKEYTOLEDS_h
#define _MATRIXKEYTOLEDS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <TimerThree.h>
#include <SPI.h>
#include <DirectIO.h>

//For Arduino UNO
/*#define LATCH_PIN 8
#define LOAD_PIN 9
#define BUZZER_PIN 10*/

//For Arduino Mega 2560
#define LATCH_PIN 53
#define LOAD_PIN 49
#define BUZZER_PIN 48

#define SCAN_INTERVAL_US 3000

#define ROWS 8
#define COLUMNS 8

//#define ENABLE_LOG

#ifdef ENABLE_LOG
#define BEGIN(x) Serial.begin(x);
#define LOG(x) Serial.print(x);
#define LOGLN(x) Serial.println(x);
#else
#define BEGIN(x)
#define LOG(x)
#define LOGLN(x)
#endif // ENABLE_LOG
#define obj MatrixKeyToLEDs

//Bit manipulation
#define READBIT(BYTE,BIT) 

#define NO_PIN 255
enum ButtonLogic {
	INACTIVE = 0x00,	//onRelease
	ACTIVE = 0x01		//onClick
};
enum ToggleStyle {
	CLICK_TOGGLE,
	PRESS_AND_HOLD
};
enum ButtonState {
	RELEASED,
	HOLDING
};
enum ReactMoment {
	ON_CLICK,
	ON_RELEASE
};
enum BuzzerType {
	SUPPLY_FREQ,
	SUPPLY_VOLTAGE
};

enum Matrix7219Code {
	OP_NOOP = 0,
	OP_DIGIT0 = 1,
	OP_DIGIT1 = 2,
	OP_DIGIT2 = 3,
	OP_DIGIT3 = 4,
	OP_DIGIT4 = 5,
	OP_DIGIT5 = 6,
	OP_DIGIT6 = 7,
	OP_DIGIT7 = 8,
	OP_DECODEMODE = 9,
	OP_INTENSITY = 10,
	OP_SCANLIMIT = 11,
	OP_SHUTDOWN = 12,
	OP_DISPLAYTEST = 15
};
class MatrixKeyToLEDsClass
{
protected:
	Output<LATCH_PIN> LATCH;
	Output<LOAD_PIN> LOAD;

	uint8_t pShiftInData;
	uint8_t pShiftOutData595[1];
	uint8_t pShiftOutData7219[2];
	uint8_t pStatusByte[ROWS];

	ButtonState pButtonStateArray[ROWS][COLUMNS];
	uint32_t pButtonTimerArray[ROWS][COLUMNS];
	uint8_t pLEDStateArray[ROWS][COLUMNS];

	uint8_t pCurrentButtonColumnIndex = 0;
	uint8_t pCurrentButtonRowIndex = 0;
	uint8_t pDebounceInterval = 50;
	uint16_t pToneFrequency = 1244;
	uint32_t pToneDuration = 30;
	uint8_t pBuzzerActiveLogicLevel = HIGH;

	BuzzerType pBuzzerType = BuzzerType::SUPPLY_VOLTAGE;
	boolean pIsBuzzerOn = false;
	uint32_t pBuzzerTimer = 0;

	ToggleStyle pToggleStyle[ROWS][COLUMNS];
	ReactMoment pReactMoment = ReactMoment::ON_CLICK;

	boolean pLockState = false;
	uint8_t pLockSwitchPin = NO_PIN;
	uint8_t pLockSwitchActiveLogicLevel = LOW;

	uint8_t pLockButtonPin = NO_PIN;
	uint8_t pLockButtonPinActiveLogicLevel = LOW;

	uint8_t pLockStateIndicatorPin = NO_PIN;

	void pReact(uint8_t row, uint8_t column, ButtonState state);

	void pShiftOut(void);
	void pShiftIn(void);
	void pBeep(void);
	void pSetLed(uint8_t row, uint8_t column, uint8_t state);
	void pTransfer7219(uint8_t opcode, uint8_t data);

	boolean pIsLockSwitchActivated(void);
	void pLock(void);
	void pUnlock(void);
	void pReactToLockButton(void);
	void pReactToLockSwitch(void);
	static void pExecute(void);
public:
	void init(void);
	void ClearAll(void);
	void SetAll(void);
	void SetToneStyle(uint16_t freq, uint32_t duration);
	void SetKeyToggleStyle(uint8_t row, uint8_t column, ToggleStyle s);
	void SetKeyToggleStyle(uint8_t index, ToggleStyle s);
	void SetRowToggleStyle(uint8_t row, ToggleStyle s);
	void SetColumnToggleStyle(uint8_t column, ToggleStyle s);
	void SetKeysToggleStyle(ToggleStyle s);

	void SetReactMoment(ReactMoment m);
	void SetDebounceInterval(uint32_t ms);
	void SetBuzzerType(BuzzerType t);
	void SetBuzzerActiveLogicLevel(uint8_t level);

	boolean IsLocked(void);

	void SetLockSwitchPin(uint8_t pin);
	void SetLockSwitchActiveLogicLevel(uint8_t level);

	void SetLockButtonPin(uint8_t pin);
	void SetLockButtonActiveLogicLevel(uint8_t level);

	void SetLockStateIndicatorPin(uint8_t pin);
	
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

