#include "Engine/TheEngine.hpp"

//Major Subsystems
#include "Engine/Input/TheInput.hpp"
#include "Engine/Audio/TheAudio.hpp"
#include "Engine/Core/TheConsole.hpp"
#include "Engine/Renderer/DebugRenderCommand.hpp"
#include "Game/TheGame.hpp"

//Major Utils
#include "Engine/Time/Time.hpp"
#include "Engine/Tools/FBXUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
TheEngine* g_theEngine = nullptr;


//--------------------------------------------------------------------------------------------------------------
static void RegisterConsoleCommands()
{
	//AES A1
	g_theConsole->RegisterCommand( "FBXList", FBXList );

	//AES A2
	g_theConsole->RegisterCommand( "FBXLoad", FBXLoad );

	//AES A3
	g_theConsole->RegisterCommand( "MeshSaveLastMeshBuilderMade", MeshSaveLastMeshBuilderMade );
	g_theConsole->RegisterCommand( "MeshLoadFromFile", MeshLoadFromFile );

	//AES A4
	g_theConsole->RegisterCommand( "SkeletonSaveLastSkeletonMade", SkeletonSaveLastSkeletonMade );
	g_theConsole->RegisterCommand( "SkeletonLoadFromFile", SkeletonLoadFromFile );
	g_theConsole->RegisterCommand( "DebugRenderClearCommands", DebugRenderClearCommands );
	ROADMAP( "Make a separate 'add' console command for each of the DebugRenderCommands!" );

	//AES A5
	g_theConsole->RegisterCommand( "AnimationLoadFromFile", AnimationLoadFromFile );
	g_theConsole->RegisterCommand( "AnimationSaveLastAnimationMade", AnimationSaveLastAnimationMade );
}


//--------------------------------------------------------------------------------------------------------------
void TheEngine::Startup( double screenWidth, double screenHeight )
{
	//-----------------------------------------------------------------------------
	//	Allocation/Constructor Calls
	//-----------------------------------------------------------------------------

	//Make sure Renderer ctor comes first so that default texture gets ID of 1. Args configure FBO dimensions.
	g_theRenderer = new TheRenderer( screenWidth, screenHeight );
	g_theDebugRenderCommands = new std::list< DebugRenderCommand* >();

	g_theAudio = new AudioSystem(); //Example usage:
	// [static] SoundID musicID = g_theAudio->CreateOrGetSound( "Data/Audio/Yume Nikki mega mix (SD).mp3" );
	// [g_bgMusicChannel =] g_theAudio->PlaySound( musicID );
		//This is declared as AudioChannelHandle g_bgMusicChannel; necessary to track for things like turning on looping.

	g_theGame = new TheGame();

	g_theInput = new TheInput();
	Vector2i screenCenter = Vector2i( (int)( screenWidth / 2.0 ), (int)( screenHeight / 2.0 ) );
	g_theInput->SetCursorSnapToPos( screenCenter );
	g_theInput->OnGainedFocus();
	g_theInput->HideCursor();

	g_theConsole = new TheConsole( 0.0, 0.0, screenWidth, screenHeight );
	RegisterConsoleCommands();

	//-----------------------------------------------------------------------------
	//	Startup/Initialization Calls
	//-----------------------------------------------------------------------------

	SeedWindowsRNG();

	g_theRenderer->PreGameStartup();
	g_theGame->Startup();
	g_theRenderer->PostGameStartup();
}


//--------------------------------------------------------------------------------------------------------------
void TheEngine::RunFrame()
{

	g_theAudio->Update();
	//	if ( g_theInput->WasKeyPressedOnce( 'X' ) ) g_theAudio->StopChannel( g_bgMusicChannel );

	this->Update( CalcDeltaSeconds() );

	this->Render();
}


//--------------------------------------------------------------------------------------------------------------
void TheEngine::Update( float deltaSeconds )
{
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_DEBUG_INFO ) ) 
		g_inDebugMode = !g_inDebugMode;

	g_theConsole->Update( deltaSeconds ); //Delta +='d into caret's alpha.

	g_theRenderer->Update( deltaSeconds, g_theGame->GetActiveCamera() );
		//Update uniforms for shader timers, scene MVP, and lights.

	ROADMAP( "Explore passing in 0 to freeze, or other values to rewind, slow, etc." );
	g_theGame->Update( deltaSeconds );

	UpdateDebugCommands( deltaSeconds );
}


//--------------------------------------------------------------------------------------------------------------
void TheEngine::RenderDebug3D()
{
	if ( g_theRenderer->IsShowingAxes() )
		AddDebugRenderCommand( new DebugRenderCommandBasis( Vector3f::ZERO, 1000.f, true, 0.f, DEPTH_TEST_DUAL, 1.f ) );

	RenderThenExpireDebugCommands3D();

	g_theGame->RenderDebug3D();
}


//--------------------------------------------------------------------------------------------------------------
void TheEngine::RenderDebug2D()
{
	g_theGame->RenderDebug2D();
}


//--------------------------------------------------------------------------------------------------------------
void TheEngine::Render()
{
	g_theRenderer->PreRenderStep();

	//-----------------------------------------------------------------------------
	//Has to stick its own render2d and render3d around TheGame's, so we can't just call to TheGame::Render() here.
	
	g_theRenderer->SetupView3D( g_theGame->GetActiveCamera() );
	g_theGame->Render3D();
	if ( g_inDebugMode )
		this->RenderDebug3D();

	g_theRenderer->SetupView2D();
	g_theGame->Render2D();
	if ( g_inDebugMode )
		this->RenderDebug2D();

	//-----------------------------------------------------------------------------
	g_theRenderer->PostRenderStep();

	g_theConsole->Render();

	//Main_Win32 should call TheApp's FlipAndPresent() next.
}


//--------------------------------------------------------------------------------------------------------------
void TheEngine::Shutdown()
{
	//Any other subsystems that have their own Shutdown() equivalent call here.
	ClearDebugCommands();
	g_theGame->Shutdown();

	//-----------------------------------------------------------------------------
	delete g_theGame;
	delete g_theInput;
	delete g_theRenderer;
	delete g_theDebugRenderCommands;
	delete g_theConsole;

	//-----------------------------------------------------------------------------
	g_theGame = nullptr;
	g_theInput = nullptr;
	g_theRenderer = nullptr;
	g_theDebugRenderCommands = nullptr;
	g_theConsole = nullptr;
}


//--------------------------------------------------------------------------------------------------------------
bool TheEngine::IsQuitting()
{
	return g_theGame->IsQuitting();
}
