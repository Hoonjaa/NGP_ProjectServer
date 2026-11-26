#include "Header/WorkerThread.h"
#include "Header/ServerTypes.h"
#include "Header/Protocol.h"
#include "Header/Common.h"
#include "PacketIO.h"
#include "Timer.h"
#include "Collision.h"
#include <type_traits>
#include <cstring>
#include <iostream>

DWORD WINAPI worker_thread(LPVOID arg)
{
	Timer logicTimer(60, Logic);

	JobContainer<PacketJob>* workContainer = static_cast<JobContainer<PacketJob>*>(arg);
	PacketJob job = {};
	while(1) // 우선은 작업 스레드가 컨테이너에서 작업을 꺼내 처리하는 루프만 구현.
	{
		/*
		재훈이가 서버클라 타이머 동기화 타이머 만들면
		워커 루프
			다음 틱까지 남은 시간만큼 패킷을 기다림
			패킷이 들어오면 처리
			사격 패킷이면 서버 월드에 총알을 추가
		틱 시간이 되면
			각 총알의 다음 위치를 계산
			이전 위치에서 다음 위치까지의 경로로 충돌 검사
			피격 결과를 대상 클라이언트의 완료 큐에 넣음
		처럼 구현해야함.
		*/
		logicTimer.Update();
		std::cout << "Logic TPS: " << logicTimer.GetTPS() << std::endl;


		if (!workContainer->TryPop(job))
			continue;

		const auto headerSize = sizeof(unsigned char) + sizeof(PACKET_TYPE);
		if (job.packet.size() < headerSize)
			continue;

		std::underlying_type_t<PACKET_TYPE> packetType = 0;
		//underlying_type_t<PACKET_TYPE>은 enum을 저장하는 데 쓰이는 정수 타입을 의미. 먼저 정수로 읽고 패킷타입과 비교

		std::memcpy( 
			&packetType,
			job.packet.data() + sizeof(unsigned char),
			sizeof(packetType)
		);

		if (packetType != C2S_LOGIN) {
			bool loggedIn = false;
			EnterCriticalSection(&cs);
			for (const auto& session : clients) {
				if (session && session->socket == job.client_sock &&
					session->player.PlayerID >= 0) {
					loggedIn = true;
					break;
				}
			}
			LeaveCriticalSection(&cs);
			if (!loggedIn) continue;
		}

		switch (packetType)
		{
		case C2S_LOGIN:         C2S_RecvLoginInfo(job);          break;
		case C2S_CHATINFO:		C2S_RecvChatInfo(job);			break;
		case C2S_MOVEINFO:		C2S_RecvMoveInfo(job);			break;
		case C2S_SHOOTINFO:		C2S_RecvShootIngInfo(job);		break;
		case C2S_FORWARDINFO:	C2S_RecvForwardInfo(job);		break;
		case C2S_RELOADING:		C2S_RecvReloadingInfo(job);		break;
		
		default:
			break;
		}
	}
	return 0;
}

void Logic()
{
	EnterCriticalSection(&cs);
	const auto targets = clients;
	LeaveCriticalSection(&cs);

	for (const auto& source : targets) {
		if (!source || source->player.PlayerID < 0)
			continue;

		source->player.Update();
		S2C_SendPlayerMoveInfo(source->player.PlayerID, source->player.Pos_x, source->player.Pos_z);
		S2C_SendOtherMoveInfo(source->player.PlayerID);
	
	}

	CheckBulletPlayerCollision(clients);
}

void PlayerInfo::Update()
{
	int forwardInput = w - s;
	int rightInput = d - a;

	float forwardX = std::cos(yaw);
	float forwardZ = std::sin(yaw);
	float rightX = -forwardZ;
	float rightZ = forwardX;

	float nextX = Pos_x + moveSpeed * (forwardX * forwardInput + rightX * rightInput);
	float nextZ = Pos_z + moveSpeed * (forwardZ * forwardInput + rightZ * rightInput);

	const bool inBounds =
		nextX >= -25.0f && nextX <= 25.0f &&
		nextZ >= -25.0f && nextZ <= 25.0f;

	if (inBounds && !CheckPlayerObstacleCollision(nextX, nextZ, BB.width * 0.5f)) {
		Pos_x = nextX;
		Pos_z = nextZ;
		BB.center.x = nextX;
		BB.center.z = nextZ;
	}

	for (auto bullet : bullets) {
		bullet.Update();
	}
}