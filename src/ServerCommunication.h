#pragma once

#include <stdio.h>
#include <Windows.h>
#include <WinInet.h>
#include <iostream>
#include <sstream>

#pragma comment( lib,"Wininet.lib")

class ServerCommunication {
	HINTERNET hSession, hConnect;
	LPWSTR hdrs;
public:
	ServerCommunication(void);
	~ServerCommunication(void);
	int SendLocation(int pawn, int x, int y);
};
