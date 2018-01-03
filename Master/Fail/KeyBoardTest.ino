//#include "Libs/MatrixKeyToLEDs.h"
//#include "Libs/MS_Communication.h"
//#include <Wire.h>


void setup()
{
	Serial.begin(9600);
	pinMode(13, OUTPUT);
	for (int i = 0; i < 1000; i++)
		Serial.println("HELLO");
	delay(100);
	//MatrixKeyToLEDs.init();

	//MatrixKeyToLEDs.SetLEDsIntensity(1);     //Intensity (brightness): 0->15
	//MatrixKeyToLEDs.SetToneStyle(1244, 50);      //frequency, duration
	//MatrixKeyToLEDs.SetBuzzerType(SUPPLY_VOLTAGE);    //SUPPLY_VOLTAGE(default)/SUPPLY_FREQ
	//MatrixKeyToLEDs.SetBuzzerActiveLogicLevel(HIGH);  //HIGH(default)/LOW
	//						  //Dat kieu nhan cho tat ca phim
	//MatrixKeyToLEDs.SetKeysToggleStyle(CLICK_TOGGLE);   //CLICK_TOGGLE(default)/PRESS_AND_HOLD

	//MatrixKeyToLEDs.SetReactMoment(ON_CLICK);     //ON_CLICK(default)/ON_RELEASE
	//MatrixKeyToLEDs.SetTempOutput(10, 1, 100, 250); // Output, button, Time_On, Time_Off

	//Wire.begin();
	//MS.init();
}

void loop()
{
	//MatrixKeyToLEDs.Execute();
	//MS.Execute();
}