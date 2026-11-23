#include <iostream>
#include <cstring>
#include <random>
#include <vector>

struct float3 {
	float x;
	float y;
	float z;
	float3(float x, float y, float z) : x(x), y(y), z(z) {}
};

struct BoundingBox {
	float3 center;
	float width;
	float height;
	BoundingBox() : center(0, 0, 0), width(1.0f), height(1.0f) {}
	BoundingBox(float3 center, float w, float h) : center(center), width(w), height(h) {}

	bool Intersects(const BoundingBox& other) const {
		return (std::abs(center.x - other.center.x) * 2 < (width + other.width)) &&
			   (std::abs(center.y - other.center.y) * 2 < (height + other.height));
	}
	
};

struct Bullet {
	Bullet() : position(0, 0, 0) {}
	Bullet(float3 pos) : position(pos) {}
	Bullet(float3 pos, BoundingBox BB) : position(pos), BB(BB) {}
	float3 position;
	BoundingBox BB;
	BoundingBox GetBB() const {	return BB;}
	void SetBB(float3 center, float w, float h) { BB = BoundingBox(center,w,h); }
};


struct PlayerInfo {
	PlayerInfo()
		: BB()
	{
		PlayerID = -1; // Not logged in yet.
		name[0] = '\0';

		Pos_x = rand() % 300;
		Pos_z = rand() % 300;
		BB.center = float3(Pos_x, 2.5f, Pos_z);
		BB.width = 3.0f;
		BB.height = 5.0f;

		HP = 200;
		Kill = 0;
		Death = 0;

		IsReroeading = false;
		AmmoCount = 30;

		Respawn_time = 0;
		Mujuck_time = 0;

		Bullet bullet(float3(10, 0, 0), BoundingBox(float3(10, 0, 0), 1, 1));
		bullets.push_back(bullet);
	}
	short	PlayerID;
	char	name[10];

	float	Pos_x;
	float	Pos_z;

	float	pitch;
	float	yaw;

	float	HP;
	int		Kill;
	int		Death;

	bool	IsReroeading;
	int		AmmoCount;

	float	Respawn_time;
	float	Mujuck_time;

	std::vector<Bullet> bullets;
	BoundingBox BB;
	BoundingBox GetBB() const { return BB; }
	void SetBB(float3 center, float w, float h) { BB = BoundingBox(center, w, h); }
};