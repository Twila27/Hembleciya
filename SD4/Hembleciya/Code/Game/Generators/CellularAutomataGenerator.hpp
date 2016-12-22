#pragma once

#include "Game/Generators/Generator.hpp"


class CellularAutomataGenerator : public Generator
{
public:
	CellularAutomataGenerator( const std::string& name ) : Generator( name ) {}

	//Only really seen by this subclass, provided to its registration object in the source file.
	static Generator* CreateGenerator( const std::string& name ) { return new CellularAutomataGenerator( name ); }
	//static BiomeGenerationProcess* CreateBiomeGenerationProcess( const XMLNode& generationProcessNode ) { return new CellularAutomataGenerationProcess( generationProcessNode ); }
		//Implement if needed for only-this-subclass-relevant-custom-XML-attributes, like the river's surface="lava".

	//Standard interface for polymorphic use in the game.
	virtual Map* CreateMapAndInitializeCells( const Vector2i& size, const std::string& mapName );
	bool GenerateOneStep( Map* map, int currentStepNumber, BiomeGenerationProcess* metadata ) override;


private:
	static GeneratorRegistration s_bigCaverns;
	static GeneratorRegistration s_gameOfLifeCaves;
	static GeneratorRegistration s_harwardCaverns;
};
