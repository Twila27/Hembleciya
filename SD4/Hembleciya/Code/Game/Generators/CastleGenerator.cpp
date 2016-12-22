#include "Game/Generators/CastleGenerator.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC GeneratorRegistration CastleGenerator::s_metropolis( "Castle", &CastleGenerator::CreateGenerator, &Generator::CreateBiomeCreationProcess );


//--------------------------------------------------------------------------------------------------------------
bool CastleGenerator::BuildEntrance( Map* map, int currentStepNumber )
{
	UNREFERENCED( currentStepNumber );

	bool didGenerateStep = false;

	//Make able to be read in via Environment! Adding 1 to the original rules to compensate for FinalizeMap's fencing in.
	const int MIN_ROOM_WIDTH = 4;
	const int MIN_ROOM_HEIGHT = 4;
	const int MAX_ROOM_WIDTH = 6;
	const int MAX_ROOM_HEIGHT = 6;

	int entranceW = GetRandomIntInRange( MIN_ROOM_WIDTH, MAX_ROOM_WIDTH );
	int entranceH = GetRandomIntInRange( MIN_ROOM_HEIGHT, MAX_ROOM_HEIGHT );

	//-----------------------------------------------------------------------------
	//"The entrance is a 3x3 to 5x5 open area in the center of the left-side of the map."
	const Vector2i& mapSize = map->GetDimensions();
	int entranceX = 0;
	int entranceY = ( mapSize.y / 2 ) - ( entranceH / 2 ); //Centers the entrance halfway up the map.
	m_entranceBounds = AABB2i( entranceX, entranceY, entranceX + entranceW, entranceY + entranceH );

	//Set cell types.
	for ( int cellX = entranceX; cellX < ( entranceX + entranceW ); cellX++ )
		for ( int cellY = entranceY; cellY < ( entranceY + entranceH ); cellY++ )
			map->SetCellTypeForIndex( map->GetIndexForPosition( Vector2i( cellX, cellY ) ), CELL_TYPE_AIR );

	didGenerateStep = true;
	return didGenerateStep;
}


//--------------------------------------------------------------------------------------------------------------
bool CastleGenerator::BuildCastle( Map* map, int currentStepNumber )
{
	UNREFERENCED( currentStepNumber );

	bool didGenerateStep = false;

	const int MOAT_MIN = 2;
	const int MOAT_MAX = 3;
	int moatLength = GetRandomIntInRange( MOAT_MIN, MOAT_MAX );

	const Vector2i& mapSize = map->GetDimensions();
	int castleLeftX = m_entranceBounds.maxs.x + moatLength;

	//Inset 1 to allow for FinalizeMap's fencing in with wall.
	int castleRightX = mapSize.x - moatLength - 2;
	int castleTopY = mapSize.y - moatLength - 2;
	int castleBottomY = moatLength + 1;

	m_castleBounds = AABB2i( castleLeftX, castleBottomY, castleRightX, castleTopY );

	//Set cell types.
	for ( int cellX = castleLeftX; cellX <= castleRightX; cellX++ )
		for ( int cellY = castleBottomY; cellY <= castleTopY; cellY++ )
			map->SetCellTypeForIndex( map->GetIndexForPosition( Vector2i( cellX, cellY ) ), CELL_TYPE_STONE_WALL );

	didGenerateStep = true;
	return didGenerateStep;
}


//--------------------------------------------------------------------------------------------------------------
bool CastleGenerator::BuildGatehouse( Map* map, int currentStepNumber )
{
	UNREFERENCED( currentStepNumber );

	bool didGenerateStep = false;

	const int MIN_ROOM_WIDTH = 6;
	const int MIN_ROOM_HEIGHT = 8;
	const int MAX_ROOM_WIDTH = 3;
	const int MAX_ROOM_HEIGHT = 5;

	int gatehouseW = GetRandomIntInRange( MIN_ROOM_WIDTH, MAX_ROOM_WIDTH );
	int gatehouseH = GetRandomIntInRange( MIN_ROOM_HEIGHT, MAX_ROOM_HEIGHT );

	int castleRoomH = m_castleBounds.maxs.y - m_castleBounds.mins.y;
	int gatehouseX = m_castleBounds.mins.x + 1;
	int gatehouseY = ( m_castleBounds.mins.y + ( castleRoomH / 2 ) ) - ( gatehouseH / 2 );

	m_gatehouseBounds = AABB2i( gatehouseX, gatehouseY, gatehouseX + gatehouseW, gatehouseY + gatehouseH );

	//Set cell types.
	for ( int cellX = gatehouseX; cellX < ( gatehouseX + gatehouseW ); cellX++ )
		for ( int cellY = gatehouseY; cellY < ( gatehouseY + gatehouseH ); cellY++ )
			map->SetCellTypeForIndex( map->GetIndexForPosition( Vector2i( cellX, cellY ) ), CELL_TYPE_AIR );

	didGenerateStep = true;
	return didGenerateStep;
}


//--------------------------------------------------------------------------------------------------------------
bool CastleGenerator::BuildDrawbridge( Map* map, int currentStepNumber )
{
	UNREFERENCED( currentStepNumber );

	bool didGenerateStep = false;

	int gatehouseH = m_gatehouseBounds.maxs.y - m_gatehouseBounds.mins.y;
	int gatehouseCenterY = m_gatehouseBounds.mins.y + ( gatehouseH / 2 );
	const int& drawbridgeY = gatehouseCenterY;

	//Go left until we hit an open air space (should be of the entrance).
	CellType leftNeighborType = CELL_TYPE_LAVA;
	for ( int drawbridgeX = m_castleBounds.mins.x; leftNeighborType != CELL_TYPE_AIR; drawbridgeX-- )
	{
		Vector2i currentDrawbridgePos = Vector2i( drawbridgeX, drawbridgeY );
		map->SetCellTypeForIndex( map->GetIndexForPosition( currentDrawbridgePos ), CELL_TYPE_AIR );
		leftNeighborType = map->GetCellForPosition( currentDrawbridgePos - Vector2i::UNIT_X ).m_cellType;
	}

	didGenerateStep = true;
	return didGenerateStep;
}


//--------------------------------------------------------------------------------------------------------------
bool CastleGenerator::BuildTowers( Map* map, int currentStepNumber )
{
	UNREFERENCED( currentStepNumber );

	bool didGenerateStep = false;
	
	const int MIN_ROOM_WIDTH = 2;
	const int MIN_ROOM_HEIGHT = 2;
	const int MAX_ROOM_WIDTH = 3;
	const int MAX_ROOM_HEIGHT = 3;

	int towerW = GetRandomIntInRange( MIN_ROOM_WIDTH, MAX_ROOM_WIDTH );
	int towerH = GetRandomIntInRange( MIN_ROOM_HEIGHT, MAX_ROOM_HEIGHT );

	//-----------------------------------------------------------------------------
	int towerLeftX = m_castleBounds.mins.x; //No walls between it and the lava, sayeth the GDD.
	int towerRightX = towerLeftX + towerW;

	int topTowerTopY = m_castleBounds.maxs.y; //Top-left corner.
	int topTowerBottomY = topTowerTopY - towerH;
	int bottomTowerBottomY = m_castleBounds.mins.y; //Bottom-left corner.
	int bottomTowerTopY = bottomTowerBottomY + towerH;

	m_topTowerBounds = AABB2i( towerLeftX, topTowerBottomY, towerRightX, topTowerTopY );
	m_bottomTowerBounds = AABB2i( towerLeftX, bottomTowerBottomY, towerRightX, bottomTowerBottomY );

	//Set cell types.
	for ( int cellX = towerLeftX; cellX < towerRightX; cellX++ )
		for ( int cellY = topTowerBottomY; cellY <= topTowerTopY; cellY++ )
			map->SetCellTypeForIndex( map->GetIndexForPosition( Vector2i( cellX, cellY ) ), CELL_TYPE_AIR );

	for ( int cellX = towerLeftX; cellX < towerRightX; cellX++ )
		for ( int cellY = bottomTowerBottomY; cellY <= bottomTowerTopY; cellY++ )
			map->SetCellTypeForIndex( map->GetIndexForPosition( Vector2i( cellX, cellY ) ), CELL_TYPE_AIR );

	//Go down from the right edge of the top tower until we hit an open air space (should be of the gatehouse).
	CellType belowNeighborType = CELL_TYPE_STONE_WALL;
	for ( int towerPathY = topTowerBottomY - 1; belowNeighborType != CELL_TYPE_AIR; towerPathY-- )
	{
		Vector2i currentTowerPathPos = Vector2i( towerRightX - 1, towerPathY );
		map->SetCellTypeForIndex( map->GetIndexForPosition( currentTowerPathPos ), CELL_TYPE_AIR );
		belowNeighborType = map->GetCellForPosition( currentTowerPathPos - Vector2i::UNIT_Y ).m_cellType;
	}

	//Go up from the right edge of the bottom tower until we hit an open air space (should be of the gatehouse).
	CellType aboveNeighborType = CELL_TYPE_STONE_WALL;
	for ( int towerPathY = bottomTowerTopY + 1; aboveNeighborType != CELL_TYPE_AIR; towerPathY++ )
	{
		Vector2i currentTowerPathPos = Vector2i( towerRightX - 1, towerPathY );
		map->SetCellTypeForIndex( map->GetIndexForPosition( currentTowerPathPos ), CELL_TYPE_AIR );
		aboveNeighborType = map->GetCellForPosition( currentTowerPathPos + Vector2i::UNIT_Y ).m_cellType;
	}

	didGenerateStep = true;
	return didGenerateStep;
}


//--------------------------------------------------------------------------------------------------------------
bool CastleGenerator::BuildInnerRoomsSwissCheese( Map* map, int currentStepNumber )
{
	UNREFERENCED( currentStepNumber );

	bool didGenerateStep = false;

	const int INSET_FROM_LEFT = 5;
	const int INSET_FROM_RIGHT = 1;
	const int INSET_FROM_ABOVE = 1;
	const int INSET_FROM_BELOW = 1;

	int innerSpaceLeftX = m_castleBounds.mins.x + INSET_FROM_LEFT;
	int innerSpaceRightX = m_castleBounds.maxs.x - INSET_FROM_RIGHT;
	int innerSpaceTopY = m_castleBounds.maxs.y - INSET_FROM_ABOVE;
	int innerSpaceBottomY = m_castleBounds.mins.y + INSET_FROM_BELOW;

	for ( int cellX = innerSpaceLeftX; cellX <= innerSpaceRightX; cellX++ )
		for ( int cellY = innerSpaceBottomY; cellY <= innerSpaceTopY; cellY++ )
			if ( GetRandomChance( .85f ) )
				map->SetCellTypeForIndex( map->GetIndexForPosition( Vector2i( cellX, cellY ) ), CELL_TYPE_AIR );

	didGenerateStep = true;
	return didGenerateStep;
}


//--------------------------------------------------------------------------------------------------------------
bool CastleGenerator::BuildInhabitants( Map* map, int currentStepNumber )
{
	UNREFERENCED( currentStepNumber );
	UNREFERENCED( map );

	bool didGenerateStep = false;

	TODO( "Add an inhabitants= XML attribute to be processed by BiomeBlueprints!" );

	didGenerateStep = true;
	return didGenerateStep;
}


//--------------------------------------------------------------------------------------------------------------
Map* CastleGenerator::CreateMapAndInitializeCells( const Vector2i& size, const std::string& mapName )
{
	Map* newMap = new Map( size, mapName );

	for ( unsigned int cellIndex = 0; cellIndex < newMap->GetCells().size(); cellIndex++ )
		newMap->SetCellTypeForIndex( cellIndex, CellType::CELL_TYPE_LAVA );

	return newMap;
}


//--------------------------------------------------------------------------------------------------------------
bool CastleGenerator::GenerateOneStep( Map* map, int currentStepNumber, BiomeGenerationProcess* metadata )
{
	UNREFERENCED( metadata );

	bool didGenerateStep = false;

	didGenerateStep = BuildEntrance( map, currentStepNumber );
	didGenerateStep = BuildCastle( map, currentStepNumber );
	didGenerateStep = BuildGatehouse( map, currentStepNumber );
	didGenerateStep = BuildDrawbridge( map, currentStepNumber );
	didGenerateStep = BuildTowers( map, currentStepNumber );
	didGenerateStep = BuildInnerRoomsSwissCheese( map, currentStepNumber );
	didGenerateStep = BuildInhabitants( map, currentStepNumber );

	return didGenerateStep;
}
