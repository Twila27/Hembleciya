#include "Game/Behaviors/WanderBehavior.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/TheConsole.hpp"
#include "Game/Agent.hpp"


//--------------------------------------------------------------------------------------------------------------
BehaviorRegistration WanderBehavior::s_wanderBehavior = BehaviorRegistration( "Wander", WanderBehavior::CreateWanderBehavior );
STATIC const float WanderBehavior::s_DEFAULT_CHANCE_TO_GO_STRAIGHT = 0.f;
STATIC const float WanderBehavior::s_DEFAULT_CHANCE_TO_REST = 0.f;


//--------------------------------------------------------------------------------------------------------------
WanderBehavior::WanderBehavior( const XMLNode& behaviorNode )
	: Behavior( behaviorNode )
	, m_chanceToGoStraight( s_DEFAULT_CHANCE_TO_GO_STRAIGHT )
	, m_chanceToRest( s_DEFAULT_CHANCE_TO_REST )
	, m_healFromResting( 1 )
{
	m_healFromResting = ReadXMLAttribute( behaviorNode, "healFromResting", m_healFromResting );
	m_chanceToGoStraight = ReadXMLAttribute( behaviorNode, "chanceToGoStraight", m_chanceToGoStraight );
	m_chanceToRest = ReadXMLAttribute( behaviorNode, "chanceToRest", m_chanceToRest );
	m_currentDirection = (MapDirection)GetRandomIntInRange( 0, MapDirection::NUM_DIRECTIONS - 1 );
}


//--------------------------------------------------------------------------------------------------------------
CooldownSeconds WanderBehavior::Run()
{
	bool canRestAndHeal = ( m_healFromResting > 0 ) && ( m_agent->GetHealth() < m_agent->GetMaxHealth() );

	if ( canRestAndHeal && GetRandomChance( m_chanceToRest ) )
	{
		if ( m_agent->AddHealthDelta( m_healFromResting ) && m_agent->IsCurrentlySeen() )
		{
			g_theConsole->Printf( "%s rested and gained +%d health.", m_agent->GetName().c_str(), m_healFromResting );
			g_theConsole->ShowConsole();
		}

		return DEFAULT_TURN_COOLDOWN * 2.f;
	}
	else
	{
		//-----------------------------------------------------------------------------
		if ( GetRandomChance( m_chanceToGoStraight ) )
			m_currentDirection = (MapDirection)GetRandomIntInRange( 0, MapDirection::NUM_DIRECTIONS - 1 );

		//-----------------------------------------------------------------------------
		MapPosition goalPosition = m_agent->GetPositionMins();
		MapPosition positionDelta = GetPositionDeltaForMapDirection( m_currentDirection );

		goalPosition += positionDelta;

		if ( m_agent->TestOneStep( goalPosition ) )
			m_agent->MoveOneStep( positionDelta );

		return DEFAULT_TURN_COOLDOWN;
	}
}


//--------------------------------------------------------------------------------------------------------------
void WanderBehavior::WriteToXMLNode( XMLNode& behaviorsNode )
{
	Behavior::WriteToXMLNode( behaviorsNode );
	XMLNode behaviorNode = behaviorsNode.getChildNode( m_name.c_str() );

	WriteXMLAttribute( behaviorNode, "healFromResting", m_healFromResting, 1 );
	WriteXMLAttribute( behaviorNode, "chanceToGoStraight", m_chanceToGoStraight, s_DEFAULT_CHANCE_TO_GO_STRAIGHT );
	WriteXMLAttribute( behaviorNode, "chanceToRest", m_chanceToRest, s_DEFAULT_CHANCE_TO_REST );
}
