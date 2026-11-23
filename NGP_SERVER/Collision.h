#pragma once
#include "Header/ServerTypes.h"
#include "Obstacle.h"

void CheckBulletPlayerCollision(std::array<std::shared_ptr<ClientSession>, 3>);

bool CheckPlayerObstacleCollision(float x, float z, float radius);

bool IntersectsObstacle(float x, float z, float radius, const Obstacle::Corner& corners);