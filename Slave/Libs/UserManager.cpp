// 
// 
// 

#include "UserManager.h"

void UserManagerClass::init()
{


}

uint8_t UserManagerClass::AddUser(String name, String tagID)
{
	User * tempUsers = new User[_usersCount + 1];

	for (uint8_t i = 0; i < _usersCount; i++)
		tempUsers[i] = _users[i];

	tempUsers[_usersCount].name = name;
	tempUsers[_usersCount].ID = _usersCount;
	tempUsers[_usersCount].tagID = tagID;

	if (_users != nullptr)
		delete[] _users;

	_users = tempUsers;
	_usersCount++;

	return _users[_usersCount - 1].ID;
}

User * UserManagerClass::GetUser(uint8_t userID)
{
	if(userID >= _usersCount)
		return nullptr;
	return &_users[userID];
}

uint8_t UserManagerClass::GetUsersCount()
{
	return _usersCount;
}


UserManagerClass UserManager;

