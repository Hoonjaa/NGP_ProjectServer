#pragma once
#include "Header/ServerTypes.h"

void C2S_RecvLoginInfo(PacketJob job);
void C2S_RecvMoveInfo(PacketJob job);
void C2S_RecvShootInfo(PacketJob job);
void C2S_RecvChatInfo(PacketJob job);
void C2S_RecvForwardInfo(PacketJob job);
void C2S_RecvReloadingInfo(PacketJob job);

void S2C_SendPlayerMoveInfo(short player_id,float,float); // id에 해당하는 플레이어에게 자신의 위치를 전송
void S2C_SendOtherMoveInfo(short player_id); // 다른 플레이어에게 자신의 위치를 전송
void S2C_SendPlayerHPInfo(short player_id, int HP);

void S2C_SendDeathInfo(short player_id, short killer_id);
void S2C_SendRespawnInfo(short player_id, float x, float y);
void S2C_SendReleaseInvincibilityInfo(short player_id);
void S2C_SendOtherForwardInfo();

void S2C_SendOtherShootInfo(short player_id, float3 pos);
void S2C_SendBulletMoveInfo(short player_id, unsigned int index, float3 pos);
void S2C_SendBulletDeleteInfo(short player_id, unsigned int index);

void S2C_LogoutClient(SOCKET socket);