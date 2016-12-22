#pragma once


#include "Game/Behaviors/Behavior.hpp"
#include "Game/GameCommon.hpp"


class AmalgamateBehavior : public Behavior
{
	AmalgamateBehavior( const XMLNode& behaviorNode );

	static Behavior* CreateAmalgamateBehavior( const XMLNode& behaviorNode ) { return new AmalgamateBehavior( behaviorNode ); }

	virtual UtilityValue CalcUtility() override; //Varies based on whether targets are weak and adjacent.
	virtual CooldownSeconds Run() override;
	virtual Behavior* CreateClone() const override { return new AmalgamateBehavior( *this ); }
	virtual void WriteToXMLNode( XMLNode& behaviorsNode ) override;

	float m_maxDistAwayToAmalgamate;
	float m_maxHealthFractionNeededToActivate;
	bool m_hasFused;

	static BehaviorRegistration s_AmalgamateBehavior;
	static const float s_DEFAULT_MAX_DIST_AWAY_TO_AMALGAMATE;
	static const float s_DEFAULT_MAX_HEALTH_FRACTION_NEEDED_TO_ACTIVATE;
};
