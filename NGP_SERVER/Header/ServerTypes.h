#pragma once

#include "JobContainer.h"

struct PacketJob { // 클라이언트별 스레드가 패킷을 받으면 이 구조체에 넣어 작업 스레드로 넘김.
	// 이렇게 구조체를 만들어서 넘기는 이유는, 작업 스레드가 패킷을 처리할 때 클라이언트 소켓이 필요하기 때문임.
	// 그냥 소켓만 넘기면 작업 스레드가 패킷을 처리할 때 어떤 클라이언트 소켓인지 알 수 없기 때문에 구조체를 만들어서 같이 넘김.
	// 반대로 패킷만 넘기면 작업 스레드가 패킷을 처리할 때 어떤 클라이언트 소켓인지 알 수 없기 때문에 구조체를 만들어서 같이 넘김.
    SOCKET client_sock; 
    std::vector<unsigned char> packet;
};

struct ClientSession { // 작업스레드가 패킷을 처리한 후 클라이언트별 스레드가 이 큐에서 패킷을 꺼내 클라이언트로 전송하기 위해 사용
	SOCKET socket;      // 클라이언트 소켓
	JobContainer<PacketJob>* workContainer; // 클라이언트 별 스레드가 패킷을 받으면 이 컨테이너에 작업을 넣어야함.
	JobContainer<std::vector<unsigned char>> outgoing; // 출력	패킷을 저장하는 큐, 작업 스레드가 패킷을 처리한 후 클라이언트별 스레드가 이 큐에서 패킷을 꺼내 클라이언트로 전송
};
