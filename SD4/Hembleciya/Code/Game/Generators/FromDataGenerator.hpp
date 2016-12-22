#pragma once
#include "Game/Generators/Generator.hpp"


class BiomeGenerationProcess;


class FromDataGenerator : public Generator
{
public:
	FromDataGenerator( const std::string& name ) : Generator( name ) {}
	static Generator* CreateGenerator( const std::string& name ) { return new FromDataGenerator( name ); }
	static BiomeGenerationProcess* CreateFromDataBiomeGenerationProcess( const XMLNode& generationProcessNode );
	//Implement if needed for only-this-subclass-relevant-custom-XML-attributes, like the river's surface="lava".

	virtual Map* CreateMapAndInitializeCells( const Vector2i& size, const std::string& mapName ) override;
	virtual bool GenerateOneStep( Map* map, int currentStepNumber, BiomeGenerationProcess* metadata ) override;

private:
	static GeneratorRegistration s_FromDataGenerator;
};
