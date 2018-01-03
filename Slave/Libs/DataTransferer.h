// DataTransferer.h
/*
TODO:
	- Add number of groups and group sync at start up

*/
#ifndef _DATATRANSFERER_h
#define _DATATRANSFERER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "DataGroup.h"

class DataTransfererClass : public DataGroupClass
{
protected:
	//uint8_t _I2CAddress = 0;
	//Role _role = Role::SLAVE;
	uint8_t _groupsCount = 0;	//number of groups
	DataGroupClass * _dataGroup = nullptr;
public:
	//void init(Role role, uint8_t address = 0);
	uint8_t AddGroup(const DataGroupClass & dataGr);
	uint8_t AddGroup();

	DataGroupClass * GetDataGroup(uint8_t index);

	/*template<typename T>
	uint8_t AddAddress(uint8_t groupIndex, T value = 0);

	uint8_t GetGroupsCount();

	virtual uint8_t * GetBytes(uint8_t groupIndex);

	template<typename T>
	boolean SetValue(uint8_t groupIndex, uint8_t dataIndex, T val);

	template<typename T>
	T GetValue(uint8_t groupIndex, uint8_t dataIndex);*/
};

extern DataTransfererClass DataTransferer;

#endif


/*
template<typename T>
inline uint8_t DataTransfererClass::AddAddress(uint8_t groupIndex, T value = 0)
{
	if (groupIndex >= _groupsCount)
		return NO_ADDRESS;
	
	return _dataGroup[groupIndex].AddAddress<T>(value);
}

template<typename T>
inline boolean DataTransfererClass::SetValue(uint8_t groupIndex, uint8_t dataIndex, T val)
{
	if (groupIndex >= _groupsCount)
		return false;
	if (dataIndex >= _dataGroup[groupIndex].GetGroupMembersCount())
		return false;
	_dataGroup[groupIndex].SetValue<T>(dataIndex, val);
	return true;
}

template<typename T>
inline T DataTransfererClass::GetValue(uint8_t groupIndex, uint8_t dataIndex)
{
	if (groupIndex >= _groupsCount)
		return 0;
	if (dataIndex >= _dataGroup[groupIndex].GetGroupMembersCount())
		return 0;
	return _dataGroup[groupIndex].GetValue<T>(dataIndex);
}*/
