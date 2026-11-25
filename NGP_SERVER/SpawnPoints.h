#pragma once

#include <array>
#include <memory>

struct ClientSession;

struct SpawnPoint {
    float x;
    float z;
};

bool TrySelectSpawnPoint(const std::array<std::shared_ptr<ClientSession>,3>& sessions, SpawnPoint& spawn);
