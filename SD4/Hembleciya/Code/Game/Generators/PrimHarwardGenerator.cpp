#include "Game/Generators/PrimHarwardGenerator.hpp"

#include "Game/Map.hpp"

#include "Engine/Math/MathUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC GeneratorRegistration PrimHarwardGenerator::s_dungeon = GeneratorRegistration( "Dungeon", &PrimHarwardGenerator::CreateGenerator, &Generator::CreateBiomeCreationProcess );
STATIC GeneratorRegistration PrimHarwardGenerator::s_sandbar = GeneratorRegistration( "Sandbar", &PrimHarwardGenerator::CreateGenerator, &Generator::CreateBiomeCreationProcess );


//--------------------------------------------------------------------------------------------------------------
PrimHarwardGenerator::~PrimHarwardGenerator()
{
	for ( AABB2i* room : m_rooms )
		if ( room != nullptr )
			delete room;
}


//--------------------------------------------------------------------------------------------------------------
bool PrimHarwardGenerator::GenerateRoom( Map* map, Vector2i roomCenterToAdd /*= Vector2i::ZERO*/ )
{
	if ( roomCenterToAdd.x < 0 || roomCenterToAdd.y < 0 )
		return false;

	Vector2i mapSize = map->GetDimensions();
	if ( roomCenterToAdd.x >= mapSize.x || roomCenterToAdd.y >= mapSize.y )
		return false;

	//Make able to be read in via Environment!
	const int MIN_ROOM_WIDTH = 6;
	const int MIN_ROOM_HEIGHT = 5;
	const int MAX_ROOM_WIDTH = 10;
	const int MAX_ROOM_HEIGHT = 10;

	//-----------------------------------------------------------------------------
	//1. Pick random x, y.
	int x;
	int y;

	if ( roomCenterToAdd == Vector2i::ZERO )
	{
		x = GetRandomIntInRange( 1, mapSize.x - 2 ); //Prevents adding rooms on map's perimeter.
		y = GetRandomIntInRange( 1, mapSize.y - 2 );
	}
	else
	{
		x = roomCenterToAdd.x;
		y = roomCenterToAdd.y;
	}

	//-----------------------------------------------------------------------------
	//2. Block it out as room with some w, h.
	int w = GetRandomIntInRange( MIN_ROOM_WIDTH, MAX_ROOM_WIDTH );
	while ( w > ( ( mapSize.x - 1 ) - x ) ) //Too wide to fit on map.
	{
		--w;
		if ( w < MIN_ROOM_WIDTH )
			return false;
	}

	int h = GetRandomIntInRange( MIN_ROOM_HEIGHT, MAX_ROOM_HEIGHT );
	while ( h > ( mapSize.y - 1 ) - y ) //Too tall to fit on map.
	{
		--h;
		if ( h < MIN_ROOM_HEIGHT )
			return false;
	}

	AABB2i roomBounds = AABB2i( x, y, x + w, y + h );

	//-----------------------------------------------------------------------------
	//3. Check every room against the new bounds, if there's overlap return (trying again could cause endless loop).
	if ( roomCenterToAdd != Vector2i::ZERO ) //Don't worry about overlap if it's a fixed point.
	{
		for ( AABB2i* room : m_rooms )
		{
			if ( DoAABBsOverlap( *room, roomBounds ) )
				return false;
		}
	}
	AABB2i fudgedBounds; //Allows some overlap by storing a slightly smaller bounds.
	fudgedBounds.maxs = roomBounds.maxs - Vector2i::ONE;
	fudgedBounds.mins = roomBounds.mins + Vector2i::ONE;
	m_rooms.push_back( new AABB2i( fudgedBounds ) );

	//Set cell types.
	for ( int cellX = x; cellX < ( x + w ); cellX++ )
	{
		for ( int cellY = y; cellY < ( y + h ); cellY++ )
		{
			Vector2i cellPos( cellX, cellY );

			//Makes room a sphere:
			int ROOM_RADIUS_SQUARED = ( MIN_ROOM_WIDTH - 2 ) * ( MIN_ROOM_HEIGHT - 2 );
			if ( CalcDistSquaredBetweenPoints( roomBounds.GetCenter(), cellPos ) < ROOM_RADIUS_SQUARED )
			{
				map->SetCellTypeForIndex( map->GetIndexForPosition( cellPos ), CELL_TYPE_AIR );
				map->GetTraversableCells().push_back( cellPos );
			}
		}
	}

	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool PrimHarwardGenerator::GenerateHallAndRoom( Map* map )
{
	bool didGenerateStep = false;

	//Make able to be read in via Environment!
	const int MIN_HALL_LENGTH = 3;
	const int MIN_ROOM_WIDTH = 6;
	const int MIN_ROOM_HEIGHT = 5;
	const int MAX_HALL_LENGTH = 6;
	const int MAX_ROOM_WIDTH = 10;
	const int MAX_ROOM_HEIGHT = 10;

	//-----------------------------------------------------------------------------
	//1. Pick random air tile, but one that's next to a wall.
	std::vector< Vector2i > airCells = map->GetTraversableCells();

	Vector2i cellPos;
	int airCellIndex = -1;
	MapDirection wallDirection;
	do
	{
		airCellIndex = GetRandomIntInRange( 0, airCells.size()-1 );
		cellPos = airCells[ airCellIndex ];
		wallDirection = map->IsCellAdjacentToType( cellPos, CELL_TYPE_STONE_WALL, true );

	} while ( wallDirection == DIRECTION_NONE );

	//-----------------------------------------------------------------------------
	//2. Step in wall's direction MIN_HALL_LENGTH to MAX_HALL_LENGTH steps.

	Vector2i newHallCellPos;
	unsigned int hallLength = GetRandomIntInRange( MIN_HALL_LENGTH, MAX_HALL_LENGTH );

	//Try to place a room at the end of the hall, if it works, create the hall.
	if ( GenerateRoom( map, cellPos + ( GetPositionDeltaForMapDirection( wallDirection ) * hallLength ) ) )
	{
		for ( unsigned int i = 0; i <= hallLength; i++ )
		{
			newHallCellPos = cellPos + ( GetPositionDeltaForMapDirection( wallDirection ) * i );
			map->SetCellTypeForIndex( newHallCellPos, CELL_TYPE_STONE_FLOOR );
			airCells.push_back( newHallCellPos );

			switch ( wallDirection )
			{
			case DIRECTION_UP_LEFT:
				map->SetCellTypeForIndex( newHallCellPos - Vector2i::UNIT_X, CELL_TYPE_STONE_FLOOR );
				map->SetCellTypeForIndex( newHallCellPos + Vector2i::UNIT_Y, CELL_TYPE_STONE_FLOOR );
				airCells.push_back( newHallCellPos - Vector2i::UNIT_X );
				airCells.push_back( newHallCellPos + Vector2i::UNIT_Y );
				break;
			case DIRECTION_UP_RIGHT:
				map->SetCellTypeForIndex( newHallCellPos + Vector2i::UNIT_X, CELL_TYPE_STONE_FLOOR );
				map->SetCellTypeForIndex( newHallCellPos + Vector2i::UNIT_Y, CELL_TYPE_STONE_FLOOR );
				airCells.push_back( newHallCellPos + Vector2i::UNIT_X );
				airCells.push_back( newHallCellPos + Vector2i::UNIT_Y ); 
				break;
			case DIRECTION_DOWN_LEFT:
				map->SetCellTypeForIndex( newHallCellPos - Vector2i::UNIT_X, CELL_TYPE_STONE_FLOOR );
				map->SetCellTypeForIndex( newHallCellPos - Vector2i::UNIT_Y, CELL_TYPE_STONE_FLOOR );
				airCells.push_back( newHallCellPos - Vector2i::UNIT_X );
				airCells.push_back( newHallCellPos - Vector2i::UNIT_Y ); 
				break;
			case DIRECTION_DOWN_RIGHT:
				map->SetCellTypeForIndex( newHallCellPos + Vector2i::UNIT_X, CELL_TYPE_STONE_FLOOR );
				map->SetCellTypeForIndex( newHallCellPos - Vector2i::UNIT_Y, CELL_TYPE_STONE_FLOOR );
				airCells.push_back( newHallCellPos + Vector2i::UNIT_X );
				airCells.push_back( newHallCellPos - Vector2i::UNIT_Y ); 
				break;
			default: break;
			}
		}
		didGenerateStep = true;
	}

	return didGenerateStep;
}


//--------------------------------------------------------------------------------------------------------------
bool PrimHarwardGenerator::GenerateOneStep( Map* map, int currentStepNumber, BiomeGenerationProcess* metadata )
{
	UNUSED( currentStepNumber );
	UNUSED( metadata );

	bool isSandbar = (m_name == "Sandbar");
	bool didGenerateRoom = false;
	bool didGenerateHallAndRoom = false;

	if ( map->GetTraversableCells().size() == 0 ) //Until we get a room down to start from,
	{
		didGenerateRoom = GenerateRoom( map );
	}
	else
	{
		didGenerateHallAndRoom = GenerateHallAndRoom( map );
	}

	//Continue on for sandbar effect! Or just to post-process open halls.
	if ( didGenerateRoom || didGenerateHallAndRoom || isSandbar )
	{
		for ( int x = 0; x < map->GetDimensions().x; x++ )
		{
			for ( int y = 0; y < map->GetDimensions().y; y++ )
			{
				Vector2i cellPos = Vector2i( x, y );
				int cellIndex = map->GetIndexForPosition( cellPos );

				if ( map->GetCells().at( cellIndex ).m_cellType != CELL_TYPE_STONE_WALL )
					continue;

				if ( isSandbar && map->GetNumNeighborsAroundCellOfType( cellPos, CELL_TYPE_STONE_WALL, 1.f ) <= 4 )
				{
					map->SetCellTypeForIndex( cellIndex, ( x > map->GetDimensions().x / 2 ) ? CELL_TYPE_LAVA : CELL_TYPE_WATER );
					map->GetTraversableCells().push_back( cellPos );
				}
				if ( map->GetNumNeighborsAroundCellOfType( cellPos, CELL_TYPE_STONE_WALL, 1.f ) <= 3 )
				{
					map->SetCellTypeForIndex( cellIndex, CELL_TYPE_STONE_FLOOR );
					map->GetTraversableCells().push_back( cellPos );
				}
			}
		}
	}

	return didGenerateRoom || didGenerateHallAndRoom;
}
