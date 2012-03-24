#include "ServerCommunication.h"

ServerCommunication::ServerCommunication(void) {
	hdrs = L"Content-Type: application/x-www-form-urlencoded";
	hSession = InternetOpenW(L"http generic", INTERNET_OPEN_TYPE_PROXY,
			L"http://127.0.0.2:8888", L"<-loopback>", 0);
	hConnect = InternetConnectW(hSession, L"localhost", 8080, NULL, NULL,
			INTERNET_SERVICE_HTTP, 0, 1);
}

ServerCommunication::~ServerCommunication(void) {
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hSession);
}
char* getheaders(HINTERNET hRequest) {
	DWORD dwInfoLevel = HTTP_QUERY_RAW_HEADERS_CRLF;
	DWORD dwInfoBufferLength = 10;
	char* pInfoBuffer = (char*) malloc(dwInfoBufferLength + 1);
	while (!HttpQueryInfo(hRequest, dwInfoLevel, pInfoBuffer,
			&dwInfoBufferLength, NULL)) {
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			free(pInfoBuffer);
			pInfoBuffer = (char*) malloc(dwInfoBufferLength + 1);
		} else {
			fprintf(stderr, "HttpQueryInfo failed, error = %lu (0x%x)\n",
					GetLastError(), (UINT) GetLastError());
			break;
		}
	}
	pInfoBuffer[dwInfoBufferLength] = '\0';
	return pInfoBuffer;
}

void readfile(HINTERNET hRequest, char** buffs, int size) {
	DWORD dwBytesAvailable;
	DWORD dwBytesRead;
	for (int i = 0; i < size; i++) {
		if (!InternetQueryDataAvailable(hRequest, &dwBytesAvailable, 0, 0))
			break;
		buffs[i] = (char*) malloc(dwBytesAvailable + 1);
		bool bResult = InternetReadFile(hRequest, buffs[i], dwBytesAvailable,
				&dwBytesRead);
		if (!bResult | (dwBytesRead == 0)) {
			break;
		}
	}
}

int ServerCommunication::SendLocation(int pawn, float x, float y) {
	LPCWSTR accept[2] = { L"*/*", NULL };

	char frmdata[100];
	sprintf(frmdata, "x=%f&y=%f", x, y);

	WCHAR path[100];
	wsprintfW(path, L"/app/pawns/%d", pawn);

	HINTERNET hRequest = HttpOpenRequestW(hConnect, L"POST", path, NULL, NULL,
			accept, 0, 0);
	if (hRequest == NULL) {
		fprintf(stderr, "HttpOpenRequest failed, code=%lu", GetLastError());
		return 1;
	}

	int send = HttpSendRequestW(hRequest, hdrs, wcslen(hdrs), frmdata,
			strlen(frmdata));
	if (!send) {
		fprintf(stderr, "HttpSendRequest failed, code=%lu", GetLastError());
		return 1;
	}

	char* heads = getheaders(hRequest);
	printf("%s\n\n\n\n", heads);

	InternetCloseHandle(hRequest);
	return 0;
}

