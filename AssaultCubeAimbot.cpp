#include <stdio.h>
#include <windows.h>
#include "PlayerInfo.h"

#define PROCESS_NAME 	"AssaultCube"

#define PLAYER_BASE			0x4E4DBC
#define ENEMIES_BASE		0x4E4E08
#define PLAYER_DATA_OFFSET	0x30
#define MAX_ENEMIES			4 // Eventually higher

void readPlayerData(HANDLE, player*, char);
int FindClosestEnemyIndex(player*, player**);
void AimAtTarget(player*);
int SelectEnemy(player**);

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
	DWORD enemyBase1;
	DWORD enemyBase2;
	HANDLE hProcess;
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
		readPlayerData(hProcess, &mainPlayer, '\0');
		for( i = 0; i < MAX_ENEMIES; i++ ) {
			readPlayerData(hProcess, &enemyPlayer[i], '1'+i);
		}

		//WriteProcessMemory(hProcess, (void*)(enemyBase2 + PLAYER_DATA_OFFSET + mainPlayerOffsets.ypos), &stuck, sizeof(int), NULL);
		if (GetKeyState(VK_RBUTTON)) {
			aiming = true;
			// currentTarget = SelectEnemy();
			// AimAtTarget(enemyPlayer[currentTarget]);
		} else {
			aiming = false;
			currentTarget = -1;
		}

		// TODO: Remove delay for real play
		Sleep(200);
	}
}

/**
  * Read all player data into struct
  */
void readPlayerData(HANDLE hProcess, player *player, char note)
{
	int i;
	DWORD playerBase;

	ReadProcessMemory(hProcess, (void*)player->baseAddress, &playerBase, sizeof(int), NULL);
	printf("%c Player base: [0x%X] = 0x%X, ", note, player->baseAddress, playerBase);
	for( i = 0; i < player->numJumps; i++ ) {
		printf("[0x%X+0x%X] =", playerBase, player->jumps[i]);
		ReadProcessMemory(hProcess, (void*)(playerBase + player->jumps[i]), &playerBase, sizeof(int), NULL);
		printf("0x%X, ", playerBase);
	}
	printf("\n");

	ReadProcessMemory(hProcess, (void*)(playerBase + player->offsets.xMouse), &player->data.xMouse, sizeof(float), NULL);
	ReadProcessMemory(hProcess, (void*)(playerBase + player->offsets.yMouse), &player->data.yMouse, sizeof(float), NULL);
	ReadProcessMemory(hProcess, (void*)(playerBase + player->offsets.xpos), &player->data.xpos, sizeof(float), NULL);
	ReadProcessMemory(hProcess, (void*)(playerBase + player->offsets.ypos), &player->data.ypos, sizeof(float), NULL);
	ReadProcessMemory(hProcess, (void*)(playerBase + player->offsets.zpos), &player->data.zpos, sizeof(float), NULL);
	ReadProcessMemory(hProcess, (void*)(playerBase + player->offsets.hp), &player->data.hp, sizeof(int), NULL);
	printf("hp:%d, xpos:%f, ypos:%f, zpos:%f xM:%f yM:%f\n", player->data.hp, player->data.xpos, player->data.ypos, player->data.zpos, player->data.xMouse, player->data.yMouse);
}

int SelectEnemy(player** enemies)
{
	int targetIndex = -1;
	int i;
	// TODO: Add functionality to lock on to an enemy
	// if selecting a specific enemy
	//   if enemy hp > 0
	//     return its index
	// Find nearest living enemy
	// return its index

	return targetIndex;
}

void AimAtTarget(player* enemy)
{
	// Calculate pitch and yaw to aim at enemy.data.xyz
	// Set the Ymouse and Xmouse
}

int FindClosestEnemyIndex(player* me, player** enemies)
{
	float min = 100.0;
	// TODO: Find min threshold where they're not worth aiming at
	int nearestEnemy = -1;
	/*
	for each enemy 'currentEnemy'
		if enemy alive
			calc distance between self and enemy
			if distance < min
				min = distance;
				nearestEnemy = currentEnemy;
	*/
	return nearestEnemy;
}
