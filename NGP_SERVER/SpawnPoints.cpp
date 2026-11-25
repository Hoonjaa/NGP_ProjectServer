#include "SpawnPoints.h"
#include "Header/ServerTypes.h"

#include <algorithm>
#include <random>

namespace {
constexpr std::array<SpawnPoint, 5> spawnPoints{{
    {-20.0f, -20.0f},
    { 20.0f, -15.0f},
    { 10.0f,  20.0f},
    {-15.0f,  15.0f},
    {  5.0f,   5.0f},
}};
constexpr float enemyExclusionRadius = 5.0f; // 이 거리 안에 있으면 스폰 포인트 지정 x

bool IsEnemyNear(const ClientSession& session, const SpawnPoint& point)
{
    const auto& player = session.player;
    if (player.PlayerID < 0 || player.HP <= 0.0f) return false;

    const float dx = player.Pos_x - point.x;
    const float dz = player.Pos_z - point.z;
    return dx * dx + dz * dz <= enemyExclusionRadius * enemyExclusionRadius;
}
}

bool TrySelectSpawnPoint(const std::array<std::shared_ptr<ClientSession>,3>&sessions, SpawnPoint& spawn)
{
    //none_of는 조건을 만족하는 세션이 하나도 없으면 true를 반환, copy_if가 그 지점을 available에 복사
    std::array<SpawnPoint, spawnPoints.size()> available;
	const auto end = std::copy_if(spawnPoints.begin(), spawnPoints.end(), available.begin(), [&](const SpawnPoint& point) { 
		return std::none_of(sessions.begin(), sessions.end(), [&](const auto& session) { 
                return session && IsEnemyNear(*session, point);  
            });
        });
    if (end == available.begin()) return false; 

    static std::mt19937 randomEngine(std::random_device{}());
    const auto count = static_cast<std::size_t>(end - available.begin());
    std::uniform_int_distribution<std::size_t> pick(0, count - 1);
    spawn = available[pick(randomEngine)];
    return true;
}
