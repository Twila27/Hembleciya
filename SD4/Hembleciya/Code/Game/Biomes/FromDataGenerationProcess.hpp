#pragma once


#include "Game/Biomes/BiomeGenerationProcess.hpp"
#include <string>

class Generator;
struct XMLNode;


class FromDataGenerationProcess : public BiomeGenerationProcess //Stores all key-value attributes from the biome XML file that are NOT custom-only-for-subclasses pairs.
{
public:
	FromDataGenerationProcess( const XMLNode& generationProcessNode );

	std::string m_xmlFilePath; //Should be "Data/Maps/*.Map.xml" regex.


private:
	static const char* XML_FILEPATH_ATTRIBUTE_NAME;
};
