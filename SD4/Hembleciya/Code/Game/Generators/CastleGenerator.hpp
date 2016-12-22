#pragma once


#include "Game/Generators/Generator.hpp"
#include "Game/GameCommon.hpp"
#include <vector>
#include <set>

//Named after the original idea of forming rooms by picking random (x,y) like throwing darts at a dartboard.
//After reading, it most resembles Kruskal's algorithm in maze generation / spanning tree graph theory.
class CastleGenerator : public Generator
{
public:
	CastleGenerator( const std::string& name ) : Generator( name ) {}

	//Only really seen by this subclass, provided to its registration object in the source file.
	static Generator* CreateGenerator( const std::string& name ) {
		return new CastleGenerator( name );
	}
	//static BiomeGenerationProcess* CreateBiomeGenerationProcess( const XMLNode& generationProcessNode ) { return new CastleGenerationProcess( generationProcessNode ); }
	//Implement if needed for only-this-subclass-relevant-custom-XML-attributes, like the river's surface="lava".

	//Standard interface for polymorphic use in the game.
	virtual Map* CreateMapAndInitializeCells( const Vector2i& size, const std::string& mapName );
	bool GenerateOneStep( Map* map, int currentStepNumber, BiomeGenerationProcess* metadata ) override;


private:
	static GeneratorRegistration s_metropolis;
	//Prof. Harward pointed out this leads to a too-perfect, boxy and inorganic look--I think of 1927's Metropolis.
	static GeneratorRegistration s_metropolisHub;
	//Only links rooms starting from the very first, creating a hub.

	bool BuildEntrance( Map* map, int currentStepNumber );
	bool BuildCastle( Map* map, int currentStepNumber );
	bool BuildGatehouse( Map* map, int currentStepNumber );
	bool BuildDrawbridge( Map* map, int currentStepNumber );
	bool BuildTowers( Map* map, int currentStepNumber );
	bool BuildInnerRoomsSwissCheese( Map* map, int currentStepNumber );
	bool BuildInhabitants( Map* map, int currentStepNumber ); //Or do in TheGame???

	//Saved between steps.
	AABB2i m_entranceBounds;
	AABB2i m_castleBounds;
	AABB2i m_gatehouseBounds;
	AABB2i m_topTowerBounds;
	AABB2i m_bottomTowerBounds;
};
