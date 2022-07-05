#define DEBUG_LOG
#define ARIZONA_MOBILE_NT

#include "WintExp.h"
#include "../vendor/armtools/armtools.h"

uintptr_t so_libGTASA = 0x0;
uintptr_t so_libSAMP = 0x0;
uintptr_t pNetGame = 0x0;

#ifdef ARIZONA_MOBILE_NT
	#define LIBRARY_SAMP_NAME "libsamp.so"

	uintptr_t pGlobalWidgets = 0x0;
	uintptr_t pChatWidget = 0x0;

	enum addresses {
		utfConvert = 0x000E72C4+1,
		chatAddMsg = 0x00117A08+1,
		globalWidgets = 0x00226204,
		netGame = 0x0022620C,
	};

	enum widgetIds {
		unknown = -1,
		chat = 96,
	};
	
	void chat_addMessage(const char* text) {
		uint32_t* msgBin;
		(( void (*)(uint32_t*, const char*))(SPL(so_libSAMP, addresses::utfConvert)))(msgBin, text);
		(( void (*)(uintptr_t, uint32_t*))(SPL(so_libSAMP, addresses::chatAddMsg)))(pChatWidget, msgBin);
	}
	
	void ARIZONA_INIT() {
		while(!pGlobalWidgets) {
			pGlobalWidgets = *(uintptr_t*)(SPL(so_libSAMP, addresses::globalWidgets));
			LOG("Waiting widgets..");
			usleep(10);
		}
	
		while(!pChatWidget) {
			pChatWidget = *(uintptr_t *)(SPL(pGlobalWidgets, widgetIds::chat));
			LOG("Waiting chat widget..");
			usleep(10);
		}
	
		chat_addMessage("Hello, world!");
	}

	void SAMP_INIT() 
	{
		while(!pNetGame) {
			pNetGame = *(uintptr_t*)(SPL(so_libSAMP, addresses::netGame));
			LOG("Waiting network..");
			usleep(10);
		}
		
		ARIZONA_INIT();
	}
#else
	#define LIBRARY_SAMP_NAME "libsamp.so"
	
	enum addresses {
		netGame = 0x0,
	};

	void SAMP_INIT() 
	{
		while(!pNetGame) {
			pNetGame = *(uintptr_t*)(SPL(so_libSAMP, addresses::netGame));
			LOG("Waiting network..");
			usleep(10);
		}
	}
#endif

void* pluginThread(void* p) 
{
	while(!so_libGTASA) {
		so_libGTASA = ARMTools::Memory::getLibraryPointer(GTASA_LIBRARY_NAME);
		LOG("Initializing GTASA Library..");
		usleep(10);
	}

	while(!so_libSAMP) {
		so_libSAMP = ARMTools::Memory::getLibraryPointer(LIBRARY_SAMP_NAME);
		LOG("Initializing SAMP Library..");
		usleep(10);
	}

	while ( *(uint32_t *)(SPL(so_libGTASA, GAME_STATE_ADDR)) != 9) {
		LOG("Waiting state..");
    	usleep(500);
	}

	SAMP_INIT();

	pthread_exit(0);
}

__attribute__((constructor))
int WintExp() {
	LOG("WintExp GTA:SA Plugin working..");
	
	srand(time(NULL));
	pthread_t plugin_thread;
	pthread_create(&plugin_thread, NULL, pluginThread, NULL);

	return 1;
}