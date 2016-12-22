#include "Game/NPCs/NPC.hpp"
#include "Game/Behaviors/Behavior.hpp"
#include "Game/FieldOfView/FieldOfView.hpp"


//--------------------------------------------------------------------------------------------------------------
void NPC::PopulateBehaviorsFromXMLNode( const XMLNode& behaviorsNode )
{
	//The complexity here comes from needing to use it for both saves and non-saves.
	//In non-saves, we just copy behaviors off the parsed-in NPCFactory's m_npcTemplate--this is hit for that template, fine and good.
	//In saves, however, we also do that--and set the right agent on it--but then this gets hit after that, demanding overwrites.

	for ( int behaviorNodeIndex = 0; behaviorNodeIndex < behaviorsNode.nChildNode(); )
	{
		XMLNode behaviorNode = behaviorsNode.getChildNode( behaviorNodeIndex );
		std::string behaviorName = behaviorNode.getName();
		Behavior* behavior = BehaviorRegistration::CreateBehaviorByNameAndXML( behaviorName, behaviorNode );
		if ( behavior == nullptr )
			continue;
		
		//WARNING: BE CAUTIOUS OF SPECIFYING DUPLICATE BEHAVIORS TO OVERRIDE ONE SET OF PARAMETERS WITH ANOTHER IN XML BECAUSE OF BELOW LOOP!

		//Look out for behavior presets added by NPC copy ctor--in the case that this is a save game and we need to overwrite, not new off.
		for ( Behavior* existingBehavior : m_behaviors ) 
		{
			if ( existingBehavior->GetName() == behaviorName )
			{
				*existingBehavior = *behavior; //Beware shallow copy issues.
				delete behavior; //Cleanup.
				goto advanceBehaviorIndex; //We don't need to continue but don't want to hit line below via a break.
			}
		}

		//If we reach here, we've determined this is not a duplicate behavior.
		m_behaviors.push_back( behavior );
		
	advanceBehaviorIndex:
		++behaviorNodeIndex;
	}
}


//--------------------------------------------------------------------------------------------------------------
NPC::NPC( const NPC& other, Map* map /*= nullptr*/, const XMLNode& instanceDataNode /*= XMLNode::emptyNode()*/ )
	: Agent( other )
{
	//This is from the template, not a game save with possibly changed XML.
	for ( Behavior* behavior : other.m_behaviors ) 
	{
		m_behaviors.push_back( behavior->CreateClone() );
		m_behaviors.back()->SetAgent( this );
	}

	if ( !instanceDataNode.isEmpty() ) //IsContentEmpty() would be true for <Wander />.
	{
		PopulateFromXMLNode( instanceDataNode, map );
		for ( Behavior* behavior : m_behaviors )
			behavior->SetAgent( this ); //Overwritten during Populate().
	}
}


//--------------------------------------------------------------------------------------------------------------
NPC::~NPC()
{
	for ( Behavior* bh : m_behaviors )
		if ( bh != nullptr )
			delete bh;
}


//--------------------------------------------------------------------------------------------------------------
CooldownSeconds NPC::Update( float deltaSeconds )
{
	UNREFERENCED( deltaSeconds );

	ASSERT_OR_DIE( m_behaviors.size() != 0, "No Behaviors To Use in NPC::Update!" );

	UpdateVisibility();

	CooldownSeconds durationUntilNextTurn = DEFAULT_TURN_COOLDOWN;
	
	durationUntilNextTurn = RunCurrentMaxUtilityBehavior(); //Can return float amounts for an agility, doing this for now for simplicity.

	//Agent::Update( deltaSeconds );

	return durationUntilNextTurn;
}


//--------------------------------------------------------------------------------------------------------------
CooldownSeconds NPC::RunCurrentMaxUtilityBehavior()
{
	UtilityValue maxUtility = 0.f; //Lowest starting utility, make a constant?
	int winningBehaviorIndex = -1;
	for ( unsigned int currentBehaviorIndex = 0; currentBehaviorIndex < m_behaviors.size(); currentBehaviorIndex++ )
	{
		Behavior* currentBehavior = m_behaviors[ currentBehaviorIndex ];

		if ( currentBehavior->IsAboveChanceToRun() == false )
			continue;

		//Change this to <= to make the lowest XML behavior element go first instead of topmost.
		UtilityValue currentBehaviorUtility = currentBehavior->CalcUtility();
		if ( maxUtility < currentBehaviorUtility )
		{
			winningBehaviorIndex = currentBehaviorIndex;
			maxUtility = currentBehaviorUtility;
		}
	}

	ASSERT_OR_DIE( winningBehaviorIndex != -1, nullptr );

	CooldownSeconds durationUntilNextTurn = DEFAULT_TURN_COOLDOWN;
		
	durationUntilNextTurn = m_behaviors[ winningBehaviorIndex ]->Run();

	return durationUntilNextTurn;
}


//--------------------------------------------------------------------------------------------------------------
void NPC::PopulateFromXMLNode( const XMLNode& npcBlueprintNode, Map* map )
{
	Agent::PopulateFromXMLNode( npcBlueprintNode, map );

	PopulateBehaviorsFromXMLNode( npcBlueprintNode.getChildNode( "Behaviors" ) );
}


//--------------------------------------------------------------------------------------------------------------
void NPC::UpdateVisibility()
{
	m_visibleAgents.clear();
	m_visibleItems.clear();
	m_visibleFeatures.clear();
	FieldOfView::CalculateFieldOfViewForAgent( this, m_viewRadius, m_map, true, m_visibleAgents, m_visibleItems, m_visibleFeatures );
	m_targetEnemy = ( m_visibleAgents.size() > 0 ) ? m_visibleAgents.begin()->second : nullptr; //Map sorted by distance, so first == closest.
}


//--------------------------------------------------------------------------------------------------------------
void NPC::WriteToXMLNode( XMLNode& out_entityDataNode )
{
	XMLNode npcNode = out_entityDataNode.addChild( "NPCBlueprint" );

	Agent::WriteToXMLNode( npcNode );

	XMLNode behaviorsNode = npcNode.addChild( "Behaviors" );
	for ( Behavior* behavior : m_behaviors )
		behavior->WriteToXMLNode( behaviorsNode );
}