#include "PacketIO.h"
#include "Header/Protocol.h"

// 민제
void C2S_RecvMoveInfo(PacketJob job) {}

// 재훈
void C2S_RecvShootIngInfo(PacketJob job) {}

// 민제
void C2S_RecvChatInfo(PacketJob job)
{
	if (job.packet.size() != sizeof(C2S_ChatInfo)) return;
	C2S_ChatInfo chat{};

	std::memcpy(&chat, job.packet.data(), sizeof(chat));// 패킷 데이터를 chat 구조체에 복사
	// 이렇게 하는 이유는 패킷을 구조체로 변환하여 사용하면 코드가 더 직관적이고 유지보수가 쉬워지기 때문임.

	S2C_Chat response{};
	response.size = sizeof(response);
	response.type = S2C_CHAT;
	response.player_id = chat.player_id;
	std::memcpy(response.msg, chat.msg, sizeof(chat.msg));

	const unsigned char* begin = reinterpret_cast<const unsigned char*>(&response);

	std::vector<unsigned char> responsePacket(begin, begin + sizeof(response));

	std::array<std::shared_ptr<ClientSession>, 3> targets{};

	EnterCriticalSection(&cs);
	targets = clients;
	LeaveCriticalSection(&cs);

	for (const auto& target : targets) {
		if (target && target->socket != job.client_sock) {
			target->outgoing.job_push(responsePacket);
		}
	}
}

// 민제
void C2S_RecvForwardInfo(PacketJob job) {}

// 민제
void C2S_RecvReloadingInfo(PacketJob job) {}

void S2C_SendOtherMoveInfo()
{
	EnterCriticalSection(&cs);
	auto targets = clients;
	LeaveCriticalSection(&cs);

	for (const auto& source : targets) {
		if (!source) continue;

		const auto& player = source->player;
		S2C_OtherMove response{};
		response.size = sizeof(response);
		response.type = S2C_OTHERMOVE;
		response.player_id = static_cast<short>(player.PlayerID);
		response.x = player.Pos_x;
		response.y = player.Pos_y;

		std::vector<unsigned char> responsepacket(sizeof(response));
		std::memcpy(responsepacket.data(), &response, sizeof(response));

		for (const auto& target : targets) {
			if (target && target != source) {
				target->outgoing.job_push(responsepacket);
			}
		}
	}
}