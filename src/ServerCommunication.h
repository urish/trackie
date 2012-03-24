#pragma once

#include <stdio.h>
#include <Windows.h>
#include <WinInet.h>
#include <iostream>
#include <sstream>

class ServerCommunication {
	HINTERNET hSession, hConnect;
	LPCWSTR hdrs;
public:
	ServerCommunication(void);
	~ServerCommunication(void);
	int SendLocation(int pawn, float x, float y);
};
