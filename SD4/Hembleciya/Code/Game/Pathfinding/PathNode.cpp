#include "Game/Pathfinding/PathNode.hpp"
#include "Engine/Renderer/TheRenderer.hpp"


//--------------------------------------------------------------------------------------------------------------
PathNode::PathNode( const Vector2i& position, PathNode* parent, float localStepCostG, float parentStepCostG, float estimatedStepCostH )
	: m_position( position )
	, m_parent( parent )
	, m_localStepCostG( localStepCostG )
	, m_parentStepCostG( parentStepCostG )
	, m_estimatedStepCostH( estimatedStepCostH )
{
}


//--------------------------------------------------------------------------------------------------------------
void PathNode::Render( const Rgba& tint )
{
	Vector2f screenPositionMins = GetScreenPositionForMapPosition( m_position - Vector2i::UNIT_Y );
	Vector2f screenPositionMaxs = GetScreenPositionForMapPosition( m_position + Vector2i::ONE - Vector2i::UNIT_Y );
	g_theRenderer->DrawAABB( TheRenderer::VertexGroupingRule::AS_QUADS, AABB2f( screenPositionMins, screenPositionMaxs ), tint );
}