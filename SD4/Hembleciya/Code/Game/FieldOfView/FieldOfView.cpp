#include "Game/FieldOfView/FieldOfView.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Agent.hpp"
#include "Game/Items/Item.hpp"
#include "Game/Features/Feature.hpp"
#include "Game/Map.hpp"

#include "Game/FieldOfView/FieldOfViewBasic.hpp"
#include "Game/FieldOfView/FieldOfViewRaycast.hpp"


STATIC FieldOfViewType FieldOfView::s_fovType = FOV_BASIC;

void FieldOfView::CalculateFieldOfViewForAgent( Agent* agentOrigin, int viewRadius, Map* map, bool preferCircleToSquareFOV, 
												ProximityOrderedAgentMap& out_visibleAgents,
												ProximityOrderedItemMap& out_visibleItems,
												ProximityOrderedFeatureMap& out_visibleFeatures )
{
	
	if ( agentOrigin->IsPlayer() )
	{
		//Reset all cells to hide those no longer actively seen (they will keep the record of whether they have ever been themselves).
		for ( Cell& cell : map->GetCells() )
			cell.SetUnseen();
		for ( ProximityOrderedItemMapPair item : out_visibleItems )
			item.second->SetUnseen();

		map->GetCellForPosition( agentOrigin->GetPositionMins() ).SetSeen(); //Else player appears at half-alpha.
	}

	std::vector< MapPosition > potentiallyViewedCells;
	MapPosition agentPos = agentOrigin->GetPositionMins();
	int viewRadiusSquared = viewRadius * viewRadius;
	switch ( s_fovType )
	{
		case FOV_BASIC:
		{
			for ( int y = -viewRadius; y < viewRadius; y++ )
			{
				for ( int x = -viewRadius; x < viewRadius; x++ )
				{
					if ( x == 0 && y == 0 ) //Not checking under you, else you'd target yourself.
						continue;

					MapPosition agentRelativePos = agentPos + MapPosition( x, y );

					if ( preferCircleToSquareFOV )
						if ( CalcDistSquaredBetweenPoints( agentPos, agentRelativePos ) > viewRadiusSquared )
							continue;

					if ( !map->IsPositionOnMap( agentRelativePos ) )
						continue;

					potentiallyViewedCells.push_back( agentRelativePos );
				}
			}

			for ( MapPosition cellPosToTest : potentiallyViewedCells )
			{
				Vector2f minsRaycastStart = Vector2f( static_cast<float>( agentPos.x ), static_cast<float>( agentPos.y ) );
				Vector2f minsRaycastEnd = Vector2f( static_cast<float>( cellPosToTest.x ), static_cast<float>( cellPosToTest.y ) );
				
				//OPTIMIZATION: cast tile corners versus tile corners, not center versus center.
				if ( FieldOfViewBasic::DoesStartHaveLineOfSightToEnd( minsRaycastStart, minsRaycastEnd, map, agentOrigin->IsPlayer() ) )
				{
					UpdateFromSeenCell( map, cellPosToTest, minsRaycastStart, minsRaycastEnd, out_visibleAgents, out_visibleItems, out_visibleFeatures, agentOrigin->IsPlayer() );
					continue;
				}
				if ( FieldOfViewBasic::DoesStartHaveLineOfSightToEnd( minsRaycastStart + Vector2f::ONE, minsRaycastEnd + Vector2f::ONE, map, agentOrigin->IsPlayer() ) )
				{
					UpdateFromSeenCell( map, cellPosToTest, minsRaycastStart, minsRaycastEnd, out_visibleAgents, out_visibleItems, out_visibleFeatures, agentOrigin->IsPlayer() );
					continue;
				}
				if ( FieldOfViewBasic::DoesStartHaveLineOfSightToEnd( minsRaycastStart + Vector2f::UNIT_X, minsRaycastEnd + Vector2f::UNIT_X, map, agentOrigin->IsPlayer() ) )
				{
					UpdateFromSeenCell( map, cellPosToTest, minsRaycastStart, minsRaycastEnd, out_visibleAgents, out_visibleItems, out_visibleFeatures, agentOrigin->IsPlayer() );
					continue;
				}
				if ( FieldOfViewBasic::DoesStartHaveLineOfSightToEnd( minsRaycastStart + Vector2f::UNIT_Y, minsRaycastEnd + Vector2f::UNIT_Y, map, agentOrigin->IsPlayer() ) )
				{
					UpdateFromSeenCell( map, cellPosToTest, minsRaycastStart, minsRaycastEnd, out_visibleAgents, out_visibleItems, out_visibleFeatures, agentOrigin->IsPlayer() );
					continue;
				}
			}

			break;
		}
		case FOV_RAYCAST: break;
	}
}


//--------------------------------------------------------------------------------------------------------------
void FieldOfView::UpdateFromSeenCell( Map* map, const MapPosition& cellPosToTest, const Vector2f& agentOriginPosition, const Vector2f& cellPosition, 
									  ProximityOrderedAgentMap& out_visibleAgents, 
									  ProximityOrderedItemMap& out_visibleItems, 
									  ProximityOrderedFeatureMap& out_visibleFeatures,
									  bool isPlayer )
{
	Cell& seenCell = map->GetCellForPosition( cellPosToTest );
	
	if ( isPlayer )
		seenCell.SetSeen();

	if ( seenCell.IsOccupiedByAgent() )
	{
		float distanceToCell = CalcDistSquaredBetweenPoints( agentOriginPosition, cellPosition );
		Agent* agentInCell = seenCell.GetAgent();

		if ( isPlayer )
			agentInCell->SetSeen();

		out_visibleAgents.insert( ProximityOrderedAgentMapPair( distanceToCell, agentInCell ) );
	}
	if ( seenCell.HasItems() )
	{
		float distanceToCell = CalcDistSquaredBetweenPoints( agentOriginPosition, cellPosition );
		std::vector< Item* >& itemsInCell = seenCell.GetItems();

		if ( isPlayer )
		{
			for ( Item* item : itemsInCell )
			{
				item->SetSeen();
				out_visibleItems.insert( ProximityOrderedItemMapPair( distanceToCell, item ) );
			}
		}
	}
	if ( seenCell.IsOccupiedByFeature() )
	{
		float distanceToCell = CalcDistSquaredBetweenPoints( agentOriginPosition, cellPosition );
		Feature* featureInCell = seenCell.GetFeature();

		if ( isPlayer )
			featureInCell->SetSeen();

		out_visibleFeatures.insert( ProximityOrderedFeatureMapPair( distanceToCell, featureInCell ) );
	}
}


//--------------------------------------------------------------------------------------------------------------
bool FieldOfView::DoesStartHaveLineOfSightToEnd( const Vector2i& start, const Vector2i& end, Map* map, bool isPlayer )
{
	Vector2f start2f( static_cast<float>( start.x ), static_cast<float>( start.y ) );
	Vector2f end2f( static_cast<float>( end.x ), static_cast<float>( end.y ) );
	return FieldOfViewBasic::DoesStartHaveLineOfSightToEnd( start2f, end2f, map, isPlayer );
}
