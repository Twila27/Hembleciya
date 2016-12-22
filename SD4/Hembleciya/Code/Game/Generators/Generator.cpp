#include "Game/Generators/Generator.hpp"

#include "Game/Biomes/BiomeGenerationProcess.hpp"
#include "Game/Features/FeatureFactory.hpp"

#include "Game/Map.hpp"

#include "Engine/Error/ErrorWarningAssert.hpp"


STATIC std::map< std::string, GeneratorRegistration* >* GeneratorRegistration::s_generatorRegistry = nullptr;


//--------------------------------------------------------------------------------------------------------------
Map* Generator::CreateMapAndInitializeCells( const Vector2i& size, const std::string& mapName )
{
	Map* newMap = new Map( size, mapName );

	for ( unsigned int cellIndex = 0; cellIndex < newMap->GetCells().size(); cellIndex++ )
		newMap->SetCellTypeForIndex( cellIndex, CellType::CELL_TYPE_STONE_WALL );
	return newMap;
}


//--------------------------------------------------------------------------------------------------------------
void Generator::FinalizeMap( Map* map )
{
	Vector2i mapSize = map->GetDimensions();

	//Ensure the perimeter is solid wall.
	for ( int x = 0; x < mapSize.x; x++ )
		for ( int y = 0; y < mapSize.y; y++ )
		{
			if ( x == 0 ||
				 y == 0 ||
				 x == mapSize.x-1 ||
				 y == mapSize.y-1 )
			map->GetCellForPosition( Vector2i( x, y ) ).m_cellType = CELL_TYPE_STONE_WALL;
		}

	map->HideOccludedCells();
	map->RefreshTraversableCells(); //Regardless of which generator this is, A* and behaviors need it.
	map->RefreshCellColors();

	//Sprinkle in some doors with 10-20% spawn rate.
	const float doorMinTilesApart = 7.5f;
	for ( MapPosition& cellPos : map->GetTraversableCells() )
	{
		//Works if my maps mark their hallways as floor instead of air, see Metropolis generators:
		unsigned int numFloorNeighbors = map->GetNumNeighborsAroundCellOfType( cellPos, CELL_TYPE_STONE_FLOOR, 1.f, false );
		unsigned int numWallNeighbors = map->GetNumNeighborsAroundCellOfType( cellPos, CELL_TYPE_STONE_WALL, 1.f, true );

		if ( numFloorNeighbors == 2 && numWallNeighbors == 6 && GetRandomChance( .13f ) ) //One entrance, one exit, perfect for a door.
		{
			if ( map->CountCellsWithFeaturesAroundCenter( cellPos, doorMinTilesApart, false ) != 0 )
				continue; //Don't want too many doors in one hall, for example.

			Feature* door = FeatureFactory::GetRandomFactoryForFeatureType( FEATURE_TYPE_DOOR )->CreateFeature( map );
			door->AttachToMapAtPosition( map, cellPos );
			map->GetCellForPosition( cellPos ).m_occupyingFeature = door;
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
STATIC MapPosition Generator::FindValidStartingPosition( Map* map )
{
	Vector2i mapSize = map->GetDimensions();

	const std::vector< Cell >& cells = map->GetCells();
	for ( unsigned int cellIndex = 0; cellIndex < cells.size(); cellIndex++ )
		if ( cells[ cellIndex ].m_cellType == CELL_TYPE_AIR )
			return map->GetPositionForIndex( cellIndex );

	return MapPosition( -1, -1 );
}


//--------------------------------------------------------------------------------------------------------------
BiomeGenerationProcess* Generator::CreateBiomeCreationProcess( const XMLNode& generationProcessNode )
{
	return new BiomeGenerationProcess( generationProcessNode );
}


//--------------------------------------------------------------------------------------------------------------
Generator* GeneratorRegistration::CreateGeneratorByName( const std::string& name )
{
	Generator* generator = nullptr;
	GeneratorRegistration* generatorRegistration = nullptr;

	ASSERT_OR_DIE( s_generatorRegistry != nullptr, "GeneratorRegistrationMap Was Found Null in CreateGeneratorByName!" );

	std::map< std::string, GeneratorRegistration* >::iterator generatorRegistrationIter = s_generatorRegistry->find( name );

	if ( generatorRegistrationIter != s_generatorRegistry->end() )
	{
		generatorRegistration = generatorRegistrationIter->second;
		generator = ( *generatorRegistration->m_creationFunc )( generatorRegistration->m_name ); //ACTUAL POINT OF INSTANTIATION OF SUBCLASS!
	}
	else DebuggerPrintf( "WARNING: CreateGeneratorByName Did Not Find %s", name );

	return generator;
}


//--------------------------------------------------------------------------------------------------------------
BiomeGenerationProcess* GeneratorRegistration::CreateBiomeGenerationProcessByNameAndXML( const std::string& name, const XMLNode& generationProcessNode )
{
	BiomeGenerationProcess* biomeGenerationProcess = nullptr;
	GeneratorRegistration* generatorRegistration = nullptr;

	ASSERT_OR_DIE( s_generatorRegistry != nullptr, "GeneratorRegistrationMap Was Found Null in CreateBiomeGenerationProcessByName!" );

	std::map< std::string, GeneratorRegistration* >::iterator generatorRegistrationIter = s_generatorRegistry->find( name );

	if ( generatorRegistrationIter != s_generatorRegistry->end() )
	{
		generatorRegistration = generatorRegistrationIter->second;
		biomeGenerationProcess = ( *generatorRegistration->m_generationProcessCreationFunc )( generationProcessNode ); //ACTUAL POINT OF INSTANTIATION OF SUBCLASS!
	}
	else DebuggerPrintf( "WARNING: CreateBiomeGenerationProcessByName Did Not Find %s\n", name.c_str() );

	return biomeGenerationProcess;
}