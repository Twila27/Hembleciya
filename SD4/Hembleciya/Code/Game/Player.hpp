#pragma once


#include "Game/Agent.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"
#include "Engine/Math/Interval.hpp"


//-----------------------------------------------------------------------------
enum PlayerAction
{
	PLAYER_ACTION_UNSPECIFIED,
	PLAYER_ACTION_MOVE,
	PLAYER_ACTION_USE_ITEM,
	PLAYER_ACTION_TOGGLE_FEATURE,
	PLAYER_ACTION_REST,
	NUM_PLAYER_ACTIONS
};


//-----------------------------------------------------------------------------
class Player : public Agent
{
public:
	Player( EntityType entityType, Map* map = nullptr, const XMLNode& playerNode = XMLNode::emptyNode() ) 
		: Agent( entityType )
		, m_numTurnsTaken( 0 )
		, m_isInvincible( false )
	{
		m_health = m_maxHealth = 30;
		m_glyph = '@';
		m_color = Rgba::CYAN;
		m_name = "Player";
		m_faction = *Faction::CreateOrGetFaction( "Player" );
		m_traversalProperties = BLOCKED_BY_SOLIDS | BLOCKED_BY_AGENTS;
		SetSeen();

		if ( playerNode.isEmpty() )
			return;

		Agent::PopulateFromXMLNode( playerNode, map );
		m_numTurnsTaken = ReadXMLAttribute( playerNode, "turnsTaken", m_numTurnsTaken );
		m_isInvincible = ( ReadXMLAttribute( playerNode, "isInvincible", 0 ) > 0 ) ? true : false;
	}
	bool ProcessInput();
	int GetNumTurns() const { return m_numTurnsTaken; }

	virtual bool IsPlayer() const override { return true; }
	virtual bool IsInvincible() const override { return m_isInvincible; }
	virtual bool IsReadyToUpdate() const override { return m_nextAction > PLAYER_ACTION_UNSPECIFIED && m_nextAction < NUM_PLAYER_ACTIONS; }
	virtual CooldownSeconds Update( float deltaSeconds ) override; //Return value == time until next turn, default of one.
	virtual void WriteToXMLNode( XMLNode& out_entityDataNode ) override;

private:
	bool ProcessMovementKeyboard( bool& out_pressedLeft, bool& out_pressedRight, bool& out_pressedUp, bool& out_pressedDown );
	bool PredictNextAction( bool pressedLeft, bool pressedRight, bool pressedUp, bool pressedDown, bool pressedUsePotion, bool pressedToggleFeature, bool pressedRest );
	void CheckForItems();

	bool m_isInvincible;
	int m_numTurnsTaken;
	PlayerAction m_nextAction;
	MapDirection m_goalDirection;
	static const Interval<int> s_PLAYER_BASE_DAMAGE_RANGE;
};
