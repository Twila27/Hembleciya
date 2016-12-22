#include "Game/Generators/FromDataGenerator.hpp"
#include "Game/Biomes/BiomeGenerationProcess.hpp"
#include "Game/Biomes/FromDataGenerationProcess.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"

//--------------------------------------------------------------------------------------------------------------
STATIC GeneratorRegistration FromDataGenerator::s_FromDataGenerator( "FromDataGenerator", &FromDataGenerator::CreateGenerator, &FromDataGenerator::CreateFromDataBiomeGenerationProcess );


//--------------------------------------------------------------------------------------------------------------
Map* FromDataGenerator::CreateMapAndInitializeCells( const Vector2i& size, const std::string& mapName )
{
	UNREFERENCED( size );

	Map* newMap = new Map( size, mapName ); //Dummy, but can't just rely on Generator or it'll make it all stone walls.
	return newMap;
}


//--------------------------------------------------------------------------------------------------------------
bool FromDataGenerator::GenerateOneStep( Map* map, int currentStepNumber, BiomeGenerationProcess* metadata ) //In case for some weird reason it ends up as a step in a Biome.xml.
{
	UNREFERENCED( currentStepNumber );

	FromDataGenerationProcess* downcastMetadata = dynamic_cast<FromDataGenerationProcess*>( metadata );
	ASSERT_OR_DIE( downcastMetadata != nullptr, "Bad Cast in FromDataGenerator::GenerateStep!" );

	Map* newMap = new Map( downcastMetadata->m_xmlFilePath, map->GetMapName() );
	map->CopyCellsFromMap( newMap );
	delete newMap;

	return true;
}


//--------------------------------------------------------------------------------------------------------------
STATIC BiomeGenerationProcess* FromDataGenerator::CreateFromDataBiomeGenerationProcess( const XMLNode& generationProcessNode )
{ 
	return new FromDataGenerationProcess( generationProcessNode ); 
}