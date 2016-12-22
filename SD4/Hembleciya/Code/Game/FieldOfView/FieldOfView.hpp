#pragma once


class Agent;
class Map;
#include "Game/GameCommon.hpp"


enum FieldOfViewType { 
	FOV_BASIC, 
	FOV_RAYCAST, 
	NUM_FOV_MODES };


//Lightweight class > static standalone functions: internalizes tracking/managing usage.
class FieldOfView
{
public:
	//Void return value: side-effects on the map argument instead.
	static void CalculateFieldOfViewForAgent( Agent* agentOrigin, int viewDistance, Map* map, bool preferCircleToSquareFOV,
											  ProximityOrderedAgentMap& out_visibleAgents,
											  ProximityOrderedItemMap& out_visibleItems,
											  ProximityOrderedFeatureMap& out_visibleFeatures );
	static void UpdateFromSeenCell( Map* map, const MapPosition& cellPosToTest, const Vector2f& agentOriginPosition, const Vector2f& cellPosition,
									ProximityOrderedAgentMap& out_visibleAgents,
									ProximityOrderedItemMap& out_visibleItems,
									ProximityOrderedFeatureMap& out_visibleFeatures, 
									bool isPlayer );
	static bool DoesStartHaveLineOfSightToEnd( const Vector2i& start, const Vector2i& end, Map* map, bool isPlayer );
	static FieldOfViewType s_fovType;
};
