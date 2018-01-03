// Matrix keypad controlling Matrix LED by maisonsmd(c) 2017
// 
// 

#include "MatrixKeyToLEDs.h"

void MatrixKeyToLEDsClass::pReact(uint8_t row, uint8_t column, ButtonState state)
{
	if (pToggleStyle == ToggleStyle::CLICK_TOGGLE)	//CLICK_TOGGLE
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

}

void MatrixKeyToLEDsClass::pShiftOut(void)
{
	LATCH = 0;	//allow shifting out
	LOAD = 0;	//deny shifting in
#pragma region BODY
	SPI.transfer(0x00);		//Firstly disable LED rows to avoid unexpected leds to light on
	SPI.transfer(0xFF);		//and disable LED columns too
	SPI.transfer(0x00);		//So does button columns
#pragma endregion
	LATCH = 1;	//deny shifting out, refresh outputs

	LATCH = 0;	//allow shifting out
#pragma region BODY
	SPI.transfer(pShiftOutData[0]);
	SPI.transfer(pShiftOutData[1]);
	SPI.transfer(pShiftOutData[2]);
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
	if (millis() - tmrTone < pToneDuration + 10)		//Wait the tone to finish
		return;
	tmrTone = millis();
	if (pBuzzerType == BuzzerType::SUPPLY_FREQ)
	{
		tone(BUZZER_PIN, pToneFrequency, pToneDuration);
	}
	else {
		pIsBuzzerOn = true;
		digitalWrite(BUZZER_PIN, HIGH);
		pBuzzerTimer = millis();
	}
}

void MatrixKeyToLEDsClass::init(void)
{
	BEGIN(115200);
	pinMode(BUZZER_PIN, OUTPUT);
	SPI.begin();
	SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));//1400000

	for (pCurrentButtonRowIndex = 0; pCurrentButtonRowIndex < ROWS; pCurrentButtonRowIndex++)
		for (pCurrentButtonColumnIndex = 0; pCurrentButtonColumnIndex < COLUMNS; pCurrentButtonColumnIndex++)
			pButtonStateArray[pCurrentButtonRowIndex][pCurrentButtonColumnIndex] = ButtonState::RELEASED;

#ifdef ENABLE_LOG
	LOGLN("START CODE");
#endif
	Timer3.initialize(SCAN_INTERVAL_US);
	Timer3.attachInterrupt(Execute);
}

void MatrixKeyToLEDsClass::Enable(void)
{
	pIsEnabled = true;
}

void MatrixKeyToLEDsClass::Disable(void)
{
	pIsEnabled = false;
}

void MatrixKeyToLEDsClass::ClearAll(void)
{
	for (uint8_t row = 0; row < ROWS; row++)
		for (uint8_t col = 0; col < COLUMNS; col++)
			pLEDStateArray[row][col] = 0;
}

void MatrixKeyToLEDsClass::SetAll(void)
{
	for (uint8_t row = 0; row < ROWS; row++)
		for (uint8_t col = 0; col < COLUMNS; col++)
			pLEDStateArray[row][col] = 1;
}

void MatrixKeyToLEDsClass::Execute(void)
{
	if (obj.pIsEnabled == false) {
		return;
	}
	//LOGLN(millis());
	uint32_t currentTick = millis();
	//uint32_t tmr = micros();

	if (obj.pBuzzerType == BuzzerType::SUPPLY_VOLTAGE && currentTick - obj.pBuzzerTimer > obj.pToneDuration && obj.pIsBuzzerOn == true)
	{
		digitalWrite(BUZZER_PIN, LOW);
		obj.pIsBuzzerOn = false;
	}

	obj.pShiftOutData[0] = 0x00;	//Turn off all LED columns
	obj.pShiftOutData[1] = 0xFF;	//Turn off all LED rows
	obj.pShiftOutData[2] = 0x00;	//Disable all buttons columns

	obj.pShiftOutData[0] |= 0x01 << (obj.pCurrentLEDColumnIndex);		// Setting this LED column bit means this column will light

	for (obj.pCurrentLEDRowIndex = 0; obj.pCurrentLEDRowIndex < ROWS; obj.pCurrentLEDRowIndex++)	//set 1st byte (LED_ROW)
		if (obj.pLEDStateArray[obj.pCurrentLEDRowIndex][obj.pCurrentLEDColumnIndex] == 0x01)
			obj.pShiftOutData[1] &= ~(0x01 << (obj.pCurrentLEDRowIndex));	// Clearing this LED row bit means this row will light (inverted by ULN2803)

	obj.pShiftOutData[2] |= 0x01 << (obj.pCurrentButtonColumnIndex);		//Setting this Button column bit means this column is ready for reading 

	obj.pShiftOut();	//push data to 595s
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
	obj.pCurrentLEDColumnIndex++;
	if (obj.pCurrentButtonColumnIndex >= COLUMNS) obj.pCurrentButtonColumnIndex = 0;
	if (obj.pCurrentLEDColumnIndex >= COLUMNS) obj.pCurrentLEDColumnIndex = 0;
	/*tmr = micros() - tmr;
	Serial.println(tmr);
	delay(100);*/
}

void MatrixKeyToLEDsClass::SetToneStyle(uint16_t freq, uint32_t duration)
{
	pToneFrequency = freq;
	pToneDuration = duration;
}

void MatrixKeyToLEDsClass::SetToggleStyle(ToggleStyle s)
{
	ClearAll();
	pToggleStyle = s;
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

void MatrixKeyToLEDsClass::GetButtonsState(uint8_t(*buttonArray)[ROWS][COLUMNS])
{
	for (uint8_t row = 0; row < ROWS; row++)
		for (uint8_t col = 0; col < COLUMNS; col++)
			(*buttonArray)[row][col] = pButtonStateArray[row][col];
}

void MatrixKeyToLEDsClass::GetButtonsState(uint8_t(*buttonArray)[ROWS * COLUMNS])
{
	for (uint8_t row = 0; row < ROWS; row++)
		for (uint8_t col = 0; col < COLUMNS; col++)
			(*buttonArray)[row * COLUMNS + col] = pButtonStateArray[row][col];
}

uint8_t MatrixKeyToLEDsClass::GetButtonState(uint8_t buttonNumber)
{
	uint8_t row = buttonNumber / COLUMNS;
	uint8_t column = buttonNumber - row*COLUMNS;
	return GetButtonState(row, column);
}

uint8_t MatrixKeyToLEDsClass::GetButtonState(uint8_t row, uint8_t column)
{
	return pButtonStateArray[row][column];
}

void MatrixKeyToLEDsClass::GetLEDsState(uint8_t(*LEDArray)[ROWS][COLUMNS])
{
	for (uint8_t row = 0; row < ROWS; row++)
		for (uint8_t col = 0; col < COLUMNS; col++)
			(*LEDArray)[row][col] = pLEDStateArray[row][col];
}

void MatrixKeyToLEDsClass::GetLEDsState(uint8_t(*LEDArray)[ROWS * COLUMNS])
{
	for (uint8_t row = 0; row < ROWS; row++)
		for (uint8_t col = 0; col < COLUMNS; col++)
			(*LEDArray)[row * COLUMNS + col] = pLEDStateArray[row][col];
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

