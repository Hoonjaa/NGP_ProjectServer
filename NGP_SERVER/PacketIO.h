#pragma once
#include "Header/ServerTypes.h"

void C2S_RecvMoveInfo(PacketJob job);
void C2S_RecvShootIngInfo(PacketJob job);
void C2S_RecvChatInfo(PacketJob job);
void C2S_RecvForwardInfo(PacketJob job);
void C2S_RecvReloadingInfo(PacketJob job);

void S2C_SendOtherMoveInfo();