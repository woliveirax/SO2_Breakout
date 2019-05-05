#include <Windows.h>
#include <tchar.h>
#include "..\Server\UnicodeConfigs.h"
#include "..\ClientDLL\Communication.h"
#include "..\Server\Messages.h"

Client * client; //TODO: add func client.quit() so that it opens it's own quit event and ends the waits
HANDLE thread = INVALID_HANDLE_VALUE;
bool CONTINUE;

DWORD WINAPI receiveBallUpdates() {

	for (;CONTINUE;) {
		GameData data = client->receiveBroadcast();

		tcout << "X: " << data.balls[0].posX;
		tcout << "Y: " << data.balls[0].posY << endl << endl;
	}
	return 0;
}

int _tmain(int argc, TCHAR ** argv) {

	if (argc != 2) {
		return -1;
	}

	HANDLE alreadyRunning = INVALID_HANDLE_VALUE;
	CONTINUE = true;
	client = getClientInstance();
	
	if (client == NULL) {
		tcout << "could not create client instance!" << endl;
	}

	alreadyRunning = CreateEvent(NULL,TRUE,FALSE, TEXT("LocalClientRunning"));
	if (alreadyRunning == INVALID_HANDLE_VALUE) {
		return -1;
	}

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		tcout << "A local client is already running, you will join as espectator!" << endl;
		thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)receiveBallUpdates, NULL, 0, NULL);
	}
	else 
	{
		//TODO: create a class to start this up

		if (client->login(argv[1])) {
			tcout << "login accepted" << endl;
			thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)receiveBallUpdates, NULL, 0, NULL);
		}
	}

	_gettchar();
	CONTINUE = false;

	WaitForSingleObject(thread, INFINITE);
	CloseHandle(alreadyRunning);
	delete client;

	return 0;
}