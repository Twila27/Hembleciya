#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Game/GameCommon.hpp"
class Map;
struct XMLNode;
class BiomeGenerationProcess;
#include <string>
#include <map>


class Generator
{
public:
	Generator( const std::string& name ) : m_name( name ) {}

	virtual Map* CreateMapAndInitializeCells( const Vector2i& size, const std::string& mapName ); //May add bounds later for village, etc!
	virtual bool GenerateOneStep( Map* map, int currentStepNumber, BiomeGenerationProcess* metadata ) = 0; //Doesn't start from scratch! Can just throw things on!
	static void FinalizeMap( Map* map );
	static MapPosition FindValidStartingPosition( Map* map ); //Currently find the first open spot of non-wall.

protected:
	static BiomeGenerationProcess* CreateBiomeCreationProcess( const XMLNode& generationProcessNode ); //Can just pass this for basic generators with no custom-for-subclass-only attributes.

	std::string m_name;
};


typedef Generator* (GeneratorCreationFunc)( const std::string& name );
typedef BiomeGenerationProcess* (GenerationProcessCreationFunc)( const XMLNode& generationProcessNode );
class GeneratorRegistration
{
private:
	std::string m_name;
	GeneratorCreationFunc* m_creationFunc;
	GenerationProcessCreationFunc* m_generationProcessCreationFunc;

public:
	GeneratorRegistration( const std::string& name, GeneratorCreationFunc* creationFunc, GenerationProcessCreationFunc* generationProcessCreationFunc )
		: m_name( name ), m_creationFunc( creationFunc ), m_generationProcessCreationFunc( generationProcessCreationFunc ) //May want to assert creationFunc != nullptr.
	{
		if ( s_generatorRegistry == nullptr )
			s_generatorRegistry = new std::map< std::string, GeneratorRegistration* >(); //another typedef, see above if can't remember.

		if ( s_generatorRegistry->find( name ) == s_generatorRegistry->end() )
			s_generatorRegistry->insert( std::pair< std::string, GeneratorRegistration* >( name, this ) );
	}
	static Generator* CreateGeneratorByName( const std::string& name );
	static BiomeGenerationProcess* CreateBiomeGenerationProcessByNameAndXML( const std::string& name, const XMLNode& generationProcessNode );

	static const std::map< std::string, GeneratorRegistration* >* GetRegistry() { return s_generatorRegistry; }

protected:
	static std::map< std::string, GeneratorRegistration* >* s_generatorRegistry;
};
