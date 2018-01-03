// DataGroup.h

#ifndef _DATAGROUP_h
#define _DATAGROUP_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#define MAX_DATA_SIZE 30	//MAX_PACKAGE_SIZE - 2 (for address and group id);
#define NO_ADDRESS 255



template<typename T>
static void toBytes(uint8_t buff[sizeof(T)], T val) {
	/*union {
		T typeVal;
		uint8_t bytes[sizeof(T)];
	} thing;
	thing.typeVal = val;
	memcpy(buff, thing.bytes, sizeof(T));*/
	*((T *)buff) = val;
}

template<typename T>
static T mergeBytes(uint8_t buff[sizeof(T)])
{
	return (*((T *)buff));
}


class DataGroupClass
{
protected:
	uint8_t _groupID = 0;
	uint8_t _groupSize = 0;			//number of bytes in each group
	uint8_t _groupMembersCount = 0;		//number of member in each group
	uint8_t _bytes[MAX_DATA_SIZE];		//data container
	uint8_t _groupMap[MAX_DATA_SIZE];	//data addresses container
public:
	DataGroupClass();
	void SetGroupID(uint8_t id);
	uint8_t GetGroupID();

	template<typename T>
	uint8_t AddAddress(T value = 0);

	uint8_t * GetGroupMap(uint8_t * size = nullptr);
	uint8_t GetGroupSize();
	uint8_t GetGroupMembersCount();
	
	void SetBytes(uint8_t arr[MAX_DATA_SIZE]);
	virtual uint8_t * GetBytes();

	template<typename T>
	boolean SetValue(uint8_t dataIndex, T val);

	template<typename T>
	T GetValue(uint8_t dataIndex);
};

template<typename T>
inline uint8_t DataGroupClass::AddAddress(T value = 0)
{
	uint8_t thisSize = sizeof(T);
	if (_groupSize + thisSize > MAX_DATA_SIZE)
		return NO_ADDRESS;

	uint8_t thisAddress = _groupSize;
	_groupSize += thisSize;
	_groupMap[_groupMembersCount] = thisAddress;
	_groupMembersCount++;
	//Serial.println("added add: " + String(thisAddress));
	SetValue<T>(_groupMembersCount-1, value);
	//return thisAddress;
	return _groupMembersCount-1;
}

template<typename T>
inline boolean DataGroupClass::SetValue(uint8_t dataIndex, T val)
{
	if (dataIndex >= _groupMembersCount)
		return false;

	uint8_t buff[sizeof(T)];
	toBytes<T>(buff, val);
	for (uint8_t i = 0; i < sizeof(T); i++)
		_bytes[_groupMap[dataIndex] + i] = buff[i];

	return true;
}

template<typename T>
inline T DataGroupClass::GetValue(uint8_t dataIndex)
{
	if (dataIndex >= _groupMembersCount)
		return 0;
	T i = mergeBytes<T>(&_bytes[_groupMap[dataIndex]]);
	//Serial.println(sizeof(T),HEX);
	return mergeBytes<T>(&_bytes[_groupMap[dataIndex]]);
}


//extern DataGroupClass DataGroup;
#endif

