#pragma once


#include "Game/GameCommon.hpp"
#include <vector>


//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class TheGame;
class Camera3D;
class Command;
class GameEntity;
class Map;
class Player;
class NPCFactory;
class ItemFactory;


//-----------------------------------------------------------------------------
// Singleton Access
//-----------------------------------------------------------------------------
extern TheGame* g_theGame;


//-----------------------------------------------------------------------------
class TheGame
{
public:

	TheGame();
	~TheGame();

	void SaveGame();
	void LoadGame();
	bool IsQuitting() const { return m_isQuitting; }
	void Startup();
	void Shutdown();

	bool Update( float deltaSeconds );
	void Render();

	bool Render2D();
	void RenderDebug2D();
	void Render3D() {}
	void RenderDebug3D() {}

	const Camera3D* GetActiveCamera() const;

	std::vector<GameEntity*> m_livingEntities;
	TurnOrderedMap m_activeAgents;
	Map* m_currentMap;
	Player* m_player;


private:

	bool m_foundSave;
	bool m_isQuitting;

	void AddCarriedItemsToEntityListForAgent( const Agent* agent );
	void AddFeaturesToEntityListForMap( Map* map );

	void PopulateCurrentMapWithNPCs_OnePlusPerFactory( int minNPCsInclusive, int maxNPCsInclusive );
	void PopulateCurrentMapWithNPCs_RangedRandom( int minNPCsInclusive, int maxNPCsInclusive );
	void PopulateCurrentMapWithItems_OnePlusPerCategory( int minNPCsInclusive, int maxNPCsInclusive );
	void PopulateCurrentMapWithItems_OnePlusPerFactory( int minNPCsInclusive, int maxNPCsInclusive );
	void PopulateCurrentMapWithItems_RangedRandom( int minNPCsInclusive, int maxNPCsInclusive );

	void AddNPC( NPCFactory* factory, int numToMake );
	void AddItem( ItemFactory* factory, int numToMake );
	
	void StartGameplay();
	void UpdatePlayedMapSimulation( float deltaSeconds );
	
	void DestroyAllGameplayEntities();
	void DestroyDeadGameplayEntities();
	void UntargetAgent( const Agent* agentToRemove );
	void HandlePlayerDeath();


	//--

	void RenderLeftDebugText2D();
	void RenderRightDebugText2D();

	//--

	bool UpdateStartup( float deltaSeconds );
	bool UpdateShutdown( float deltaSeconds );
	bool UpdateMainMenu( float deltaSeconds );
	bool UpdateMapSelection( float deltaSeconds );
	bool UpdateMapGenerating( float deltaSeconds );

	void FinalizeMap();

	bool UpdatePlaying( float deltaSeconds );
	bool UpdatePaused( float deltaSeconds );

	//--

	bool RenderStartup2D();
	bool RenderShutdown2D();
	bool RenderMainMenu2D();
	bool RenderMapSelection2D();
	bool RenderMapGenerating2D();
	bool RenderPlaying2D();
	bool RenderPaused2D();

	//--

	static Camera3D* s_playerCamera;
	float m_fadeoutTimer;
	const float m_FADEOUT_LENGTH_SECONDS;
};
