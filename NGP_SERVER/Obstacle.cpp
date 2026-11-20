#include "Obstacle.h"
#include <cmath>

// 9개 장애물들의 중심점, 가로세로길이, 회전각 정보 설정
const std::array<Obstacle::Layout, Obstacle::Count> Obstacle::layouts{ {
    {{ -8.1f, -14.5f}, 8.0f,  2.0f,  30.0f},
    {{  5.5f, -14.0f}, 6.0f,  4.0f, -15.0f},
    {{ 15.2f,  -8.2f}, 3.0f, 12.0f,   0.0f},
    {{-15.6f,  -2.9f}, 4.0f,  4.0f,  30.0f},
    {{ -0.2f,   0.9f}, 4.0f,  4.0f,   0.0f},
    {{-13.8f,  10.0f}, 6.0f,  3.0f,   0.0f},
    {{ -9.3f,  13.5f}, 3.0f, 10.0f,   0.0f},
    {{  6.3f,  15.2f}, 6.0f,  2.0f,  30.0f},
    {{ 10.5f,  10.1f}, 7.0f,  2.0f,  55.0f},
} };

Obstacle::Obstacle()
{
    constexpr float degreesToRadians = 3.14159265358979323846f / 180.0f;

    // coners에 9개 장애물의 점들을 저장해두는 거임 (회전까지 된거)
    for (std::size_t i = 0; i < layouts.size(); ++i)
    {
        const Layout& layout = layouts[i];
        const float halfWidth = layout.width * 0.5f;
        const float halfDepth = layout.depth * 0.5f;
        const float yaw = layout.yawDegrees * degreesToRadians;
        const float cosYaw = std::cos(yaw);
        const float sinYaw = std::sin(yaw);

        const Corner localCorners{ {
            {-halfWidth, -halfDepth},
            { halfWidth, -halfDepth},
            { halfWidth,  halfDepth},
            {-halfWidth,  halfDepth},
        } };

        for (std::size_t j = 0; j < localCorners.size(); ++j)
        {
            const Point& local = localCorners[j];
            corners[i][j] = {
                layout.center.x + local.x * cosYaw + local.z * sinYaw,
                layout.center.z - local.x * sinYaw + local.z * cosYaw,
            };
        }
    }
}

Obstacle::~Obstacle()
{
}