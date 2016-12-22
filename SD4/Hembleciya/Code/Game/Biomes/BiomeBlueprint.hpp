#pragma once


#include <map>
#include <vector>
#include "Engine/Math/Vector2.hpp"
struct XMLNode;
class BiomeGenerationProcess;
class Map;
class Generator;


class BiomeBlueprint
{
public:
	BiomeBlueprint( const XMLNode& blueprintNode )
		: m_currentActiveProcess( 0 )
		, m_currentActiveGenerator( nullptr )
	{ 
		PopulateFromXMLNode( blueprintNode ); 
	}
	~BiomeBlueprint();

	static void LoadAllBiomeBlueprints(); //ALL the level has to call, because this fixes the directory to load from and loads all from "Data/Environments", etc.
		//Ask engine for all the files in a preordained relative directory (the active working directory).
		//Use OpenXMLDocument, construct the new blueprint, and store it in a static registry like textures.
		//Now Game code doesn't have to tell everything to load this and that--this function grabs it all at once.
		//And this doesn't actually create the generators yet--could be 300 of them--just lightweight GenerationProcesses into m_procs through GenRegistrations.
	static bool ClearRegistry()
	{
		for ( std::pair< std::string, BiomeBlueprint* > bp : s_loadedBiomeBlueprintsRegistry )
			delete bp.second;

		s_loadedBiomeBlueprintsRegistry.clear();

		return true;
	}

	Map* InitializeBlueprint(); //Runs processes 0 to end in order.
	bool FullyGenerateBlueprint( Map* map );
	bool PartiallyGenerateBlueprint( Map* map, bool areStepsInfinite );
	void PreviousProcess();
	void NextProcess();
	static const std::map< std::string, BiomeBlueprint* >& GetRegistry() { return s_loadedBiomeBlueprintsRegistry; }

protected:
	static std::map< std::string, BiomeBlueprint* > s_loadedBiomeBlueprintsRegistry;

	void PopulateFromXMLNode( const XMLNode& blueprintNode );
		//Constructor doesn't parse XML directly, instead calling this from constructor.
		//Public if we have to call it anywhere else as needed, can be protected/private hitherto.

	std::string m_name;
	Vector2i m_size;
	std::vector< BiomeGenerationProcess* > m_processes;

	unsigned int m_currentActiveProcess;
	Generator* m_currentActiveGenerator;
};
