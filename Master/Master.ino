/*#include <Wire.h>
#include "MatrixKeyToLEDs.h"


void setup()
{
	Serial.begin(115200);

	MatrixKeyToLEDs.init();

	MatrixKeyToLEDs.SetLEDsIntensity(15);     //Intensity (brightness): 0->15
	MatrixKeyToLEDs.SetBuzzerType(SUPPLY_VOLTAGE);    //SUPPLY_VOLTAGE(default)/SUPPLY_FREQ
	MatrixKeyToLEDs.SetBuzzerActiveLogicLevel(HIGH);  //HIGH(default)/LOW
	//Dat kieu nhan cho tat ca phim
	MatrixKeyToLEDs.SetKeysToggleStyle(CLICK_TOGGLE);   //CLICK_TOGGLE(default)/PRESS_AND_HOLD

	MatrixKeyToLEDs.SetReactMoment(ON_CLICK);     //ON_CLICK(default)/ON_RELEASE
	MatrixKeyToLEDs.SetTempOutput(10, 1); // Output, button, Time_On, Interval

	MatrixKeyToLEDs.SetLockButton(12, LOW, 7, 20);	//chan, muc logic tich cuc, tgian tu khoa (ms)

	MatrixKeyToLEDs.SetShortLockIndicatorPin(13);	//den bao trang thai khoa
	MatrixKeyToLEDs.SetLockIndicatorPin(14);	//den bao trang thai dang nhap
}

void loop()
{
	MatrixKeyToLEDs.Execute();
}*/


#include <Wire.h>
#include "MatrixKeyToLEDs.h"


void setup()
{
	Serial.begin(115200);

	MatrixKeyToLEDs.init();

	MatrixKeyToLEDs.SetLEDsIntensity(15);     //Intensity (brightness): 0->15
	MatrixKeyToLEDs.SetBuzzerType(SUPPLY_VOLTAGE);    //SUPPLY_VOLTAGE(default)/SUPPLY_FREQ
	MatrixKeyToLEDs.SetBuzzerActiveLogicLevel(HIGH);  //HIGH(default)/LOW
	//Dat kieu nhan cho tat ca phim
	MatrixKeyToLEDs.SetKeysToggleStyle(CLICK_TOGGLE);   //CLICK_TOGGLE(default)/PRESS_AND_HOLD

	MatrixKeyToLEDs.SetReactMoment(ON_CLICK);     //ON_CLICK(default)/ON_RELEASE
	MatrixKeyToLEDs.SetTempOutput(10, 1); // Output, button, Time_On, Interval

	MatrixKeyToLEDs.SetLockButton(12, LOW, 7);	//chan, muc logic tich cuc, tgian tu khoa (ms)
	MatrixKeyToLEDs.SetLockSwitch(11, LOW);

	MatrixKeyToLEDs.SetShortLockIndicatorPin(13);	//den bao trang thai khoa
}

void loop()
{
	MatrixKeyToLEDs.Execute();
}