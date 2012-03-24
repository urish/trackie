#include "ServerCommunication.h"

ServerCommunication::ServerCommunication(void)
{
	hdrs = L"Content-Type: application/x-www-form-urlencoded";
	hSession = InternetOpenW(L"http generic",INTERNET_OPEN_TYPE_PROXY, L"http://127.0.0.2:8888", L"<-loopback>", 0);
	hConnect = InternetConnectW(hSession, L"localhost",8080, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1);
}

ServerCommunication::~ServerCommunication(void)
{
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);
}
char* getheaders(HINTERNET hRequest){
    DWORD dwInfoLevel=HTTP_QUERY_RAW_HEADERS_CRLF;
    DWORD dwInfoBufferLength=10;
    char* pInfoBuffer=(char*)malloc(dwInfoBufferLength+1);
    while(!HttpQueryInfo(hRequest,dwInfoLevel,pInfoBuffer,&dwInfoBufferLength,NULL)){
        if (GetLastError()==ERROR_INSUFFICIENT_BUFFER){
            free(pInfoBuffer);
            pInfoBuffer=(char*)malloc(dwInfoBufferLength+1);
        }else{
            fprintf(stderr,"HttpQueryInfo failed, error = %d (0x%x)\n",GetLastError(),GetLastError());
            break;
        }
    }
    pInfoBuffer[dwInfoBufferLength] = '\0';
    return pInfoBuffer;
}

void readfile(HINTERNET hRequest,char** buffs,int size){
    DWORD dwBytesAvailable;
    DWORD dwBytesRead;
    for(int i=0;i<size;i++){
        if(!InternetQueryDataAvailable(hRequest,&dwBytesAvailable,0,0)) break;
        buffs[i]=(char*)malloc(dwBytesAvailable+1);
        bool bResult=InternetReadFile(hRequest,buffs[i],dwBytesAvailable,&dwBytesRead);
        if(!bResult | dwBytesRead==0) break;
    }
}

int ServerCommunication::SendLocation(int pawn, int x, int y)
{
	LPCWSTR accept[2]={L"*/*", NULL};

	std::stringstream position;
	position << L"x=" << x << L"&y=" << y;
    LPSTR frmdata=(LPSTR)position.str().c_str();

	std::stringstream pawnString;
	pawnString << L"/app/pawns/" << pawn;
    LPCWSTR query=(LPCWSTR)pawnString.str().c_str();

    HINTERNET hRequest = HttpOpenRequestW(hConnect, L"POST",query, NULL, NULL, accept, 0, 0);
    //ERROR_INSUFFICIENT_BUFFER (122) with "accept".

    int send=HttpSendRequestW(hRequest, hdrs, wcslen(hdrs), frmdata,strlen(frmdata));
    if(!send){
        printf("HttpSendRequest failed, code=%d",GetLastError());
        system("pause>nul");
        return 0;
    }

    char* heads=getheaders(hRequest);
    printf("%S\n\n\n\n",heads);

	InternetCloseHandle(hRequest);
}


