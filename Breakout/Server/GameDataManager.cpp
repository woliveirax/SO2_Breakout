#include "GameDataManager.h"
#include "Server.h"
#include "time.h"

GameDataManager::GameDataManager(GameData * gdata) {
	gameData = gdata;
	hAccessMutex = CreateMutex(NULL, FALSE, NULL);
	if (hAccessMutex == NULL)
	{
		throw 1;
	}

	hGameEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hGameEvent == NULL) {
		throw 2;
	}
}

GameDataManager::~GameDataManager() {
	CloseHandle(hAccessMutex);
	CloseHandle(hGameEvent);
}

void GameDataManager::lockAccessGameData() {
	WaitForSingleObject(hAccessMutex, INFINITE);
}

void GameDataManager::releaseAccessGameData() {
	ReleaseMutex(hAccessMutex);
}

void GameDataManager::setGameEvent() {
	SetEvent(hGameEvent);
}

void GameDataManager::waitForGameEvent() {
	HANDLE game[2];

	game[0] = hGameEvent;
	game[1] = Server::sharedMemory.hExitEvent;

	WaitForMultipleObjects(2, game, FALSE, INFINITE);
}

GameData * GameDataManager::getGameData() {
	return gameData;
}

void GameDataManager::setupGameStart()
{
	SecureZeroMemory(gameData, sizeof(GameData));

	setupBall();
	setupPlayers();

}

void GameDataManager::generateLevel(int difficulty) {
	//TODO: Code here
	//Criar os tijolos bonus
	//Criar tijolos destrutivris 
	//Criar os tijolos destrutiveis,
}

void GameDataManager::setupBall() {
	WaitForSingleObject(hAccessMutex, INFINITE);

	for (auto &ball : gameData->balls) {
		ball.active = false;
		ball.width = BALL_DEFAULT_WIDTH;
		ball.height = BALL_DEFAULT_HEIGHT;
	}

	//Enable and set ball [0] in the center of the screen
	gameData->balls->active = true;
	gameData->balls->posX = MAX_GAME_WIDTH / 2;
	gameData->balls->posY = MAX_GAME_HEIGHT / 2;
	gameData->balls->up = false;
	gameData->balls->right = true;

	ReleaseMutex(hAccessMutex);
}
void retrieveCertainBonus() {
	Server::config.getBonusDropRate();
}

void GameDataManager::setupTiles(int difficulty) {
	Server::gameData.lockAccessGameData();

	int ocuppancyArray[18][24] = { {0} };
	int n_undestructables;
	int n_bonus;
	int n_normals;
	int x;
	int y;

	n_undestructables = 3 * difficulty + 5;

	srand(time(NULL));

	for (int i = 0; i < n_undestructables; i++) {
		do {
			x = rand() % 16 + 4;
			y = rand() % 18;
		} while (ocuppancyArray[x][y] != 0);
		ocuppancyArray[x][y] = 1;

		gameData->tiles[i].width = TILE_DEFAULT_WIDTH;
		gameData->tiles[i].height = TILE_DEFAULT_HEIGHT;
		gameData->tiles[i].active = true;
		gameData->tiles[i].bonus = NORMAL;
		gameData->tiles[i].resistance = UNBREAKABLE;
		gameData->tiles[i].posX = x* TILE_DEFAULT_WIDTH;
		gameData->tiles[i].posY = y * TILE_DEFAULT_HEIGHT + 2;
	}
	
	n_bonus = -2 * difficulty + 40;
	
	for (int i = 0; i < n_bonus; i++) {
		do {
			x = rand() % 24;
			y = rand() % 18;
		} while (ocuppancyArray[x][y] != 0);
		ocuppancyArray[x][y] = 1;

		gameData->tiles[i].width = TILE_DEFAULT_WIDTH;
		gameData->tiles[i].height = TILE_DEFAULT_HEIGHT;
		gameData->tiles[i].active = true;

		//gameData->tiles[i].bonus = retrieveCertainBonus();
		gameData->tiles[i].resistance = 1; //TODO: confirm
		gameData->tiles[i].posX = x * TILE_DEFAULT_WIDTH;
		gameData->tiles[i].posY = y * TILE_DEFAULT_HEIGHT + 2;
	}

	for (int i = 0; i < 24; i++) {
		for (int j = 0; j < 18; j++) {
			if (ocuppancyArray[i][j] != 0)
				continue;
			gameData->tiles[i].width = TILE_DEFAULT_WIDTH;
			gameData->tiles[i].height = TILE_DEFAULT_HEIGHT;
			gameData->tiles[i].active = true;

			gameData->tiles[i].bonus = NORMAL;
			gameData->tiles[i].resistance = 4; //TODO: confirm
			gameData->tiles[i].posX = i * TILE_DEFAULT_WIDTH;
			gameData->tiles[i].posY = y * TILE_DEFAULT_HEIGHT + 2;
		}
	}
	Server::gameData.releaseAccessGameData();
}

void GameDataManager::setupPlayers() {
	WaitForSingleObject(hAccessMutex, INFINITE);

	int screenSize = MAX_GAME_WIDTH;
	int separator = MAX_GAME_WIDTH /  (Server::clients.getClientArray().size() + 1) ;

	for (int i = 0; i < MAX_PLAYERS; i++) {
		gameData->players[i].id = i;
		gameData->players[i].height = PLAYER_DEFAULT_HEIGHT;
		gameData->players[i].width = PLAYER_DEFAULT_WIDTH;
		gameData->players[i].lives = Server::config.getInitialLives();
		gameData->players[i].points = 0;
		gameData->players[i].posX = i + separator - PLAYER_DEFAULT_WIDTH/2;
		gameData->players[i].posY = 30;
	}

	for (auto & clients : Server::clients.getClientArray()) {
		clients->getPlayer()->active = true;
		_tcscpy(clients->getPlayer()->name, clients->getName().c_str());
	}

	ReleaseMutex(hAccessMutex);
}

void GameDataManager::movePlayer(Player * selectedPlayer, int direction) {
	int selectedPlayerLeft, selectedPlayerRight;
	int speed;
	bool collision = false;

	WaitForSingleObject(hAccessMutex, INFINITE);

	speed = Server::config.getMovementSpeed();
	selectedPlayerLeft = selectedPlayer->posX;
	selectedPlayerRight = selectedPlayer->posX + selectedPlayer->width;

	//if it's on the edge of the world leaves without change
	if (selectedPlayerLeft == MIN_GAME_WIDTH || selectedPlayerRight == MAX_GAME_WIDTH) {
		ReleaseMutex(hAccessMutex);
		return;
	}

	if (direction == LEFT)
	{
		for (const auto & player : gameData->players) {
			if (!player.active)
				continue;

			//if there is a collision ativates a flag
			if ((selectedPlayerLeft - speed) < player.posX + player.width) {
				collision = true;
				break;
			}
		}

		if (!collision)
			selectedPlayer->posX -= speed;
	}
	else
	{
		for (const auto & player : gameData->players) {
			if (!player.active)
				continue;

			//if there is a collision ativates a flag
			if ((selectedPlayerRight + speed) < player.posX) {
				collision = true;
				break;
			}
		}

		if (!collision)
			selectedPlayer->posX += speed;
	}

	ReleaseMutex(hAccessMutex);
}

void GameDataManager::movePlayerPrecise(Player * selectedPlayer, int x) {
	int selectedPlayerLeft, selectedPlayerRight;
	bool collision = false;

	WaitForSingleObject(hAccessMutex, INFINITE);
	
	selectedPlayerRight = selectedPlayer->posX + selectedPlayer->width;
	selectedPlayerLeft = selectedPlayer->posX;

	x -= selectedPlayer->width / 2;
	
	//Verify if the positions are within bounds and fix them if they are not
	if (x < MIN_GAME_WIDTH) {
		x = MIN_GAME_WIDTH;
	}
	else if (x + selectedPlayer->width > MAX_GAME_WIDTH) {
		x -= (x + selectedPlayer->width) - MAX_GAME_WIDTH;
	}

	for (const auto & player : gameData->players) {
		if (!player.active)
			continue;

		//if there is a collision ativates a flag
		if (x <= player.posX + player.width || x + selectedPlayer->width >= player.posX) {
			collision = true;
			break;
		}
	}

	if (!collision)
		selectedPlayer->posX = x;

	ReleaseMutex(hAccessMutex);
}

Player * GameDataManager::getAvailablePlayer() {
	WaitForSingleObject(hAccessMutex, INFINITE);

	for (auto &player : gameData->players) {
		if (!player.active) {
			ReleaseMutex(hAccessMutex);
			return &player;
		}
	}

	ReleaseMutex(hAccessMutex);

	return nullptr;
}