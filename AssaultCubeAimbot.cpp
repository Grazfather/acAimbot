#include <stdio.h>
#include <windows.h>
#include <math.h>
#include "PlayerInfo.h"

#define PROCESS_NAME 	"AssaultCube"

#define PLAYER_BASE			0x4E4DBC
#define ENEMIES_BASE		0x4E4E08
#define MAX_ENEMIES			4 // Eventually higher

#define DEBUG
#ifdef DEBUG
 #define DEBUG_PRINT(...) do{printf("DEBUG:"__VA_ARGS__);}while(0);
#else
 #define DEBUG_PRINT(...) do{}while(0);
#endif

#define PI 3.14159265

void readPlayerData(HANDLE, player*);
void writePlayerData(HANDLE, player*, void*, size_t, int);
void printPlayerData(const player*);
int FindClosestEnemyIndex(player*, player[]);
float distanceBetweenPlayers(player*, player*);
void AimAtTarget(player*, player*);

HANDLE hProcess;

int main()
{
	int i;
	// Main player
	player mainPlayer;
	// TODO: Somehow distinguish friend from foe
	player enemyPlayer[4];
	// Offsets into player data to find hp, pos, etc.
	//					   zpos, xpos, ypos, xM,   yM,   hp
	mainPlayer.baseAddress = PLAYER_BASE;
	mainPlayer.offsets = { 0x34, 0x38, 0x3C, 0x40, 0x44, 0xF4 };
	mainPlayer.numJumps = 0;
	enemyPlayer[0].baseAddress = ENEMIES_BASE;
	enemyPlayer[0].offsets = { 0x34, 0x38, 0x3C, 0x40, 0x44, 0xF4 };
	enemyPlayer[0].numJumps = 1;
	enemyPlayer[0].jumps = (int[]){ 0x4 };
	enemyPlayer[1].baseAddress = ENEMIES_BASE;
	enemyPlayer[1].offsets = { 0x34, 0x38, 0x3C, 0x40, 0x44, 0xF4 };
	enemyPlayer[1].numJumps = 1;
	enemyPlayer[1].jumps = (int[]){ 0x8 };
	enemyPlayer[2].baseAddress = ENEMIES_BASE;
	enemyPlayer[2].offsets = { 0x34, 0x38, 0x3C, 0x40, 0x44, 0xF4 };
	enemyPlayer[2].numJumps = 1;
	enemyPlayer[2].jumps = (int[]){ 0xC };
	enemyPlayer[3].baseAddress = ENEMIES_BASE;
	enemyPlayer[3].offsets = { 0x34, 0x38, 0x3C, 0x40, 0x44, 0xF4 };
	enemyPlayer[3].numJumps = 1;
	enemyPlayer[3].jumps = (int[]){ 0x10 };

	bool gameFound = false;
	bool aiming = false;
	bool focusingOnEnemy = false;
	int currentTarget = -1;

	DWORD playerBase;
	DWORD processId;
	HWND hWnd = FindWindow(0, PROCESS_NAME);
	if (!hWnd) {
		fprintf(stderr, "Error: cannot find window.\n");
		return -1;
	}

	do {
		system("cls");
		GetWindowThreadProcessId(hWnd, &processId);
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
		if (!hProcess) {
			fprintf(stderr, "Cannot open process.\n");
		} else {
			gameFound = true;
		}
		Sleep(100);
	} while(!gameFound);

	while( true ) {
		system("cls");

		currentTarget = -1;
		readPlayerData(hProcess, &mainPlayer);

		printf("Player: ");
		printPlayerData(&mainPlayer);
		for( i = 0; i < MAX_ENEMIES; i++ ) {
			readPlayerData(hProcess, &enemyPlayer[i]);
			printf("Enemy %d: ", i);
			printPlayerData(&enemyPlayer[i]);
			printf("Distance: %f\n", distanceBetweenPlayers(&mainPlayer, &enemyPlayer[i]));
		}

		currentTarget = FindClosestEnemyIndex(&mainPlayer, enemyPlayer);
		DEBUG_PRINT("Key state: 0x%4.4X\n", GetKeyState(VK_RBUTTON));
		if (GetKeyState(VK_RBUTTON) & 0x8000) {
			aiming = true;
			if (currentTarget >= 0)
				AimAtTarget(&mainPlayer, &enemyPlayer[currentTarget]);
		} else {
			aiming = false;
			currentTarget = -1;
		}

		// TODO: Keep track of time elapsed and try to instead aim a consistent number of times per second.
		Sleep(50);
	}
}

/**
  * Read all player data into struct
  */
void readPlayerData(HANDLE hProcess, player *player)
{
	int i;
	DWORD playerBase;

	ReadProcessMemory(hProcess, (void*)player->baseAddress, &playerBase, sizeof(int), NULL);
	for( i = 0; i < player->numJumps; i++ ) {
		ReadProcessMemory(hProcess, (void*)(playerBase + player->jumps[i]), &playerBase, sizeof(int), NULL);
	}

	ReadProcessMemory(hProcess, (void*)(playerBase + player->offsets.xMouse), &player->data.xMouse, sizeof(float), NULL);
	ReadProcessMemory(hProcess, (void*)(playerBase + player->offsets.yMouse), &player->data.yMouse, sizeof(float), NULL);
	ReadProcessMemory(hProcess, (void*)(playerBase + player->offsets.xpos), &player->data.xpos, sizeof(float), NULL);
	ReadProcessMemory(hProcess, (void*)(playerBase + player->offsets.ypos), &player->data.ypos, sizeof(float), NULL);
	ReadProcessMemory(hProcess, (void*)(playerBase + player->offsets.zpos), &player->data.zpos, sizeof(float), NULL);
	ReadProcessMemory(hProcess, (void*)(playerBase + player->offsets.hp), &player->data.hp, sizeof(int), NULL);
}

/**
  * Write data to a player's struct at the specified offset.
  */
void writePlayerData(HANDLE hProcess, player *player, void *data, size_t size, int offset)
{
	int i;
	DWORD playerBase;

	ReadProcessMemory(hProcess, (void*)player->baseAddress, &playerBase, sizeof(int), NULL);
	for( i = 0; i < player->numJumps; i++ ) {
		ReadProcessMemory(hProcess, (void*)(playerBase + player->jumps[i]), &playerBase, sizeof(int), NULL);
	}

	WriteProcessMemory(hProcess, (void*)(playerBase + offset), data, size, NULL);
}

void printPlayerData(const player *player)
{
	printf("hp:%d, xpos:%f, ypos:%f, zpos:%f\n", player->data.hp, player->data.xpos, player->data.ypos, player->data.zpos);
}

/**
  * Calculate the yMouse and xMouse required to aim at the specified enemy.
  * Also write it to the game process.
  */
void AimAtTarget(player* me, player* enemy)
{
	float xMouse;
	float yMouse;
	float dx, dy, dz;
	float a, b, c;

#ifdef DEBUG
	DEBUG_PRINT("Aiming at: \n");
	printPlayerData(enemy);
#endif

	// Need to find what cartesian quadrant we are in.
	/*
			 |0*
	   (-,-) | (-,+)
	     D   |   A
	 ----------------->z
       (-,+) | (+,+)
	     C   |   B
	         v+x
	*/
	dx = enemy->data.xpos - me->data.xpos;
	dy = enemy->data.ypos - me->data.ypos;
	// TODO: Aim slightly lower on the enemy (instead of the head) to improve
	// accuracy.
	dz = enemy->data.zpos - me->data.zpos;

	if (dx < 0.0 && dz > 0.0) { // A
		xMouse = (atan(dz/-dx) * 180 / PI);
	} else if (dx < 0.0 && dz < 0.0) { // D
		xMouse = 360 - (atan(-dx/-dz) * 180 / PI);
	} else if (dx > 0.0 && dz < 0.0) { // C
		xMouse = 180 + (atan(-dz/dx) * 180 / PI);
	} else if (dx > 0.0 && dz > 0.0) { // B
		xMouse = 90 + (atan(dx/dz) * 180 / PI);
	}

	yMouse = atan((dy)/sqrt(fabs(dz*dz + dx*dx))) * 180/PI;
	DEBUG_PRINT("xMouse: %f yMouse: %f\n", xMouse, yMouse);
	// Set the Ymouse and Xmouse
	if (xMouse == xMouse && yMouse == yMouse) {
		writePlayerData(hProcess, me, &xMouse, sizeof(float), me->offsets.xMouse);
		writePlayerData(hProcess, me, &yMouse, sizeof(float), me->offsets.yMouse);
	} else {
		// We've done bad math.
		printf("Error! %f %f\n\n", xMouse, yMouse);
		exit(0);
	}
}

float distanceBetween(float x1, float y1, float z1, float x2, float y2, float z2)
{
	return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)+ (z1-z2)*(z1-z2));
}

float distanceBetweenPlayers(player* p1, player* p2)
{
	return distanceBetween(p1->data.xpos, p1->data.ypos, p1->data.zpos,
							p2->data.xpos, p2->data.ypos, p2->data.zpos);
}

int FindClosestEnemyIndex(player* me, player enemies[])
{
	// TODO: Find max distance they're worth aiming from.
	float min = 100.0;
	float distance;
	int nearestEnemy = -1;
	int i;

	for( i = 0; i < MAX_ENEMIES; i++ ) {
		if (enemies[i].data.hp > 0 && enemies[i].data.hp <= 100) {
			distance = distanceBetweenPlayers(me, &enemies[i]);
			if (distance < min) {
				nearestEnemy = i;
				min = distance;
			}
		}
	}
	DEBUG_PRINT("Nearest enemy '%d' is '%f' units away.\n", nearestEnemy, min);

	return nearestEnemy;
}
