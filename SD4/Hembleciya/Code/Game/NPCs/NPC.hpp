#pragma once


#include "Game/Agent.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"
#include <vector>

class Behavior;


class NPC : public Agent
{
public:
	NPC( const XMLNode& npcBlueprintNode, Map* map ) : Agent( ENTITY_TYPE_NPC ) { PopulateFromXMLNode( npcBlueprintNode, map ); }
	NPC( const NPC& other, Map* map = nullptr, const XMLNode& instanceDataNode = XMLNode::emptyNode() ); //Else m_behaviors gets shallowed copied between all instances of the NPCFactory!
	virtual ~NPC() override;
	virtual CooldownSeconds Update( float deltaSeconds ) override;

	std::vector< Behavior* >& GetBehaviors() { return m_behaviors; }

	virtual void WriteToXMLNode( XMLNode& out_entityDataNode ) override;


private:
	virtual void PopulateFromXMLNode( const XMLNode& npcBlueprintNode, Map* map ) override;
	void PopulateBehaviorsFromXMLNode( const XMLNode& behaviorsNode );

	void UpdateVisibility();
	CooldownSeconds RunCurrentMaxUtilityBehavior();
	std::vector< Behavior* > m_behaviors;
};
