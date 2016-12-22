#include "Game/Pathfinding/Pathfinder.hpp"
#include "Engine/Renderer/TheRenderer.hpp"
#include "Game/Map.hpp"


//--------------------------------------------------------------------------------------------------------------
bool Path::Pathfind( int numStepsToTake /*= -1*/ ) //-1 for as many as necessary to hit goal.
{
	m_currentActiveNode = nullptr;
	int numIteration = 0;

	do
	{
		if ( m_openList.empty() )
			return false; //NO_PATH, e.g. goal was on the other side of a room-wide wall.

		//RemoveLowestOpenListNodeWithLowestCostF.
		m_currentActiveNode = m_openList.begin()->second;
		m_openList.erase( m_openList.begin() );

		//Only check if it's a goal after adding to closed list.
		m_closedList[ m_currentActiveNode->m_position ] = m_currentActiveNode;
		if ( m_currentActiveNode->m_position == m_goal )
		{
			RecursivelyBuildPathBackToStartFromNode( m_currentActiveNode );
			return true;
		}

		std::vector< PathNode* > candidateNeighbors;
		m_map->BuildPathNodesForTraversableNeighbors( m_currentActiveNode, m_goal, candidateNeighbors, m_traversalProperties );

		for ( PathNode* currentNeighbor : candidateNeighbors )
		{
			Vector2i neighborMapPosition = currentNeighbor->m_position;

			if ( m_closedList.count( neighborMapPosition ) > 0 )
				continue; //Do not consider nodes that are already in closed list, only open list.

			bool shouldAddToOpenList = CheckAgainstOpenList( currentNeighbor );
			
			if ( shouldAddToOpenList )
				m_openList.insert( std::pair< float, PathNode* >( currentNeighbor->GetWeightedStepCostF(), currentNeighbor ) );
		}
	} 
	while ( numStepsToTake == -1 || numIteration++ < numStepsToTake );
	
	RecursivelyBuildPathBackToStartFromNode( m_currentActiveNode );
	return true;
}


//--------------------------------------------------------------------------------------------------------------
void Path::RecursivelyBuildPathBackToStartFromNode( PathNode* goalNode )
{
	m_finalPathResult.clear();
	PathNode* currentNode = goalNode;
	do 
	{
		m_finalPathResult.push_back( currentNode ); //Note goal will be the front() and start will be the back().
		currentNode = currentNode->m_parent; //Loop back up the subset of the closed list that links start with goal.
	} 
	while ( currentNode != nullptr ); //May consider making this currentNode->m_parent to skip out on including the start in the final path.
		//Alternatively a counter holding current progress through the list might be usable.
}


//--------------------------------------------------------------------------------------------------------------
bool Path::Render()
{
	//Render any PathNode the way we do a Map tile! Rip that logic, drawing an AABB up from the mins to the maxs scaled by the same amounts as Entities.
	//Actually, just loop over the PathNodes describing each of the below and call Render() on them to do this!

	//Render the open list.
	for ( std::pair< float, PathNode* > node : m_openList )
		node.second->Render( Rgba::BLUE );

	//Render the closed list.
	for ( std::pair< Vector2i, PathNode* > node : m_closedList )
		node.second->Render( Rgba::RED );

	//Render the active path on top of those lists.
	for ( PathNode* node : m_finalPathResult )
		node->Render( Rgba::GREEN );

	//Render the goal square.
	Vector2f goalScreenPositionMins = GetScreenPositionForMapPosition( m_goal - Vector2i::UNIT_Y );
	Vector2f goalScreenPositionMaxs = GetScreenPositionForMapPosition( m_goal + Vector2i::ONE - Vector2i::UNIT_Y );
	g_theRenderer->DrawAABB( TheRenderer::VertexGroupingRule::AS_QUADS, AABB2f( goalScreenPositionMins, goalScreenPositionMaxs ), Rgba::YELLOW );

	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool Path::CheckAgainstOpenList( PathNode* currentNeighbor )
{
	bool shouldAddToOpenList = true;

	PathNode* existingNode = nullptr;
	for ( auto openListIter = m_openList.cbegin(); openListIter != m_openList.cend(); ++openListIter )
	{
		PathNode* openListNode = openListIter->second;
		if ( openListNode->m_position == currentNeighbor->m_position ) //Search for a match on position, if any exists.
		{
			existingNode = openListNode;
			break;
		}
	}

	if ( existingNode != nullptr )
	{
		shouldAddToOpenList = false;
		if ( currentNeighbor->GetTotalCostG() < existingNode->GetTotalCostG() ) //Overwrite if we found a faster way to a same position.
		{
			existingNode->SetFasterPath( currentNeighbor->m_parent, currentNeighbor->m_localStepCostG, currentNeighbor->m_parentStepCostG );
		}
	}

	return shouldAddToOpenList;
}
