#include <iostream>
#include <cstring>
#include <random>
struct PlayerInfo {
	PlayerInfo() {
		Pos_x = rand() % 300;
		Pos_y = rand() % 300;

		HP = 200;
		Kill = 0;
		Death = 0;
		Assist = 0;

		IsReroeading = false;
		AmmoCount = 30;

		Respawn_time = 0;
		Mujuck_time = 0;
	}
	int		PlayerID;
	char	name[10];

	float	Pos_x;
	float	Pos_y;

	float	HP;
	int		Kill;
	int		Death;
	int		Assist;

	bool	IsReroeading;
	int		AmmoCount;

	float	Respawn_time;
	float	Mujuck_time;
};