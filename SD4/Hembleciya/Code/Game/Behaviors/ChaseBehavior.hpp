#pragma once


#include "Game/Behaviors/Behavior.hpp"
#include "Game/GameCommon.hpp"


class ChaseBehavior : public Behavior
{
	ChaseBehavior( const XMLNode& behaviorNode );

	static Behavior* CreateChaseBehavior( const XMLNode& behaviorNode ) { return new ChaseBehavior( behaviorNode ); }

	virtual UtilityValue CalcUtility() override; //Varies based on whether chase-able things are near.
	virtual CooldownSeconds Run() override;
	virtual Behavior* CreateClone() const override { return new ChaseBehavior( *this ); }
	virtual void WriteToXMLNode( XMLNode& behaviorsNode ) override;

	bool m_ignoreUnarmedAgents;
	float m_maxTurnsToChase; //After which it gives up the chase.
	float m_maxTilesAwayToStartChasing; //Float to e.g. specify 1.5 to include diagonals sqrt(2) or ~1.4 tiles away.
	std::string m_chaseTarget;

	static BehaviorRegistration s_ChaseBehavior;
	static const float s_DEFAULT_MAX_TURNS_TO_CHASE;
	static const float s_DEFAULT_MAX_TILES_AWAY_TO_START_CHASING;
};
