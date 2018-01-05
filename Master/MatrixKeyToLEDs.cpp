// Matrix keypad controlling Matrix LED by maisonsmd(c) 2017
// 
// 

#include "MatrixKeyToLEDs.h"

boolean MatrixKeyToLEDsClass::pScanGroup(uint8_t row, uint8_t column, uint8_t state, uint8_t toggleStyle)
{
	if (pKeyGroupsCount == 0)
		return false;

	//this section is for KeyGroup
	for (uint8_t groupIndex = 0; groupIndex < pKeyGroupsCount; groupIndex++)
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
					pLEDStateArray[row][column] = !pLEDStateArray[row][column];
				}
				else
				{
					if (state == HOLDING)
						pLEDStateArray[row][column] = 1;
					else	//RELEASED
						pLEDStateArray[row][column] = 0;
				}
				SetLed(row, column, pLEDStateArray[row][column]);

				LOGLN("TOGGLED LED (" + String(row) + "," + String(column) + ") -> " + String(pLEDStateArray[row][column]));
				//if just lighted on this LED
				if (pLEDStateArray[row][column] == 1)
					//then turn off other LEDs
					for (uint8_t memberIndexAgain = 0; memberIndexAgain < pKeyGroup[groupIndex].size; memberIndexAgain++)
						if (pKeyGroup[groupIndex].member[memberIndexAgain] != currentBtnIndex)
						{
							uint8_t r, c;	//current row, col
							toRowCol(pKeyGroup[groupIndex].member[memberIndexAgain], &r, &c);
							pLEDStateArray[r][c] = 0;
							SetLed(r, c, pLEDStateArray[r][c]);
						}
				//Done, exit function and force to exti pReact();
				return true;
			}
		}
	//end of KeyGroup section

	//Done, exit function and continue pReact();*/
	return false;
}

void MatrixKeyToLEDsClass::pReact(uint8_t row, uint8_t column, uint8_t state)
{
	if (IsLocked())
		return;

	////Timer3.stop();
	if (pToggleStyle[row][column] == CLICK_TOGGLE)	//CLICK_TOGGLE
	{
		if ((pReactMoment == ON_CLICK && state == HOLDING)
			|| (pReactMoment == ON_RELEASE && state == RELEASED))
		{
			if (GetLEDState(row, column) == 0)	//if this LED is OFF
				pFlashTempOutput(toIndex(row, column));
			//if this key belogs to any group. exit;
			if (pScanGroup(row, column, state, CLICK_TOGGLE))
				return;

			pLEDStateArray[row][column] = !pLEDStateArray[row][column];//Toggle
			LOGLN("TOGGLED LED (" + String(row) + "," + String(column) + ") -> " + String(pLEDStateArray[row][column]));
		}
	}
	else						//PRESS_AND_HOLD
	{
		//if this key belogs to any group. exit;
		if (state == HOLDING)
		{
			pFlashTempOutput(toIndex(row, column));
		}
		if (pScanGroup(row, column, state, PRESS_AND_HOLD))
			return;

		if (state == HOLDING)
		{
			pLEDStateArray[row][column] = 1;	//Turn LED on
			LOGLN("PRESSING LED (" + String(row) + "," + String(column) + ") -> " + String(pLEDStateArray[row][column]));
		}
		else
		{
			pLEDStateArray[row][column] = 0;	//Turn LED off
			LOGLN("RELEASED LED (" + String(row) + "," + String(column) + ") -> " + String(pLEDStateArray[row][column]));
		}
	}
	SetLed(row, column, pLEDStateArray[row][column]);
	////Timer3.start();
}

void MatrixKeyToLEDsClass::pShiftOut(void)
{
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

void MatrixKeyToLEDsClass::Beep(void)
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

void MatrixKeyToLEDsClass::SetLed(uint8_t row, uint8_t column, uint8_t state)
{
	pLEDStateArray[row][column] = state;
	uint8_t val = 0x00;
	val = B10000000 >> column;
	if (state)
		pStatusByte[row] = pStatusByte[row] | val;
	else {
		val = ~val;
		pStatusByte[row] = pStatusByte[row] & val;
	}
	pTransfer7219(row + 1, pStatusByte[row]);
	LOGLN("Set Led");
}

void MatrixKeyToLEDsClass::SetLed(uint8_t index, uint8_t state)
{
	uint8_t row;
	uint8_t col;
	toRowCol(index, &row, &col);
	SetLed(row, col, state);
}

void MatrixKeyToLEDsClass::pTransfer7219(uint8_t opcode, uint8_t data)
{
	//memset(pShiftOutData7219, 0, sizeof(pShiftOutData7219));
	pShiftOutData7219[1] = opcode;
	pShiftOutData7219[0] = data;
	//pShiftOut();
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
#ifdef USE_RFID
	return false;
#else
	return  digitalRead(pLockSwitchPin) == pLockSwitchActiveLogicLevel;
#endif // !USE_RFID
}

void MatrixKeyToLEDsClass::pHandleLockStateIndicator(void)
{
	uint32_t currentTick = millis();
	switch (pLockState)
	{
	case LockState::LOCKED:
		pLockIndicatorState = ON;
#ifdef USE_RFID
		pShortLockIndicatorState = OFF;
#else 
		pShortLockIndicatorState = ON;
#endif

		break;
	case LockState::SHORT_LOCKED:
		pLockIndicatorState = OFF;
		pShortLockIndicatorState = ON;
		break;
	case LockState::UNLOCKED:
		pLockIndicatorState = OFF;
		pShortLockIndicatorState = OFF;
		break;
	}

	//blink when its going to lock
	if (GetLockState() == SHORT_LOCKED)
	{
		uint32_t lockTick = pLockAutoEnableCounter + pLockDelayTime;	//Lock moment
		if (lockTick > currentTick)		//valid math
		{
			if (currentTick > lockTick - LOCK_INDICATOR_BLINK_TIME || lockTick < LOCK_INDICATOR_BLINK_TIME)
			{
				pLockIndicatorState = BLINK;
			}
		}
	}

	if (pShortLockIndicatorPin != NO_PIN)
	{
		switch (pShortLockIndicatorState)
		{
		case ON:
			digitalWrite(pShortLockIndicatorPin, HIGH);
			break;
		case OFF:
			digitalWrite(pShortLockIndicatorPin, LOW);
			break;
		}
	}
	if (pLockIndicatorPin != NO_PIN)
	{
		switch (pLockIndicatorState)
		{
		case ON:
			digitalWrite(pLockIndicatorPin, HIGH);
			break;
		case OFF:
			digitalWrite(pLockIndicatorPin, LOW);
			break;
		case BLINK:
			static uint32_t flashTimer = 0;	//Wait to flash the indicator
			static uint32_t holdTimer = 0;	//Wait to turn off the indicator
			static boolean isTurnedOn = false;
			const uint32_t blinkInterval = 1000;
			const uint32_t holdInterval = 100;
			if (flashTimer > currentTick)
				flashTimer = currentTick;
			if (holdTimer > currentTick)
				holdTimer = currentTick;
			if (!isTurnedOn)
			{
				if (currentTick - flashTimer > blinkInterval)
				{
					//flash now!
					isTurnedOn = true;
					holdTimer = currentTick;
					Beep();
				}
			}
			if (isTurnedOn)
			{
				if (currentTick - holdTimer > holdInterval)
				{
					isTurnedOn = false;
					flashTimer = currentTick;
				}
			}

			digitalWrite(pLockIndicatorPin, isTurnedOn);

			break;
		}
	}
}

#ifdef USE_RFID
void MatrixKeyToLEDsClass::pHandleLockButton(void)
{
	if (pShortLockButtonPin == NO_PIN)
		return;

	uint32_t currentTick = millis();
	//Auto lock
	if (pLockDelayTime != 0)
	{
		if (currentTick < pLockAutoEnableCounter)
			pLockAutoEnableCounter = currentTick;
		if (currentTick - pLockAutoEnableCounter > pLockDelayTime)
		{
			if (GetLockState() == LockState::SHORT_LOCKED)
			{
				Beep();
				Lock();
			}
		}
	}

	static uint8_t buttonState = RELEASED;
	static uint32_t clickTmr = 0;

	//Manual lock
	//if (digitalRead(pLockButtonPin) == pLockButtonActiveLogicLevel)
	if (digitalRead(pShortLockButtonPin) == pLockButtonActiveLogicLevel)
	{
		clickTmr = currentTick;
		if (buttonState == RELEASED)
		{
			Beep();
			pLockButtonPressTime = currentTick;
			buttonState = HOLDING;
			pResetAutoLockTimers();
		}

		else	//holding
		{
			switch (GetLockState())
			{
			case LockState::LOCKED:
				//Wait for RFID tag
				break;
			default:	//SHORT_LOCKED & UNLOCKED
				if (pLockButtonPressTime > currentTick)
					pLockButtonPressTime = currentTick;
				if (currentTick - pLockButtonPressTime > pLockButtonHoldTime)
				{
					LOGLNF("PRESS AND HOLD LOCK BUTTON, LOCKING...");
					Lock();
					Beep();
				}
				break;
			}
		}
	}
	else
	{
		if (currentTick < clickTmr)
			clickTmr = currentTick;
		if (currentTick - clickTmr > pDebounceInterval)
		{
			//RELEASED
			buttonState = RELEASED;
		}
	}
}


#else
void MatrixKeyToLEDsClass::pHandleLockSwitch(void)
{
	static boolean lastSwitchState = !pIsLockSwitchActivated();
	if (pIsLockSwitchActivated() != lastSwitchState)
	{
		//Changed state
		Beep();
		lastSwitchState = pIsLockSwitchActivated();
		if (pIsLockSwitchActivated() == true)	//LOCK COMMAND
		{
			LOGLNF("LOCK SWITCH ACTIVATED");
			if (!IsLocked())
				Lock();
		}
		else					//UNLOCK COMMAND
		{
			LOGLNF("LOCK SWITCH DEACTIVATED");
			if (IsLocked())
				Unlock();
		}
	}
}
#endif // USE_RFID


void MatrixKeyToLEDsClass::ShortLock(void)
{
	pLockState = SHORT_LOCKED;
	LOGLNF("SHORT LOCKED");
	pResetAutoLockTimers();
}

void MatrixKeyToLEDsClass::Unlock(void)
{
	//Timer3.stop();
	pLockState = UNLOCKED;
	LOGLNF("UNLOCKED");
	//We refresh LEDs after unlock
	for (uint8_t row = 0; row < ROWS; row++)
		for (uint8_t col = 0; col < COLUMNS; col++)
			if (pToggleStyle[row][col] == PRESS_AND_HOLD)
				SetLed(row, col, pButtonStateArray[row][col]);
	pResetAutoLockTimers();
	//Timer3.start();
}

void MatrixKeyToLEDsClass::Lock(void)
{
	pLockState = LOCKED;
	LOGLNF("LOCKED");
	pResetAutoLockTimers();
}

void MatrixKeyToLEDsClass::pResetAutoLockTimers(void)
{
	pShortLockAutoEnableCounter = millis();
#ifdef USE_RFID
	pLockAutoEnableCounter = millis();
#endif // USE_RFID

}

void MatrixKeyToLEDsClass::pHandleShortLockButton(void)
{
	if (pShortLockButtonPin == NO_PIN)
		return;

	uint32_t currentTick = millis();

	//Auto short lock
	if (pShortLockDelayTime != 0)
	{
		if (currentTick < pShortLockAutoEnableCounter)
			pShortLockAutoEnableCounter = currentTick;
		if (currentTick - pShortLockAutoEnableCounter > pShortLockDelayTime)
		{
			if (GetLockState() == LockState::UNLOCKED)
			{
				Beep();
				ShortLock();
			}
		}
	}
	static uint8_t buttonState = RELEASED;
	static uint32_t clickTmr = 0;


	//check if clicked
	if (digitalRead(pShortLockButtonPin) == pShortLockButtonPinActiveLogicLevel)
	{
		clickTmr = currentTick;
		if (buttonState == RELEASED)
		{
			buttonState = HOLDING;
			LOGLNF("SHORT LOCK BUTTON CLICKED");
			Beep();
			pResetAutoLockTimers();
			if (pReactMoment == ON_CLICK)
				if (pIsLockSwitchActivated() == false)
				{
					switch (GetLockState())
					{
					case LockState::LOCKED:
						break;
					case LockState::SHORT_LOCKED:
						Unlock();
						break;
					case LockState::UNLOCKED:
						ShortLock();
						break;
					}
				}
				else
				{
					LOGLNF("SHORT LOCK SWITCH ACTIVATED, CANNOT CHANGE LOCK STATE");
				}
		}
	}
	if (clickTmr > currentTick)
		clickTmr = currentTick;


	//check if released
	if (currentTick - clickTmr > pDebounceInterval)
	{
		if (buttonState == HOLDING)
		{
			buttonState = RELEASED;
			LOGLNF("SHORT LOCK BUTTON RELEASED");
			if (pReactMoment == ON_RELEASE)
				if (pIsLockSwitchActivated() == false)
				{
					switch (GetLockState())
					{
					case LockState::LOCKED:
						break;
					case LockState::SHORT_LOCKED:
						Unlock();
						break;
					case LockState::UNLOCKED:
						ShortLock();
						break;
					}
				}
				else
				{
					LOGLNF("SHORT LOCK SWITCH ACTIVATED, CANNOT CHANGE LOCK STATE");
				}
		}
	}
}

void MatrixKeyToLEDsClass::pHandleTempOutput()
{
	if (pTempOutputPin == NO_PIN)
		return;

	uint8_t row;
	uint8_t col;
	toRowCol(pTempOutputKeyIndex, &row, &col);
	digitalWrite(pTempOutputPin, pLEDStateArray[row][col]);

	//The code below used to auto turn on-off temp output. Customer asked to disable!
	/*if (digitalRead(pTempOutputPin) == LOW && !isWaitingToTurnTempOutputOn)
		return;

	if (pTempOutputInitTmr > millis())
		pTempOutputInitTmr = millis();
	if (pTempOutputIntervalTmr > millis())
		pTempOutputIntervalTmr = millis();

	if (millis() - pTempOutputInitTmr < pTempOutputInitTime)
		return;

	if (isWaitingToTurnTempOutputOn)
	{
		isWaitingToTurnTempOutputOn = false;
		digitalWrite(pTempOutputPin, HIGH);
		pTempOutputIntervalTmr = millis();
	}
	if (millis() - pTempOutputIntervalTmr > pTempOutputInterval)
	{
		digitalWrite(pTempOutputPin, LOW);
	}*/
}

void MatrixKeyToLEDsClass::pFlashTempOutput(uint8_t keyIndex)
{
	/*if (keyIndex != pTempOutputKeyIndex)
		return;
	isWaitingToTurnTempOutputOn = true;
	pTempOutputInitTmr = millis();			//start counting to turn on*/
}

void MatrixKeyToLEDsClass::init(void)
{
	memset(pButtonStateArray, 0, sizeof(pButtonStateArray));
	memset(pToggleStyle, 0, sizeof(pToggleStyle));
	memset(pLEDStateArray, 0, sizeof(pLEDStateArray));
	memset(pShiftOutData7219, 0, sizeof(pShiftOutData7219));
	memset(pShiftOutData595, 0, sizeof(pShiftOutData595));

	pinMode(LATCH_PIN, OUTPUT);
	pinMode(LOAD_PIN, OUTPUT);
	pinMode(BUZZER_PIN, OUTPUT);
	Beep();
	SPI.begin();
	SPI.beginTransaction(SPISettings(1400000, MSBFIRST, SPI_MODE0));//1400000


	pTransfer7219(OP_DISPLAYTEST, 0);
	pShiftOut();
	pTransfer7219(OP_INTENSITY, 15);
	pShiftOut();
	pTransfer7219(OP_SCANLIMIT, 7);
	pShiftOut();
	pTransfer7219(OP_DECODEMODE, 0);
	pShiftOut();
	pTransfer7219(OP_SHUTDOWN, 1);
	pShiftOut();
	ClearAll();
	//Timer3.initialize(SCAN_INTERVAL_US);
	//Timer3.attachInterrupt(Execute);
#ifdef USE_RFID
	MS.init();
#endif // USE_RFID

	LOGLNF("START CODE");
}

void MatrixKeyToLEDsClass::SetLEDsIntensity(uint8_t intensity)
{
	//Timer3.stop();
	if (intensity > 15) intensity = 15;
	pTransfer7219(OP_INTENSITY, intensity);
	//Timer3.start();
}

void MatrixKeyToLEDsClass::ClearAll(void)
{
	//Timer3.stop();	//this job take much time, so we disable Timer3 to avoid overloading the MCU & stack
	for (uint8_t row = 0; row < ROWS; row++)
		for (uint8_t col = 0; col < COLUMNS; col++)
		{
			pLEDStateArray[row][col] = 0;
		}
	for (uint8_t row = 0; row < ROWS; row++)
	{
		pStatusByte[row] = 0x00;
		pTransfer7219(row + 1, pStatusByte[row]);
		pShiftOut();
	}
	//Timer3.start();
}

void MatrixKeyToLEDsClass::SetAll(void)
{
	//Timer3.stop();	//this job take much time, so we disable Timer3 to avoid overloading the MCU & stack
	for (uint8_t row = 0; row < ROWS; row++)
		for (uint8_t col = 0; col < COLUMNS; col++)
		{
			pLEDStateArray[row][col] = 1;
		}
	for (uint8_t row = 0; row < ROWS; row++)
	{
		pStatusByte[row] = 0xFF;
		pTransfer7219(row + 1, pStatusByte[row]);
	}
	//Timer3.start();
}

boolean MatrixKeyToLEDsClass::IsLocked(void)
{
	if (pLockState == UNLOCKED)
		return false;
	return true;
}

LoginState MatrixKeyToLEDsClass::GetLoginState(void)
{
	return pLoginState;
}

LockState MatrixKeyToLEDsClass::GetLockState(void)
{
	return pLockState;
}

void MatrixKeyToLEDsClass::Execute(void)
{
#ifdef USE_RFID
	MS.Execute();
	obj.pHandleLockButton();
#else
	obj.pHandleLockSwitch();
#endif // USE_RFID

	obj.pHandleLockStateIndicator();
	obj.pHandleShortLockButton();
	obj.pHandleTempOutput();

	static uint32_t tmrStable = millis();
	if (tmrStable > millis())
		tmrStable = millis();
	if (millis() - tmrStable < SCAN_INTERVAL_MS)
		return;
	tmrStable = millis();

	static int8_t keysClicked = 0;
	//LOGLN(millis());
	uint32_t currentTick = millis();
	//uint32_t tmr = micros();

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
			if (obj.pButtonStateArray[obj.pCurrentButtonRowIndex][obj.pCurrentButtonColumnIndex] == 0)
			{
				LOGLN("CLICKED  (" + String(obj.pCurrentButtonRowIndex) + "," + String(obj.pCurrentButtonColumnIndex) + ")");
				keysClicked++;
				obj.Beep();
				obj.pReact(obj.pCurrentButtonRowIndex, obj.pCurrentButtonColumnIndex, HOLDING);
				obj.pButtonStateArray[obj.pCurrentButtonRowIndex][obj.pCurrentButtonColumnIndex] = HOLDING;
			}
			obj.pButtonTimerArray[obj.pCurrentButtonRowIndex][obj.pCurrentButtonColumnIndex] = currentTick;
		}
	}

	if (keysClicked > 0)	//We only check keys when there is at least 1 key clicked
	{
		for (uint8_t row = 0; row < ROWS; row++)
			for (uint8_t column = 0; column < COLUMNS; column++)
			{
				if (obj.pButtonTimerArray[row][column] > currentTick)
					obj.pButtonTimerArray[row][column] = currentTick;	//reset the timer

				if (currentTick - obj.pButtonTimerArray[row][column] > obj.pDebounceInterval)
				{
					if (obj.pButtonStateArray[row][column] == HOLDING)
					{
						LOGLN("RELEASED (" + String(row) + "," + String(column) + ")");
						keysClicked--;
						if (keysClicked < 0)
							keysClicked = 0;
						obj.pReact(row, column, RELEASED);
						obj.pButtonStateArray[row][column] = RELEASED;

						obj.pShortLockAutoEnableCounter = millis();
					}
				}
			}
	}

	//Complete 1 revolution of scanning?
	obj.pCurrentButtonColumnIndex++;
	//obj.pCurrentLEDColumnIndex++;
	if (obj.pCurrentButtonColumnIndex >= COLUMNS) obj.pCurrentButtonColumnIndex = 0;
	//if (obj.pCurrentLEDColumnIndex >= COLUMNS) obj.pCurrentLEDColumnIndex = 0;

	//tmr = micros() - tmr;
	//Serial.println(tmr);
	//delay(100);
}

void MatrixKeyToLEDsClass::SetToneStyle(uint16_t freq, uint32_t duration)
{
	pToneFrequency = freq;
	pToneDuration = duration;
}

void MatrixKeyToLEDsClass::SetKeyToggleStyle(uint8_t row, uint8_t column, uint8_t toggleStyle)
{
	//Timer3.stop();
	//LOGLN("KEY: " + String(row) + ":" + String(column) + "\t" +String(toggleStyle));
	pToggleStyle[row][column] = toggleStyle;
	if (toggleStyle == PRESS_AND_HOLD)
		if (pButtonStateArray[row][column] == RELEASED)
			SetLed(row, column, 0);
	//Timer3.start();
}

void MatrixKeyToLEDsClass::SetKeyToggleStyle(uint8_t index, uint8_t toggleStyle)
{
	uint8_t row = index / COLUMNS;
	uint8_t column = index - row * COLUMNS;
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
	//Timer3.stop();
	if (pKeyGroupsCount >= MAX_GROUPS
		|| size > MAX_GR_MEMBERS)
		return;

	pKeyGroup[pKeyGroupsCount].size = size;
	//Copy data to array
	for (uint8_t memberIndex = 0; memberIndex < size; memberIndex++)
		pKeyGroup[pKeyGroupsCount].member[memberIndex] = btnArray[memberIndex];


#ifdef ENABLE_LOG
	LOGF("ADDED: {");
	for (uint8_t memberIndex = 0; memberIndex < pKeyGroup[pKeyGroupsCount].size; memberIndex++)
	{
		LOG(pKeyGroup[pKeyGroupsCount].member[memberIndex]);
		if (memberIndex < pKeyGroup[pKeyGroupsCount].size - 1)
			LOGF(",");
	}
	LOGF("} TO GROUP ");
	LOGLN(pKeyGroupsCount);
#endif // ENABLE_LOG

	pKeyGroupsCount++;


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

		//Timer3.start();
}

#ifdef USE_RFID

void MatrixKeyToLEDsClass::SetLockIndicatorPin(uint8_t pin)
{
	pLockIndicatorPin = pin;
	pinMode(pLockIndicatorPin, OUTPUT);
}

#else
void MatrixKeyToLEDsClass::SetLockSwitch(uint8_t pin, uint8_t level)
{
	pLockSwitchPin = pin;
	pinMode(pLockSwitchPin, INPUT);
	pLockSwitchActiveLogicLevel = level;
}
#endif // !USE_RFID

void MatrixKeyToLEDsClass::SetLockButton(uint8_t pin, uint8_t level, uint32_t shortLockDelayTime, uint32_t lockDelayTime)
{
	pShortLockButtonPin = pin;
	pinMode(pShortLockButtonPin, INPUT);
	pShortLockButtonPinActiveLogicLevel = level;
	pLockButtonActiveLogicLevel = level;
	pShortLockDelayTime = shortLockDelayTime * 1000;
	pLockDelayTime = lockDelayTime * 1000;
}

void MatrixKeyToLEDsClass::SetShortLockIndicatorPin(uint8_t pin)
{
	pShortLockIndicatorPin = pin;
	pinMode(pShortLockIndicatorPin, OUTPUT);
	digitalWrite(pShortLockIndicatorPin, LOW);
}

//void MatrixKeyToLEDsClass::SetTempOutput(uint8_t pin, uint8_t keyIndex, uint32_t initTime, uint32_t interval = 100)
void MatrixKeyToLEDsClass::SetTempOutput(uint8_t pin, uint8_t keyIndex)
{
	pTempOutputPin = pin;
	//pTempOutputInitTime = initTime;
	//pTempOutputInterval = interval;
	pTempOutputKeyIndex = keyIndex;
	pinMode(pTempOutputPin, OUTPUT);
	digitalWrite(pTempOutputPin, LOW);
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
	uint8_t column = buttonNumber - row * COLUMNS;
	return GetButtonState(row, column);
}

uint8_t MatrixKeyToLEDsClass::GetButtonState(uint8_t row, uint8_t column)
{
	return pButtonStateArray[row][column] == HOLDING ? 1 : 0;
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
	uint8_t column = LEDNumber - row * COLUMNS;
	return GetLEDState(row, column);
}

uint8_t MatrixKeyToLEDsClass::GetLEDState(uint8_t row, uint8_t column)
{
	return pLEDStateArray[row][column];
}


MatrixKeyToLEDsClass MatrixKeyToLEDs;

