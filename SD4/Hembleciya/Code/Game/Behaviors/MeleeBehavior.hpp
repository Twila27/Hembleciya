#pragma once


#include "Game/Behaviors/Behavior.hpp"
#include "Engine/Math/Interval.hpp"
#include "Game/GameCommon.hpp"


class MeleeBehavior : public Behavior
{
	MeleeBehavior( const XMLNode& behaviorNode );

	static Behavior* CreateMeleeBehavior( const XMLNode& behaviorNode ) { return new MeleeBehavior( behaviorNode );	}

	virtual UtilityValue CalcUtility() override; //Should be nil when targets are not adjacent.
	virtual CooldownSeconds Run() override;
	virtual Behavior* CreateClone() const override { return new MeleeBehavior( *this );	}
	virtual void WriteToXMLNode( XMLNode& behaviorsNode ) override;

	Interval<int> m_baseDamageRange;
	float m_chanceToHit;

	static BehaviorRegistration s_MeleeBehavior;
	static const Interval<int> s_DEFAULT_BASE_DAMAGE_RANGE;
	static const float s_DEFAULT_CHANCE_TO_HIT;
};
