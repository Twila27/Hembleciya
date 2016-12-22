#include "Game/Generators/KruskalDartboardGenerator.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include <algorithm>


//--------------------------------------------------------------------------------------------------------------
STATIC GeneratorRegistration KruskalDartboardGenerator::s_metropolis( "Metropolis", &KruskalDartboardGenerator::CreateGenerator, &Generator::CreateBiomeCreationProcess );
STATIC GeneratorRegistration KruskalDartboardGenerator::s_metropolisHub( "MetropolisHub", &KruskalDartboardGenerator::CreateGenerator, &Generator::CreateBiomeCreationProcess );


//--------------------------------------------------------------------------------------------------------------
KruskalDartboardGenerator::~KruskalDartboardGenerator()
{
	for ( AABB2i* room : m_rooms )
		if ( room != nullptr )
			delete room;
	for ( std::set< AABB2i* >* clump : m_connectedRooms )
		if ( clump != nullptr )
			delete clump; //AABB2i*'s deleted in above loop.
}


//--------------------------------------------------------------------------------------------------------------
bool KruskalDartboardGenerator::GenerateRoom( Map* map, int currentStepNumber )
{
	bool didGenerateStep = false;

	//Make able to be read in via Environment!
	int MIN_ROOM_WIDTH = 5;
	int MIN_ROOM_HEIGHT = 5;
	const int MAX_ROOM_WIDTH = 10;
	const int MAX_ROOM_HEIGHT = 10;

	//-----------------------------------------------------------------------------
	//1. Pick random x, y.
	Vector2i mapSize = map->GetDimensions();
	int x;
	int y;

	if ( currentStepNumber == 1 && m_name == "MetropolisHub" )
	{
		MIN_ROOM_WIDTH = MAX_ROOM_WIDTH;
		MIN_ROOM_HEIGHT = MAX_ROOM_HEIGHT;
		x = ( ( mapSize.x - 1 ) / 2 ) - ( MAX_ROOM_WIDTH / 2 ); //Center the hub.
		y = ( ( mapSize.y - 1 ) / 2 ) - ( MAX_ROOM_HEIGHT / 2 );
	}
	else
	{
		x = GetRandomIntInRange( 1, mapSize.x - 2 ); //Prevents adding rooms on map's perimeter.
		y = GetRandomIntInRange( 1, mapSize.y - 2 );
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
	for ( AABB2i* room : m_rooms )
	{
		if ( DoAABBsOverlap( *room, roomBounds ) )
			return false;
	}

	//Set cell types.
	for ( int cellX = x; cellX < ( x + w ); cellX++ )
		for ( int cellY = y; cellY < ( y + h ); cellY++ )
			map->SetCellTypeForIndex( map->GetIndexForPosition( Vector2i( cellX, cellY ) ), CELL_TYPE_AIR );

	m_rooms.push_back( new AABB2i( roomBounds ) );

	return didGenerateStep;
}


//--------------------------------------------------------------------------------------------------------------
bool KruskalDartboardGenerator::CollapseClumps()
{
	unsigned int originalNumClumps = m_connectedRooms.size();

	//Create array of sets to be removed. If we just remove while looping, it'll invalidate outer loop.
	bool* isIndexToBeErasedAt = new bool[ originalNumClumps ];
	for ( unsigned int i = 0; i < originalNumClumps; i++ ) isIndexToBeErasedAt[ i ] = false;

	for ( unsigned int clumpIndex = 0; clumpIndex < originalNumClumps; clumpIndex++ )
	{
		const std::set< AABB2i* >* currentClump = m_connectedRooms[ clumpIndex ];

		for ( unsigned int otherClumpIndex = 0; otherClumpIndex < originalNumClumps; otherClumpIndex++ )
		{
			if ( clumpIndex == otherClumpIndex )
				continue;

			const std::set< AABB2i* >* otherClump = m_connectedRooms[ otherClumpIndex ];

			//-----------------------------------------------------------------------------
			//1. If outer set == inner set, remove one.
			if ( *currentClump == *otherClump ) 
			{
				isIndexToBeErasedAt[ otherClumpIndex ] = true;
				continue;
			}

			//-----------------------------------------------------------------------------
			//1.5. Configure union of the two sets.
			std::vector< AABB2i* > clumpUnionResult( currentClump->size() + otherClump->size() );
			std::vector< AABB2i* >::iterator clumpUnionEndIter = std::set_union( currentClump->begin(), currentClump->end(), 
																					otherClump->begin(), otherClump->end(), 
																					clumpUnionResult.begin() );

			clumpUnionResult.resize( clumpUnionEndIter - clumpUnionResult.begin() ); //Removes unused elements.
			
			//-----------------------------------------------------------------------------
			//2. If |outer union inner set| == |outer set|, remove inner--it's a subset.
			if ( clumpUnionResult.size() == currentClump->size() )
			{
				isIndexToBeErasedAt[ otherClumpIndex ] = true;
				continue;
			}

			//-----------------------------------------------------------------------------
			//3. If |outer union inner set| == |inner set|, remove outer--it's a subset.
				//Case #2 will handle it under a nested loop context! Keeps loops simpler.
		}
	}

	//Actual removal.
	for ( int clumpIndex = originalNumClumps - 1; clumpIndex >= 0; clumpIndex-- )
		if ( isIndexToBeErasedAt[ clumpIndex ] )
			m_connectedRooms.erase( m_connectedRooms.begin() + clumpIndex );

	return originalNumClumps != m_connectedRooms.size(); //If we did collapse anything, returns true.
}


//--------------------------------------------------------------------------------------------------------------
bool KruskalDartboardGenerator::GenerateClumps()
{
	//1. For each AABB2, create and push a set with its index.
	for ( unsigned int roomIndex = 0; roomIndex < m_rooms.size(); roomIndex++ )
	{
		m_connectedRooms.push_back( new std::set< AABB2i* >() );
		m_connectedRooms.back()->insert( m_rooms[ roomIndex ] );
		const AABB2i* currentRoom = m_rooms[ roomIndex ];

		//2. Check versus all others in list.
		for ( unsigned int otherRoomIndex = 0; otherRoomIndex < m_rooms.size(); otherRoomIndex++ )
		{
			if ( otherRoomIndex == roomIndex )
				continue; //Don't re-insert self.
	
			//3. If they overlap (OK to recalculate), push to the active room's set.
			if ( DoAABBsOverlap( *currentRoom, *m_rooms[ otherRoomIndex ] ) )
				m_connectedRooms.back()->insert( m_rooms[ otherRoomIndex ] );
		}
	}

	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool KruskalDartboardGenerator::GenerateLink( Map* map, int currentStepNumber )
{
	//1. Find the closest clumps by comparing their AABB2i* members, anchoring to #0 if hub variant.
	float currentMinDistSquared = map->GetDimensions().CalcFloatLengthSquared();
	int indexOfClosestClumps1 = -1;
	int indexOfClosestClumps2 = -1;
	Vector2i centerOfClosestRoomInClosestClump1;
	Vector2i centerOfClosestRoomInClosestClump2;

	for ( unsigned int clumpIndex = 0; clumpIndex < m_connectedRooms.size(); clumpIndex++ )
	{
		const std::set< AABB2i* >* currentClump = m_connectedRooms[ clumpIndex ];

		for ( unsigned int otherClumpIndex = 0; otherClumpIndex < m_connectedRooms.size(); otherClumpIndex++ )
		{
			if ( clumpIndex == otherClumpIndex )
				continue;

			const std::set< AABB2i* >* otherClump = m_connectedRooms[ otherClumpIndex ];

			//Can assume no overlap exists at this stage.
			for ( AABB2i* roomBounds : *currentClump )
			{
				for ( AABB2i* otherRoomBounds : *otherClump )
				{
					Vector2i centersDisplacement = roomBounds->GetCenter() - otherRoomBounds->GetCenter();
					float centersDistanceSquared = centersDisplacement.CalcFloatLengthSquared();
					if ( centersDistanceSquared < currentMinDistSquared )
					{
						indexOfClosestClumps1 = clumpIndex;
						indexOfClosestClumps2 = otherClumpIndex;
						currentMinDistSquared = centersDistanceSquared;
						centerOfClosestRoomInClosestClump1 = roomBounds->GetCenter();
						centerOfClosestRoomInClosestClump2 = otherRoomBounds->GetCenter();
					}
				}
			}
		}

		if ( m_name == "MetropolisHub" ) 
			break;
	}

	//2. Connect the closest two clumps via their closest two rooms:
	const Vector2i& c1 = centerOfClosestRoomInClosestClump1;
	const Vector2i& c2 = centerOfClosestRoomInClosestClump2;
	if ( c1.x == c2.x ) //Straight above/below case.
	{
		if ( c2.y > c1.y )
		{
			for ( int y = c1.y; y <= c2.y; y++ )
				map->SetCellTypeForIndex( map->GetIndexForPosition( Vector2i( c1.x, y ) ), CELL_TYPE_STONE_FLOOR );
		}
		else
		{
			for ( int y = c2.y; y <= c1.y; y++ )
				map->SetCellTypeForIndex( map->GetIndexForPosition( Vector2i( c1.x, y ) ), CELL_TYPE_STONE_FLOOR );
		}
	}
	else if ( c1.y == c2.y ) //Straight left/right case.
	{
		if ( c2.x > c1.x )
		{
			for ( int x = c1.x; x <= c2.x; x++ )
				map->SetCellTypeForIndex( map->GetIndexForPosition( Vector2i( x, c1.y ) ), CELL_TYPE_STONE_FLOOR );
		}
		else
		{
			for ( int x = c2.x; x <= c1.x; x++ )
				map->SetCellTypeForIndex( map->GetIndexForPosition( Vector2i( x, c1.y ) ), CELL_TYPE_STONE_FLOOR );
		}
	}
	else //Non-parallel case, so use right legs of hypotenuse.
	{
		if ( ( currentStepNumber & 0b10 ) == 0 ) //Arbitrary predicate to decide which pair of right legs to use.
		{
			//map->SetCellTypesAlongStraightLine( c1, Vector2i( c1.x, c2.y ) );
			//map->SetCellTypesAlongStraightLine( c2, Vector2i( c1.x, c2.y ) );
			if ( c2.y > c1.y )
			{
				for ( int y = c1.y; y <= c2.y; y++ )
					map->SetCellTypeForIndex( map->GetIndexForPosition( Vector2i( c1.x, y ) ), CELL_TYPE_STONE_FLOOR );
			}
			else
			{
				for ( int y = c2.y; y <= c1.y; y++ )
					map->SetCellTypeForIndex( map->GetIndexForPosition( Vector2i( c1.x, y ) ), CELL_TYPE_STONE_FLOOR );
			}

			if ( c2.x > c1.x )
			{
				for ( int x = c1.x; x <= c2.x; x++ )
					map->SetCellTypeForIndex( map->GetIndexForPosition( Vector2i( x, c2.y ) ), CELL_TYPE_STONE_FLOOR );
			}
			else
			{
				for ( int x = c2.x; x <= c1.x; x++ )
					map->SetCellTypeForIndex( map->GetIndexForPosition( Vector2i( x, c2.y ) ), CELL_TYPE_STONE_FLOOR );
			}
		}
		else
		{
			//map->SetCellTypesAlongStraightLine( c1, Vector2i( c2.x, c1.y ) );
			//map->SetCellTypesAlongStraightLine( c2, Vector2i( c2.x, c1.y ) );
			if ( c2.y > c1.y )
			{
				for ( int y = c1.y; y <= c2.y; y++ )
					map->SetCellTypeForIndex( map->GetIndexForPosition( Vector2i( c2.x, y ) ), CELL_TYPE_STONE_FLOOR );
			}
			else
			{
				for ( int y = c2.y; y <= c1.y; y++ )
					map->SetCellTypeForIndex( map->GetIndexForPosition( Vector2i( c2.x, y ) ), CELL_TYPE_STONE_FLOOR );
			}

			if ( c2.x > c1.x )
			{
				for ( int x = c1.x; x <= c2.x; x++ )
					map->SetCellTypeForIndex( map->GetIndexForPosition( Vector2i( x, c1.y ) ), CELL_TYPE_STONE_FLOOR );
			}
			else
			{
				for ( int x = c2.x; x <= c1.x; x++ )
					map->SetCellTypeForIndex( map->GetIndexForPosition( Vector2i( x, c1.y ) ), CELL_TYPE_STONE_FLOOR );
			}
		}
	}

	if ( indexOfClosestClumps1 == -1 || indexOfClosestClumps2 == -1 )
		return false; //Nothing to connect!

	//3. Merge clumps, erase other. No big union this time because I can do an in-place append.
	if ( m_name != "MetropolisHub" ) //For the hub case, we don't want it to link rooms to anywhere but the first room.
		m_connectedRooms[ indexOfClosestClumps1 ]->insert( m_connectedRooms[ indexOfClosestClumps2 ]->begin(), m_connectedRooms[ indexOfClosestClumps2 ]->end() );
	m_connectedRooms.erase( m_connectedRooms.begin() + indexOfClosestClumps2 );

	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool KruskalDartboardGenerator::GenerateOneStep( Map* map, int currentStepNumber, BiomeGenerationProcess* metadata )
{
	UNREFERENCED( metadata );

	bool didGenerateStep = false;

	//Make read in by Environment!
	static const int roomPhaseSteps = 20;

	if ( currentStepNumber < roomPhaseSteps ) 
		didGenerateStep = GenerateRoom( map, currentStepNumber );
	if ( currentStepNumber == roomPhaseSteps )
	{
		GenerateClumps();
		CollapseClumps();
	}
	if ( currentStepNumber >= roomPhaseSteps )
		didGenerateStep = GenerateLink( map, currentStepNumber );

	return didGenerateStep;
}
