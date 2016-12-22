#include "Game/Biomes/BiomeGenerationProcess.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"


const char* BiomeGenerationProcess::GENERATOR_ATTRIBUTE_NAME = "generator";
const char* BiomeGenerationProcess::STEPS_ATTRIBUTE_NAME = "steps";


//--------------------------------------------------------------------------------------------------------------
BiomeGenerationProcess::BiomeGenerationProcess( const XMLNode& generationProcessNode )
{
	m_generatorName = ReadXMLAttribute( generationProcessNode, GENERATOR_ATTRIBUTE_NAME, m_generatorName );
	sscanf_s( generationProcessNode.getAttribute( STEPS_ATTRIBUTE_NAME ), "%d", &m_numGeneratorSteps );
}
