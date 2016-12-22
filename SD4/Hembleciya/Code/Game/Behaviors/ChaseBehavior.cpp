#include "Game/Behaviors/ChaseBehavior.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Agent.hpp"
#include "Game/Pathfinding/Pathfinder.hpp"
#include "Engine/Core/TheConsole.hpp"


//--------------------------------------------------------------------------------------------------------------
BehaviorRegistration ChaseBehavior::s_ChaseBehavior = BehaviorRegistration( "Chase", ChaseBehavior::CreateChaseBehavior );
STATIC const float ChaseBehavior::s_DEFAULT_MAX_TURNS_TO_CHASE = 5.f;
STATIC const float ChaseBehavior::s_DEFAULT_MAX_TILES_AWAY_TO_START_CHASING = 5.f;


//--------------------------------------------------------------------------------------------------------------
ChaseBehavior::ChaseBehavior( const XMLNode& behaviorNode )
	: Behavior( behaviorNode )
	, m_chaseTarget( "" )
	, m_ignoreUnarmedAgents( false )
	, m_maxTilesAwayToStartChasing( s_DEFAULT_MAX_TILES_AWAY_TO_START_CHASING )
	, m_maxTurnsToChase( s_DEFAULT_MAX_TURNS_TO_CHASE )
{
	m_chaseTarget = ReadXMLAttribute( behaviorNode, "chaseTarget", m_chaseTarget );
	m_maxTilesAwayToStartChasing = ReadXMLAttribute( behaviorNode, "maxTilesAwayToStartChasing", m_maxTilesAwayToStartChasing );
	m_maxTurnsToChase = ReadXMLAttribute( behaviorNode, "maxTurnsToChase", m_maxTurnsToChase );
	m_ignoreUnarmedAgents = ( ReadXMLAttribute( behaviorNode, "ignoreUnarmedAgents", 0 ) > 0 ) ? true : false;
}


//--------------------------------------------------------------------------------------------------------------
UtilityValue ChaseBehavior::CalcUtility()
{
	const Agent* targetEnemy = m_agent->GetTargetEnemy();

	if ( targetEnemy == nullptr )
		return NO_UTILITY_VALUE;

	if ( m_chaseTarget != "" )
	{
		for ( const ProximityOrderedAgentMapPair& visibleAgent : m_agent->GetVisibleAgents() )
		{
			if ( visibleAgent.second->GetName() == m_chaseTarget )
			{
				Path* outPath = nullptr;
				outPath = PathFactory::StartPathfinding( outPath, m_agent->GetMap(), 
														 m_agent->GetPositionMins(), 
														 visibleAgent.second->GetPositionMins(), 
														 m_agent->GetTraversalProperties() );

				if ( !outPath->Pathfind() ) //Just 1 step, since target may not wait around for us.
					continue; //Was no path to target.

				m_agent->SetTargetEnemy( visibleAgent.second );
				break;
			}
		}
	}

	if ( m_ignoreUnarmedAgents && targetEnemy->GetBestOfEquippedItemType( ITEM_TYPE_WEAPON ) == nullptr )
		return NO_UTILITY_VALUE;

	int enemyProximity = CalcDistSquaredBetweenPoints( m_agent->GetPositionMins(), targetEnemy->GetPositionMins() );
	if ( enemyProximity > ( m_maxTilesAwayToStartChasing * m_maxTilesAwayToStartChasing ) )
	{	
		if ( m_agent->GetPath() != nullptr ) //Get rid of old Path.
			m_agent->SetPath( nullptr );

		return NO_UTILITY_VALUE; //Ran outside our chasing range.
	}

	return MIN_UTILITY_VALUE + ( m_maxTilesAwayToStartChasing - (int)sqrt(enemyProximity) );
}


//--------------------------------------------------------------------------------------------------------------
CooldownSeconds ChaseBehavior::Run()
{
	if ( m_agent->GetTargetEnemy() == nullptr )
		return NO_TURN_COOLDOWN;

	static bool hasTalkedBefore = false;
	if ( !hasTalkedBefore && m_agent->IsPlayer() )
	{
		hasTalkedBefore = true;
		g_theConsole->Printf( "The %s chased after you with a snarl!" );
		g_theConsole->ShowConsole();
	}

	m_agent->SetPath( nullptr ); //Cleanup previous.
	const MapPosition& agentPos = m_agent->GetPositionMins();
	const MapPosition& targetPos = m_agent->GetTargetEnemy()->GetPositionMins();

	//Attempt it two ways, colliding and not colliding other agents, but default to not collide:
	//Suggested modification if we don't like that this way here makes us take a longer path to the goal, when waiting might be better,
	//1st CalcPath with the blocked bit on, 2nd with it off, no matter whether the 1st succeeded, and then pick the shorter of the two.
	Path* outPath = nullptr;
	outPath = PathFactory::StartPathfinding( outPath, m_agent->GetMap(), agentPos, targetPos, m_agent->GetTraversalProperties() | TraversalProperties::BLOCKED_BY_AGENTS );
	if ( outPath->Pathfind( 1 ) ) //Only one step, as target may not just wait around for us.
	{
		m_agent->SetPath( outPath );
	}
	else
	{
		outPath = PathFactory::StartPathfinding( outPath, m_agent->GetMap(), agentPos, targetPos, m_agent->GetTraversalProperties() & ~TraversalProperties::BLOCKED_BY_AGENTS );
		outPath->Pathfind( 1 ); //Only one step, as target may not just wait around for us.
		m_agent->SetPath( outPath );
	}


	outPath->GetNextPositionAlongPath(); //Burn through the zeroth start position where the agent already is.
	MapDirection goalDirection = GetDirectionBetweenMapPositions( agentPos, outPath->GetNextPositionAlongPath() );
	MapPosition positionDelta = GetPositionDeltaForMapDirection( goalDirection );

	if ( m_agent->TestOneStep( m_agent->GetPositionMins() + positionDelta ) )
		m_agent->MoveOneStep( positionDelta );

	return DEFAULT_TURN_COOLDOWN;
}


//--------------------------------------------------------------------------------------------------------------
void ChaseBehavior::WriteToXMLNode( XMLNode& behaviorsNode )
{
	Behavior::WriteToXMLNode( behaviorsNode );
	XMLNode behaviorNode = behaviorsNode.getChildNode( m_name.c_str() );

	WriteXMLAttribute( behaviorNode, "chaseTarget", m_chaseTarget, std::string() );
	WriteXMLAttribute( behaviorNode, "ignoreUnarmedAgents", m_ignoreUnarmedAgents ? 1 : 0, 0 );
	WriteXMLAttribute( behaviorNode, "maxTurnsToChase", m_maxTurnsToChase, s_DEFAULT_MAX_TURNS_TO_CHASE );
	WriteXMLAttribute( behaviorNode, "maxTilesAwayToStartChasing", m_maxTilesAwayToStartChasing, s_DEFAULT_MAX_TILES_AWAY_TO_START_CHASING );
}
