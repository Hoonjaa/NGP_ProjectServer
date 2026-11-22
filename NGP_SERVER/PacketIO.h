#pragma once
#include "Header/ServerTypes.h"

void C2S_RecvLoginInfo(PacketJob job);
void C2S_RecvMoveInfo(PacketJob job);
void C2S_RecvShootIngInfo(PacketJob job);
void C2S_RecvChatInfo(PacketJob job);
void C2S_RecvForwardInfo(PacketJob job);
void C2S_RecvReloadingInfo(PacketJob job);

void S2C_SendOtherMoveInfo(short player_id);
void S2C_SendPlayerHPInfo(short player_id, int HP);

void S2C_SendDeathInfo(short player_id, short killer_id);
void S2C_SendRespawnInfo(short player_id, float x, float y);
void S2C_SendReleaseInvincibilityInfo(short player_id);