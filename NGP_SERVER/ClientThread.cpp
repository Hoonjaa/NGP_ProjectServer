#include "Header/ClientThread.h"
#include "Header/ServerTypes.h"
#include "Header/Protocol.h"
#include <memory>

DWORD WINAPI ProcessClient(LPVOID arg)
{
    std::shared_ptr<ClientSession> session(static_cast<ClientSession*>(arg)); // session을 스마트 포인터로 관리

    unsigned char packet_size = 0;

	int recved_bytes = recv(session->socket, reinterpret_cast<char*>(&packet_size), sizeof(packet_size), 0); // packet_size에 패킷 크기 수신
    if (recved_bytes != sizeof(packet_size)) {
        closesocket(session->socket); // 받은 바이트 수가 예상과 다르면 연결 끊기
		return 0;
    }

	const auto headersize = sizeof(packet_size) + sizeof(PACKET_TYPE); // 패킷 헤더 크기 계산
	if (packet_size < headersize) {
        closesocket(session->socket); // 패킷 크기가 헤더 크기보다 작으면 연결 끊기
		return 0;
	}

	std::vector<unsigned char> packet(packet_size); // 패킷 크기만큼 벡터 생성
	packet[0] = packet_size; // 패킷 크기 저장

	int total_receved = 1; // 이미 받은 바이트 수 (패킷 크기 1바이트)

	while (total_receved < packet_size) {
		const int recved = recv(session->socket, reinterpret_cast<char*>(packet.data() + total_receved), 
								packet_size - total_receved, MSG_WAITALL);
		if (recved <= 0) break;

		total_receved += recved; // 총 받은 바이트 수 업데이트
	}

	if(total_receved != packet_size) {
		closesocket(session->socket); // 받은 바이트 수가 패킷 크기와 다르면 연결 끊기

    closesocket(session->socket);
    return 0;
}
