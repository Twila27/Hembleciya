#pragma once


#include "Game/GameCommon.hpp"


//--------------------------------------------------------------------------------------------------------------
struct PathNode
{
	PathNode( const Vector2i& position, PathNode* parent, float localStepCostG, float parentStepCostG, float estimatedStepCostH );

	Vector2i m_position;
	PathNode* m_parent; //Previous position.
	float m_localStepCostG;
	float m_parentStepCostG; //The actual summed step cost previous history of where we've been, not including the new/local step g-cost.
	float m_estimatedStepCostH; //Heuristic comes into play here! Manhattan distance common for 2D grids.

	inline float GetTotalCostG() const { return m_localStepCostG + m_parentStepCostG; }
	inline float GetWeightedStepCostF() const { return GetTotalCostG() + m_estimatedStepCostH; }
	inline void SetFasterPath( PathNode* newParent, float newLocalStepCostG, float newParentStepCostG );

	void Render( const Rgba& tint );
};


//--------------------------------------------------------------------------------------------------------------
void PathNode::SetFasterPath( PathNode* newParent, float newLocalStepCostG, float newParentStepCostG )
{
	m_parent = newParent;
	m_localStepCostG = newLocalStepCostG;
	m_parentStepCostG = newParentStepCostG;
}