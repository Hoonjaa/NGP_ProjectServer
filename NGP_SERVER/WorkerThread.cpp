#include "Header/WorkerThread.h"
#include "Header/ServerTypes.h"
#include "Header/Protocol.h"
#include "Header/Common.h"
#include "PacketIO.h"
#include <type_traits>
#include <cstring>



DWORD WINAPI worker_thread(LPVOID arg)
{
	JobContainer<PacketJob>* workContainer = static_cast<JobContainer<PacketJob>*>(arg);
	PacketJob job = {};
	while(workContainer->WaitPop(job)) // 우선은 작업 스레드가 컨테이너에서 작업을 꺼내 처리하는 루프만 구현.
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
		switch (packetType)
		{
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