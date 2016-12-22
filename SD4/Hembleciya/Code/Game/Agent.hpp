#pragma once


#include "Game/GameEntity.hpp"
#include "Game/GameCommon.hpp"
#include "Game/FactionSystem.hpp"
#include "Game/Items/Item.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"


//-----------------------------------------------------------------------------
class Path;
class Inventory;


//-----------------------------------------------------------------------------
class Agent : public GameEntity
{
public:
	Agent( EntityType entityType );
	Agent( const Agent& other );

	virtual ~Agent() override;
	
	virtual bool AttachToMapAtPosition( Map* map, const MapPosition& position ) override;
	virtual void ResolvePointersToEntities( std::map< EntityID, GameEntity* >& loadedEntities ) override;

	bool TestOneStep( const MapPosition& goalPosition ); //Will not actually move you, it's an IsLegal. True/false is whether it was.
	bool MoveOneStep( const Vector2i& positionDelta ); //Actual movement. True/false is whether it was.
	void UpdateFieldOfView();

	virtual bool Render() override;
	virtual bool IsReadyToUpdate() const { return true; }
	virtual CooldownSeconds Update( float deltaSeconds ) override;
	
	const ProximityOrderedAgentMap& GetVisibleAgents() const { return m_visibleAgents; }
	Agent* GetTargetEnemy() const { return m_targetEnemy; }
	void SetTargetEnemy( Agent* newTarget ) { m_targetEnemy = newTarget; }
	void SetOccupiedCellsAgentTo( Agent* agent );

	Path* GetPath() const { return m_currentPath; }
	void SetPath( Path* newPath );

	bool DoesNotHarmFaction( FactionID faction );
	void AdjustFactionStatus( Agent* instigator, FactionAction action );
	FactionID GetFactionID() const;
	std::string GetFactionName() const { return m_faction.GetName(); }
	
	int GetNumKills() const { return m_numMonstersKilled; }
	void AddKill() { ++m_numMonstersKilled; }

	void AddDamageBonus( int damageDelta ) { m_damageBonus += damageDelta; }
	int GetDamageBonus() const { return m_damageBonus; }

	virtual void WriteToXMLNode( XMLNode& out_agentNode ) override; 
	void WriteEquipmentToXMLNode( XMLNode &out_agentNode );

	virtual void PopulateFromXMLNode( const XMLNode& instanceDataNode, Map* map ) override;
	void RemoveRelationsWithEntity( const Agent* agentToRemove ); //Usually when they die.

	Item* GetBestOfEquippedItemType( ItemType type ) const;
	std::vector< Item* > GetEquipmentAndItems() const;
	void PickUpItemAndAutoEquip();
	bool UseLastAcquiredPotion();
	bool DropFirstUnequippedItem( bool showMessages = true );
	bool DropAllItems();
	void Die(); //Drops all items.

	void ToggleAdjacentFeatures();
	bitfield_int GetTraversalProperties() const { return m_traversalProperties; }

protected:
	void Agent::PrintStatusOnFaction( FactionID factionID );
	void PopulateFactionsFromXMLNode( const XMLNode& agentNode );
	bool AutoEquipItem( Item* grabbedItem );
	Faction m_faction; //Doesn't need to be a pointer, if you have reason to do factionless agent could here via nullptr though.

	static const bitfield_int s_DEFAULT_TRAVERSAL_PROPERTIES = BLOCKED_BY_AGENTS | BLOCKED_BY_SOLIDS | SLOWED_BY_WATER | SLOWED_BY_LAVA;
	bitfield_int m_traversalProperties;
	Inventory* m_unequippedItems;
	Item* m_equippedItems[ NUM_EQUIPMENT_SLOTS ];
	int m_numMonstersKilled;
	int m_damageBonus; //Added to every attack.
	int m_viewRadius;
	Path* m_currentPath;
	ProximityOrderedAgentMap m_visibleAgents;
	ProximityOrderedItemMap m_visibleItems;
	ProximityOrderedFeatureMap m_visibleFeatures;
	Agent* m_targetEnemy; //"Auto-targeting" to allow 1+ behaviors to act upon this.

	static const int s_DEFAULT_DAMAGE_BONUS = 1;
};
