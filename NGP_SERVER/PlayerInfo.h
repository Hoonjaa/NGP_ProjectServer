#include <iostream>
#include <cstring>
#include <cmath>
#include <vector>
#include <cmath>

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
	float depth;
	BoundingBox() : center(0, 0, 0), width(1.0f), height(1.0f), depth(1.0f) {}
	BoundingBox(float3 center, float w, float h) : center(center), width(w), height(h) {}
	BoundingBox(float3 center, float w, float h, float d) : center(center), width(w), height(h), depth(d) {}

	bool Intersects(const BoundingBox& other) const {
		return (std::abs(center.x - other.center.x) * 2 < (width + other.width)) &&
			   (std::abs(center.y - other.center.y) * 2 < (height + other.height)) &&
				(std::abs(center.z - other.center.z) * 2 < (depth + other.depth));
	}
	
};

struct Bullet {
	Bullet() : position(0, 0, 0) {}
	Bullet(float3 pos) : position(pos) {}
	Bullet(float3 pos, float3 dir) : position(pos), direction(dir) {}
	Bullet(float3 pos, BoundingBox BB) : position(pos), BB(BB) {}
	Bullet(float3 pos, BoundingBox BB, float3 dir) : position(pos), BB(BB), direction(dir) {}
	float3 position;
	BoundingBox BB;
	BoundingBox GetBB() const {	return BB;}
	void SetBB(float3 center, float w, float h) { BB = BoundingBox(center,w,h); }

	float3 direction{ 0.0f, 0.0f, -1.0f };
	float speed = 50.0f;
	float lifeTime = 5.0f;
	float currentTime = 0.0f;
	void Update()
	{
		constexpr float tickSeconds = 1.0f / 60.0f;
		const float distance = speed * tickSeconds;

		position.x += direction.x * distance;
		position.y += direction.y * distance;
		position.z += direction.z * distance;

		BB.center = position;
		currentTime += tickSeconds;
	}
};


struct PlayerInfo {
	PlayerInfo()
		: BB()
	{
		PlayerID = -1; // Not logged in yet.
		name[0] = '\0';
		isalive = true;

		Pos_x = 0.0f; //임시
		Pos_z = 6.0f; //임시
		BB.center = float3(Pos_x, 2.5f, Pos_z);
		BB.width = 3.0f;
		BB.height = 5.0f;
		BB.depth = 3.0f;

		HP = 200;
		Kill = 0;
		Death = 0;

		IsReroeading = false;
		AmmoCount = 30;

		Respawn_time = 0;
		Mujuck_time = 0;
	}
	bool	isalive;

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

	std::vector<Bullet*> bullets;
	void AddBullet(float3 pos, float3 dir)
	{
		Bullet* b = new Bullet(pos, dir);
		bullets.push_back(b);
	}

	BoundingBox BB;

	void SetPosition(float x, float z)
	{
		Pos_x = x;
		Pos_z = z;
		BB.center = float3(x, 2.5f, z);
	}

	BoundingBox GetBB() const { return BB; }
	void SetBB(float3 center, float w, float h) { BB = BoundingBox(center, w, h); }

	unsigned char w = 0;
	unsigned char a = 0;
	unsigned char s = 0;
	unsigned char d = 0;
	float moveSpeed = 0.05f;
	void Update();
};