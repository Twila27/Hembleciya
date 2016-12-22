#pragma once

#include <map>
#include <vector>
#include "Game/GameCommon.hpp"
#include "Game/Pathfinding/PathNode.hpp"


class Map;



class Path //Runs the path-finding algorithm.
{
public:
	Path( Map* map, Vector2i start, Vector2i goal, bitfield_int traversalProperties )
		: m_goal( goal )
		, m_map( map )
		, m_currentActiveNode( nullptr )
		, m_currentStepThroughFinalResult( -1 )
		, m_traversalProperties( traversalProperties )
	{
		PathNode* startNode = new PathNode( start, nullptr, 0.f, 0.f, 0.f );
		m_openList.insert( std::pair< float, PathNode* >( startNode->GetWeightedStepCostF(), startNode ) );
	}


	bool Render();//For debug visualization.
	bool Pathfind( int numStepsToTake = -1 ); //-1 for as many as necessary to hit goal.
	bool IsFinished() { return ( m_finalPathResult.size() > 0 ) && ( m_goal == m_finalPathResult.front()->m_position ); }
	PathNode* m_currentActiveNode;
	MapPosition GetNextPositionAlongPath() //Counts backwards because the 0th element is the goal, and the last is the start.
	{
		if ( m_currentStepThroughFinalResult == -1 ) //Done this way to restart over again on completion.
			m_currentStepThroughFinalResult = m_finalPathResult.size() - 1;
		if ( m_currentStepThroughFinalResult >= 0 )
			return m_finalPathResult[ m_currentStepThroughFinalResult-- ]->m_position;
		else
			return -MapPosition::ONE;
	}

private:
	bool CheckAgainstOpenList( PathNode* currentNeighbor );
	void RecursivelyBuildPathBackToStartFromNode( PathNode* goalNode );

	Map* m_map;	TODO( "Create Proxy!" );
	Vector2i m_goal;
	std::vector< PathNode* > m_finalPathResult;
	std::map< Vector2i, PathNode* > m_closedList; //Visited. Superset of the final path.
	std::multimap< float, PathNode* > m_openList; //Unvisited.

	int m_currentStepThroughFinalResult;
	bitfield_int m_traversalProperties;
};


class PathFactory //Constructs Path objects so more than instance of the path-finding can be running at once.
{
public:
	static Path* StartPathfinding( Path* out_path, Map* map, const Vector2i& start, const Vector2i& goal, bitfield_int traversalProperties )
	{
		if ( out_path != nullptr )
			delete out_path;

		out_path = new Path( map, start, goal, traversalProperties );
		return out_path;
	}

private:
	static std:: vector< Path* > s_PathRegistry;
};
