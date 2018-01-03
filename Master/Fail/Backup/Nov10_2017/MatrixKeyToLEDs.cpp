// Matrix keypad controlling Matrix LED by maisonsmd(c) 2017
// 
// 

#include "MatrixKeyToLEDs.h"

boolean MatrixKeyToLEDsClass::pScanGroup(uint8_t row, uint8_t column, uint8_t state, uint8_t toggleStyle)
{
	//LOGLN("CALLING " + String(row) + ":" + String(column) + ":" + String(state) + ":" + String(toggleStyle));
#pragma region CHECK_GROUP
	//this section is for KeyGroup
	for (uint8_t groupIndex = 0; groupIndex < pKeyGroupCount; groupIndex++)
		for (uint8_t memberIndex = 0; memberIndex < pKeyGroup[groupIndex].size; memberIndex++)
		{
			uint8_t currentBtnIndex = toIndex(row, column);
			LOGLN("PRESSING: " + String(currentBtnIndex) + "\t" + String(groupIndex) + "," + String(memberIndex) + " = " + String(pKeyGroup[groupIndex].member[memberIndex]));
			//if clicked key belongs to any group
			if (currentBtnIndex == pKeyGroup[groupIndex].member[memberIndex])
			{
				LOGLN("(" + String(row) + "," + String(column) + ") <=> " + String(currentBtnIndex) + " BELONGS TO GROUP " + String(groupIndex));
				if (toggleStyle == CLICK_TOGGLE)
				{
					//Toggle this LED
					TOGGLE_BIT(pLEDStateArray[column], row);
				}
				else
				{
					if (state == HOLDING)
						SET_BIT(pLEDStateArray[column], row);
					else	//RELEASED
						CLEAR_BIT(pLEDStateArray[column], row);
				}
				pSetLed(row, column, READ_BIT(pLEDStateArray[column], row));

				LOGLN("TOGGLED LED (" + String(row) + "," + String(column) + ") -> " + String(READ_BIT(pLEDStateArray[column], row)));
				//if just lighted on this LED
				if (READ_BIT(pLEDStateArray[column], row) == 1)
					//then turn off other LEDs
					for (uint8_t memberIndexAgain = 0; memberIndexAgain < pKeyGroup[groupIndex].size; memberIndexAgain++)
						if (pKeyGroup[groupIndex].member[memberIndexAgain] != currentBtnIndex)
						{
							uint8_t r, c;	//current row, col
							toRowCol(pKeyGroup[groupIndex].member[memberIndexAgain], &r, &c);
							CLEAR_BIT(pLEDStateArray[c], r);
							pSetLed(r, c, READ_BIT(pLEDStateArray[c], r));
						}
				//Done, exit function
				return true;
			}
		}
	//end of KeyGroup section

#pragma endregion
	return false;
}

void MatrixKeyToLEDsClass::pReact(uint8_t row, uint8_t column, uint8_t state)
{
	if (pLockState == true)
		return;

	//Timer3.stop();
	if (READ_BIT(pToggleStyle[column], row) == CLICK_TOGGLE)	//CLICK_TOGGLE
	{
		if ((pReactMoment == ON_CLICK && state == HOLDING)
			|| (pReactMoment == ON_RELEASE && state == RELEASED))
		{
			//if this key belogs to any group. exit;
			if (pScanGroup(row, column, state, CLICK_TOGGLE))
				return;

			TOGGLE_BIT(pLEDStateArray[column], row);//Toggle
			LOGLN("TOGGLED LED (" + String(row) + "," + String(column) + ") -> " + String(READ_BIT(pLEDStateArray[column], row)));
		}
	}
	else						//PRESS_AND_HOLD
	{
		//if this key belogs to any group. exit;
		if (pScanGroup(row, column, state, PRESS_AND_HOLD))
			return;

		if (state == HOLDING)
		{
			SET_BIT(pLEDStateArray[column], row);	//Turn LED on
			LOGLN("PRESSING LED (" + String(row) + "," + String(column) + ") -> " + String(READ_BIT(pLEDStateArray[column], row)));
		}
		else
		{
			CLEAR_BIT(pLEDStateArray[column], row);	//Turn LED off
			LOGLN("RELEASED LED (" + String(row) + "," + String(column) + ") -> " + String(READ_BIT(pLEDStateArray[column], row)));
		}
	}
	pSetLed(row, column, READ_BIT(pLEDStateArray[column], row));
	//Timer3.start();
}

void MatrixKeyToLEDsClass::pShiftOut(void)
{
	CLEAR_BIT(LATCH_PORT, LATCH_BIT);	//allow shifting out
	CLEAR_BIT(LOAD_PORT, LOAD_BIT);	//deny shifting in
#pragma region BODY
			/*SPI.transfer(0x00);		//Firstly disable LED rows to avoid unexpected leds to light on
			SPI.transfer(0xFF);		//and disable LED columns too
			SPI.transfer(0x00);		//So does button columns*/
	SPI.transfer(pShiftOutData7219[1]);
	SPI.transfer(pShiftOutData7219[0]);
	SPI.transfer(0x00);	//disable all key column which causing noise when switch to next column
#pragma endregion
	SET_BIT(LATCH_PORT, LATCH_BIT);	//deny shifting out, refresh outputs

	CLEAR_BIT(LATCH_PORT, LATCH_BIT);	//allow shifting out
#pragma region BODY
	SPI.transfer(pShiftOutData7219[1]);
	SPI.transfer(pShiftOutData7219[0]);
	SPI.transfer(pShiftOutData595[0]);
#pragma endregion
	SET_BIT(LATCH_PORT, LATCH_BIT);	//deny shifting out, refresh outputs
}

void MatrixKeyToLEDsClass::pShiftIn(void)
{
	SET_BIT(LATCH_PORT, LATCH_BIT);	//deny shifting out
	SET_BIT(LOAD_PORT, LOAD_BIT);	//allow shifting in
#pragma region BODY
	pShiftInData = SPI.transfer(0x00);
	//pShiftInData = 0;
#pragma endregion
	CLEAR_BIT(LOAD_PORT, LOAD_BIT);	//deny shifting in
}

void MatrixKeyToLEDsClass::pBeep(void)
{
	static uint32_t tmrTone = 0;
	if (tmrTone > millis())
		tmrTone = millis();

	if (millis() - tmrTone < pToneDuration + 10)		//Wait the tone to finish
		return;
	tmrTone = millis();
	if (pBuzzerType == SUPPLY_FREQ)
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
	WRITE_BIT(pLEDStateArray[column], row, state);
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

uint8_t MatrixKeyToLEDsClass::toIndex(uint8_t row, uint8_t col)
{
	uint8_t index = (row * ROWS + col);
	//LOGLN(String(row) + ":" + String(col) + " HAS INDEX: " + String(index));
	return index;
}

void MatrixKeyToLEDsClass::toRowCol(uint8_t index, uint8_t * row, uint8_t * col)
{
	*row = index / COLUMNS;
	*col = index - *row*COLUMNS;
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
	LOGLNF("LOCKED");
}

void MatrixKeyToLEDsClass::pUnlock(void)
{
	Timer3.stop();
	pLockState = false;
	if (pLockStateIndicatorPin != NO_PIN)
		digitalWrite(pLockStateIndicatorPin, LOW);
	LOGLNF("UNLOCKED");
	//We refresh LEDs after unlock
	for (uint8_t row = 0; row < ROWS; row++)
		for (uint8_t col = 0; col < COLUMNS; col++)
			if (READ_BIT(pToggleStyle[col], row) == PRESS_AND_HOLD)
				pSetLed(row, col, READ_BIT(pButtonStateArray[col], row));
	Timer3.start();
}

void MatrixKeyToLEDsClass::pReactToLockButton(void)
{
	if (pLockButtonPin == NO_PIN)
		return;
	static uint8_t buttonState = RELEASED;
	static uint32_t clickTmr = 0;
	uint32_t currentTick = millis();
	if (digitalRead(pLockButtonPin) == pLockButtonPinActiveLogicLevel)
	{
		clickTmr = currentTick;
		if (buttonState == RELEASED)
		{
			buttonState = HOLDING;
			LOGLNF("LOCK BUTTON CLICKED");
			pBeep();
			if (pReactMoment == ON_CLICK)
				if (pIsLockSwitchActivated() == false)
				{
					if (pLockState == true)
						pUnlock();
					else
						pLock();
				}
				else
				{
					LOGLNF("LOCK SWITCH ACTIVATED, CANNOT CHANGE LOCK STATE");
				}
		}
	}
	if (clickTmr > currentTick)
		clickTmr = currentTick;

	if (currentTick - clickTmr > pDebounceInterval)
	{
		if (buttonState == HOLDING)
		{
			buttonState = RELEASED;
			LOGLNF("LOCK BUTTON RELEASED");
			if (pReactMoment == ON_RELEASE)
				if (pIsLockSwitchActivated() == false)
				{
					if (pLockState == true)
						pUnlock();
					else
						pLock();
				}
				else
				{
					LOGLNF("LOCK SWITCH ACTIVATED, CANNOT CHANGE LOCK STATE");
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
			LOGLNF("LOCK SWITCH ACTIVATED");
			if (pLockState == false)
				pLock();
		}
		else					//UNLOCK COMMAND
		{
			LOGLNF("LOCK SWITCH DEACTIVATED");
			if (pLockState == true)
				pUnlock();
		}
	}
}

void MatrixKeyToLEDsClass::init(void)
{
	memset(pButtonStateArray, 0, sizeof(pButtonStateArray));
	memset(pToggleStyle, 0, sizeof(pToggleStyle));
	memset(pLEDStateArray, 0, sizeof(pLEDStateArray));

	pinMode(LATCH_PIN, OUTPUT);
	pinMode(LOAD_PIN, OUTPUT);
	pinMode(BUZZER_PIN, OUTPUT);
	BEGIN(115200);
	SPI.begin();
	SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));//1400000


	pTransfer7219(OP_DISPLAYTEST, 0);
	pTransfer7219(OP_INTENSITY, 15);
	pTransfer7219(OP_SCANLIMIT, 7);
	pTransfer7219(OP_DECODEMODE, 0);
	pTransfer7219(OP_SHUTDOWN, 1);
	ClearAll();
	Timer3.initialize(SCAN_INTERVAL_US);
	Timer3.attachInterrupt(pExecute);

	LOGLNF("START CODE");
}

void MatrixKeyToLEDsClass::SetLEDsIntensity(uint8_t intensity)
{
	Timer3.stop();
	if (intensity > 15) intensity = 15;
	pTransfer7219(OP_INTENSITY, intensity);
	Timer3.start();
}

void MatrixKeyToLEDsClass::ClearAll(void)
{
	Timer3.stop();	//this job take much time, so we disable Timer3 to avoid overloading the MCU & stack
	for (uint8_t col = 0; col < COLUMNS; col++)
	{
		pLEDStateArray[col] = 0x00;
	}
	for (uint8_t row = 0; row < ROWS; row++)
	{
		pStatusByte[row] = 0x00;
		pTransfer7219(row + 1, pStatusByte[row]);
	}
	Timer3.start();
}

void MatrixKeyToLEDsClass::SetAll(void)
{
	Timer3.stop();	//this job take much time, so we disable Timer3 to avoid overloading the MCU & stack
	for (uint8_t col = 0; col < COLUMNS; col++)
	{
		pLEDStateArray[col] = 0xFF;
	}
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
	/*uint32_t tmr = micros();*/

	if (obj.pBuzzerType == SUPPLY_VOLTAGE && currentTick - obj.pBuzzerTimer > obj.pToneDuration && obj.pIsBuzzerOn == true)
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
		if (READ_BIT(obj.pShiftInData, obj.pCurrentButtonRowIndex) == ACTIVE)	//check each bit
		{
			if (READ_BIT(obj.pButtonStateArray[obj.pCurrentButtonColumnIndex], obj.pCurrentButtonRowIndex) == 0)
			{
				LOGLN("CLICKED  (" + String(obj.pCurrentButtonRowIndex) + "," + String(obj.pCurrentButtonColumnIndex) + ")");
				obj.pBeep();
				obj.pReact(obj.pCurrentButtonRowIndex, obj.pCurrentButtonColumnIndex, HOLDING);
				WRITE_BIT(obj.pButtonStateArray[obj.pCurrentButtonColumnIndex], obj.pCurrentButtonRowIndex, HOLDING);
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
				if (READ_BIT(obj.pButtonStateArray[column], row) == HOLDING)
				{
					LOGLN("RELEASED (" + String(row) + "," + String(column) + ")");
					obj.pReact(row, column, RELEASED);
					WRITE_BIT(obj.pButtonStateArray[column], row, RELEASED);
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

void MatrixKeyToLEDsClass::SetKeyToggleStyle(uint8_t row, uint8_t column, uint8_t toggleStyle)
{
	Timer3.stop();
	//LOGLN("KEY: " + String(row) + ":" + String(column) + "\t" +String(toggleStyle));
	WRITE_BIT(pToggleStyle[column], row, toggleStyle);
	if (toggleStyle == PRESS_AND_HOLD)
		if (READ_BIT(pButtonStateArray[column], row) == RELEASED)
			pSetLed(row, column, 0);
	Timer3.stop();
}

void MatrixKeyToLEDsClass::SetKeyToggleStyle(uint8_t index, uint8_t toggleStyle)
{
	uint8_t row = index / COLUMNS;
	uint8_t column = index - row*COLUMNS;
	SetKeyToggleStyle(row, column, toggleStyle);
}

void MatrixKeyToLEDsClass::SetRowToggleStyle(uint8_t row, uint8_t toggleStyle)
{
	for (uint8_t col = 0; col < COLUMNS; col++)
		SetKeyToggleStyle(row, col, toggleStyle);
}

void MatrixKeyToLEDsClass::SetColumnToggleStyle(uint8_t column, uint8_t toggleStyle)
{
	for (uint8_t row = 0; row < ROWS; row++)
		SetKeyToggleStyle(row, column, toggleStyle);
}

void MatrixKeyToLEDsClass::SetKeysToggleStyle(uint8_t toggleStyle)
{
	for (uint8_t row = 0; row < ROWS; row++)
		for (uint8_t col = 0; col < COLUMNS; col++)
			SetKeyToggleStyle(row, col, toggleStyle);
}

void MatrixKeyToLEDsClass::SetKeysToggleStyle(uint8_t * key, uint8_t size, uint8_t toggleStyle)
{
	for (uint8_t i = 0; i < size; i++)
	{
		SetKeyToggleStyle(key[i], toggleStyle);
		//Serial.println(key[i]);
	}
}

void MatrixKeyToLEDsClass::SetReactMoment(uint8_t reactMoment)
{
	pReactMoment = reactMoment;
}

void MatrixKeyToLEDsClass::SetDebounceInterval(uint32_t ms)
{
	pDebounceInterval = ms;
}

void MatrixKeyToLEDsClass::SetBuzzerType(uint8_t buzzerType)
{
	pBuzzerType = buzzerType;
}

void MatrixKeyToLEDsClass::SetBuzzerActiveLogicLevel(uint8_t level)
{
	pBuzzerActiveLogicLevel = level;
}

void MatrixKeyToLEDsClass::AddKeyGroup(uint8_t * btnArray, uint8_t size)
{
	Timer3.stop();

	KeyGroup * tempKeyGroup = new KeyGroup[pKeyGroupCount + 1];
	//Create a large array and copy old data into it
	for (uint8_t groupIndex = 0; groupIndex < pKeyGroupCount; groupIndex++)
		tempKeyGroup[groupIndex] = pKeyGroup[groupIndex];

	tempKeyGroup[pKeyGroupCount].size = size;
	tempKeyGroup[pKeyGroupCount].member = btnArray;

	//return it to the previous array
	delete[] pKeyGroup;
	pKeyGroup = tempKeyGroup;

	LOGF("ADDED: {");
	for (uint8_t m = 0; m < pKeyGroup[pKeyGroupCount].size; m++)
	{
		LOG(pKeyGroup[pKeyGroupCount].member[m]);
		if (m < pKeyGroup[pKeyGroupCount].size - 1)
			LOGF(",");
	}
	LOGF("} TO GROUP ");
	LOGLN(pKeyGroupCount);

	pKeyGroupCount++;


	/*
		uint8_t ** tempKeyGroup = new uint8_t*[pKeyGroupCount + 1];
		uint8_t * tempKeyGroupSize = new uint8_t[pKeyGroupCount + 1];
		//Create a large array and copy old data into it
		for (uint8_t groupIndex = 0; groupIndex < pKeyGroupCount; groupIndex++)
		{
			tempKeyGroup[groupIndex] = pKeyGroup[groupIndex];
			tempKeyGroupSize[groupIndex] = pKeyGroupSize[groupIndex];
		}

		tempKeyGroupSize[pKeyGroupCount] = size;
		tempKeyGroup[pKeyGroupCount] = btnArray;

		//return it to the previous array
		delete[] pKeyGroup;
		delete[] pKeyGroupSize;
		pKeyGroup = tempKeyGroup;
		pKeyGroupSize = tempKeyGroupSize;

		LOGF("ADDED: {");
		for (uint8_t m = 0; m < pKeyGroupSize[pKeyGroupCount]; m++)
		{
			LOG(pKeyGroup[pKeyGroupCount][m]);
			if (m < pKeyGroupSize[pKeyGroupCount] - 1)
				LOGF(",");
		}
		LOGF("} TO GROUP ");
		LOGLN(pKeyGroupCount);

		pKeyGroupCount++;
		*/

	Timer3.start();
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
	return READ_BIT(pButtonStateArray[column], row) == HOLDING ? 1 : 0;
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
	return READ_BIT(pLEDStateArray[column], row);
}


MatrixKeyToLEDsClass MatrixKeyToLEDs;

