#include "Game/TheGame.hpp"

#include "Engine/Input/TheInput.hpp"
#include "Engine/Core/Command.hpp"
#include "Engine/FileUtils/FileUtils.hpp"
#include "Engine/Core/TheConsole.hpp"

//SD4
#include "Game/GameEntity.hpp"
#include "Game/Player.hpp"
#include "Game/Map.hpp"
#include "Game/Biomes/BiomeBlueprint.hpp"
#include "Game/Generators/Generator.hpp"
#include "Game/FieldOfView/FieldOfView.hpp"
#include "Game/NPCs/NPCFactory.hpp"
#include "Game/NPCs/NPC.hpp"
#include "Game/Items/ItemFactory.hpp"
#include "Game/Items/Item.hpp"
#include "Game/Features/FeatureFactory.hpp"
#include "Game/Features/Feature.hpp"
#include "Engine/Audio/TheAudio.hpp"
#include "Game/FactionSystem.hpp"

//Engine Overhaul
#include "Engine/Math/Camera3D.hpp"


//--------------------------------------------------------------------------------------------------------------
TheGame* g_theGame = nullptr;
static SoundID g_menuAcceptSoundID = 0;
static SoundID g_menuDeclineSoundID = 0;
STATIC Camera3D* TheGame::s_playerCamera = new Camera3D( Vector3f::ZERO );


//--------------------------------------------------------------------------------------------------------------
static void RegisterConsoleCommands()
{
	//SD4 A2
	g_theConsole->RegisterCommand( "ShowMap", Map::ShowMap );
}


//-----------------------------------------------------------------------------
TheGame::TheGame()
	: m_isQuitting( false )
	, m_foundSave( false ) 
	, m_currentMap( nullptr )
	, m_player( nullptr )
	, m_FADEOUT_LENGTH_SECONDS( 15.0f )
	, m_fadeoutTimer( 0.f )
{
	g_menuAcceptSoundID = g_theAudio->CreateOrGetSound( "Data/Audio/MenuAccept.wav" );;
	g_menuDeclineSoundID = g_theAudio->CreateOrGetSound( "Data/Audio/MenuDecline.wav" );;
}


//-----------------------------------------------------------------------------
TheGame::~TheGame()
{
	DestroyAllGameplayEntities();
}


//-----------------------------------------------------------------------------
const Camera3D* TheGame::GetActiveCamera() const
{
	return s_playerCamera;
}


//-----------------------------------------------------------------------------
void TheGame::LoadGame()
{
	m_foundSave = ( CountFilesInDirectory( "Data/XML/Saves", "*.Save.xml" ) > 0 );
	if ( !m_foundSave )
		return;

	std::string saveFilename = "Data/XML/Saves/Save000.Save.xml";

	const XMLNode& mapNode = XMLNode::openFileHelper( saveFilename.c_str(), "MapData" );
	m_currentMap = new Map( mapNode ); //Map reads in mapSize=, <Legend>, TileData, VisibilityData.

	const XMLNode& entityDataNode = mapNode.getChildNode( "EntityData" );
	std::map< EntityID, GameEntity* > loadedEntities;

	GameEntity* newEntity = nullptr;
	int childIndex = 0;
	for ( int numIteration = 0; numIteration < entityDataNode.nChildNode( "NPCBlueprint" ); numIteration++ ) //Incremented by getChildNode(str,&int).
	{
		const XMLNode& npcNode = entityDataNode.getChildNode( "NPCBlueprint", &childIndex );

		std::string factoryName = npcNode.getAttribute( "name" );

		std::map< std::string, NPCFactory* >::iterator found = NPCFactory::GetRegistry().find( factoryName );

		if ( found == NPCFactory::GetRegistry().end() )
		{
			DebuggerPrintf( "TheGame::LoadGame() failed to find NPCFactory for %s!", factoryName.c_str() );
			continue;
		}

		NPC* newNPC = found->second->CreateNPC( m_currentMap, npcNode );
		AddCarriedItemsToEntityListForAgent( newNPC );
		newEntity = newNPC;

		loadedEntities.insert( std::pair< EntityID, GameEntity* >( newEntity->GetSavedID(), newEntity ) );
		m_activeAgents.insert( TurnOrderedMapPair( .1f, (Agent*)newEntity ) ); //Not 0 so the player can precede them for first move.
	}

	childIndex = 0;
	for ( int numIteration = 0; numIteration < entityDataNode.nChildNode( "ItemBlueprint" ); numIteration++ ) //Incremented by getChildNode(str,&int).
	{
		XMLNode itemNode = entityDataNode.getChildNode( "ItemBlueprint", &childIndex );

		std::string factoryName = itemNode.getAttribute( "name" );
		std::string itemTypeAsString = itemNode.getAttribute( "type" );
		ItemType itemType = GetItemTypeForString( itemTypeAsString );

		ItemFactoryCategory& registry = ItemFactory::GetRegistryForItemType( itemType );

		ItemFactoryCategory::iterator found = registry.find( factoryName );

		if ( found == registry.end() )
		{
			DebuggerPrintf( "TheGame::LoadGame() failed to find ItemFactory for %s %s!", itemTypeAsString.c_str(), factoryName.c_str() );
			continue;
		}

		newEntity = found->second->CreateItem( m_currentMap, itemNode );

		loadedEntities.insert( std::pair< EntityID, GameEntity* >( newEntity->GetSavedID(), newEntity ) );
	}
	
	childIndex = 0;
	for ( int numIteration = 0; numIteration < entityDataNode.nChildNode( "FeatureBlueprint" ); numIteration++ ) //Incremented by getChildNode(str,&int).
	{
		XMLNode featureNode = entityDataNode.getChildNode( "FeatureBlueprint", &childIndex );

		std::string factoryName = featureNode.getAttribute( "name" );
		std::string featureTypeAsString = featureNode.getAttribute( "type" );
		FeatureType featureType = GetFeatureTypeForString( featureTypeAsString );

		FeatureFactoryCategory& registry = FeatureFactory::GetRegistryForFeatureType( featureType );

		FeatureFactoryCategory::iterator found = registry.find( factoryName );

		if ( found == registry.end() )
		{
			DebuggerPrintf( "TheGame::LoadGame() failed to find FeatureFactory for %s %s!", featureTypeAsString.c_str(), factoryName.c_str() );
			continue;
		}

		newEntity = found->second->CreateFeature( m_currentMap, featureNode );

		loadedEntities.insert( std::pair< EntityID, GameEntity* >( newEntity->GetSavedID(), newEntity ) );
	}

	ASSERT_OR_DIE( 1 == entityDataNode.nChildNode( "PlayerBlueprint" ), "Only One PlayerBlueprint!" );
	{
		//Player handle separately via m_player.
		const XMLNode& playerNode = entityDataNode.getChildNode( "PlayerBlueprint" );

		m_player = new Player( ENTITY_TYPE_PLAYER, m_currentMap, playerNode );
		newEntity = m_player;

		AddCarriedItemsToEntityListForAgent( m_player );
		loadedEntities.insert( std::pair< EntityID, GameEntity* >( newEntity->GetSavedID(), newEntity ) );
		m_activeAgents.insert( TurnOrderedMapPair( 0.f, m_player ) ); //Player will go first if all else inserted > 0.f.
	}

	for ( std::pair< EntityID, GameEntity* > entity : loadedEntities )
	{
		entity.second->ResolvePointersToEntities( loadedEntities ); //Function on GameEntity, override by Agent, Item, Feature, each GE type.
		m_livingEntities.push_back( entity.second );
	}

	g_theConsole->Printf( "Game successfully loaded from %s. File deleted.", saveFilename.c_str() );

	g_showFullMap = false;
	m_currentMap->RefreshCellOccupantVisibility();
	m_player->UpdateFieldOfView();

	g_theConsole->ShowConsole();

#define DELETE_ENABLED
#ifdef DELETE_ENABLED
	remove( saveFilename.c_str() );
	m_foundSave = false;
#endif

}


//-----------------------------------------------------------------------------
void TheGame::SaveGame()
{
	if ( m_player->IsAlive() == false )
	{
		m_foundSave = false; 
		return; //Nice try.
	}

	XMLNode mapNode = XMLNode::createXMLTopNode( "MapData" );
	m_currentMap->WriteToXMLNode( mapNode ); //Map writes out mapSize=, <Legend>, TileData, VisibilityData.
	XMLNode entityDataNode = mapNode.addChild( "EntityData" );

	for ( GameEntity* entity : m_livingEntities )
		entity->WriteToXMLNode( entityDataNode ); //A virtual function on the base GameEntity class.

	std::string saveFilename = Stringf( "Data/XML/Saves/Save000.Save.xml" );
	mapNode.writeToFile( saveFilename.c_str() );

	g_theConsole->ShowConsole();
	g_theConsole->Printf( "Game successfully saved to %s", saveFilename.c_str() );
	m_foundSave = true;
}


//-----------------------------------------------------------------------------
void TheGame::Startup()
{
	RegisterConsoleCommands();

	BiomeBlueprint::LoadAllBiomeBlueprints();
	Faction::LoadAllFactions(); //Put before NPCs so they can override these.
	ItemFactory::LoadAllItemBlueprints(); //Put before NPCs so some can be born with these.
	NPCFactory::LoadAllNPCBlueprints(); 
	FeatureFactory::LoadAllFeatureBlueprints();
}


//-----------------------------------------------------------------------------
void TheGame::Shutdown()
{
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::UpdateStartup( float /*deltaSeconds*/ )
{
	return SetGameState( GameState::GAME_STATE_MAIN_MENU );
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::UpdateMainMenu( float /*deltaSeconds*/ )
{
	static bool needToSearchSaves = true;
	if ( needToSearchSaves )
	{
		needToSearchSaves = false;
		if ( CountFilesInDirectory( "Data/XML/Saves", "*.Save.xml" ) > 0 )
			m_foundSave = true;
	}

	bool didUpdate = false;

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_BEGIN_GAME ) || g_theInput->WasButtonPressedOnce( BUTTON_TO_BEGIN_GAME, Controllers::CONTROLLER_ONE ) )
	{
		g_theAudio->PlaySound( g_menuAcceptSoundID );
		m_fadeoutTimer = m_FADEOUT_LENGTH_SECONDS; 
		didUpdate = SetGameState( GameState::GAME_STATE_MAP_SELECTION );
		needToSearchSaves = true;
	}

	if ( g_theInput->WasKeyPressedOnce( 'Q' ) || g_theInput->WasButtonPressedOnce( BUTTON_TO_EXIT_GAME, Controllers::CONTROLLER_ONE ) )
	{
		g_theAudio->PlaySound( g_menuDeclineSoundID );
		didUpdate = SetGameState( GameState::GAME_STATE_SHUTDOWN );
	}

	if ( m_foundSave && g_theInput->WasKeyPressedOnce( 'C' ) )
	{
		LoadGame();
		g_theAudio->PlaySound( g_menuAcceptSoundID );
		m_fadeoutTimer = m_FADEOUT_LENGTH_SECONDS;
		didUpdate = SetGameState( GameState::GAME_STATE_PLAYING );
		needToSearchSaves = true;
	}

	return didUpdate;
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::UpdateMapGenerating( float /*deltaSeconds*/ )
{
	bool didUpdate = false;

	if ( g_generationMode == GENERATION_MODE_AUTO ) //Then generate the entire map, and move to playing.
	{
		g_pickedBiome->FullyGenerateBlueprint( m_currentMap );

		FinalizeMap();

		didUpdate = SetGameState( GameState::GAME_STATE_PLAYING );
	}
	else g_showFullMap = true;

	TODO( "Compute cell dimensions and screen position once, store on cell." );

	if ( g_theInput->WasKeyPressedOnce( VK_SPACE ) || g_theInput->WasButtonPressedOnce( ControllerButtons::A_BUTTON, Controllers::CONTROLLER_ONE ) )
	{
		if ( g_generationMode == GENERATION_MODE_MANUAL_INFINITE_STEPS )
			g_pickedBiome->PartiallyGenerateBlueprint( m_currentMap, true );
		else
			g_pickedBiome->PartiallyGenerateBlueprint( m_currentMap, false );

	}
	
	if ( g_theInput->WasKeyJustPressed( VK_LEFT ) || g_theInput->WasButtonPressedOnce( LEFT_SHOULDER_BUTTON, CONTROLLER_ONE ) )
	{
		g_pickedBiome->PreviousProcess();
	}
	
	if ( g_theInput->WasKeyJustPressed( VK_RIGHT ) || g_theInput->WasButtonPressedOnce( RIGHT_SHOULDER_BUTTON, CONTROLLER_ONE ) )
	{
		g_pickedBiome->NextProcess();
	}
	
	if ( g_theInput->WasKeyPressedOnce( VK_ENTER ) || g_theInput->WasButtonPressedOnce( ControllerButtons::A_BUTTON, Controllers::CONTROLLER_ONE ) )
	{
		g_theAudio->PlaySound( g_menuAcceptSoundID );

		FinalizeMap();

		didUpdate = SetGameState( GameState::GAME_STATE_PLAYING );
	}
	
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_BEGIN_GAME ) || g_theInput->WasButtonPressedOnce( BUTTON_TO_BEGIN_GAME, Controllers::CONTROLLER_ONE ) )
	{
		g_theAudio->PlaySound( g_menuAcceptSoundID );
		didUpdate = SetGameState( GameState::GAME_STATE_PLAYING );
	}
	
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_EXIT_GAME ) || g_theInput->WasButtonPressedOnce( BUTTON_TO_EXIT_GAME, Controllers::CONTROLLER_ONE ) )
	{
		g_theAudio->PlaySound( g_menuDeclineSoundID );
		didUpdate = SetGameState( GameState::GAME_STATE_MAP_SELECTION );
	}

	return true;
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::FinalizeMap()
{
	Generator::FinalizeMap( m_currentMap );
	AddFeaturesToEntityListForMap( m_currentMap );
	PopulateCurrentMapWithNPCs_RangedRandom( 10, 15 );
	PopulateCurrentMapWithItems_RangedRandom( 10, 20 );
//	PopulateCurrentMapWithItems_OnePlusPerFactory( 1, 1 );
//	PopulateCurrentMapWithItems_OnePlusPerCategory( 1, 1 );
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::PopulateCurrentMapWithNPCs_RangedRandom( int minNPCsInclusive, int maxNPCsInclusive )
{
	std::map< std::string, NPCFactory* >& factories = NPCFactory::GetRegistry();
	int numNPCsToSpawn = GetRandomIntInRange( minNPCsInclusive, maxNPCsInclusive );
	for ( int factoryIteration = 0; factoryIteration < numNPCsToSpawn; factoryIteration++ )
	{
		//Get a random factory to use for this iteration.
		int randomFactoryIndex = GetRandomIntInRange( 0, factories.size() - 1 );
		std::map< std::string, NPCFactory* >::iterator factoryIter = factories.begin();
		std::advance( factoryIter, randomFactoryIndex );

		//Create the NPC for this iteration.
		NPCFactory* factory = factoryIter->second;
		AddNPC( factory, 1 );
	}
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::PopulateCurrentMapWithNPCs_OnePlusPerFactory( int minNPCsInclusive, int maxNPCsInclusive )
{
	//Spawn at least one of each type.
	std::map< std::string, NPCFactory* >& factories = NPCFactory::GetRegistry();
	for ( std::map< std::string, NPCFactory* >::iterator factoryIter = factories.begin(); factoryIter != factories.end(); ++factoryIter )
	{
		int numToInstantiate = GetRandomIntInRange( minNPCsInclusive, maxNPCsInclusive );
		NPCFactory* factory = factoryIter->second;
		AddNPC( factory, numToInstantiate );
	}
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::PopulateCurrentMapWithItems_RangedRandom( int minItemsInclusive, int maxItemsInclusive )
{
	int numItemsToSpawn = GetRandomIntInRange( minItemsInclusive, maxItemsInclusive );
	for ( int factoryIteration = 0; factoryIteration < numItemsToSpawn; factoryIteration++ )
	{
		//Get a random category of factory to use for this iteration.
		int randomItemCategoryIndex = GetRandomIntInRange( 0, NUM_ITEM_TYPES - 1 );
		std::map< std::string, ItemFactory* >& factories = ItemFactory::GetRegistryForItemType( (ItemType)randomItemCategoryIndex );

		//Get a random factory to use for this iteration.
		int randomFactoryIndex = GetRandomIntInRange( 0, factories.size() - 1 );
		std::map< std::string, ItemFactory* >::iterator factoryIter = factories.begin();
		std::advance( factoryIter, randomFactoryIndex );

		//Create the Item for this iteration.
		ItemFactory* factory = factoryIter->second;
		AddItem( factory, 1 );
	}
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::PopulateCurrentMapWithItems_OnePlusPerFactory( int minItemsInclusive, int maxItemsInclusive )
{
	//Spawn at least one of each type.
	for ( int itemCategoryIndex = 0; itemCategoryIndex < NUM_ITEM_TYPES; itemCategoryIndex++ )
	{
		ItemFactoryCategory& factories = ItemFactory::GetRegistryForItemType( (ItemType)itemCategoryIndex );
		for ( ItemFactoryCategory::iterator factoryIter = factories.begin(); factoryIter != factories.end(); ++factoryIter )
		{
			int numToInstantiate = GetRandomIntInRange( minItemsInclusive, maxItemsInclusive );
			ItemFactory* factory = factoryIter->second;
			AddItem( factory, numToInstantiate );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::PopulateCurrentMapWithItems_OnePlusPerCategory( int minItemsInclusive, int maxItemsInclusive )
{
	//Spawn at least one of each type.
	for ( int itemCategoryIndex = 0; itemCategoryIndex < NUM_ITEM_TYPES; itemCategoryIndex++ )
	{
		ItemFactoryCategory& factories = ItemFactory::GetRegistryForItemType( (ItemType)itemCategoryIndex );
		int randomFactoryIndex = GetRandomIntInRange( 0, factories.size() - 1 );
		int numToInstantiate = GetRandomIntInRange( minItemsInclusive, maxItemsInclusive );

		ItemFactoryCategory::iterator factoryIter = factories.begin();
		std::advance( factoryIter, randomFactoryIndex );

		ItemFactory* factory = factoryIter->second;
		AddItem( factory, numToInstantiate );
	}
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::AddNPC( NPCFactory* factory, int numToMake )
{
	ASSERT_OR_DIE( m_currentMap != nullptr, nullptr );

	for ( int numIteration = 0; numIteration < numToMake; numIteration++ )
	{
		NPC* newNPC = factory->CreateNPC();
		newNPC->AttachToMapAtPosition( m_currentMap, m_currentMap->GetRandomMapPosition( newNPC->GetTraversalProperties() ) );
		m_livingEntities.push_back( newNPC );
		m_activeAgents.insert( TurnOrderedMapPair( .1f, newNPC ) ); //Not 0 so the player can precede them for first move.
		m_currentMap->RefreshTraversableCells();
	}
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::AddItem( ItemFactory* factory, int numToMake )
{
	ASSERT_OR_DIE( m_currentMap != nullptr, nullptr );

	for ( int numIteration = 0; numIteration < numToMake; numIteration++ )
	{
		Item* newItem = factory->CreateItem();
		newItem->AttachToMapAtPosition( m_currentMap, m_currentMap->GetRandomMapPosition( BLOCKED_BY_SOLIDS | BLOCKED_BY_LAVA ) );
		m_livingEntities.push_back( newItem );
		m_currentMap->RefreshTraversableCells();
	}
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::UpdateMapSelection( float /*deltaSeconds*/ )
{
	bool didUpdate = false;

	unsigned int questNumber = 1;
	std::map< std::string, BiomeBlueprint*>::const_iterator biomeBlueprintIterEnd = BiomeBlueprint::GetRegistry().cend();

	for ( std::map< std::string, BiomeBlueprint*>::const_iterator biomeBlueprintIter = BiomeBlueprint::GetRegistry().cbegin(); biomeBlueprintIter != biomeBlueprintIterEnd; ++biomeBlueprintIter )
	{
		if ( g_theInput->WasKeyPressedOnce( (unsigned char)questNumber + '0' ) )
		{
			//Cleaned up in TheGame::Shutdown(), so we can just overwrite.
			g_pickedBiome = biomeBlueprintIter->second;

			if ( m_currentMap != nullptr )
				delete m_currentMap;
			m_currentMap = g_pickedBiome->InitializeBlueprint();

			g_theAudio->PlaySound( g_menuAcceptSoundID );
			return SetGameState( GameState::GAME_STATE_MAP_GENERATING );
		}
		++questNumber;
	}

	if ( g_theInput->WasKeyPressedOnce( VK_TAB ) || g_theInput->WasButtonPressedOnce( ControllerButtons::X_BUTTON, Controllers::CONTROLLER_ONE ) )
	{
		switch ( g_generationMode )
		{
		case GENERATION_MODE_AUTO: g_generationMode = GENERATION_MODE_MANUAL_FINITE_STEPS; break;
		case GENERATION_MODE_MANUAL_FINITE_STEPS: g_generationMode = GENERATION_MODE_MANUAL_INFINITE_STEPS; break;
		case GENERATION_MODE_MANUAL_INFINITE_STEPS: g_generationMode = GENERATION_MODE_AUTO; break;
		}

	}

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_EXIT_GAME ) || g_theInput->WasButtonPressedOnce( BUTTON_TO_EXIT_GAME, Controllers::CONTROLLER_ONE ) )
	{
		g_theAudio->PlaySound( g_menuDeclineSoundID );
		return SetGameState( GameState::GAME_STATE_MAIN_MENU );
	}

	return didUpdate;
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::UpdatePaused( float /*deltaSeconds*/ )
{
	bool didUpdate = false;

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_PAUSE_GAME ) || g_theInput->WasButtonPressedOnce( BUTTON_TO_PAUSE_GAME, Controllers::CONTROLLER_ONE ) )
	{
		g_theAudio->PlaySound( g_menuAcceptSoundID );
		didUpdate = SetGameState( GameState::GAME_STATE_PLAYING );
	}
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_EXIT_GAME ) || g_theInput->WasButtonPressedOnce( BUTTON_TO_EXIT_GAME, Controllers::CONTROLLER_ONE ) )
	{
		DestroyAllGameplayEntities();

		g_theAudio->PlaySound( g_menuDeclineSoundID );
		didUpdate = SetGameState( GameState::GAME_STATE_MAIN_MENU );
	}

	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::UpdatePlaying( float deltaSeconds )
{
	bool didUpdate = false;

	if ( m_player == nullptr )
		StartGameplay();

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_SAVE_GAME ) || g_theInput->WasKeyPressedOnce( KEY_TO_SAVE_GAME2 ) )
	{
		SaveGame();
		DestroyAllGameplayEntities();

		g_theAudio->PlaySound( g_menuDeclineSoundID );
		didUpdate = SetGameState( GAME_STATE_MAIN_MENU );
		return didUpdate;
	}

	if ( !m_player->IsAlive() )
		m_fadeoutTimer = GetMax( 0.f, m_fadeoutTimer - deltaSeconds );

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_PAUSE_GAME ) || g_theInput->WasButtonPressedOnce( BUTTON_TO_PAUSE_GAME, Controllers::CONTROLLER_ONE ) )
	{
		//Note currently not able to pause.
		g_theAudio->PlaySound( g_menuAcceptSoundID );
		didUpdate = SetGameState( GameState::GAME_STATE_PAUSED );
	}
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_EXIT_GAME ) || g_theInput->WasButtonPressedOnce( BUTTON_TO_PAUSE_GAME, Controllers::CONTROLLER_ONE ) )
	{
		DestroyAllGameplayEntities();

		g_theAudio->PlaySound( g_menuDeclineSoundID );
		didUpdate = SetGameState( GameState::GAME_STATE_MAIN_MENU );
		return didUpdate;
	}

	if ( !didUpdate )
		m_player->ProcessInput();

	UpdatePlayedMapSimulation( deltaSeconds );

	return didUpdate;
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::StartGameplay()
{
	g_showFullMap = false;

	g_theConsole->ClearConsoleLog();
	m_player = new Player( ENTITY_TYPE_PLAYER );
	m_player->AttachToMapAtPosition( m_currentMap, Generator::FindValidStartingPosition( m_currentMap ) );
	m_player->UpdateFieldOfView();
	
	//Be forewarned that NPCs have already been pushed into m_entities via PopulateMap call.
	m_livingEntities.push_back( m_player );	
	m_activeAgents.insert( TurnOrderedMapPair( 0.f, m_player ) ); //Player will go first if all else inserted > 0.f.
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::DestroyAllGameplayEntities()
{
	if ( m_currentMap != nullptr )
	{
		delete m_currentMap;
		m_currentMap = nullptr;
	}

	for ( GameEntity* ge : m_livingEntities )
	{
		if ( ge != nullptr )
		{
			delete ge;
			ge = nullptr;
		}
	}
	m_livingEntities.clear();
	m_activeAgents.clear();

	//Now player is deleted in the entities container above, so no delete m_player.
	m_player = nullptr; //Ensure TheGame's copy of it is also nullptr.
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::DestroyDeadGameplayEntities()
{
	bool didSeeDeath = false;
	bool didHearDeath = false;

	//Let the last loop (over the widest amount of entities) do the deleting!
	for ( TurnOrderedMap::iterator activeAgentIter = m_activeAgents.begin(); activeAgentIter != m_activeAgents.end(); )
	{
		Agent* currentActiveAgent = activeAgentIter->second;

		ASSERT_OR_DIE( currentActiveAgent != nullptr, "Null Found in TheGame::DestroyDeadGameplayEntities!" );

		if ( currentActiveAgent->IsAlive() == false )
		{
			if ( !currentActiveAgent->IsPlayer() )
			{
				didHearDeath = true;
				didSeeDeath = currentActiveAgent->IsCurrentlySeen();

				g_theConsole->ShowConsole();
				if ( didSeeDeath )
				{
					g_theConsole->SetTextColor( Rgba::RED * Rgba::GRAY );
					g_theConsole->Printf( "%s lost their ability to wake up!", currentActiveAgent->GetName().c_str() );
					g_theConsole->SetTextColor();
				}
			}

			UntargetAgent( currentActiveAgent );
			currentActiveAgent->Die();
			activeAgentIter = m_activeAgents.erase( activeAgentIter );
		}
		else ++activeAgentIter;
	}

	for ( std::vector<GameEntity*>::iterator entityIter = m_livingEntities.begin(); entityIter != m_livingEntities.end(); )
	{
		GameEntity* currentEntity = *entityIter;

		ASSERT_OR_DIE( currentEntity != nullptr, "Null Found in TheGame::DestroyDeadGameplayEntities!" );

		if ( currentEntity->IsAlive() == false )
		{
			if ( currentEntity->IsPlayer() )
				HandlePlayerDeath();
			else 
				delete currentEntity;

			entityIter = m_livingEntities.erase( entityIter );
		}
		else ++entityIter;
	}

	static SoundID deathSoundID = g_theAudio->CreateOrGetSound( "Data/Audio/EnemyDeath.wav" );
	if ( didHearDeath && m_player->IsAlive() )
		g_theAudio->PlaySound( deathSoundID, VOLUME_MULTIPLIER * .05f );

	if ( didHearDeath && !didSeeDeath )
	{
		g_theConsole->SetTextColor( Rgba::RED * Rgba::GRAY );
		g_theConsole->Printf( "From far off you hear a dream's dying wail, never to wake again..." );
		g_theConsole->SetTextColor();
	}
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::UntargetAgent( const Agent* agentToRemove )
{
	for ( TurnOrderedMap::iterator otherActiveAgentIter = m_activeAgents.begin(); otherActiveAgentIter != m_activeAgents.end(); ++otherActiveAgentIter )
	{
		Agent* otherActiveAgent = otherActiveAgentIter->second;

		if ( otherActiveAgent->IsAlive() == false )
			continue;

		if ( otherActiveAgent->GetTargetEnemy() == agentToRemove )
			otherActiveAgent->SetTargetEnemy( nullptr );

		otherActiveAgent->RemoveRelationsWithEntity( agentToRemove );
	}
}


//--------------------------------------------------------------------------------------------------------------
static FMOD::Channel* g_backgroundMusic = nullptr;
void TheGame::HandlePlayerDeath()
{
	static SoundID gameOverSoundID = g_theAudio->CreateOrGetSound( "Data/Audio/dead_music.s3m" );

	if ( g_backgroundMusic == nullptr ) 
		g_backgroundMusic = g_theAudio->PlaySound( gameOverSoundID, VOLUME_MULTIPLIER );

	g_showFullMap = true;

	g_theConsole->HideConsole();
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::UpdatePlayedMapSimulation( float deltaSeconds )
{
	CooldownSeconds duration = 0.f;
	bool isSimulating = true;
	bool shouldAdvanceSimulationTimer = true;

	while ( isSimulating ) //Enables us to stop it at some # actions and debug for infinite loops.
	{
		TurnOrderedMap::iterator agentIter = m_activeAgents.begin();
		Agent* agent = agentIter->second;

		MapPosition currentEntityPos = agent->GetPositionMins();

		if ( m_currentMap->GetCellForPosition( currentEntityPos ).IsCurrentlySeen() )
			agent->SetSeen();
		else
			agent->SetUnseen();

		if ( agentIter->first > g_mapSimulationTimer )
			break; //We haven't yet reached the time at which this agent takes its turn.

		if ( !agent->IsReadyToUpdate() )
		{
			shouldAdvanceSimulationTimer = false;
			break;
		}

		m_activeAgents.erase( agentIter );

		if ( agent->IsAlive() )
			duration = agent->Update( deltaSeconds ); //Allows returning varied cooldown amounts depending on actions.

		if ( agent->IsAlive() ) //Be sure you kill yourself in Update above!
		{
			m_activeAgents.insert( TurnOrderedMapPair( g_mapSimulationTimer + duration, agent ) ); //Note they may actually go again next while loop iteration!
				//You can make player actions near-no-delay while returning .01, etc.
				//Simulation clock can also be faster than real-time.
				//Can include a bool to allow things NOT to wait on player, i.e. be more real-time.
				//Because of this, will empty out until player is the only one left at the top of the map--anyone coming in after will wait on the player.
		}

		DestroyDeadGameplayEntities();
			//If not called here, but instead outside this loop, one could kill a next Agent to be updated.
	}

	if ( shouldAdvanceSimulationTimer )
		g_mapSimulationTimer += g_mapSimulationDelta;
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::UpdateShutdown( float /*deltaSeconds*/ )
{
	m_isQuitting = true;

	g_theAudio->PlaySound( g_menuDeclineSoundID );
	return BiomeBlueprint::ClearRegistry();
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::Update( float deltaSeconds )
{
	if ( g_theInput->WasKeyPressedOnce( 'X' ) && g_backgroundMusic != nullptr )
		g_backgroundMusic->stop();

	if ( g_theInput->WasKeyPressedOnce( 'M' ) )
		g_theConsole->RunCommand( "ShowMap" );

	//Debug GameCommon flag setting. Toggling back and forth will cause some chunks to become and stay dirty until updated (usually by player ray-cast dirtying VAO), hence it's just for debug.
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_DEBUG_INFO ) ) g_inDebugMode = !g_inDebugMode;


	//-----------------------------------------------------------------------------
	TODO( "MOVE INSIDE THE ENGINE SINGLETON'S UPDATE, CALLED FROM MAIN_WIN32" );
	//-----------------------------------------------------------------------------
	UpdateDebugCommands( deltaSeconds );

	bool didUpdate = false;
	switch ( GetGameState() )
	{
	case GameState::GAME_STATE_STARTUP: didUpdate = UpdateStartup( deltaSeconds ); break;
	case GameState::GAME_STATE_MAIN_MENU: didUpdate = UpdateMainMenu( deltaSeconds ); break;
	case GameState::GAME_STATE_MAP_SELECTION: didUpdate = UpdateMapSelection( deltaSeconds ); break;
	case GameState::GAME_STATE_MAP_GENERATING: didUpdate = UpdateMapGenerating( deltaSeconds ); break;
	case GameState::GAME_STATE_PLAYING: didUpdate = UpdatePlaying( deltaSeconds ); break;
	case GameState::GAME_STATE_PAUSED: didUpdate = UpdatePaused( deltaSeconds ); break;
	case GameState::GAME_STATE_SHUTDOWN: didUpdate = UpdateShutdown( deltaSeconds ); break;
	}

	return didUpdate;

}
