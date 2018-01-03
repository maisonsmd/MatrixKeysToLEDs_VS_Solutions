//modpoll.exe -m rtu -b 9600 -p none -o 0.1 -t 1 -a 10 -c 4 -r 1 COM5

#include "MatrixKeyToLEDs.h"

#define LOCK_LED_INDICATOR	13
#define LOCK_SWITCH		12
#define LOCK_BUTTON		11

void setup()
{
	Serial.begin(115200);

	MatrixKeyToLEDs.init();
	MatrixKeyToLEDs.SetLEDsIntensity(15);			//Intensity (brightness): 0->15
	MatrixKeyToLEDs.SetToneStyle(1244, 10);			//frequency, duration
	MatrixKeyToLEDs.SetBuzzerType(SUPPLY_VOLTAGE);		//SUPPLY_VOLTAGE(default)/SUPPLY_FREQ
	MatrixKeyToLEDs.SetBuzzerActiveLogicLevel(HIGH);	//HIGH(default)/LOW

	MatrixKeyToLEDs.SetReactMoment(ON_CLICK);			//ON_CLICK(default)/ON_RELEASE
	//Dat kieu nhan cho tat ca phim
	MatrixKeyToLEDs.SetKeysToggleStyle(CLICK_TOGGLE);		//CLICK_TOGGLE(default)/PRESS_AND_HOLD
	//Dat kieu nhan cho 1 phim (hang,cot)
	MatrixKeyToLEDs.SetKeyToggleStyle(2, 0, CLICK_TOGGLE);	//row, column, CLICK_TOGGLE(default)/PRESS_AND_HOLD
	//Dat kieu nhan cho 1 phim (so thu tu, 0->64)
	MatrixKeyToLEDs.SetKeyToggleStyle(16, CLICK_TOGGLE);	//row, column, CLICK_TOGGLE(default)/PRESS_AND_HOLD
	//Dat kieu nhan cho 1 hang
	MatrixKeyToLEDs.SetRowToggleStyle(1, CLICK_TOGGLE);		//row, CLICK_TOGGLE(default)/PRESS_AND_HOLD
	//Dat kieu nhan cho 1 cot
	MatrixKeyToLEDs.SetColumnToggleStyle(1, CLICK_TOGGLE);	//column, CLICK_TOGGLE(default)/PRESS_AND_HOLD
									//Mang chua cac phim nhan muon set
	uint8_t danhSachCacPhim[11] = { 0,1,2,3,4,5,6,7,8,9,10 };
	MatrixKeyToLEDs.SetKeysToggleStyle(danhSachCacPhim, sizeof(danhSachCacPhim), CLICK_TOGGLE);	//sizeof(danhSachCacPhim) tuong duong 5

	//Them nhom phim nhan
	static uint8_t btnGroup1[4] = { 0,1,2,3 };	// "static" là bat buoc
	MatrixKeyToLEDs.AddKeyGroup(btnGroup1, sizeof(btnGroup1));

	static uint8_t btnGroup2[6] = { 8,9,10,11,16,17 };
	MatrixKeyToLEDs.AddKeyGroup(btnGroup2, sizeof(btnGroup2));
	// or MatrixKeyToLEDs.AddKeyGroup(btnGroup2, 6);	//sizeof(btnGroup2) = 6;

	//Truyen vao chan Cong tac khoa
	MatrixKeyToLEDs.SetLockSwitchPin(LOCK_SWITCH);
	MatrixKeyToLEDs.SetLockSwitchActiveLogicLevel(LOW);	//HIGH/LOW(default)

	//Truyen vao chan Nut nhan khoa
	MatrixKeyToLEDs.SetLockButtonPin(LOCK_BUTTON);
	MatrixKeyToLEDs.SetLockButtonActiveLogicLevel(LOW);	//HIGH/LOW(default)

	//Truyen vao chan LED trang thai khoa
	MatrixKeyToLEDs.SetLockStateIndicatorPin(LOCK_LED_INDICATOR);


	//Sang tat ca:
	MatrixKeyToLEDs.SetAll();
	delay(200);
	//Tat tat ca:
	MatrixKeyToLEDs.ClearAll();
	delay(200);
	//Sang tat ca:
	MatrixKeyToLEDs.SetAll();
	delay(200);
	//Tat tat ca:
	MatrixKeyToLEDs.ClearAll();
	delay(200);
}

void loop()
{
	//Lay trang thai khoa:
	bool isLocked = MatrixKeyToLEDs.IsLocked();

	//Serial.println("Trang thai khoa: " + String(isLocked));

	//Lay trang thai nut nhan:
	bool btnZeroZero = MatrixKeyToLEDs.GetButtonState(0, 0);
	Serial.println("Trang thai phim (0,0): " + String(btnZeroZero));

	//Lay trang thai den:
	//bool LedZeroZero = MatrixKeyToLEDs.GetLEDState(0, 0);
	//Serial.println("Trang thai den  (0,0): " + String(LedZeroZero));

	Serial.println();
	delay(5000);
}