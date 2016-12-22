#include "Game/Biomes/BiomeBlueprint.hpp"

#include "Engine/FileUtils/FileUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Biomes/BiomeGenerationProcess.hpp"
#include "Game/Generators/Generator.hpp"
#include "Engine/String/StringUtils.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Game/Map.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"

//--------------------------------------------------------------------------------------------------------------
STATIC std::map< std::string, BiomeBlueprint* > BiomeBlueprint::s_loadedBiomeBlueprintsRegistry = std::map< std::string, BiomeBlueprint* >();


//--------------------------------------------------------------------------------------------------------------
static void GetBiomeNameFromXML( const char* xmlFilename, const XMLNode& root, std::string& out_name )
{
	char biomeName[ 80 ];
	sscanf_s( xmlFilename, "Data/XML/Biomes/%[^.].Biome.xml", &biomeName, _countof( biomeName ) );

	std::string nameInsideFile;
	nameInsideFile = ReadXMLAttribute( root, "name", nameInsideFile );

	if ( nameInsideFile != "" )
		out_name = nameInsideFile;
	else
		out_name = biomeName;
}


//--------------------------------------------------------------------------------------------------------------
BiomeBlueprint::~BiomeBlueprint()
{
	for ( BiomeGenerationProcess* process : m_processes )
		delete process;
}


//--------------------------------------------------------------------------------------------------------------
STATIC void BiomeBlueprint::LoadAllBiomeBlueprints()
{
	std::vector< std::string > m_biomeFiles = EnumerateFilesInDirectory( "Data/XML/Biomes", "*.Biome.xml" );

	for ( unsigned int biomeBlueprintIndex = 0; biomeBlueprintIndex < m_biomeFiles.size(); biomeBlueprintIndex++ )
	{
		const char* xmlFilename = m_biomeFiles[ biomeBlueprintIndex ].c_str();
		XMLNode blueprintRoot = XMLNode::openFileHelper( xmlFilename, "BiomeBlueprint" );
		
		BiomeBlueprint* newBiomeBlueprint = new BiomeBlueprint( blueprintRoot );
		GetBiomeNameFromXML( xmlFilename, blueprintRoot, newBiomeBlueprint->m_name );
		
		if ( s_loadedBiomeBlueprintsRegistry.find( newBiomeBlueprint->m_name ) != s_loadedBiomeBlueprintsRegistry.end() )
			ERROR_AND_DIE( Stringf( "Found duplicate Biome name %s in LoadBiomeBlueprints!", newBiomeBlueprint->m_name.c_str() ) );
		
		s_loadedBiomeBlueprintsRegistry.insert( std::pair< std::string, BiomeBlueprint* >( newBiomeBlueprint->m_name, newBiomeBlueprint ) );
	}
}


//--------------------------------------------------------------------------------------------------------------
Map* BiomeBlueprint::InitializeBlueprint()
{
	if ( m_processes.size() == 0 )
		ERROR_AND_DIE( "Called InitializeBlueprint() before pushing any processes!\nPlease ensure generator= name matches a GeneratorRegistration name!" );

	Map* map = nullptr;
	if ( m_currentActiveGenerator != nullptr )
		delete m_currentActiveGenerator;
	m_currentActiveGenerator = GeneratorRegistration::CreateGeneratorByName( m_processes[0]->m_generatorName );
	map = m_currentActiveGenerator->CreateMapAndInitializeCells( this->m_size, this->m_name );
	return map;
}


//--------------------------------------------------------------------------------------------------------------
bool BiomeBlueprint::FullyGenerateBlueprint( Map* map )
{
	bool success = false;

	for ( BiomeGenerationProcess* process : m_processes )
	{
		if ( m_currentActiveGenerator == nullptr )
			m_currentActiveGenerator = GeneratorRegistration::CreateGeneratorByName( process->m_generatorName );
		
		for ( int& currentStep = map->GetCurrentGeneratorStepNum(); currentStep <= process->m_numGeneratorSteps; currentStep++ )
		{
			success = m_currentActiveGenerator->GenerateOneStep( map, currentStep, process );
			if ( !success )
				DebuggerPrintf( "%s::GenerateStep returned false for map %s, step %d!",
								process->m_generatorName.c_str(),
								map->GetMapName().c_str(),
								currentStep );
		}

		delete m_currentActiveGenerator;
		m_currentActiveGenerator = nullptr;
		
		m_currentActiveProcess++;
	}

	m_currentActiveProcess = 0; //In case it's invoked again.

	return success;
}


//--------------------------------------------------------------------------------------------------------------
bool BiomeBlueprint::PartiallyGenerateBlueprint( Map* map, bool areStepsInfinite )
{
	bool success = false;

	if ( m_currentActiveProcess < 0 || m_currentActiveProcess >= m_processes.size() )
		m_currentActiveProcess = 0; //Handling if we exit back to the main menu and restart a map's generating.

	BiomeGenerationProcess* currentProcess = m_processes[ m_currentActiveProcess ];
	if ( m_currentActiveGenerator == nullptr )
		m_currentActiveGenerator = GeneratorRegistration::CreateGeneratorByName( currentProcess->m_generatorName );

	int& currentGeneratorStep = map->GetCurrentGeneratorStepNum();

	success = m_currentActiveGenerator->GenerateOneStep( map, currentGeneratorStep, currentProcess );
	++currentGeneratorStep;

	if ( !success )
		DebuggerPrintf( "%s::GenerateStep returned false for map %s, step %d!",
						currentProcess->m_generatorName.c_str(),
						map->GetMapName().c_str(),
						currentGeneratorStep );

	if ( !areStepsInfinite && currentGeneratorStep > currentProcess->m_numGeneratorSteps )
	{
		delete m_currentActiveGenerator;
		m_currentActiveGenerator = nullptr;

		m_currentActiveProcess++;

		if ( m_currentActiveProcess == m_processes.size() ) //It was our last run.
			m_currentActiveProcess = 0;
	}

	return success;
}


//--------------------------------------------------------------------------------------------------------------
void BiomeBlueprint::NextProcess()
{
	if ( m_currentActiveProcess == m_processes.size() - 1 )
		return;

	if ( m_currentActiveGenerator != nullptr )
	{
		delete m_currentActiveGenerator;
		m_currentActiveGenerator = nullptr;
	}

	++m_currentActiveProcess;
}


//--------------------------------------------------------------------------------------------------------------
void BiomeBlueprint::PreviousProcess()
{
	if ( m_currentActiveProcess == 0 )
		return;

	if ( m_currentActiveGenerator != nullptr )
	{
		delete m_currentActiveGenerator;
		m_currentActiveGenerator = nullptr;
	}

	--m_currentActiveProcess;
}

//--------------------------------------------------------------------------------------------------------------
void BiomeBlueprint::PopulateFromXMLNode( const XMLNode& blueprintNode )
{
	std::string sizeString;
	sizeString = ReadXMLAttribute( blueprintNode, "size", sizeString );
	sscanf_s( sizeString.c_str(), "%d,%d", &m_size.x, &m_size.y );

	for ( int i = 0; i < blueprintNode.nChildNode(); )
	{
		XMLNode processNode = blueprintNode.getChildNode( "GenerationProcessData", &i );
		std::string generatorName;
		generatorName = ReadXMLAttribute( processNode, "generator", generatorName );
		BiomeGenerationProcess* process = GeneratorRegistration::CreateBiomeGenerationProcessByNameAndXML( generatorName, processNode );
		if ( process != nullptr )
			m_processes.push_back( process );
	}
}