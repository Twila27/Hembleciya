#pragma once


#include "Game/Generators/Generator.hpp"
#include "Engine/Math/AABB2.hpp"
#include <vector>


//From Prof. Harward's idea of slowly building out rooms from "frontier" air tiles adjacent to walls.
//After reading, it most resembles Prim's algorithm in maze generation / spanning tree graph theory.
class PrimHarwardGenerator : public Generator
{
public:
	PrimHarwardGenerator( const std::string& name ) : Generator( name ) {}
	~PrimHarwardGenerator();

	//Only really seen by this subclass, provided to its registration object in the source file.
	static Generator* CreateGenerator( const std::string& name ) { return new PrimHarwardGenerator( name ); }
	//static BiomeGenerationProcess* CreateBiomeGenerationProcess( const XMLNode& generationProcessNode ) { return new KruskalDartboardGenerationProcess( generationProcessNode ); }
		//Implement if needed for only-this-subclass-relevant-custom-XML-attributes, like the river's surface="lava".

	//Standard interface for polymorphic use in the game.
	//virtual Map* CreateMapAndInitializeCells( const Vector2if& size, const std::string& mapName );
	bool GenerateOneStep( Map* map, int currentStepNumber, BiomeGenerationProcess* metadata ) override;


private:

	std::vector< AABB2i* > m_rooms;

	bool GenerateRoom( Map* map, Vector2i roomCenterToAdd = Vector2i::ZERO );
	bool GenerateHallAndRoom( Map* map );

	static GeneratorRegistration s_dungeon;
	static GeneratorRegistration s_sandbar;
};
