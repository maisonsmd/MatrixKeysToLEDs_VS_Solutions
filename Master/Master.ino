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
	MatrixKeyToLEDs.SetTempOutput(10, 1, 500, 250); // Output, button, Time_On, Interval

	MatrixKeyToLEDs.SetShortLockButton(12, LOW, 20000);	//chan, muc logic tich cuc, tgian tu khoa (ms)
	MatrixKeyToLEDs.SetLockButton(11, LOW, 10000, 1000);//chan, muc logic tich cuc, tgian tu khoa (ms), tgian nhan giu (ms)

	MatrixKeyToLEDs.SetLockStateIndicatorPin(13);	//den bao trang thai khoa
	MatrixKeyToLEDs.SetLoginStateIndicatorPin(14);	//den bao trang thai dang nhap
}

void loop()
{
	MatrixKeyToLEDs.Execute();
}