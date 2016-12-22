#include "Game/GameCommon.hpp"
#include "Engine/String/StringUtils.hpp"
#include "Game/Generators/Generator.hpp"
#include "Game/Biomes/BiomeBlueprint.hpp"
#include "Engine/Core/TheConsole.hpp"
#include "Game/Cell.hpp"
#include "Game/Inventory.hpp"


//--------------------------------------------------------------------------------------------------------------
//Game State Machine
//--------------------------------------------------------------------------------------------------------------
static GameState currentState = GAME_STATE_STARTUP;
static GameState previousState;

//-----------------------------------------------------------------------------
GameState GetGameState() { return currentState; }


//-----------------------------------------------------------------------------
GameState GetPreviousGameState() { return previousState; }


//-----------------------------------------------------------------------------
const char* GetGameStateName( GameState state )
{
	switch ( state )
	{
		case GameState::GAME_STATE_MAIN_MENU: return "MAIN_MENU";
		case GameState::GAME_STATE_MAP_GENERATING: return "MAP_GENERATING";
		case GameState::GAME_STATE_MAP_SELECTION: return "MAP_SELECTION";
		case GameState::NUM_GAME_STATES: return "NUM_GAME_STATES";
		case GameState::GAME_STATE_PAUSED: return "PAUSED";
		case GameState::GAME_STATE_PLAYING: return "PLAYING";
		case GameState::GAME_STATE_SHUTDOWN: return "SHUTDOWN";
		case GameState::GAME_STATE_STARTUP: return "STARTUP";
		default: DebuggerPrintf( "GetStateName Default Branch Unexpectedly Hit!" ); return "Not a State!";
	}
}


//-----------------------------------------------------------------------------
Vector2f GetScreenPositionForMapPosition( const MapPosition& mapPos )
{
	Vector2f mapPosAsFloat(
		static_cast<float>( mapPos.x ),
		static_cast<float>( mapPos.y )
		);

	return mapPosAsFloat * 24.f + Vector2f( 40.f, 100.f );
}


//-----------------------------------------------------------------------------
bool SetGameState( GameState newState )
{
	bool didChange = false; //Recommends that unless it's so-simple like a getter, that something could fail, keep it a bool retval for success of operation.

	GameState oldState = GetGameState();
	if ( oldState != newState ) //notice no m_state.
	{
		std::string result = Stringf( "SetState Change: %d => %d!", oldState, newState );
		if ( g_inDebugMode )
			g_theConsole->Printf( result.c_str() );
		else
			DebuggerPrintf( result.c_str() );
		oldState = currentState;
		currentState = newState;
		didChange = true;
	}
	else ERROR_AND_DIE( Stringf( "SetState Trying to Change To Same State: %s!", GetGameStateName( newState ) ) );

	return didChange;
}


//--------------------------------------------------------------------------------------------------------------
// Map
//--------------------------------------------------------------------------------------------------------------
bool g_showFullMap = true;
float g_mapSimulationDelta = 1.f;
float g_mapSimulationTimer = 0.f;


//-----------------------------------------------------------------------------
char GetGlyphForCellType( CellType type )
{
	return (char)type;
}


//-----------------------------------------------------------------------------
extern char GetGlyphForCell( const Cell& cell, bool ignoreItem ) //Handles dream cells being most any glyph.
{
	if ( cell.m_cellType == CELL_TYPE_DREAM )
		return cell.m_parsedMapGlyph;
	else if ( ignoreItem || !cell.HasItems() )
		return GetGlyphForCellType( cell.m_cellType );
	else if ( cell.HasMoreItemsThan( 1 ) )
	{
		cell.m_occupyingItems->HideItems();
		return '*';
	}
	else
	{
		cell.m_occupyingItems->ShowItem();
		return cell.GetGlyphForTopItem();
	}
}

//-----------------------------------------------------------------------------
Rgba GetColorForCellType( CellType type )
{
	switch ( type )
	{
	case CELL_TYPE_AIR: return Rgba::BLACK;
	case CELL_TYPE_STONE_FLOOR: return Rgba( .4f, .4f, .4f );
	case CELL_TYPE_STONE_WALL: return Rgba::WHITE;
	case CELL_TYPE_WATER: return Rgba::BLUE;
	case CELL_TYPE_LAVA: return Rgba::RED;
	}
	return Rgba::MAGENTA;
}


//--------------------------------------------------------------------------------------------------------------
//Generation
//--------------------------------------------------------------------------------------------------------------
BiomeBlueprint* g_pickedBiome = nullptr;
GenerationMode g_generationMode = GENERATION_MODE_AUTO;

//--------------------------------------------------------------------------------------------------------------
MapPosition GetPositionDeltaForMapDirection( MapDirection direction )
{
	switch ( direction )
	{
		case DIRECTION_LEFT: return -MapPosition::UNIT_X;
		case DIRECTION_RIGHT: return MapPosition::UNIT_X;
		case DIRECTION_DOWN: return -MapPosition::UNIT_Y;
		case DIRECTION_UP: return MapPosition::UNIT_Y;
		case DIRECTION_DOWN_LEFT: return -MapPosition::ONE;
		case DIRECTION_DOWN_RIGHT: return MapPosition::UNIT_X - MapPosition::UNIT_Y;
		case DIRECTION_UP_LEFT: return MapPosition::UNIT_Y - MapPosition::UNIT_X;
		case DIRECTION_UP_RIGHT: return MapPosition::ONE;
		default: return MapPosition::ZERO;
	}
}


//--------------------------------------------------------------------------------------------------------------
MapDirection GetDirectionBetweenMapPositions( const MapPosition& startFromPos, const MapPosition& endToPos )
{
	Vector2i displacement = endToPos - startFromPos;
	if ( displacement.x > 0 )
	{
		if ( displacement.y > 0 )
			return DIRECTION_UP_RIGHT;
		else if ( displacement.y < 0 )
			return DIRECTION_DOWN_RIGHT;
		else
			return DIRECTION_RIGHT;
	}
	else if ( displacement.x < 0 )
	{
		if ( displacement.y > 0 )
			return DIRECTION_UP_LEFT;
		else if ( displacement.y < 0 )
			return DIRECTION_DOWN_LEFT;
		else
			return DIRECTION_LEFT;
	}
	else //displacement.x == 0 :
	{
		if ( displacement.y > 0 )
			return DIRECTION_UP;
		else if ( displacement.y < 0 )
			return DIRECTION_DOWN;
		else
			return DIRECTION_NONE;
	}

}