#pragma once


#include "Game/Generators/Generator.hpp"
#include "Game/GameCommon.hpp"
#include <vector>
#include <set>

//Named after the original idea of forming rooms by picking random (x,y) like throwing darts at a dartboard.
//After reading, it most resembles Kruskal's algorithm in maze generation / spanning tree graph theory.
class KruskalDartboardGenerator : public Generator
{
public:
	KruskalDartboardGenerator( const std::string& name ) : Generator( name ) {}
	~KruskalDartboardGenerator();

	//Only really seen by this subclass, provided to its registration object in the source file.
	static Generator* CreateGenerator( const std::string& name ) { return new KruskalDartboardGenerator( name ); }

	//Standard interface for polymorphic use in the game.
	//virtual Map* CreateMapAndInitializeCells( const Vector2if& size, const std::string& mapName );
	bool GenerateOneStep( Map* map, int currentStepNumber, BiomeGenerationProcess* metadata ) override;


private:
	static GeneratorRegistration s_metropolis;
		//Prof. Harward pointed out this leads to a too-perfect, boxy and inorganic look--I think of 1927's Metropolis.
	static GeneratorRegistration s_metropolisHub;
		//Only links rooms starting from the very first, creating a hub.

	bool GenerateRoom( Map* map, int currentStepNumber );
	bool GenerateClumps();
	bool CollapseClumps();
	bool GenerateLink( Map* map, int currentStepNumber );


	//Saved between steps.
	std::vector< AABB2i* > m_rooms;
	std::vector< std::set< AABB2i* >* > m_connectedRooms; //Each set is a clump connected of room indices.
};
