#include "Game/FactionSystem.hpp"
#include "Engine/FileUtils/FileUtils.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"
#include "Game/Agent.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC std::map< std::string, Faction* > Faction::s_factionRegistry;
STATIC FactionID Faction::s_BASE_FACTION_ID = 1;
STATIC float Faction::s_EXTRAPOLATION_RATIO = .3f; //How much a between-agents status delta alters the agent's view of the other's Faction overall.


//--------------------------------------------------------------------------------------------------------------
float Faction::CalcExtrapolationRatio( FactionID instigatorFaction )
{
	//This way, for example, some factions could be more/less apt to think all others of the instigatorFaction are [un]like the instigator.

	//e.g. at an extreme, tick off a certain faction, they instantly hate all members of that type, seeming very war-prone and territorial.

	//For now, as a first pass, keep it simple:
	UNREFERENCED( instigatorFaction );
	return s_EXTRAPOLATION_RATIO;
}


//--------------------------------------------------------------------------------------------------------------
STATIC std::string Faction::GetNameForFactionID( FactionID factionID )
{
	for ( std::pair< const std::string, Faction* >& faction : s_factionRegistry )
		if ( faction.second->GetID() == factionID )
			return faction.second->GetName();
	
	ERROR_AND_DIE( "Did not find faction in global registry! Verify it's in Faction or NPC XML files?" );
}


//--------------------------------------------------------------------------------------------------------------
void Faction::RemoveRelationsWithEntity( const Agent* agentToRemove )
{
	EntityID agentToRemoveID = agentToRemove->GetEntityID();
	for ( auto agentRelationIter = m_agentRelations.begin(); agentRelationIter != m_agentRelations.end(); )
	{
		if ( agentRelationIter->first == agentToRemoveID )
		{
			delete agentRelationIter->second;
			agentRelationIter = m_agentRelations.erase( agentRelationIter );
			return; //Map wouldn't allow a duplicate for the same agentToRemove.
		}
		else ++agentRelationIter;
	}
}


//--------------------------------------------------------------------------------------------------------------
void Faction::ResolvePointersToEntities( std::map< EntityID, GameEntity* >& loadedEntities )
{
	//Handle that trick performed down in RestoreFromXMLNode. Note the value's already parsed in that function.
	for ( std::pair< EntityID, FactionRelationship* > relation : m_agentRelations )
	{
		std::map< EntityID, GameEntity* >::iterator relationTargetIter = loadedEntities.find( relation.first );
		GUARANTEE_OR_DIE( relationTargetIter != loadedEntities.end(), "Referenced relationTargetID not found in loaded entities!" );

		Agent* relationTarget = (Agent*)relationTargetIter->second;
		relation.first = relationTarget->GetEntityID();
		relation.second->m_towardsThisFactionName = relationTarget->GetFactionName();
		relation.second->m_factionID = relationTarget->GetFactionID();
	}
}


//--------------------------------------------------------------------------------------------------------------
void Faction::CloneAndOverwriteMyFactionFromXML( const XMLNode& myFactionNode )
{
	std::string factionString = ReadXMLAttribute( myFactionNode, "name", std::string() );
	Faction* faction = Faction::CreateOrGetFaction( factionString );
	*this = *faction; //Be wary of copy minutia with FactionRelationship pointers.
	PopulateFromXMLNode( myFactionNode ); //Since this is a copy, not modifying faction in registry.
}


//--------------------------------------------------------------------------------------------------------------
void Faction::RestoreFromXMLNode( const XMLNode& factionsNode )
{		
	//Copies raw values rather than loves/hates lingo.
	/* Example
		<Factions>
			<FactionRelations>
				<FactionRelation name="Goblin" value="#" />
			</FactionRelations>
			<EntityRelations>
				<EntityRelations entity="1" value="-22" /> //with player, see below
			</EntityRelations>
		</Factions>
	*/

	//Relationships with factions first.
	const XMLNode& factionRelationsNode = factionsNode.getChildNode( "FactionRelations" );
	std::string factionString;
	for ( int childIndex = 0; childIndex < factionRelationsNode.nChildNode(); childIndex++ )
	{
		const XMLNode& factionRelationNode = factionsNode.getChildNode( childIndex );
		factionString = ReadXMLAttribute( factionRelationNode, "faction", factionString );
		if ( factionString == "" )
			continue;
		Faction* faction = Faction::CreateOrGetFaction( factionString );
		FactionID factionID = faction->GetID();
		int relationValue = ReadXMLAttribute( factionRelationNode, "value", 0 );
		FactionRelationship* relation = new FactionRelationship( factionString, factionID, relationValue );
		m_factionRelations.insert( std::pair< FactionID, FactionRelationship* >( factionID, relation ) ); //Will kill duplicate keys.
	}

	//Relationships with individual entities afterward, as written out in WriteToXMLNode.
	const XMLNode& agentRelationsNode = factionsNode.getChildNode( "AgentRelations" );
	for ( int childIndex = 0; childIndex < agentRelationsNode.nChildNode(); childIndex++ )
	{
		const XMLNode& agentRelationNode = agentRelationsNode.getChildNode( childIndex );

		//Set to actual values in Faction::ResolveEntityPointers.
		EntityID unresolvedEntityID = ReadXMLAttribute( agentRelationNode, "entity", 0 );
		int relationshipValue = ReadXMLAttribute( agentRelationNode, "value", 0 );
		FactionRelationship* relation = new FactionRelationship( "", -1, relationshipValue );

		m_agentRelations.insert( std::pair< EntityID, FactionRelationship* >( unresolvedEntityID, relation ) );
	}
}

//--------------------------------------------------------------------------------------------------------------
void Faction::WriteToXMLNode( XMLNode& out_agentNode )
{
	if ( m_name == "" )
		return;

	WriteXMLAttribute( out_agentNode, "faction", m_name, std::string() );

	XMLNode factionsNode = out_agentNode.addChild( "Factions" );

	//First, relationships with Factions.
	XMLNode factionRelationsNode = factionsNode.addChild( "FactionRelations" );
	for ( std::pair< FactionID, FactionRelationship* > relation : m_factionRelations )
	{
		XMLNode relationNode = factionRelationsNode.addChild( "FactionRelation" );
		WriteXMLAttribute( relationNode, "faction", relation.second->m_towardsThisFactionName, std::string() );
		WriteXMLAttribute( relationNode, "value", relation.second->m_relationshipValue, 0 );
	}

	//Second, relationships with Agents.
	XMLNode agentRelationsNode = factionsNode.addChild( "AgentRelations" );
	for ( std::pair< EntityID, FactionRelationship* > relation : m_agentRelations )
	{
		XMLNode relationNode = agentRelationsNode.addChild( "AgentRelation" );
		WriteXMLAttribute( relationNode, "entity", relation.first, GameEntity::s_INVALID_ID );
		WriteXMLAttribute( relationNode, "value", relation.second->m_relationshipValue, 0 );
	}
}


//--------------------------------------------------------------------------------------------------------------
int Faction::GetStatusValueForFaction( FactionID factionID ) const
{
	size_t count = m_factionRelations.count( factionID );

	if ( count == 0 ) //If newly encountered, neutral towards it.
		return FACTION_STATUS_NEUTRAL;

	ASSERT_OR_DIE( count == 1, "Unexpected Faction Count Found in Faction::GetCurrentStandingWithFaction()!" );
	return m_factionRelations.at( factionID )->m_relationshipValue;
}


//--------------------------------------------------------------------------------------------------------------
FactionRelationship* Faction::CreateOrGetRelationshipWithFaction( FactionID factionID )
{
	size_t count = m_factionRelations.count( factionID );

	if ( count == 0 ) //If newly encountered, start neutral.
		return AddNewlyMetFaction( factionID );

	ASSERT_OR_DIE( count == 1, "Unexpected Faction Count Found in Faction::GetCurrentStandingWithFaction()!" );
	return m_factionRelations.at( factionID );
}


//--------------------------------------------------------------------------------------------------------------
FactionRelationship* Faction::CreateOrGetRelationshipWithAgent( EntityID agentID )
{
	size_t count = m_agentRelations.count( agentID );


	if ( count == 0 ) //If newly encountered, start neutral.
		return AddNewlyMetAgent( agentID );

	ASSERT_OR_DIE( count == 1, "Unexpected Agent Count in Faction::GetCurrentStandingWithAgent()!" );
	return m_factionRelations.at( agentID );
}


//--------------------------------------------------------------------------------------------------------------
void Faction::AdjustFactionStatus( Agent* instigator, FactionAction action ) //It's being done by instigator to (*this).
{
	FactionID instigatorFactionID = instigator->GetFactionID();
	EntityID instigatorID = instigator->GetEntityID();

	if ( m_factionRelations.count( instigatorFactionID ) == 0 ) //This faction had no entry for this other faction before.
	{
		std::string newFactionName = instigator->GetFactionName();
		FactionRelationship* newRelation = new FactionRelationship( newFactionName, instigatorFactionID, FACTION_STATUS_NEUTRAL );
		m_factionRelations.insert( std::pair< FactionID, FactionRelationship* >( instigatorFactionID, newRelation ) );
	}
	int& instigatorFactionCurrentStanding = m_factionRelations.at( instigatorFactionID )->m_relationshipValue;

	//Update for instigator.
	auto found = m_agentRelations.find( instigatorID );
	if ( found == m_agentRelations.end() )
	{
		FactionRelationship* newRelation = new FactionRelationship( instigator->GetFactionName(), instigatorFactionID, instigatorFactionCurrentStanding + action );
		m_agentRelations.insert( std::pair< EntityID, FactionRelationship* >( instigatorID, newRelation ) );
	}
	else found->second->m_relationshipValue += action;

	//Update for instigator's faction.
	instigatorFactionCurrentStanding += static_cast<int>( CalcExtrapolationRatio( instigatorFactionID ) * action );
}


//--------------------------------------------------------------------------------------------------------------
FactionRelationship* Faction::AddNewlyMetFaction( FactionID factionID )
{
	std::string newFactionName = Faction::GetNameForFactionID( factionID );
	FactionRelationship* newRelation = new FactionRelationship( newFactionName, factionID, FACTION_STATUS_NEUTRAL );
	m_factionRelations.insert( std::pair< FactionID, FactionRelationship* >( factionID, newRelation ) );
	return newRelation;
}


//--------------------------------------------------------------------------------------------------------------
FactionRelationship* Faction::AddNewlyMetAgent( EntityID agentID )
{
	FactionRelationship* newRelation = new FactionRelationship( "", agentID, FACTION_STATUS_NEUTRAL );
	m_factionRelations.insert( std::pair< EntityID, FactionRelationship* >( agentID, newRelation ) );
	return newRelation;
}


//--------------------------------------------------------------------------------------------------------------
void Faction::LoadAllFactions()
{
	//Note we may have more than one Faction in a file.
	std::vector< std::string > m_factionFiles = EnumerateFilesInDirectory( "Data/XML/Factions", "*.Faction.xml" );

	for ( unsigned int factionFileIndex = 0; factionFileIndex < m_factionFiles.size(); factionFileIndex++ )
	{
		const char* xmlFilename = m_factionFiles[ factionFileIndex ].c_str();
		XMLNode factionsRoot = XMLNode::openFileHelper( xmlFilename, "Factions" );

		for ( int factionIndex = 0; factionIndex < factionsRoot.nChildNode(); factionIndex++ )
		{
			XMLNode factionNode = factionsRoot.getChildNode( factionIndex );

			std::string factionName;
			factionName = ReadXMLAttribute( factionNode, "name", factionName );

			Faction* newFaction = CreateOrGetFaction( factionName );
			newFaction->PopulateFromXMLNode( factionNode );
		}
	}

}


//--------------------------------------------------------------------------------------------------------------
Faction* Faction::CreateOrGetFaction( const std::string& factionName )
{
	//Not .find(), because it will compare both fields, and we only want uniqueness limited to the string name.
	for ( std::map< std::string, Faction* >::iterator factionIter = s_factionRegistry.begin(); factionIter != s_factionRegistry.end(); ++factionIter )
	{
		if ( strcmp( factionIter->first.c_str(), factionName.c_str() ) == 0 ) //Because XMLNode doesn't do null-termination, std::string does.
			return factionIter->second;
	}
	
	Faction* newFaction = new Faction( factionName );
	s_factionRegistry.insert( std::pair< std::string, Faction* >( factionName, newFaction ) );
	return newFaction;
}


//--------------------------------------------------------------------------------------------------------------
void Faction::PopulateFromXMLNode( const XMLNode& factionNode )
{
	//Generic copying of loves/hates preferences, and only values at these intervals.
	std::string attributeAsString;
	std::vector< std::string > factions;

	attributeAsString = ReadXMLAttribute( factionNode, "loves", attributeAsString );
	if ( attributeAsString != "" )
	{
		factions = SplitString( attributeAsString.c_str(), ',' );
		for ( const std::string& factionName : factions )
		{
			FactionID id = CreateOrGetFaction( factionName )->m_factionID;
			if ( m_factionRelations.count( id ) == 0 )
				m_factionRelations.insert( std::pair< FactionID, FactionRelationship* >( id, new FactionRelationship( factionName, id, FACTION_STATUS_LOVES ) ) );
			else
				m_factionRelations.at( id )->m_relationshipValue = FACTION_STATUS_LOVES;
		}
	}
	attributeAsString = "";

	attributeAsString = ReadXMLAttribute( factionNode, "likes", attributeAsString );
	if ( attributeAsString != "" )
	{
		factions = SplitString( attributeAsString.c_str(), ',' );
		for ( const std::string& factionName : factions )
		{
			FactionID id = CreateOrGetFaction( factionName )->m_factionID;
			if ( m_factionRelations.count( id ) == 0 )
				m_factionRelations.insert( std::pair< FactionID, FactionRelationship* >( id, new FactionRelationship( factionName, id, FACTION_STATUS_LIKES ) ) );
			else
				m_factionRelations.at( id )->m_relationshipValue = FACTION_STATUS_LIKES;
		}
	}
	attributeAsString = "";

	attributeAsString = ReadXMLAttribute( factionNode, "neutral", attributeAsString );
	if ( attributeAsString != "" )
	{
		factions = SplitString( attributeAsString.c_str(), ',' );
		for ( const std::string& factionName : factions )
		{
			FactionID id = CreateOrGetFaction( factionName )->m_factionID;
			if ( m_factionRelations.count( id ) == 0 )
				m_factionRelations.insert( std::pair< FactionID, FactionRelationship* >( id, new FactionRelationship( factionName, id, FACTION_STATUS_NEUTRAL ) ) );
			else
				m_factionRelations.at( id )->m_relationshipValue = FACTION_STATUS_NEUTRAL;
		}
	}
	attributeAsString = "";

	attributeAsString = ReadXMLAttribute( factionNode, "dislikes", attributeAsString );
	if ( attributeAsString != "" )
	{
		factions = SplitString( attributeAsString.c_str(), ',' );
		for ( const std::string& factionName : factions )
		{
			FactionID id = CreateOrGetFaction( factionName )->m_factionID;
			if ( m_factionRelations.count( id ) == 0 )
				m_factionRelations.insert( std::pair< FactionID, FactionRelationship* >( id, new FactionRelationship( factionName, id, FACTION_STATUS_DISLIKES ) ) );
			else
				m_factionRelations.at( id )->m_relationshipValue = FACTION_STATUS_DISLIKES;
		}
	}
	attributeAsString = "";

	attributeAsString = ReadXMLAttribute( factionNode, "hates", attributeAsString );
	if ( attributeAsString != "" )
	{
		factions = SplitString( attributeAsString.c_str(), ',' );
		for ( const std::string& factionName : factions )
		{
			FactionID id = CreateOrGetFaction( factionName )->m_factionID;
			if ( m_factionRelations.count( id ) == 0 )
				m_factionRelations.insert( std::pair< FactionID, FactionRelationship* >( id, new FactionRelationship( factionName, id, FACTION_STATUS_HATES ) ) );
			else
				m_factionRelations.at( id )->m_relationshipValue = FACTION_STATUS_HATES;
		}
	}
	attributeAsString = "";
}
