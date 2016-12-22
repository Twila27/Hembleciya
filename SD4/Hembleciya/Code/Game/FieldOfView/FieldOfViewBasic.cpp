#include "Game/FieldOfView/FieldOfViewBasic.hpp"
#include "Game/Map.hpp"

//--------------------------------------------------------------------------------------------------------------
STATIC const int FieldOfViewBasic::RAYCAST_NUM_STEPS = 100;
STATIC RaycastMode FieldOfViewBasic::s_raycastMode = AMANATIDES_WOO;

//--------------------------------------------------------------------------------------------------------------
STATIC bool FieldOfViewBasic::LineOfSightTest_StepAndSample( const Vector2f& start, const Vector2f& end, Map* map, bool isPlayer )
{
	//"Step and Sample" Poor Man's 2D Tile-based Raycast Algorithm
	//
	const int NUM_STEPS_INT = FieldOfViewBasic::RAYCAST_NUM_STEPS;
	const float NUM_STEPS_FLOAT = static_cast<float>( NUM_STEPS_INT );

	Vector2f displacement = end - start;
	Vector2f stepAlongDisplacement = displacement / NUM_STEPS_FLOAT;
	Vector2f currentPos = start;

	for ( int i = 0; i < NUM_STEPS_INT; i++ )
	{
		currentPos += stepAlongDisplacement;
		MapPosition currentPos2i = Vector2i( static_cast<int>( currentPos.x ), static_cast<int>( currentPos.y ) );
		if ( !map->IsPositionOnMap( currentPos2i ) )
			return false;

		if ( isPlayer )
			map->GetCellForPosition( currentPos2i ).SetSeen();

		if ( map->DoesBlockLineOfSightAtPosition( currentPos2i ) )
			return false;
	}

	return true;
}


//--------------------------------------------------------------------------------------------------------------
STATIC bool FieldOfViewBasic::LineOfSightTest_AmanatidesWoo( const Vector2f& start, const Vector2f& end, Map* map, bool isPlayer )
{

	//Initialization of Regan-cast
	Vector2f currentGridPos = start;

	if ( map->DoesBlockLineOfSightAtPosition( currentGridPos ) )
	{
		if ( isPlayer )
			map->GetCellForPosition( currentGridPos ).SetSeen(); //Wall now visible.
		return false;
	}

	Vector2f rayDisplacement = end - start;
	rayDisplacement += .00001f; //Just to nudge it enough off division by zero if tile is straight across or up/down.
		//If this above line is removed, the row and column of the source agent will likely not be seen!

	//tDelta: how much t it takes to cross to the next x/y/z.
	float tDeltaX = abs( 1.0f / rayDisplacement.x );
	float tDeltaY = abs( 1.0f / rayDisplacement.y );

	//Which direction to step along, computable because a ray's dir is constant.
	int tileStepX = ( rayDisplacement.x > 0 ) ? 1 : -1;
	int tileStepY = ( rayDisplacement.y > 0 ) ? 1 : -1;

	int offsetToLeadingEdgeX = ( tileStepX + 1 ) >> 1; //replaces div by 2^1.
	int offsetToLeadingEdgeY = ( tileStepY + 1 ) >> 1; //replaces div by 2^1.

	float firstIntersectionOnX = (float)( currentGridPos.x + offsetToLeadingEdgeX );
	float firstIntersectionOnY = (float)( currentGridPos.y + offsetToLeadingEdgeY );

	float tOfNextCrossingOnX = abs( firstIntersectionOnX - start.x ) * tDeltaX;
	float tOfNextCrossingOnY = abs( firstIntersectionOnY - start.y ) * tDeltaY;

	//Main Loop of Regan-cast
	bool shutUpVisualStudioWarning = true;
	while ( shutUpVisualStudioWarning ) // Loop until impact or end of line segment.
	{
		//Find min of tOfNextCrossingOnX,Y,Z to determine what line gets crossed next.

		//if OnX is lowest
		if ( ( tOfNextCrossingOnX < tOfNextCrossingOnY ) )
		{
			if ( tOfNextCrossingOnX > 1 )
				return true; //No impact, i.e. next crossing past endpoint.

			currentGridPos.x += tileStepX; //move into next tile on x

			if ( map->DoesBlockLineOfSightAtPosition( currentGridPos ) ) //Impact.
			{
				if ( isPlayer )
					map->GetCellForPosition( currentGridPos ).SetSeen(); //Make the wall visible.
				return false;
			}
			else tOfNextCrossingOnX += tDeltaX; //Move t to next crossing onX.			
		}

		//if OnY is lowest
		else // ( ( tOfNextCrossingOnY < tOfNextCrossingOnX ) )
		{
			if ( tOfNextCrossingOnY > 1 )
				return true; //No impact, i.e. next crossing past endpoint.

			currentGridPos.y += tileStepY; //move into next tile on y
			if ( map->DoesBlockLineOfSightAtPosition( currentGridPos ) ) //Impact.
			{
				if ( isPlayer )
					map->GetCellForPosition( currentGridPos ).SetSeen(); //Make the wall visible.
				return false;
			}
			else tOfNextCrossingOnY += tDeltaY; //Move t to next crossing onY.
		}
	}

	return true;
}


