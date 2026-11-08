#include "Header/ClientThread.h"
#include "Header/Protocol.h"
#include "Header/ServerTypes.h"
#include "Header/WorkerThread.h"
#include <memory>
// --------------------------------------------------------------전역변수--------------------------------------------------
CRITICAL_SECTION cs;
std::vector<HANDLE> threads;
//-------------------------------------------------------------------------------------------------------------------------
int main()
{
	//--------------------------------------------------------------서버 초기화--------------------------------------------------
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
	//-------------------------------------------------------------------------------------------------------------------------
	InitializeCriticalSection(&cs); // 크리티컬 섹션 초기화
	JobContainer<PacketJob> Work_Container; // 클라별 스레드들이 패킷을 받으면 이 컨테이너에 작업을 넣음. 작업 스레드가 이 컨테이너에서 작업을 꺼내 처리함.
	HANDLE worker = CreateThread(NULL, 0, worker_thread, &Work_Container, 0, NULL); //작업 스레드 생성

    while (1) {
		// 종료된 스레드 핸들 닫고 벡터에서 제거
		for (auto it = threads.begin(); it != threads.end();) {
            if (WaitForSingleObject(*it, 0) == WAIT_OBJECT_0) { 
                CloseHandle(*it);
                it = threads.erase(it);
            }
            else ++it;
        }

		// 클라이언트 연결 수락, 완료 컨테이너 생성
        struct sockaddr_in clientaddr = {};
        int addrlen = sizeof(clientaddr);
        SOCKET client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) break;

        auto session = std::make_unique<ClientSession>();
        session->socket = client_sock;
		session->workContainer = &Work_Container;
        ClientSession* session_arg = session.release();
        HANDLE clientThread = CreateThread(NULL, 0, ProcessClient, session_arg, 0, NULL);
        if (clientThread == NULL) {
            session.reset(session_arg);
            closesocket(client_sock);
        }
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