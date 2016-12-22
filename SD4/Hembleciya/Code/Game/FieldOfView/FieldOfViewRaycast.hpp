#pragma once


#include "Game/GameCommon.hpp"


class Map;


class FieldOfViewRaycast
{
public:
	static bool DoesStartHaveLineOfSightToEnd( const Vector2f& start, const Vector2f& end, Map* map, bool isPlayer )
	{
		if ( s_raycastMode == AMANATIDES_WOO )
			return LineOfSightTest_AmanatidesWoo( start, end, map, isPlayer );
		else
			return LineOfSightTest_StepAndSample( start, end, map, isPlayer );
	}
	static RaycastMode s_raycastMode;

private:
	static bool LineOfSightTest_StepAndSample( const Vector2f& start, const Vector2f& end, Map* map, bool isPlayer );
	static bool LineOfSightTest_AmanatidesWoo( const Vector2f& start, const Vector2f& end, Map* map, bool isPlayer );

	static const int RAYCAST_NUM_STEPS;
};
