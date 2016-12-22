#pragma once


#include <map>
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------
struct XMLNode;
class Agent;
class GameEntity;


//-----------------------------------------------------------------------------
enum FactionAction
{
	FACTION_ACTION_ATTACKS = -5,
	FACTION_ACTION_ATTEMPTS_ATTACK = -3, //MEANS "ATTACKED AND MISSED".
	FACTION_ACTION_KILLS = -10,
	FACTION_ACTION_HEALS = 5 //HAS TO BE DAMAGED FIRST.
};
/* PROF. HARWARD'S VALUES: 
	-12 IF YOU KILL ALLY, 
	DAMAGE ME -7, 
	TRY DAMAGE ME -3, 
	DAMAGE ALLY -2, 
	HEAL ALLY +3, 
	HEAL ME +5, 
	SAVE ALLY CLOSE TO DEATH +11, 
	SAVED ME FROM DYING +20.
*/


//-----------------------------------------------------------------------------
enum FactionStatus
{
	FACTION_STATUS_HATES = -20,
	FACTION_STATUS_DISLIKES = -10, //Etc. #'s only matter relative to overall scale.
	FACTION_STATUS_NEUTRAL = 0, //Spans -10 to +10 inclusive.
	FACTION_STATUS_LIKES = 10,
	FACTION_STATUS_LOVES = 20
}; //Can now say DoesLike() { return ( m_factionStatus >= FACTION_STATUS_LIKES ); }


//-----------------------------------------------------------------------------
struct FactionRelationship
{
	FactionRelationship( const std::string& towardThisFactionName, int factionID, int relationshipValue )
		: m_towardsThisFactionName( towardThisFactionName )
		, m_factionID( factionID )
		, m_relationshipValue( relationshipValue )
	{
	}

	int m_relationshipValue; //Confer FactionStatus for intervals.
	FactionID m_factionID;
	std::string m_towardsThisFactionName; //Redundant for self-reference purposes, e.g. printing table of all these.
};


//-----------------------------------------------------------------------------
class Faction
{
public:
	Faction() {}
	Faction( const std::string& name ) //Need to match the factionID of the global faction, despite being another instance.
		: m_name( name )
	{
		m_factionID = ( s_factionRegistry.count( name ) != 0 ) ? s_factionRegistry.at( name )->m_factionID : s_BASE_FACTION_ID++;
	}
	Faction( const Faction& other ) //Being wary about pointers to Relationships being copied shallowly...
		: m_name( other.m_name )
		, m_factionID( other.m_factionID )
	{
		for ( auto& relationPair : other.m_factionRelations )
			m_factionRelations.insert( std::pair< FactionID, FactionRelationship* >( relationPair.first, new FactionRelationship( *relationPair.second ) ) );
	}
	inline void operator=( const Faction& other )
	{
		m_name = other.m_name;
		m_factionID = other.m_factionID;
		for ( auto& relationPair : other.m_factionRelations )
			m_factionRelations.insert( std::pair< FactionID, FactionRelationship* >( relationPair.first, new FactionRelationship( *relationPair.second ) ) );
	}

	void PopulateFromXMLNode( const XMLNode& factionNode ); //From non-capture NPC files.
	void RestoreFromXMLNode( const XMLNode& factionsNode ); //From state captures.
	void CloneAndOverwriteMyFactionFromXML( const XMLNode& myFactionNode );

	~Faction()
	{
		for ( auto iter = m_factionRelations.begin(); iter != m_factionRelations.end(); ++iter )
			delete iter->second;
		for ( auto iter = m_agentRelations.begin(); iter != m_agentRelations.end(); ++iter )
			delete iter->second;
	}
	
	std::string GetName() const { return m_name; }
	FactionID GetID() const { return m_factionID; }
	int GetStatusValueForFaction( FactionID factionID ) const;
	FactionRelationship* CreateOrGetRelationshipWithFaction( FactionID factionID );
	FactionRelationship* CreateOrGetRelationshipWithAgent( EntityID entityID );
	FactionRelationship* AddNewlyMetFaction( FactionID factionID );
	FactionRelationship* Faction::AddNewlyMetAgent( EntityID agentID );
	void AdjustFactionStatus( Agent* instigator, FactionAction action );

	//-----------------------------------------------------------------------------
	static void LoadAllFactions();
	static Faction* CreateOrGetFaction( const std::string& name );
	void WriteToXMLNode( XMLNode& out_agentNode );
	void ResolvePointersToEntities( std::map< EntityID, GameEntity* >& loadedEntities );
	void RemoveRelationsWithEntity( const Agent* agentToRemove );


private:
	float CalcExtrapolationRatio( FactionID instigatorFaction );
	static std::string GetNameForFactionID( FactionID factionID );
	std::string m_name;
	FactionID m_factionID;

	//Containers kept by value, not pointer, so each entity develops their own world-views.
	std::map< FactionID, FactionRelationship* > m_factionRelations;
	std::map< EntityID, FactionRelationship* > m_agentRelations;

	//-----------------------------------------------------------------------------
	static std::map< std::string, Faction* > s_factionRegistry;
	static FactionID s_BASE_FACTION_ID;
	static float s_EXTRAPOLATION_RATIO;
};
