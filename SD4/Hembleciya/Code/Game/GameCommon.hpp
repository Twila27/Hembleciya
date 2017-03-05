#pragma once


//--------------------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------------------------------------------------
#include "Engine/EngineCommon.hpp"
	ROADMAP( "Create a GameContext/EngineContext class replacing GameCommon/EngineCommon." );
#include "Engine/EngineCommon.hpp"
#include "Engine/Renderer/DebugRenderCommand.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Matrix4x4.hpp"
#include "Engine/Input/TheInput.hpp"
#include "Engine/Memory/BitUtils.hpp"
#include <map>


//--------------------------------------------------------------------------------------------------------------
// Forward Declarations
//--------------------------------------------------------------------------------------------------------------
class Generator;
class BiomeBlueprint;
class BitmapFont;
class Agent;
class Item;


//--------------------------------------------------------------------------------------------------------------
// Macros
//--------------------------------------------------------------------------------------------------------------
#define UNUSED(x) (void)(x);
#define STATIC
#define VK_SPACE 0x20
#define VK_F2 0x71
#define VK_F3 0x72
#define VK_F4 0x73
#define VK_F6 0x75
#define VK_OEM_COMMA 0xBC
#define VK_OEM_PERIOD 0xBE


//--------------------------------------------------------------------------------------------------------------
// Operating System
//--------------------------------------------------------------------------------------------------------------
static const char* g_appName = "Save/Load, Items, Features, Terrain (Hembleciya Final) by Benjamin D. Gibson";


//--------------------------------------------------------------------------------------------------------------
// Audio
//--------------------------------------------------------------------------------------------------------------
static const float VOLUME_MULTIPLIER = .15f; //1.0f is default.


//--------------------------------------------------------------------------------------------------------------
// Keybinds
//--------------------------------------------------------------------------------------------------------------
static const char KEY_TO_TOGGLE_GODMODE = VK_F2;
static const char KEY_TO_TEST_PATHING_ONESTEP = VK_F3;
static const char KEY_TO_TEST_PATHING_MULTISTEP = VK_F4;
static const int KEY_TO_PICK_UP_ITEM = VK_OEM_COMMA;
static const int KEY_TO_DROP_FIRST_UNEQUIPPED_ITEM = VK_OEM_PERIOD;
static const char KEY_TO_USE_POTION = 'Q';
static const char KEY_TO_TOGGLE_FEATURE = 'A';
static const char KEY_TO_REST = 'R';
static const char KEY_TO_MUTE_MUSIC = 'X';
static const char KEY_TO_TOGGLE_SHOW_MAP = 'M';


//-----------------------------------------------------------------------------
// Control Scheme #1: Roguelike Standard
//-----------------------------------------------------------------------------
static const char KEY_TO_MOVE_LEFT = 'H';
static const char KEY_TO_MOVE_RIGHT = 'L';
static const char KEY_TO_MOVE_UP = 'K';
static const char KEY_TO_MOVE_DOWN = 'J';
static const char KEY_TO_MOVE_UP_LEFT = 'Y';
static const char KEY_TO_MOVE_UP_RIGHT = 'U';
static const char KEY_TO_MOVE_DOWN_LEFT = 'B';
static const char KEY_TO_MOVE_DOWN_RIGHT = 'N';

//-----------------------------------------------------------------------------
// Control Scheme #2: Numpad
//-----------------------------------------------------------------------------
static const char KEY_TO_MOVE_LEFT2 = VK_NUMPAD4;
static const char KEY_TO_MOVE_RIGHT2 = VK_NUMPAD6;
static const char KEY_TO_MOVE_UP2 = VK_NUMPAD8;
static const char KEY_TO_MOVE_DOWN2 = VK_NUMPAD2;
static const char KEY_TO_MOVE_UP_LEFT2 = VK_NUMPAD7;
static const char KEY_TO_MOVE_UP_RIGHT2 = VK_NUMPAD9;
static const char KEY_TO_MOVE_DOWN_LEFT2 = VK_NUMPAD1;
static const char KEY_TO_MOVE_DOWN_RIGHT2 = VK_NUMPAD3;

//-----------------------------------------------------------------------------
// Control Scheme #3: Arrow Keys
//-----------------------------------------------------------------------------
static const char KEY_TO_MOVE_LEFT3 = VK_LEFT;
static const char KEY_TO_MOVE_RIGHT3 = VK_RIGHT;
static const char KEY_TO_MOVE_UP3 = VK_UP;
static const char KEY_TO_MOVE_DOWN3 = VK_DOWN;

//-----------------------------------------------------------------------------
// Scene Flow Transition Keys
//-----------------------------------------------------------------------------
static const char KEY_TO_BEGIN_GAME = 'N';
static const ControllerButtons BUTTON_TO_BEGIN_GAME = ControllerButtons::START_BUTTON;
static const char KEY_TO_PAUSE_GAME = VK_ESCAPE;
static const ControllerButtons BUTTON_TO_PAUSE_GAME = ControllerButtons::START_BUTTON;
static const char KEY_TO_EXIT_GAME = VK_ESCAPE; //Except from main menu, where it's Q.
static const ControllerButtons BUTTON_TO_EXIT_GAME = ControllerButtons::BACK_BUTTON;
static const char KEY_TO_SAVE_GAME = 'S';
static const char KEY_TO_SAVE_GAME2 = VK_F6;


//--------------------------------------------------------------------------------------------------------------
// Entity System
//--------------------------------------------------------------------------------------------------------------
enum EntityType {
	ENTITY_TYPE_ITEM,
	ENTITY_TYPE_NPC,
	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_FEATURE,
	NUM_ENTITY_TYPES
};

//-----------------------------------------------------------------------------
typedef std::multimap< float, Agent* > TurnOrderedMap;
typedef std::pair< float, Agent* > TurnOrderedMapPair;
typedef std::multimap< float, Agent* > ProximityOrderedAgentMap; //Closest first.
typedef std::pair< float, Agent* > ProximityOrderedAgentMapPair;
typedef std::multimap< float, Item* > ProximityOrderedItemMap; //Closest first.
typedef std::pair< float, Item* > ProximityOrderedItemMapPair;
typedef std::multimap< float, void* > ProximityOrderedFeatureMap; //Closest first.
typedef std::pair< float, void* > ProximityOrderedFeatureMapPair;
typedef int EntityID;
typedef int FactionID;


//--------------------------------------------------------------------------------------------------------------
// Game State Machine
//--------------------------------------------------------------------------------------------------------------
enum GameState { 
	GAME_STATE_STARTUP, 
	GAME_STATE_SHUTDOWN, 
	GAME_STATE_MAIN_MENU, 
	GAME_STATE_MAP_SELECTION, 
	GAME_STATE_MAP_GENERATING, 
	GAME_STATE_PLAYING, 
	GAME_STATE_PAUSED, 
	NUM_GAME_STATES 
};

//-----------------------------------------------------------------------------
extern GameState GetGameState();
extern bool SetGameState( GameState newState );
extern const char* GetGameStateName( GameState state );


//--------------------------------------------------------------------------------------------------------------
// Map
//--------------------------------------------------------------------------------------------------------------
enum TraversalProperties : bitfield_int
	//Has 32 fields in the below member int.  Similar to Vertex formats and limits.
	//std::bit_container or struct MovementFlags if more needed.
{
	//e.g. flying == ignore water and lava == 0 for bits 3 through 6.
	BLOCKED_BY_SOLIDS = GET_BIT_AT_BITFIELD_INDEX( 0 ),
	BLOCKED_BY_AIR = GET_BIT_AT_BITFIELD_INDEX( 1 ),
	BLOCKED_BY_AGENTS = GET_BIT_AT_BITFIELD_INDEX( 2 ),
	BLOCKED_BY_LAVA = GET_BIT_AT_BITFIELD_INDEX( 3 ),
	SLOWED_BY_LAVA = GET_BIT_AT_BITFIELD_INDEX( 4 ),
	BLOCKED_BY_WATER = GET_BIT_AT_BITFIELD_INDEX( 5 ),
	SLOWED_BY_WATER = GET_BIT_AT_BITFIELD_INDEX( 6 ),
	ONLY_MOVES_DIAGONAL = GET_BIT_AT_BITFIELD_INDEX( 7 ),
	BLOCKED_BY_DIAGONAL = GET_BIT_AT_BITFIELD_INDEX( 8 )
};
enum MapDirection {
	DIRECTION_NONE,
	DIRECTION_UP,
	DIRECTION_DOWN,
	DIRECTION_LEFT,
	DIRECTION_RIGHT,
	DIRECTION_DOWN_LEFT,
	DIRECTION_DOWN_RIGHT,
	DIRECTION_UP_LEFT,
	DIRECTION_UP_RIGHT,
	NUM_DIRECTIONS
};
enum CellType : char {
	CELL_TYPE_AIR = '.',
	CELL_TYPE_STONE_WALL = '#',
	CELL_TYPE_STONE_FLOOR = ':',
	CELL_TYPE_WATER = '~',
	CELL_TYPE_LAVA = '$',
	CELL_TYPE_DREAM //Do not define below, acts as catch-all supporting arbitrary dream glyphs.
};
static const char NON_DREAM_GLYPHS[] = { '.', '#', ':', '~', '$' };

//-----------------------------------------------------------------------------
extern bool g_showFullMap;
extern float g_mapSimulationDelta; //The speed at which you watch them be rendered across the map as they take actions.
extern float g_mapSimulationTimer;
static const float CELL_FONT_SCALE = .3f;
static const BitmapFont* CELL_FONT_OBJECT = nullptr; //Default Renderer font.
static const int DEFAULT_VIEW_RADIUS = 10;
static const int LAVA_DAMAGE_PER_TURN = 1;

typedef float CooldownSeconds;
static const CooldownSeconds NO_TURN_COOLDOWN = 0.f;
static const CooldownSeconds DEFAULT_TURN_COOLDOWN = 1.f;

typedef float UtilityValue;
static const UtilityValue NO_UTILITY_VALUE = 0.f;
static const UtilityValue MIN_UTILITY_VALUE = 1.f;
static const UtilityValue HIGH_UTILITY_VALUE = 100.f;


//-----------------------------------------------------------------------------
class Cell;
extern char GetGlyphForCellType( CellType type );
extern char GetGlyphForCell( const Cell& cell, bool ignoreItem ); //Distinct for saving out the wild-card dream glyphs.
extern Rgba GetColorForCellType( CellType type );


//--------------------------------------------------------------------------------------------------------------
// Map Generation
//--------------------------------------------------------------------------------------------------------------
enum GenerationMode {
	GENERATION_MODE_AUTO,
	GENERATION_MODE_MANUAL_FINITE_STEPS,
	GENERATION_MODE_MANUAL_INFINITE_STEPS,
	NUM_GENERATION_MODES
};

//-----------------------------------------------------------------------------
extern GenerationMode g_generationMode;
extern BiomeBlueprint* g_pickedBiome;



//--------------------------------------------------------------------------------------------------------------
// Coordinate System
//--------------------------------------------------------------------------------------------------------------
typedef int CellIndex;
typedef Vector2i MapPosition; //May be negative.

extern MapDirection GetDirectionBetweenMapPositions( const MapPosition& startFromPos, const MapPosition& endToPos );
extern MapPosition GetPositionDeltaForMapDirection( MapDirection direction );
extern Vector2f GetScreenPositionForMapPosition( const MapPosition& mapPos );
extern Matrix4x4f GetWorldChangeOfBasis( Ordering ordering );


//--------------------------------------------------------------------------------------------------------------
// Field of View
//--------------------------------------------------------------------------------------------------------------
enum RaycastMode {
	STEP_AND_SAMPLE, 
	AMANATIDES_WOO, 
	NUM_RAYCAST_MODES
};
