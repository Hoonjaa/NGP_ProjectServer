#include "Header/Common.h"
#include "Header/Protocol.h"
// --------------------------------------------------------------전역변수--------------------------------------------------
CRITICAL_SECTION cs;
std::vector<HANDLE> threads;
//-------------------------------------------------------------------------------------------------------------------------
DWORD WINAPI ProcessClient(LPVOID arg) //클라이언트 연결 스레드가 수행할 함수.
{

}

void worker_thread() //작업 스레드.
{

}

int main()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");
    struct sockaddr_in serveraddr = {};
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(PORT);

    if (bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
        err_quit("bind()");
    if (listen(listen_sock, SOMAXCONN) == SOCKET_ERROR) err_quit("listen()");

    InitializeCriticalSection(&cs);


    while (1) {
		for (auto it = threads.begin(); it != threads.end();) {
            if (WaitForSingleObject(*it, 0) == WAIT_OBJECT_0) { 
                CloseHandle(*it);
                it = threads.erase(it);
            }
            else ++it;
        }

        struct sockaddr_in clientaddr = {};
        int addrlen = sizeof(clientaddr);
        SOCKET client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) break;
        HANDLE clientThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock, 0, NULL); //클라이언트별 스레드 생성

        if (clientThread == NULL) closesocket(client_sock);
        else threads.push_back(clientThread);
    }

    closesocket(listen_sock);



    for (HANDLE hThread : threads) {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }

    DeleteCriticalSection(&cs);
    WSACleanup();

    return 0;

}