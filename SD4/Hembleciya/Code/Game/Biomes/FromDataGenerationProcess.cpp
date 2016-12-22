#include "Game/Biomes/FromDataGenerationProcess.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"


//--------------------------------------------------------------------------------------------------------------
const char* FromDataGenerationProcess::XML_FILEPATH_ATTRIBUTE_NAME = "file";


//--------------------------------------------------------------------------------------------------------------
FromDataGenerationProcess::FromDataGenerationProcess( const XMLNode& generationProcessNode )
	: BiomeGenerationProcess( generationProcessNode ) //Handle non-subclass attribute reading.
{
	m_xmlFilePath = ReadXMLAttribute( generationProcessNode, XML_FILEPATH_ATTRIBUTE_NAME, m_xmlFilePath );
	ASSERT_OR_DIE( m_xmlFilePath != "", "FromDataGenerationProcess couldn't find file attribute in XML file! " );
}