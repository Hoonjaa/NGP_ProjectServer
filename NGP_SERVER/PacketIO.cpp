#include <cmath>
#include "PacketIO.h"
#include "Header/Protocol.h"
#include "Collision.h"

void C2S_RecvLoginInfo(PacketJob job) 
{
	if (job.packet.size() != sizeof(C2S_Login)) return;

	C2S_Login login{};
	std::memcpy(&login, job.packet.data(), sizeof(login));
	if (login.size != sizeof(login) || login.type != C2S_LOGIN) return;

	EnterCriticalSection(&cs);
	auto targets = clients;
	LeaveCriticalSection(&cs);

	std::shared_ptr<ClientSession> source;
	std::array<bool, 3> usedIDs{};
	for (const auto& target : targets) {
		if (!target) continue;
		if (target->socket == job.client_sock) source = target;
		const short id = target->player.PlayerID;
		if (id >= 0 && static_cast<std::size_t>(id) < usedIDs.size())
			usedIDs[id] = true;
	}

	if (!source || source->player.PlayerID >= 0) return;
	short playerID = -1;
	for (std::size_t i = 0; i < usedIDs.size(); ++i) {
		if (!usedIDs[i]) {
			playerID = static_cast<short>(i);
			break;
		}
	}
	if (playerID < 0) return;

	source->player.PlayerID = playerID;
	std::memcpy(source->player.name, login.name, sizeof(source->player.name));
	source->player.name[sizeof(source->player.name) - 1] = '\0';

	const auto queuePacket = [](ClientSession& target, const auto& packet) {
		std::vector<unsigned char> bytes(sizeof(packet));
		std::memcpy(bytes.data(), &packet, sizeof(packet));
		target.outgoing.job_push(bytes);
	};

	S2C_LoginResult result{}; // 로그인 결과를 새로 로그인한 클라이언트에게 전송
	result.size = static_cast<unsigned char>(sizeof(result));
	result.type = S2C_LOGINRESULT;
	result.player_id = playerID;
	queuePacket(*source, result);

	S2C_OtherLogin newcomer{}; // 새로운 플레이어 정보를 다른 클라이언트에게 전송
	newcomer.size = static_cast<unsigned char>(sizeof(newcomer));
	newcomer.type = S2C_OTHERLOGIN;
	newcomer.player_id = playerID;
	std::memcpy(newcomer.name, source->player.name, sizeof(newcomer.name));

	for (const auto& target : targets) {
		if (!target || target == source || target->player.PlayerID < 0) continue;
		queuePacket(*target, newcomer);

		S2C_OtherLogin existing{}; // 기존 플레이어 정보를 새로 로그인한 클라이언트에게 전송
		existing.size = static_cast<unsigned char>(sizeof(existing));
		existing.type = S2C_OTHERLOGIN;
		existing.player_id = target->player.PlayerID;
		std::memcpy(existing.name, target->player.name, sizeof(existing.name));
		queuePacket(*source, existing);
	}
}
// 민제
void C2S_RecvMoveInfo(PacketJob job) 
{
	if (job.packet.size() != sizeof(C2S_MoveInfo)) return;
	C2S_MoveInfo response{};
	std::memcpy(&response, job.packet.data(), sizeof(response));

	EnterCriticalSection(&cs);
	auto targets = clients;
	LeaveCriticalSection(&cs);
	for (const auto& source : targets) {
		if (!source || source->socket != job.client_sock) continue;
		
		source->player.w = response.w;
		source->player.a = response.a;
		source->player.s = response.s;
		source->player.d = response.d;
	}
}

// 재훈
void C2S_RecvShootIngInfo(PacketJob job) {}

// 민제
void C2S_RecvChatInfo(PacketJob job)
{
	if (job.packet.size() != sizeof(C2S_ChatInfo)) return;
	C2S_ChatInfo chat{};

	std::memcpy(&chat, job.packet.data(), sizeof(chat));// 패킷 데이터를 chat 구조체에 복사
	// 이렇게 하는 이유는 패킷을 구조체로 변환하여 사용하면 코드가 더 직관적이고 유지보수가 쉬워지기 때문임.


	printf(
		"Received chat from player %d: %.*s\n",
		chat.player_id,
		MAX_CHAT_MSG_LEN,
		chat.msg
	);

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
		if (target && target->player.PlayerID >= 0 && target->socket != job.client_sock) {
			target->outgoing.job_push(responsePacket);
		}
	}
}

// 민제
void C2S_RecvForwardInfo(PacketJob job) 
{
	if (job.packet.size() != sizeof(C2S_ForwardInfo)) return;
	C2S_ForwardInfo response{};
	std::memcpy(&response, job.packet.data(), sizeof(response));

	EnterCriticalSection(&cs);
	auto targets = clients;
	LeaveCriticalSection(&cs);
	for (const auto& source : targets) { 
		if (!source || source->socket != job.client_sock) continue;
		source->player.pitch = response.pitch;
		source->player.yaw = response.yaw;
	}
	S2C_SendOtherForwardInfo();
}

// 민제
void C2S_RecvReloadingInfo(PacketJob job) {}

void S2C_SendPlayerMoveInfo(short player_id, float x, float z)
{
	EnterCriticalSection(&cs);
	auto targets = clients;
	LeaveCriticalSection(&cs);

	S2C_OtherMove response{};
	response.size = sizeof(response);
	response.type = S2C_OTHERMOVE;
	response.player_id = static_cast<short>(player_id);
	response.x = x;
	response.z = z;

	std::vector<unsigned char> responsepacket(sizeof(response));
	std::memcpy(responsepacket.data(), &response, sizeof(response));

	for (const auto& target : targets) {
		if (target && target->player.PlayerID >= 0 && target->player.PlayerID == player_id) {
			target->outgoing.job_push(responsepacket);
		}
	}
}

void S2C_SendOtherMoveInfo(short player_id)
{
	EnterCriticalSection(&cs);
	auto targets = clients;
	LeaveCriticalSection(&cs);
	float x = 0.0f, z = 0.0f;

	for (const auto& target : targets) {
		if (!target || target->player.PlayerID < 0 || target->player.PlayerID != player_id) continue;
		x = target->player.Pos_x;
		z = target->player.Pos_z;
	}

	S2C_OtherMove response{};
	response.size = sizeof(response);
	response.type = S2C_OTHERMOVE;
	response.player_id = static_cast<short>(player_id);
	response.x = x;
	response.z = z;

	std::vector<unsigned char> responsepacket(sizeof(response));
	std::memcpy(responsepacket.data(), &response, sizeof(response));

	for (const auto& target : targets) {
		if (target && target->player.PlayerID >= 0 && target->player.PlayerID != player_id) {
			target->outgoing.job_push(responsepacket);
		}
	}	
}

void S2C_SendPlayerHPInfo(short player_id, int HP)
{
	EnterCriticalSection(&cs);
	auto targets = clients;
	LeaveCriticalSection(&cs);
	S2C_HitInfo response{};
	response.size = sizeof(response);
	response.type = S2C_HITINFO;
	response.player_id = player_id;
	response.HP = HP;
	std::vector<unsigned char> responsepacket(sizeof(response));
	std::memcpy(responsepacket.data(), &response, sizeof(response));
	for (const auto& target : targets) {
		if (target && target->player.PlayerID >= 0) {
			target->outgoing.job_push(responsepacket);
		}
	}
	
}

void S2C_SendDeathInfo(short player_id, short killer_id)
{
	EnterCriticalSection(&cs);
	auto targets = clients;
	LeaveCriticalSection(&cs);
	S2C_Death response{};
	response.size = sizeof(response);
	response.type = S2C_DEATH;
	response.player_id = player_id;
	response.killer_id = killer_id;
	std::vector<unsigned char> responsepacket(sizeof(response));
	std::memcpy(responsepacket.data(), &response, sizeof(response));
	for (const auto& target : targets) {
		if (target && target->player.PlayerID >= 0) {
			target->outgoing.job_push(responsepacket);
		}
	}
	
}

void S2C_SendRespawnInfo(short player_id, float x, float y)
{
	EnterCriticalSection(&cs);
	auto targets = clients;
	LeaveCriticalSection(&cs);
	S2C_Respawn response{};
	response.size = sizeof(response);
	response.type = S2C_REASPAWN;
	response.player_id = player_id;
	response.x = x;
	response.z = y;
	std::vector<unsigned char> responsepacket(sizeof(response));
	std::memcpy(responsepacket.data(), &response, sizeof(response));
	for (const auto& target : targets) {
		if (target && target->player.PlayerID >= 0) {
			target->outgoing.job_push(responsepacket);
		}
	}
	
}

void S2C_SendReleaseInvincibilityInfo(short player_id)
{
	EnterCriticalSection(&cs);
	auto targets = clients;
	LeaveCriticalSection(&cs);
	S2C_ReleaseInvincibility response{};
	response.size = sizeof(response);
	response.type = S2C_RELEASEINVINCIBILITY;
	response.player_id = player_id;
	std::vector<unsigned char> responsepacket(sizeof(response));
	std::memcpy(responsepacket.data(), &response, sizeof(response));
	for (const auto& target : targets) {
		if (target && target->player.PlayerID >= 0) {
			target->outgoing.job_push(responsepacket);
		}
	}
	
}

void S2C_SendOtherForwardInfo()
{
	EnterCriticalSection(&cs);
	auto targets = clients;
	LeaveCriticalSection(&cs);

	for (const auto& source : targets) {
		if (!source || source->player.PlayerID < 0) continue;
		const auto& player = source->player;
		S2C_OtherForward response{};
		response.size = sizeof(response);
		response.type = S2C_OTHERFORWARD;
		response.player_id = static_cast<short>(player.PlayerID);
		response.pitch = player.pitch;
		response.yaw = player.yaw;
		std::vector<unsigned char> responsepacket(sizeof(response));
		std::memcpy(responsepacket.data(), &response, sizeof(response));
		for (const auto& target : targets) {
			if (target && target->player.PlayerID >= 0 && target != source) {
				target->outgoing.job_push(responsepacket);
			}
		}
	}
}