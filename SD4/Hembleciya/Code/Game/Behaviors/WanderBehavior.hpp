#pragma once


#include "Game/Behaviors/Behavior.hpp"
#include "Game/GameCommon.hpp"


class WanderBehavior : public Behavior
{
	WanderBehavior( const XMLNode& behaviorNode );

	static Behavior* CreateWanderBehavior( const XMLNode& behaviorNode ) { return new WanderBehavior( behaviorNode ); }

	virtual UtilityValue CalcUtility() override { return MIN_UTILITY_VALUE; } //Always a contender.
	virtual CooldownSeconds Run() override;
	virtual Behavior* CreateClone() const override { return new WanderBehavior( *this ); }
	virtual void WriteToXMLNode( XMLNode& behaviorsNode ) override;

	MapDirection m_currentDirection;
	float m_chanceToGoStraight;
	float m_chanceToRest;
	int m_healFromResting;

	static BehaviorRegistration s_wanderBehavior;
	static const float s_DEFAULT_CHANCE_TO_GO_STRAIGHT;
	static const float s_DEFAULT_CHANCE_TO_REST;
};
