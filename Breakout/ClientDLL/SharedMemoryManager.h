#pragma once
#include <Windows.h>
#include "../Server/Messages.h"
#include "../Server/GeneralConstants.h"
#include "../Server/GameData.h"

class SharedMemoryManager
{
private:
	HANDLE hGameData;
	HANDLE hServerBuffer;
	HANDLE hClientBuffer;

	bool ready;
	void initSharedMemory();
	void initSyncVariables();

public:
	GameData * viewGameData;
	ClientMsgBuffer * viewClientBuffer;
	ServerMsgBuffer * viewServerBuffer;

	HANDLE hReadyForUpdate;
	HANDLE hUpdateEvent;

	HANDLE hExitEvent;
	HANDLE hServerSemEmpty;
	HANDLE hServerSemFilled;

	HANDLE hClientSemEmpty;
	HANDLE hClientSemFilled;
	HANDLE hClientWriteMutex;
	HANDLE hClientReadMutex;

	bool getResourceReadyNotifier(tstring name);
	bool isReady() const;
	SharedMemoryManager();
	~SharedMemoryManager();
};

