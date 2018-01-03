// UserManager.h

#ifndef _USERMANAGER_h
#define _USERMANAGER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

enum LoginState {
	LOGGED_IN, 
	LOGGED_OUT
};

struct User {
	String name = "NULL";
	uint8_t ID = 0;
	String tagID = "00000000";
	LoginState loginStatus = LoginState::LOGGED_OUT;
};
class UserManagerClass
{
 protected:
	uint8_t _usersCount;
	User * _users = nullptr;

 public:
	void init();
	uint8_t AddUser(String name, String tagID);
	User * GetUser(uint8_t userID);
	uint8_t GetUsersCount();
	//void 
};

extern UserManagerClass UserManager;

#endif

