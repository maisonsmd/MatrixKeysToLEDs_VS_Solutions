// 
// 
// 

#include "DataTransferer.h"



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//void DataTransfererClass::init(Role role, uint8_t address)
//{
//	_role = role;
//	_I2CAddress = address;
//}

uint8_t DataTransfererClass::AddGroup(const DataGroupClass & dataGr)
{
	DataGroupClass * tempDataGroup = new DataGroupClass[_groupsCount + 1];
	//copy all data to new array
	for (uint8_t i = 0; i < _groupsCount; i++)
		tempDataGroup[i] = _dataGroup[i];
	//add last data
	tempDataGroup[_groupsCount] = dataGr;
	tempDataGroup[_groupsCount].SetGroupID(_groupsCount);
	//delete previous array
	if (_dataGroup != nullptr)
		delete[] _dataGroup;
	//point to new array
	_dataGroup = tempDataGroup;
	_groupsCount++;
	return _groupsCount - 1; //- 1 for index in array
}

uint8_t DataTransfererClass::AddGroup()
{
	DataGroupClass * tempDataGroup = new DataGroupClass[_groupsCount + 1];
	//copy all data to new array
	for (uint8_t i = 0; i < _groupsCount; i++)
		tempDataGroup[i] = _dataGroup[i];

	tempDataGroup[_groupsCount].SetGroupID(_groupsCount);
	//delete previous array
	if (_dataGroup != nullptr)
		delete[] _dataGroup;
	//point to new array
	_dataGroup = tempDataGroup;
	_groupsCount++;
	return _groupsCount - 1; //- 1 for index in array
}

DataGroupClass * DataTransfererClass::GetDataGroup(uint8_t index)
{
	if (index < _groupsCount)
		return &_dataGroup[index];
	return nullptr;
}
/*
uint8_t DataTransfererClass::GetGroupsCount()
{
	return _groupsCount;
}

uint8_t * DataTransfererClass::GetBytes(uint8_t groupIndex)
{
	return _dataGroup[groupIndex].GetBytes();
}
*/
DataTransfererClass DataTransferer;




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////



