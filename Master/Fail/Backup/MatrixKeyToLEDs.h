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

#define SCAN_INTERVAL_US 2500

#define ROWS 8
#define COLUMNS 8

#define ENABLE_LOG

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
enum BuzzerType{
	SUPPLY_FREQ,
	SUPPLY_VOLTAGE
};

class MatrixKeyToLEDsClass
{
protected:
	Output<LATCH_PIN> LATCH;
	Output<LOAD_PIN> LOAD;

	boolean pIsEnabled = true;
	uint8_t pShiftInData;
	uint8_t pShiftOutData[3];

	ButtonState pButtonStateArray[ROWS][COLUMNS];
	uint32_t pButtonTimerArray[ROWS][COLUMNS];
	uint8_t pLEDStateArray[ROWS][COLUMNS];

	uint8_t pCurrentButtonColumnIndex = 0;
	uint8_t pCurrentButtonRowIndex = 0;
	uint8_t pCurrentLEDColumnIndex = 0;
	uint8_t pCurrentLEDRowIndex = 0;
	uint8_t pDebounceInterval = 50;
	uint16_t pToneFrequency = 1244;
	uint32_t pToneDuration = 30;

	BuzzerType pBuzzerType = BuzzerType::SUPPLY_VOLTAGE;
	boolean pIsBuzzerOn = false;
	uint32_t pBuzzerTimer = 0;

	ToggleStyle pToggleStyle = ToggleStyle::CLICK_TOGGLE;
	ReactMoment pReactMoment = ReactMoment::ON_CLICK;

	void pReact(uint8_t row, uint8_t column, ButtonState state);
	void pShiftOut(void);
	void pShiftIn(void);
	void pBeep(void);
public:
	void init(void);
	void Enable(void);
	void Disable(void);
	void ClearAll(void);
	void SetAll(void);
	static void Execute(void);
	void SetToneStyle(uint16_t freq, uint32_t duration);
	void SetToggleStyle(ToggleStyle s);
	void SetReactMoment(ReactMoment m);
	void SetDebounceInterval(uint32_t ms);
	void SetBuzzerType(BuzzerType t);

	void GetButtonsState(uint8_t (*buttonArray)[ROWS][COLUMNS]);
	void GetButtonsState(uint8_t (*buttonArray)[ROWS*COLUMNS]);
	uint8_t GetButtonState(uint8_t buttonNumber);
	uint8_t GetButtonState(uint8_t row, uint8_t column);

	void GetLEDsState(uint8_t(*LEDArray)[ROWS][COLUMNS]);
	void GetLEDsState(uint8_t(*LEDArray)[ROWS*COLUMNS]);
	uint8_t GetLEDState(uint8_t LEDNumber);
	uint8_t GetLEDState(uint8_t row, uint8_t column);
};

extern MatrixKeyToLEDsClass MatrixKeyToLEDs;

#endif

