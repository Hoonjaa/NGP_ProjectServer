#include "Header/WorkerThread.h"
#include "Header/ServerTypes.h"

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

	}
	return 0;
}
