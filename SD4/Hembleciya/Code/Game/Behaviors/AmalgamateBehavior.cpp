#include "Game/Behaviors/AmalgamateBehavior.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Agent.hpp"
#include "Game/Pathfinding/Pathfinder.hpp"
#include "Engine/Core/TheConsole.hpp"
#include "Engine/Audio/TheAudio.hpp"


//--------------------------------------------------------------------------------------------------------------
BehaviorRegistration AmalgamateBehavior::s_AmalgamateBehavior = BehaviorRegistration( "Amalgamate", AmalgamateBehavior::CreateAmalgamateBehavior );
STATIC const float AmalgamateBehavior::s_DEFAULT_MAX_DIST_AWAY_TO_AMALGAMATE = 1.5f; //diagonals included because sqrt(2)
STATIC const float AmalgamateBehavior::s_DEFAULT_MAX_HEALTH_FRACTION_NEEDED_TO_ACTIVATE = .1f;


//--------------------------------------------------------------------------------------------------------------
AmalgamateBehavior::AmalgamateBehavior( const XMLNode& behaviorNode )
	: Behavior( behaviorNode )
	, m_hasFused( false )
	, m_maxDistAwayToAmalgamate( s_DEFAULT_MAX_DIST_AWAY_TO_AMALGAMATE ) 
	, m_maxHealthFractionNeededToActivate( s_DEFAULT_MAX_HEALTH_FRACTION_NEEDED_TO_ACTIVATE )
{
	m_hasFused = ( ReadXMLAttribute( behaviorNode, "hasFused", 0 ) > 0 ) ? true : false;
	m_maxDistAwayToAmalgamate = ReadXMLAttribute( behaviorNode, "maxDistAwayToAmalgamate", m_maxDistAwayToAmalgamate );
	m_maxHealthFractionNeededToActivate = ReadXMLAttribute( behaviorNode, "maxHealthFractionNeededToActivate", m_maxHealthFractionNeededToActivate );
}


//--------------------------------------------------------------------------------------------------------------
UtilityValue AmalgamateBehavior::CalcUtility()
{
	if ( m_hasFused )
		return NO_UTILITY_VALUE;

	const Agent* targetEnemy = m_agent->GetTargetEnemy();

	if ( targetEnemy == nullptr )
		return NO_UTILITY_VALUE;

	if ( targetEnemy->IsPlayer() )
		return NO_UTILITY_VALUE; //Was too sudden when you were the player, felt unfair.

	if ( m_agent->GetHealth() > ( m_agent->GetMaxHealth() * m_maxHealthFractionNeededToActivate ) )
		return NO_UTILITY_VALUE; //Agent still too healthy.

	//May want to instead search adjacent tiles for a non-targetEnemy to amalgam with. Since that's harder to view, for now:
	if ( targetEnemy->GetHealth() > ( targetEnemy->GetMaxHealth() * m_maxHealthFractionNeededToActivate ) )
		return NO_UTILITY_VALUE; //Enemy still too healthy.

	int enemyProximity = CalcDistSquaredBetweenPoints( m_agent->GetPositionMins(), m_agent->GetTargetEnemy()->GetPositionMins() );
	if ( enemyProximity > ( m_maxDistAwayToAmalgamate * m_maxDistAwayToAmalgamate ) )
		return NO_UTILITY_VALUE;

	return MIN_UTILITY_VALUE + HIGH_UTILITY_VALUE * HIGH_UTILITY_VALUE;
}


//--------------------------------------------------------------------------------------------------------------
CooldownSeconds AmalgamateBehavior::Run()
{
	static SoundID amalgamateSoundID = g_theAudio->CreateOrGetSound( "Data/Audio/Amalgamate.wav" );
	g_theAudio->PlaySound( amalgamateSoundID, VOLUME_MULTIPLIER );

	ROADMAP( "In long run, want to make it spawn a multi-tile Amalgam NPCFactory product.\n \
			Maybe make an NPC Amalgam subclass, push any # NPCs into it.\n \
			Then they can be inserted into the turn order as a group?" );

	Agent* targetEnemy = m_agent->GetTargetEnemy();

	if ( targetEnemy == nullptr )
		return NO_TURN_COOLDOWN;

	Rgba mergedColor = Rgba::AverageColors( m_agent->GetColor(), targetEnemy->GetColor() );

	int gainedHealth = targetEnemy->GetHealth();
	int gainedDamage = targetEnemy->GetDamageBonus();

	m_agent->SetMaxHealth( m_agent->GetMaxHealth() + targetEnemy->GetMaxHealth() );
	m_agent->AddHealthDelta( targetEnemy->GetHealth() );
	m_agent->AddDamageBonus( targetEnemy->GetDamageBonus() );
	m_agent->SetColor( mergedColor );
	m_agent->SetGlyph( (char)toupper( (int)m_agent->GetGlyph() ) );
	targetEnemy->SubtractHealthDelta( targetEnemy->GetHealth() );

	if ( m_agent->IsCurrentlySeen() || targetEnemy->IsCurrentlySeen() )
	{
		g_theConsole->SetTextColor( Rgba::YELLOW * Rgba::GRAY );
		g_theConsole->Printf( "%s amalgamated %s, gaining %d health and %d damage!",
								  m_agent->GetName().c_str(),
								  targetEnemy->GetName().c_str(),
								  gainedHealth,
								  gainedDamage );
		g_theConsole->SetTextColor();
	}
	else
	{
		g_theConsole->SetTextColor( Rgba::YELLOW * Rgba::DARK_GRAY );
		g_theConsole->Printf( "From far off you hear the sounds of dreams fusing together..." );
		g_theConsole->SetTextColor();
	}

	targetEnemy = nullptr;

	m_hasFused = true;

	return DEFAULT_TURN_COOLDOWN * 2.f;
}


//--------------------------------------------------------------------------------------------------------------
void AmalgamateBehavior::WriteToXMLNode( XMLNode& behaviorsNode )
{
	Behavior::WriteToXMLNode( behaviorsNode );
	XMLNode behaviorNode = behaviorsNode.getChildNode( m_name.c_str() );

	WriteXMLAttribute( behaviorNode, "hasFused", m_hasFused ? 1 : 0, 0 );
	WriteXMLAttribute( behaviorNode, "maxDistAwayToAmalgamate", m_maxDistAwayToAmalgamate, s_DEFAULT_MAX_DIST_AWAY_TO_AMALGAMATE );
	WriteXMLAttribute( behaviorNode, "maxHealthFractionNeededToActivate", m_maxHealthFractionNeededToActivate, s_DEFAULT_MAX_HEALTH_FRACTION_NEEDED_TO_ACTIVATE );
}
