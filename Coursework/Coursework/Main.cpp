// Main.cpp
#include "System.h"
#include "App1.h"

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 675
#define V_SYNC true
#define FULL_SCREEN false

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	App1* app = new App1();
	System* system;

	// Create the system object.
	system = new System(app, WINDOW_WIDTH, WINDOW_HEIGHT, V_SYNC, FULL_SCREEN);

	// Initialize and run the system object.
	system->run();

	// Shutdown and release the system object.
	delete system;
	system = 0;

	return 0;
}