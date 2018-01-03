// 
// 
// 

#include "DataGroup.h"

DataGroupClass::DataGroupClass()
{
	memset(_bytes, 0, MAX_DATA_SIZE);
}

void DataGroupClass::SetGroupID(uint8_t id)
{
	//Serial.println("Set GRID: " + String(id));
	_groupID = id;
}

uint8_t DataGroupClass::GetGroupID()
{
	//Serial.println("Get GRID: " + String(_groupID));
	return _groupID;
}

uint8_t * DataGroupClass::GetGroupMap(uint8_t * size)
{
	if (size != nullptr)
		*size = _groupMembersCount;
	return _groupMap;
}

uint8_t DataGroupClass::GetGroupSize()
{
	return _groupSize;
}

uint8_t DataGroupClass::GetGroupMembersCount()
{
	return _groupMembersCount;
}

uint8_t * DataGroupClass::GetBytes()
{
	return _bytes;
}

void DataGroupClass::SetBytes(uint8_t arr[MAX_DATA_SIZE])
{
	for (uint8_t i = 0; i < MAX_DATA_SIZE; i++)
		_bytes[i] = arr[i];
}
