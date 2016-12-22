#include "Game/Behaviors/MeleeBehavior.hpp"
#include "Game/Agent.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"
#include "Game/CombatSystem.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC BehaviorRegistration MeleeBehavior::s_MeleeBehavior = BehaviorRegistration( "MeleeAttack", MeleeBehavior::CreateMeleeBehavior );
STATIC const Interval<int> MeleeBehavior::s_DEFAULT_BASE_DAMAGE_RANGE = Interval<int>( 5, 5 );
STATIC const float MeleeBehavior::s_DEFAULT_CHANCE_TO_HIT = 1.0f;


//--------------------------------------------------------------------------------------------------------------
MeleeBehavior::MeleeBehavior( const XMLNode& behaviorNode )
	: Behavior( behaviorNode )
	, m_baseDamageRange( s_DEFAULT_BASE_DAMAGE_RANGE )
	, m_chanceToHit( s_DEFAULT_CHANCE_TO_HIT )
{
	m_baseDamageRange = ReadXMLAttribute( behaviorNode, "baseDamage", m_baseDamageRange );
	m_chanceToHit = ReadXMLAttribute( behaviorNode, "chanceToHit", m_chanceToHit );
}


//--------------------------------------------------------------------------------------------------------------
UtilityValue MeleeBehavior::CalcUtility() //Should be nil when targets are not adjacent.
{
	const Agent* targetEnemy = m_agent->GetTargetEnemy();

	if ( targetEnemy == nullptr )
		return NO_UTILITY_VALUE;

	if ( m_agent->DoesNotHarmFaction( targetEnemy->GetFactionID() ) )
		return NO_UTILITY_VALUE;

	//1.5 is upper limit to include diagonally adjacent case, which results in distance sqrt(2) <= 1.5.
	if ( CalcDistBetweenPoints( m_agent->GetPositionMins(), targetEnemy->GetPositionMins() ) <= 1.5 )
		return MIN_UTILITY_VALUE + HIGH_UTILITY_VALUE;
	else
		return NO_UTILITY_VALUE;
}


//-----------------------------------------------------------------------------------------------------
CooldownSeconds MeleeBehavior::Run()
{
	Agent* targetEnemy = m_agent->GetTargetEnemy();
	if ( targetEnemy == nullptr )
		return NO_TURN_COOLDOWN;

	AttackData attackData;

	attackData.baseDamage = m_baseDamageRange.GetRandomElement();
	attackData.chanceToHit = m_chanceToHit;
	attackData.instigator = m_agent;
	attackData.target = targetEnemy;
	attackData.instigatorWeapon = m_agent->GetBestOfEquippedItemType( ITEM_TYPE_WEAPON );
	attackData.targetArmor = targetEnemy->GetBestOfEquippedItemType( ITEM_TYPE_ARMOR );

	CombatSystem::PerformAttack( attackData );

	return DEFAULT_TURN_COOLDOWN;
}


//--------------------------------------------------------------------------------------------------------------
void MeleeBehavior::WriteToXMLNode( XMLNode& behaviorsNode )
{
	Behavior::WriteToXMLNode( behaviorsNode );
	XMLNode behaviorNode = behaviorsNode.getChildNode( m_name.c_str() );

	WriteXMLAttribute( behaviorNode, "baseDamage", m_baseDamageRange, s_DEFAULT_BASE_DAMAGE_RANGE );
}
