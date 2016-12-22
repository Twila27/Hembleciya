#pragma once


#include <string>
class Generator;
struct XMLNode;


class BiomeGenerationProcess //Stores all key-value attributes from the biome XML file that are NOT custom-only-for-subclasses pairs.
{
public:
	BiomeGenerationProcess( const XMLNode& generationProcessNode );
	virtual void MakeMeValidForDynamicCast() const {}

	std::string m_generatorName;
	int m_numGeneratorSteps;

private:
	static const char* GENERATOR_ATTRIBUTE_NAME;
	static const char* STEPS_ATTRIBUTE_NAME;
};
