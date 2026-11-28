#include "Collision.h"
#include "PacketIO.h"
#include <algorithm>
void CheckBulletPlayerCollision(std::array<std::shared_ptr<ClientSession>, 3> clients)
{
	for (std::shared_ptr<ClientSession> FirstPlayer : clients) {
		if (!FirstPlayer) continue;
        auto& bullets = FirstPlayer->player.bullets;

        for (auto it = bullets.begin(); it != bullets.end(); ) {
			std::shared_ptr<ClientSession> targetPlayer = nullptr;
            for (const auto& target : clients) {
                if (!target || target == FirstPlayer) continue;
                if (!(*it)->BB.Intersects(target->player.BB)) continue;

				targetPlayer = target;
                break;
            }
            if (!targetPlayer) {
                ++it;
                continue;
            }
            delete* it;
            it = bullets.erase(it);
            if (targetPlayer->player.Mujuck_time <= 0) {
                int Damage = 20;
                if (targetPlayer->player.HP > Damage) {
                    targetPlayer->player.HP -= Damage;
                    S2C_SendPlayerHPInfo(targetPlayer->player.PlayerID, targetPlayer->player.HP);
                }
                else {
                    targetPlayer->player.HP = 0;
                    S2C_SendDeathInfo(targetPlayer->player.PlayerID, FirstPlayer->player.PlayerID);
                }
            }
        }
	}
}

bool CheckPlayerObstacleCollision(float x, float z, float radius)
{
    static const Obstacle obstacles;

    for (const auto& corners : obstacles.GetCorners()) {
        if (IntersectsObstacle(x, z, radius, corners))
            return true;
    }

    return false;
}

bool IntersectsObstacle(float x, float z, float radius, const Obstacle::Corner& corners)
{
    const auto& origin = corners[0];
    const float dx = x - origin.x;
    const float dz = z - origin.z;

    const float ux = corners[1].x - origin.x;
    const float uz = corners[1].z - origin.z;
    const float vx = corners[3].x - origin.x;
    const float vz = corners[3].z - origin.z;

    // 각 변에서 0은 시작점, 1은 끝점입니다.
    const float u = (dx * ux + dz * uz) / (ux * ux + uz * uz);
    const float v = (dx * vx + dz * vz) / (vx * vx + vz * vz);
    const float clampedU = (std::min)(1.0f, (std::max)(0.0f, u));
    const float clampedV = (std::min)(1.0f, (std::max)(0.0f, v));

    const float gapX = dx - (clampedU * ux + clampedV * vx);
    const float gapZ = dz - (clampedU * uz + clampedV * vz);

    return gapX * gapX + gapZ * gapZ <= radius * radius;
}