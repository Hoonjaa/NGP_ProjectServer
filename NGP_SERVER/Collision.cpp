#include "Collision.h"
#include "PacketIO.h"
void CheckBulletPlayerCollision(std::array<std::shared_ptr<ClientSession>, 3> clients)
{
	for (std::shared_ptr<ClientSession> FirstPlayer : clients) {
		if (!FirstPlayer) continue;
        auto& bullets = FirstPlayer->player.bullets;

        for (auto it = bullets.begin(); it != bullets.end(); ) {
			std::shared_ptr<ClientSession> targetPlayer = nullptr;
            for (const auto& target : clients) {
                if (!target || target == FirstPlayer) continue;
                if (!it->BB.Intersects(target->player.BB)) continue;

				targetPlayer = target;
                break;
            }

            if (targetPlayer) {
                it = bullets.erase(it);
                int Damage = 210;
                if (targetPlayer->player.HP > Damage) {
                    targetPlayer->player.HP -= Damage;
					S2C_SendPlayerHPInfo(targetPlayer->player.PlayerID, targetPlayer->player.HP);
				}
                else {
                    targetPlayer->player.HP = 0;
					S2C_SendDeathInfo(targetPlayer->player.PlayerID, FirstPlayer->player.PlayerID);
                }
            }
            else {
                ++it;
            }
        }
	}
}