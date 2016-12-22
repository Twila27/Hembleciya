#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Game/TheApp.hpp"
#include "Engine/TheEngine.hpp"


//--------------------------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	UNREFERENCED_PARAMETER( commandLineString );

	g_theApp = new TheApp();
	g_theApp->Startup( applicationInstanceHandle );

	while ( !g_theApp->IsQuitting() )
	{
		g_theApp->HandleInput();
		g_theEngine->RunFrame();
		g_theApp->FlipAndPresent();
	}

	g_theApp->Shutdown();
	delete g_theApp;

	return 0;
}