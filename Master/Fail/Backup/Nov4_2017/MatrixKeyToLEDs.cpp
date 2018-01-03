// Matrix keypad controlling Matrix LED by maisonsmd(c) 2017
// 
// 

#include "MatrixKeyToLEDs.h"

void MatrixKeyToLEDsClass::pReact(uint8_t row, uint8_t column, ButtonState state)
{
	/*
		if (obj.pIsEnabled == false && obj.pKeyLockStyle == KeyLockStyle::UNLOCK)
			return;
		if (obj.pKeyLockPin >= 0 && obj.pKeyLockStyle == KeyLockStyle::LOCK)
			if (digitalRead(obj.pKeyLockPin) == obj.pKeyLockActiveLogic)
				return;
	*/
	if (pLockState == true)
		return;
	if (pToggleStyle[row][column] == ToggleStyle::CLICK_TOGGLE)	//CLICK_TOGGLE
	{
		if ((pReactMoment == ReactMoment::ON_CLICK && state == ButtonState::HOLDING)
			|| (pReactMoment == ReactMoment::ON_RELEASE && state == ButtonState::RELEASED))
		{
			pLEDStateArray[row][column] = !pLEDStateArray[row][column]; //Toggle
#ifdef ENABLE_LOG
			LOGLN("TOGGLED LED (" + String(row) + "," + String(column) + ") -> " + String(pLEDStateArray[row][column]));
#endif
		}
	}
	else						//PRESS_AND_HOLD
	{
		if (state == ButtonState::HOLDING)
		{
			pLEDStateArray[row][column] = 1;	//Turn LED on

#ifdef ENABLE_LOG
			LOGLN("TOGGLED LED (" + String(row) + "," + String(column) + ") -> " + String(pLEDStateArray[row][column]));
#endif
		}
		else
		{
			pLEDStateArray[row][column] = 0;	//Turn LED off
#ifdef ENABLE_LOG
			LOGLN("TOGGLED LED (" + String(row) + "," + String(column) + ") -> " + String(pLEDStateArray[row][column]));
#endif
		}
	}
	pSetLed(row, column, pLEDStateArray[row][column]);

}

void MatrixKeyToLEDsClass::pShiftOut(void)
{
	/*//this section help avoid calling this function when its last invoke is still not completed (SPI transferring)
	static uint32_t lastTick = 0;
	if (lastTick == millis())
		return;		//but this lose data
	lastTick = millis();*/

	LATCH = 0;	//allow shifting out
	LOAD = 0;	//deny shifting in
#pragma region BODY
			/*SPI.transfer(0x00);		//Firstly disable LED rows to avoid unexpected leds to light on
			SPI.transfer(0xFF);		//and disable LED columns too
			SPI.transfer(0x00);		//So does button columns*/
	SPI.transfer(pShiftOutData7219[1]);
	SPI.transfer(pShiftOutData7219[0]);
	SPI.transfer(0x00);	//disable all key column which causing noise when switch to next column
#pragma endregion
	LATCH = 1;	//deny shifting out, refresh outputs

	LATCH = 0;	//allow shifting out
#pragma region BODY
	SPI.transfer(pShiftOutData7219[1]);
	SPI.transfer(pShiftOutData7219[0]);
	SPI.transfer(pShiftOutData595[0]);
#pragma endregion
	LATCH = 1;	//deny shifting out, refresh outputs
}

void MatrixKeyToLEDsClass::pShiftIn(void)
{
	LATCH = 1;	//deny shifting out
	LOAD = 1;	//allow shifting in
#pragma region BODY
	pShiftInData = SPI.transfer(0x00);
	//pShiftInData = 0;
#pragma endregion
	LOAD = 0;	//deny shifting in
}

void MatrixKeyToLEDsClass::pBeep(void)
{
	static uint32_t tmrTone = 0;
	if (tmrTone > millis())
		tmrTone = millis();

	if (millis() - tmrTone < pToneDuration + 10)		//Wait the tone to finish
		return;
	tmrTone = millis();
	if (pBuzzerType == BuzzerType::SUPPLY_FREQ)
	{
		tone(BUZZER_PIN, pToneFrequency, pToneDuration);
	}
	else {
		pIsBuzzerOn = true;
		digitalWrite(BUZZER_PIN, pBuzzerActiveLogicLevel);
		pBuzzerTimer = millis();
	}
}

void MatrixKeyToLEDsClass::pSetLed(uint8_t row, uint8_t column, uint8_t state)
{
	uint8_t val = 0x00;
	val = B10000000 >> column;
	if (state)
		pStatusByte[row] = pStatusByte[row] | val;
	else {
		val = ~val;
		pStatusByte[row] = pStatusByte[row] & val;
	}
	pTransfer7219(row + 1, pStatusByte[row]);
}

void MatrixKeyToLEDsClass::pTransfer7219(uint8_t opcode, uint8_t data)
{
	memset(pShiftOutData7219, 0, sizeof(pShiftOutData7219));
	pShiftOutData7219[1] = opcode;
	pShiftOutData7219[0] = data;
	pShiftOut();
}

boolean MatrixKeyToLEDsClass::pIsLockSwitchActivated(void)
{
	if (pLockSwitchPin == NO_PIN)
		return false;
	return digitalRead(pLockSwitchPin) == pLockSwitchActiveLogicLevel;
}

void MatrixKeyToLEDsClass::pLock(void)
{
	pLockState = true;
	if (pLockStateIndicatorPin != NO_PIN)
		digitalWrite(pLockStateIndicatorPin, HIGH);
	LOGLN("LOCKED");
}

void MatrixKeyToLEDsClass::pUnlock(void)
{
	Timer3.stop();
	pLockState = false;
	if (pLockStateIndicatorPin != NO_PIN)
		digitalWrite(pLockStateIndicatorPin, LOW);
	LOGLN("UNLOCKED");
	//We clear all led after unlock
	/*if (pToggleStyle == ToggleStyle::PRESS_AND_HOLD)
	{
		ClearAll();
		for (uint8_t row = 0; row < ROWS; row++)
			for (uint8_t col = 0; col < COLUMNS; col++)
				if (pButtonStateArray[row][col] == 1)
					pSetLed(row, col, 1);
	}*/
	/*for (uint8_t row = 0; row < ROWS; row++)
		for (uint8_t col = 0; col < COLUMNS; col++)
			if (pToggleStyle[row][col] == ToggleStyle::PRESS_AND_HOLD)
			{
				if (pButtonStateArray[row][col] == 1)
					pSetLed(row, col, 1);
			}*/
	Timer3.start();
}

void MatrixKeyToLEDsClass::pReactToLockButton(void)
{
	if (pLockButtonPin == NO_PIN)
		return;
	static ButtonState buttonState = ButtonState::RELEASED;
	static uint32_t clickTmr = 0;
	uint32_t currentTick = millis();
	if (digitalRead(pLockButtonPin) == pLockButtonPinActiveLogicLevel)
	{
		clickTmr = currentTick;
		if (buttonState == ButtonState::RELEASED)
		{
			buttonState = ButtonState::HOLDING;
			LOGLN("LOCK BUTTON CLICKED");
			pBeep();
			if (pReactMoment == ReactMoment::ON_CLICK)
				if (pIsLockSwitchActivated() == false)
				{
					if (pLockState == true)
						pUnlock();
					else
						pLock();
				}
				else
				{
					LOGLN("LOCK SWITCH ACTIVATED, CANNOT CHANGE LOCK STATE");
				}
		}
	}
	if (clickTmr > currentTick)
		clickTmr = currentTick;

	if (currentTick - clickTmr > pDebounceInterval)
	{
		if (buttonState == ButtonState::HOLDING)
		{
			buttonState = ButtonState::RELEASED;
			LOGLN("LOCK BUTTON RELEASED");
			if (pReactMoment == ReactMoment::ON_RELEASE)
				if (pIsLockSwitchActivated() == false)
				{
					if (pLockState == true)
						pUnlock();
					else
						pLock();
				}
				else
				{
					LOGLN("LOCK SWITCH ACTIVATED, CANNOT CHANGE LOCK STATE");
				}
		}
	}
}

void MatrixKeyToLEDsClass::pReactToLockSwitch(void)
{

	static boolean lastSwitchState = !pIsLockSwitchActivated();
	if (pIsLockSwitchActivated() != lastSwitchState)
	{
		//Changed state
		pBeep();
		lastSwitchState = pIsLockSwitchActivated();
		if (pIsLockSwitchActivated() == true)	//LOCK COMMAND
		{
			LOGLN("LOCK SWITCH ACTIVATED");
			if (pLockState == false)
				pLock();
		}
		else					//UNLOCK COMMAND
		{
			LOGLN("LOCK SWITCH DEACTIVATED");
			if (pLockState == true)
				pUnlock();
		}
	}
}

void MatrixKeyToLEDsClass::init(void)
{
	BEGIN(115200);
	pinMode(BUZZER_PIN, OUTPUT);
	SPI.begin();
	SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));//1400000

	for (pCurrentButtonRowIndex = 0; pCurrentButtonRowIndex < ROWS; pCurrentButtonRowIndex++)
		for (pCurrentButtonColumnIndex = 0; pCurrentButtonColumnIndex < COLUMNS; pCurrentButtonColumnIndex++)
			pButtonStateArray[pCurrentButtonRowIndex][pCurrentButtonColumnIndex] = ButtonState::RELEASED;

#ifdef ENABLE_LOG
	LOGLN("START CODE");
#endif
	pTransfer7219(OP_DISPLAYTEST, 0);
	pTransfer7219(OP_INTENSITY, 15);
	pTransfer7219(OP_SCANLIMIT, 7);
	pTransfer7219(OP_DECODEMODE, 0);
	pTransfer7219(OP_SHUTDOWN, 1);
	ClearAll();
	Timer3.initialize(SCAN_INTERVAL_US);
	Timer3.attachInterrupt(pExecute);
}

//CHECK THIS 2 FUNCTIONS
void MatrixKeyToLEDsClass::ClearAll(void)
{
	Timer3.stop();	//this job take much time, so we disable timer3 to avoid overloading the MCU & stack
	for (uint8_t row = 0; row < ROWS; row++)
	{
		pStatusByte[row] = 0x00;
		pTransfer7219(row + 1, pStatusByte[row]);
	}
	Timer3.start();
}

void MatrixKeyToLEDsClass::SetAll(void)
{
	Timer3.stop();	//this job take much time, so we disable timer3 to avoid overloading the MCU & stack
	for (uint8_t row = 0; row < ROWS; row++)
	{
		pStatusByte[row] = 0xFF;
		pTransfer7219(row + 1, pStatusByte[row]);
	}
	Timer3.start();
}

boolean MatrixKeyToLEDsClass::IsLocked(void)
{
	return pLockState;
}

void MatrixKeyToLEDsClass::pExecute(void)
{
	//LOGLN(millis());
	uint32_t currentTick = millis();
	//uint32_t tmr = micros();

	if (obj.pBuzzerType == BuzzerType::SUPPLY_VOLTAGE && currentTick - obj.pBuzzerTimer > obj.pToneDuration && obj.pIsBuzzerOn == true)
	{
		digitalWrite(BUZZER_PIN, !obj.pBuzzerActiveLogicLevel);
		obj.pIsBuzzerOn = false;
	}

	obj.pShiftOutData595[0] = 0x00;	//Disable all buttons columns

	obj.pShiftOutData595[0] |= 0x01 << (obj.pCurrentButtonColumnIndex);		//Setting this Button column bit means this column is ready for reading 

	obj.pShiftOut();	//push data to 595s & 7219
	obj.pShiftIn();		//read data from 165

	for (obj.pCurrentButtonRowIndex = 0; obj.pCurrentButtonRowIndex < ROWS; obj.pCurrentButtonRowIndex++)
	{
		if ((obj.pShiftInData >> (obj.pCurrentButtonRowIndex)) & 0x01 == ButtonLogic::ACTIVE)	//check each bit
		{
			if (obj.pButtonStateArray[obj.pCurrentButtonRowIndex][obj.pCurrentButtonColumnIndex] == ButtonState::RELEASED)
			{
#ifdef ENABLE_LOG
				LOGLN("CLICKED  (" + String(obj.pCurrentButtonRowIndex) + "," + String(obj.pCurrentButtonColumnIndex) + ")");
#endif
				obj.pBeep();
				obj.pReact(obj.pCurrentButtonRowIndex, obj.pCurrentButtonColumnIndex, ButtonState::HOLDING);
				obj.pButtonStateArray[obj.pCurrentButtonRowIndex][obj.pCurrentButtonColumnIndex] = ButtonState::HOLDING;
			}
			obj.pButtonTimerArray[obj.pCurrentButtonRowIndex][obj.pCurrentButtonColumnIndex] = currentTick;
		}
	}

	for (uint8_t row = 0; row < ROWS; row++)
		for (uint8_t column = 0; column < COLUMNS; column++)
		{
			if (obj.pButtonTimerArray[row][column] > currentTick)
				obj.pButtonTimerArray[row][column] = currentTick;	//reset the timer

			if (currentTick - obj.pButtonTimerArray[row][column] > obj.pDebounceInterval)
			{
				if (obj.pButtonStateArray[row][column] == ButtonState::HOLDING)
				{
#ifdef ENABLE_LOG
					LOGLN("RELEASED (" + String(row) + "," + String(column) + ")");
#endif
					obj.pReact(row, column, ButtonState::RELEASED);
					obj.pButtonStateArray[row][column] = ButtonState::RELEASED;
				}
			}
		}

	//Complete 1 revolution of scanning?
	obj.pCurrentButtonColumnIndex++;
	//obj.pCurrentLEDColumnIndex++;
	if (obj.pCurrentButtonColumnIndex >= COLUMNS) obj.pCurrentButtonColumnIndex = 0;
	//if (obj.pCurrentLEDColumnIndex >= COLUMNS) obj.pCurrentLEDColumnIndex = 0;

	obj.pReactToLockButton();
	obj.pReactToLockSwitch();
	/*tmr = micros() - tmr;
	Serial.println(tmr);
	delay(100);*/
}

void MatrixKeyToLEDsClass::SetToneStyle(uint16_t freq, uint32_t duration)
{
	pToneFrequency = freq;
	pToneDuration = duration;
}

void MatrixKeyToLEDsClass::SetKeyToggleStyle(uint8_t row, uint8_t column, ToggleStyle s)
{
	ClearAll();
	pToggleStyle[row][column] = s;
}

void MatrixKeyToLEDsClass::SetReactMoment(ReactMoment m)
{
	pReactMoment = m;
}

void MatrixKeyToLEDsClass::SetDebounceInterval(uint32_t ms)
{
	pDebounceInterval = ms;
}

void MatrixKeyToLEDsClass::SetBuzzerType(BuzzerType t)
{
	pBuzzerType = t;
}

void MatrixKeyToLEDsClass::SetBuzzerActiveLogicLevel(uint8_t level)
{
	pBuzzerActiveLogicLevel = level;
}

void MatrixKeyToLEDsClass::SetLockSwitchPin(uint8_t pin)
{
	pLockSwitchPin = pin;
	pinMode(pLockSwitchPin, INPUT);
}

void MatrixKeyToLEDsClass::SetLockSwitchActiveLogicLevel(uint8_t level)
{
	pLockSwitchActiveLogicLevel = level;
}

void MatrixKeyToLEDsClass::SetLockButtonPin(uint8_t pin)
{
	pLockButtonPin = pin;
	pinMode(pLockButtonPin, INPUT);
}

void MatrixKeyToLEDsClass::SetLockButtonActiveLogicLevel(uint8_t level)
{
	pLockButtonPinActiveLogicLevel = level;
}

void MatrixKeyToLEDsClass::SetLockStateIndicatorPin(uint8_t pin)
{
	pLockStateIndicatorPin = pin;
	pinMode(pLockStateIndicatorPin, OUTPUT);
	digitalWrite(pLockStateIndicatorPin, LOW);
}

void MatrixKeyToLEDsClass::GetButtonsState(uint8_t(*buttonArray)[ROWS][COLUMNS])
{
	for (uint8_t row = 0; row < ROWS; row++)
		for (uint8_t col = 0; col < COLUMNS; col++)
			(*buttonArray)[row][col] = GetButtonState(row, col);
}

void MatrixKeyToLEDsClass::GetButtonsState(uint8_t(*buttonArray)[ROWS * COLUMNS])
{
	for (uint8_t index = 0; index < ROWS * COLUMNS; index)
		(*buttonArray)[index] = GetButtonState(index);
}

uint8_t MatrixKeyToLEDsClass::GetButtonState(uint8_t buttonNumber)
{
	uint8_t row = buttonNumber / COLUMNS;
	uint8_t column = buttonNumber - row*COLUMNS;
	return GetButtonState(row, column);
}

uint8_t MatrixKeyToLEDsClass::GetButtonState(uint8_t row, uint8_t column)
{
	return pButtonStateArray[row][column] == ButtonState::HOLDING ? 1 : 0;
}

void MatrixKeyToLEDsClass::GetLEDsState(uint8_t(*LEDArray)[ROWS][COLUMNS])
{
	for (uint8_t row = 0; row < ROWS; row++)
		for (uint8_t col = 0; col < COLUMNS; col++)
			(*LEDArray)[row][col] = GetLEDState(row, col);
}

void MatrixKeyToLEDsClass::GetLEDsState(uint8_t(*LEDArray)[ROWS * COLUMNS])
{
	for (uint8_t index = 0; index < ROWS * COLUMNS; index)
		(*LEDArray)[index] = GetLEDState(index);
}

uint8_t MatrixKeyToLEDsClass::GetLEDState(uint8_t LEDNumber)
{
	uint8_t row = LEDNumber / COLUMNS;
	uint8_t column = LEDNumber - row*COLUMNS;
	return GetLEDState(row, column);
}

uint8_t MatrixKeyToLEDsClass::GetLEDState(uint8_t row, uint8_t column)
{
	return pLEDStateArray[row][column];
}


MatrixKeyToLEDsClass MatrixKeyToLEDs;

