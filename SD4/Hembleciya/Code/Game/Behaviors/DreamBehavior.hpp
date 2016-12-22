#pragma once


#include "Game/Behaviors/Behavior.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Cell.hpp"


//-----------------------------------------------------------------------------
class Map;


//-----------------------------------------------------------------------------
class DreamBehavior : public Behavior
{
	DreamBehavior( const XMLNode& behaviorNode );
	DreamBehavior( const DreamBehavior& other ); //Has a pointer, needs deep copy.
	~DreamBehavior();
	virtual inline void operator=( const Behavior& other ) override;

	static Behavior* CreateDreamBehavior( const XMLNode& behaviorNode ) { return new DreamBehavior( behaviorNode ); }

	virtual UtilityValue CalcUtility() override; //Varies based on whether Dream-able things are near.
	virtual CooldownSeconds Run() override;

	bool OverwriteMap();

	virtual Behavior* CreateClone() const override { return new DreamBehavior( *this );	}
	virtual void WriteToXMLNode( XMLNode& behaviorsNode ) override;

	Vector2i GetSpawnInDreamMap() const;

	bool m_isDreaming;
	float m_maxHealthFractionNeededToActivate;
	Map* m_dreamMap;
	Rgba m_dreamTint;
	MapPosition m_dreamMapSpawnPositionInRealMap; //Where the agent was when the dream behavior ran the first time.
		//The agent may move around before it gets broken, so map restore can't use its position. Need this.

	static BehaviorRegistration s_DreamBehavior;
	static const std::string s_DEFAULT_DREAM_NAME;
	static const Rgba s_DEFAULT_DREAM_TINT;
	static const float s_DEFAULT_MAX_HEALTH_FRACTION_NEEDED_TO_ACTIVATE;
};

/*
	Read the below into a map stored on this DreamBehavior via m_dreamMap = Map::Initialize( behaviorNode.getText() );
		For CalcUtility(), check if targetEnemy is the player, if so,
			if m_targetEnemy.position - agent.position
			else return zero.
		On Run(), if !m_isDreaming, SwapTiles( agent.m_map ), if !agent.isAlive(), SwapTiles( agent.m_map ).
		SwapTiles(Map* agentMap) loops over m_dreamMap.GetCells(), and for each cell,
			get its signed x and y away from the S cell and store in an offset,
			write map.GetCellAtPos( agent.GetPos() + that offset ) to a CellType temp, 
			and call agentMap.SetCellType( agent.GetPos() + that offset, currentCell.type ),
			and complete the iteration by writing currentCell.type = temp type.
	Loop over m_dreamMap, find 'S' spawn start source tile, store in m_spawnStartInDreamMap.
	In Run() when swapping, calc positions with this stored MapPosition.

<Dream>
	#####
	#S..#
	#...#
	#...#
	#####
</Dream>
*/