typedef struct {
	int zpos;
	int xpos;
	int ypos;
	int xMouse;
	int yMouse;
	int hp;
} playerDataOffsets;

typedef struct {
	float xMouse;
	float yMouse;
	float xpos;
	float ypos;
	float zpos;
	int hp;
} playerData;

typedef struct {
	int baseAddress;
	int numJumps;
	int* jumps;
	playerDataOffsets offsets;
	playerData data;
} player;