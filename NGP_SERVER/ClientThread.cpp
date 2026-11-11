#include "Header/ClientThread.h"
#include "Header/ServerTypes.h"
#include "Header/Protocol.h"

#include <memory>

// 이 파일 안에서만 사용하는 송수신 보조 함수
namespace {
// 현재 도착한 바이트만 읽고, 완성된 패킷들을 공용 작업 큐에 등록
bool receive_packets(ClientSession& session, std::vector<unsigned char>& pending,
                     bool& progressed)
{
    unsigned char buffer[4096];
    const int received = recv(session.socket, reinterpret_cast<char*>(buffer),
                              sizeof(buffer), 0);
    // WSAEWOULDBLOCK은 지금 읽을 데이터가 없다는 뜻임.
    // recv가 0을 반환하면 상대의 연결 종료, 그 밖의 오류면 통신 실패
    if (received <= 0)
        return received == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK;

    progressed = true;
    // 이전에 덜 받은 패킷 뒤에 새 데이터를 붙입니다
    pending.insert(pending.end(), buffer, buffer + received);

    constexpr auto headerSize = sizeof(unsigned char) + sizeof(PACKET_TYPE);
    std::size_t consumed = 0;
    while (consumed < pending.size()) {
        const std::size_t packetSize = pending[consumed];
        if (packetSize < headerSize)
            return false;
        // 패킷이 아직 완성되지 않았으면 남겨둠
        if (pending.size() - consumed < packetSize)
            break;

        const unsigned char* begin = pending.data() + consumed;
        // 패킷 하나의 바이트를 복사하므로 수신 버퍼를 정리해도 워커의 데이터는 유지
        PacketJob job{session.socket, {begin, begin + packetSize}};
        if (!session.workContainer->job_push(job))
            return false;

        consumed += packetSize;
    }

    // 큐에 등록한 패킷만 지우고, 미완성 패킷은 다음 수신까지 보관
    pending.erase(pending.begin(), pending.begin() + consumed);
    return true;
}

// 완료 큐의 패킷을 한 번 송신
bool send_packet(ClientSession& session, std::vector<unsigned char>& pending,
                 std::size_t& sent, bool& progressed)
{
    if (pending.empty()) {
        if (!session.outgoing.TryPop(pending))
            return true;
        sent = 0;
        if (pending.empty())
            return true;
    }

    // sent는 이미 보낸 바이트 수입니다. 아직 보내지 못한 부분만 전송
    const int written = send(session.socket,
                             reinterpret_cast<const char*>(pending.data() + sent),
                             static_cast<int>(pending.size() - sent), 0);
    // 송신 공간 부족으로 WSAEWOULDBLOCK이 발생하면 데이터와 위치를 유지
    if (written <= 0)
        return written == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK;

    progressed = true;
    sent += written;

    // 패킷 전체를 보냈으므로 다음 완료 작업을 받을 수 있게 버퍼를 비웁니다.
    if (sent == pending.size()) {
        pending.clear();
        sent = 0;
    }
    return true;
}

}

DWORD WINAPI ProcessClient(LPVOID arg)
{
    std::shared_ptr<ClientSession> session(static_cast<ClientSession*>(arg));

    // recv와 send가 데이터나 버퍼 공간을 기다리며 스레드를 멈추지 않도록 설정
    u_long nonblocking = 1;
    if (ioctlsocket(session->socket, FIONBIO, &nonblocking) == SOCKET_ERROR) {
        closesocket(session->socket);
        return 0;
    }

    std::size_t slot = clients.size();
    EnterCriticalSection(&cs);
    for (std::size_t i = 0; i < clients.size(); ++i) {
        if (!clients[i]) {
            clients[i] = session;
            slot = i;
            break;
        }
    }
    LeaveCriticalSection(&cs);

    if (slot == clients.size()) {
        closesocket(session->socket);
        return 0;
    }

	// 반복 밖에 버퍼를 두어 수신과 송신을 번갈아 처리
    std::vector<unsigned char> pendingInput;
    std::vector<unsigned char> pendingOutput;
    std::size_t sent = 0;
	constexpr DWORD idleWaitMilliseconds = 10; // 수신과 송신이 없을 때 이벤트를 기다리는 시간

    for (;;) {
        // 같은 스레드에서 수신과 송신을 번갈아 처리
        // 어느 쪽이든 바이트를 처리했다면 대기 없이 다음 반복으로 진행
        bool progressed = false;
        if (!receive_packets(*session, pendingInput, progressed))
            break;
        if (!send_packet(*session, pendingOutput, sent, progressed))
            break;

        // 할 일이 없을 때 이벤트를 기다려 빈 반복을 줄이기. 워커의 job_push가 즉시 깨우고,
        // 10ms가 지나도 깨어나 수신과 송신을 다시 시도
        if (!progressed && WaitForSingleObject(session->outgoing.GetReadyEvent(),
                                               idleWaitMilliseconds) == WAIT_FAILED)
            break;
    }

    session->outgoing.Stop();
    EnterCriticalSection(&cs);
    clients[slot].reset();
    LeaveCriticalSection(&cs);

    closesocket(session->socket);
    return 0;
}
