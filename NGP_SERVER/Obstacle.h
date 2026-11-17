#pragma once

#include <array>

class Obstacle
{
public:
	struct Point
	{
		float x;
		float z;
	};

	static constexpr std::size_t Count = 9;
	using Corner = std::array<Point, 4>;

	Obstacle();
	~Obstacle();

	const std::array<Corner, Count>& GetCorners() const
	{
		return corners;
	}

private:
	struct Layout
	{
		Point center;
		float width;       // X 방향 길이
		float depth;       // Z 방향 길이
		float yawDegrees;  // Y축 회전각
	};

	static const std::array<Layout, Count> layouts;
	std::array<Corner, Count> corners{}; // 장애물 9개의 꼭짓점(4개)들
};

