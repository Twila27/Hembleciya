#include "Game/Agent.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/FieldOfView/FieldOfView.hpp"
#include "Game/Pathfinding/Pathfinder.hpp"
#include "Engine/Core/TheConsole.hpp"
#include "Game/Inventory.hpp"
#include "Game/Cell.hpp"
#include "Game/Features/Feature.hpp"
#include "Game/CombatSystem.hpp"


//--------------------------------------------------------------------------------------------------------------
Agent::Agent( EntityType entityType )
	: GameEntity( entityType )
	, m_traversalProperties( s_DEFAULT_TRAVERSAL_PROPERTIES )
	, m_viewRadius( DEFAULT_VIEW_RADIUS )
	, m_currentPath( nullptr )
	, m_targetEnemy( nullptr )
	, m_damageBonus( s_DEFAULT_DAMAGE_BONUS )
	, m_numMonstersKilled( 0 )
	, m_unequippedItems( new Inventory() )
{
	for ( int slotIndex = 0; slotIndex < NUM_EQUIPMENT_SLOTS; slotIndex++ )
		m_equippedItems[ slotIndex ] = nullptr;
}


//--------------------------------------------------------------------------------------------------------------
Agent::Agent( const Agent& other )
	: GameEntity( other )
	, m_traversalProperties( other.m_traversalProperties )
	, m_viewRadius( other.m_viewRadius )
	, m_currentPath( other.m_currentPath )
	, m_targetEnemy( other.m_targetEnemy )
	, m_faction( other.m_faction )
	, m_damageBonus( other.m_damageBonus )
	, m_numMonstersKilled( other.m_numMonstersKilled )
	, m_unequippedItems( new Inventory( *other.m_unequippedItems ) )
{
	for ( int slotIndex = 0; slotIndex < NUM_EQUIPMENT_SLOTS; slotIndex++ )
		m_equippedItems[ slotIndex ] = other.m_equippedItems[ slotIndex ];
}


//--------------------------------------------------------------------------------------------------------------
Agent::~Agent()
{
	if ( m_currentPath != nullptr )
	{
		delete m_currentPath;
		m_currentPath = nullptr;
	}
	if ( m_unequippedItems != nullptr )
		delete m_unequippedItems;
}


//--------------------------------------------------------------------------------------------------------------
bool Agent::TestOneStep( const MapPosition& goalPosition )
{
	return m_map->DoesPositionSatisfyTraversalProperties( goalPosition, m_traversalProperties );
}


//--------------------------------------------------------------------------------------------------------------
bool Agent::MoveOneStep( const Vector2i& positionDelta )
{
	if ( positionDelta == Vector2i::ZERO )
		return false;

	//Clear cells where we are before moving.
	SetOccupiedCellsAgentTo( nullptr );

	//Actually move.
	m_positionBounds->AddOffset( positionDelta );

	//Occupy cells where we are after moving.
	SetOccupiedCellsAgentTo( this );

	return true;
}


//--------------------------------------------------------------------------------------------------------------
void Agent::UpdateFieldOfView()
{
	FieldOfView::CalculateFieldOfViewForAgent( this, m_viewRadius, m_map, true, m_visibleAgents, m_visibleItems, m_visibleFeatures );
}


//--------------------------------------------------------------------------------------------------------------
void Agent::SetOccupiedCellsAgentTo( Agent* agent )
{
	const MapPosition& positionMins = GetPositionMins();
	const MapPosition& positionMaxs = m_positionBounds->maxs;
	for ( int cellY = positionMins.y; cellY < positionMaxs.y; cellY++ )
		for ( int cellX = positionMins.x; cellX < positionMaxs.x; cellX++ )
			m_map->GetCellForPosition( MapPosition( cellX, cellY ) ).m_occupyingAgent = agent;
}


//--------------------------------------------------------------------------------------------------------------
bool Agent::Render()
{
	bool didRender = false;

	if ( g_inDebugMode && m_currentPath != nullptr )
		didRender = m_currentPath->Render();

	didRender = GameEntity::Render(); //Render glyph on top of pathfinding, if it's active.

	return didRender;
}


//--------------------------------------------------------------------------------------------------------------
float Agent::Update( float deltaSeconds )
{
	UNREFERENCED( deltaSeconds );
	static bool hasBeenBurned = false;

	//Apply damage if standing in lava during turn--call this function at end of update to be most lenient.
	if ( m_map->GetCellForPosition( GetPositionMins() ).m_cellType == CELL_TYPE_LAVA )
	{
		if ( !IsInvincible() )
		{
			SubtractHealthDelta( LAVA_DAMAGE_PER_TURN );
			if ( IsPlayer() )
				g_theConsole->Printf( "You take %d damage for walking on lava.%s",
									  LAVA_DAMAGE_PER_TURN,
									  hasBeenBurned ? "" : " Pinching yourself wouldn't work either, then." );

			CombatSystem::PlayHurtSound( this );
			hasBeenBurned = true;
		}
	}

	return DEFAULT_TURN_COOLDOWN;
}


//--------------------------------------------------------------------------------------------------------------
void Agent::SetPath( Path* newPath )
{
	if ( m_currentPath != nullptr )
		delete m_currentPath;

	m_currentPath = newPath;
}


//--------------------------------------------------------------------------------------------------------------
bool Agent::DoesNotHarmFaction( FactionID faction )
{
	return m_faction.GetStatusValueForFaction( faction ) > 0;
}


//--------------------------------------------------------------------------------------------------------------
void Agent::AdjustFactionStatus( Agent* instigator, FactionAction action )
{
	m_faction.AdjustFactionStatus( instigator, action );
	//PrintStatusOnFaction( instigator->GetFactionID() );
}


//--------------------------------------------------------------------------------------------------------------
FactionID Agent::GetFactionID() const
{
	return m_faction.GetID();
}


//--------------------------------------------------------------------------------------------------------------
bool Agent::AttachToMapAtPosition( Map* map, const MapPosition& position )
{
	bool isValid = GameEntity::AttachToMapAtPosition( map, position );
		
	if ( isValid )
		SetOccupiedCellsAgentTo( this );

	return isValid;
}


//--------------------------------------------------------------------------------------------------------------
void Agent::ResolvePointersToEntities( std::map< EntityID, GameEntity* >& loadedEntities )
{
	//Handle that trick performed down in PopulateFromXMLNode.
	if ( m_targetEnemy != nullptr )
	{
		std::map< EntityID, GameEntity* >::iterator targetEnemyIter = loadedEntities.find( (EntityID)m_targetEnemy );
		GUARANTEE_OR_DIE( targetEnemyIter != loadedEntities.end(), "Referenced targetEnemyID not found in loaded entities!" );
		m_targetEnemy = (Agent*)targetEnemyIter->second;
	}

	m_faction.ResolvePointersToEntities( loadedEntities );
}


//--------------------------------------------------------------------------------------------------------------
void Agent::PrintStatusOnFaction( FactionID factionID )
{
	if ( !IsCurrentlySeen() && !IsPlayer() )
		return;

	FactionRelationship* relation = m_faction.CreateOrGetRelationshipWithFaction( factionID );
	g_theConsole->Printf( "%s now %d toward %s-faction agents.", 
						  m_name.c_str(), 
						  relation->m_relationshipValue, 
						  relation->m_towardsThisFactionName.c_str() );
	g_theConsole->ShowConsole();
}


//--------------------------------------------------------------------------------------------------------------
void Agent::WriteToXMLNode( XMLNode& out_agentNode )
{
	GameEntity::WriteToXMLNode( out_agentNode );

	if ( m_targetEnemy != nullptr )
		WriteXMLAttribute( out_agentNode, "targetEnemyID", m_targetEnemy->GetEntityID(), GameEntity::s_INVALID_ID );
	WriteXMLAttribute( out_agentNode, "numMonstersKilled", m_numMonstersKilled, 0 );
	WriteXMLAttribute( out_agentNode, "damageBonus", m_damageBonus, s_DEFAULT_DAMAGE_BONUS );
	WriteXMLAttribute( out_agentNode, "viewRadius", m_viewRadius, DEFAULT_VIEW_RADIUS );
	WriteXMLAttribute( out_agentNode, "traversableProperties", m_traversalProperties, s_DEFAULT_TRAVERSAL_PROPERTIES );

	m_faction.WriteToXMLNode( out_agentNode );	

	WriteEquipmentToXMLNode( out_agentNode );

	XMLNode inventoryNode = out_agentNode.addChild( "Inventory" );
	m_unequippedItems->WriteToXMLNode( inventoryNode );
}


//--------------------------------------------------------------------------------------------------------------
void Agent::WriteEquipmentToXMLNode( XMLNode &out_agentNode )
{
	XMLNode equipmentNode = out_agentNode.addChild( "Equipment" );
	for ( unsigned int slotIndex = 0; slotIndex < NUM_EQUIPMENT_SLOTS; slotIndex++ )
	{
		if ( m_equippedItems[ slotIndex ] == nullptr )
			continue;

		std::string equipmentSlotAsString = GetStringForEquipmentSlot( (EquipmentSlot)slotIndex );
		XMLNode equipmentItemNode = equipmentNode.addChild( equipmentSlotAsString.c_str() );
		WriteXMLAttribute( equipmentItemNode, "item", m_equippedItems[ slotIndex ]->GetName(), std::string() );
	}
}


//--------------------------------------------------------------------------------------------------------------
void Agent::PopulateFromXMLNode( const XMLNode& instanceDataNode, Map* map )
{
	GameEntity::PopulateFromXMLNode( instanceDataNode, map );
	if ( m_map != nullptr )
		SetOccupiedCellsAgentTo( this );

	m_targetEnemy = (Agent*)ReadXMLAttribute( instanceDataNode, "targetEnemyID", 0 ); //TRICK, fix in ResolveEntityPointer.
	m_numMonstersKilled = ReadXMLAttribute( instanceDataNode, "numMonstersKilled", m_numMonstersKilled );
	m_damageBonus = ReadXMLAttribute( instanceDataNode, "damageBonus", m_damageBonus );
	TODO( "Turn duration and turn order, see A4 spec!" );
	m_viewRadius = ReadXMLAttribute( instanceDataNode, "viewRadius", m_viewRadius );
	m_traversalProperties = ReadXMLAttribute( instanceDataNode, "traversalProperties", m_traversalProperties );

	PopulateFactionsFromXMLNode( instanceDataNode );

	XMLNode inventoryNode = instanceDataNode.getChildNode( "Inventory" );
	m_unequippedItems->PopulateFromXMLNode( inventoryNode );
}


//-------------------------------------------------------------------------------------
void Agent::PopulateFactionsFromXMLNode( const XMLNode& agentNode )
{
	//First check for faction as attribute. This is a straight clone of global, with NO overrides.
	std::string factionString;
	factionString = ReadXMLAttribute( agentNode, "faction", factionString );
	bool hasFactionAttribute = ( factionString != "" );
	if ( hasFactionAttribute )
	{
		Faction* faction = Faction::CreateOrGetFaction( factionString );
		m_faction = *faction; //Be wary of copy minutia with FactionRelationship pointers.
	}

	//Second check for faction as child node (not attribute, as in first check) of NPCBlueprint, 
	//and if it already exists, then in its LOCAL copy override the global faction values.
	const XMLNode& singleFactionNode = agentNode.getChildNode( "Faction" );
	bool hasSingleFactionElement = !singleFactionNode.isEmpty();
	if ( hasSingleFactionElement )
		m_faction.CloneAndOverwriteMyFactionFromXML( singleFactionNode );

	//Finally, check for the multi-faction variation parented under a Factions node e.g. by game state capturing.
	const XMLNode& rawFactionsNode = agentNode.getChildNode( "Factions" );
	bool hasMultipleFactionsElement = !rawFactionsNode.isEmpty();
	if ( !hasMultipleFactionsElement )
		m_faction.RestoreFromXMLNode( rawFactionsNode );

	GUARANTEE_RECOVERABLE( hasFactionAttribute || hasSingleFactionElement || hasMultipleFactionsElement,
						   Stringf( "%s has no faction attribute, <Faction> element, or <Factions> element detected!", m_name.c_str() ) );
}


//--------------------------------------------------------------------------------------------------------------
void Agent::RemoveRelationsWithEntity( const Agent* agentToRemove )
{
	m_faction.RemoveRelationsWithEntity( agentToRemove );
}


//--------------------------------------------------------------------------------------------------------------
Item* Agent::GetBestOfEquippedItemType( ItemType type ) const
{
	Item* currentBest = nullptr;
	for ( unsigned int slotIndex = 0; slotIndex < NUM_EQUIPMENT_SLOTS; slotIndex++ )
	{
		Item* candidate = m_equippedItems[ slotIndex ];

		if ( ( candidate == nullptr ) || ( candidate->GetItemType() != type ) )
			continue;

		if ( currentBest == nullptr || currentBest < candidate  )
			currentBest = candidate;
	}
	return currentBest;
}


//--------------------------------------------------------------------------------------------------------------
std::vector< Item* > Agent::GetEquipmentAndItems() const
{
	std::vector< Item* > m_items;
	m_items.insert( m_items.end(), m_unequippedItems->GetItems().begin(), m_unequippedItems->GetItems().end() );
	for ( unsigned int slotIndex = 0; slotIndex < NUM_EQUIPMENT_SLOTS; slotIndex++ )
		if ( m_equippedItems[ slotIndex ] != nullptr )
			m_items.push_back( m_equippedItems[ slotIndex ] );

	return m_items;
}


//--------------------------------------------------------------------------------------------------------------
bool Agent::AutoEquipItem( Item* grabbedItem )
{
	bool didEquip = false;

	//Note no-slot items like potions go straight into inventory.
	if ( !grabbedItem->IsEquippable() )
	{
		m_unequippedItems->PushItem( grabbedItem );
		didEquip = false;
	}
	else
	{
		//First, get the slot of the item.
		const std::vector< EquipmentSlot >& validSlots = grabbedItem->GetValidSlots();

		//Second, check all of this agent's slots for an empty one, add to first one found.
		for ( EquipmentSlot validSlot : validSlots ) //Means the first slot listed in its XML has highest priority.
		{
			if ( m_equippedItems[ validSlot ] == nullptr )
			{
				m_equippedItems[ validSlot ] = grabbedItem;

				if ( IsPlayer() )
					g_theConsole->Printf( "You pick up and equip the %s to your %s.",
										grabbedItem->GetName().c_str(),
										GetStringForEquipmentSlot( validSlot ).c_str() );
				else if ( IsCurrentlySeen() )
					g_theConsole->Printf( "You see %s pick up and equip the %s to its %s.",
										this->GetName().c_str(),
										grabbedItem->GetName().c_str(),
										GetStringForEquipmentSlot( validSlot ).c_str() );

				didEquip = true;
				break;
			}
		}

		if ( !didEquip )
		{
			//Third, check against those slots again, separately, but to override any weaker items. 
			for ( EquipmentSlot validSlot : validSlots ) //Means we prefer empty slots first.
			{
				if ( m_equippedItems[ validSlot ] < grabbedItem ) //Note operator overload, add future logic there.
				{
					Item* itemBeingUnequipped = m_equippedItems[ validSlot ];
					m_unequippedItems->PushItem( itemBeingUnequipped );
					m_equippedItems[ validSlot ] = grabbedItem;

					if ( IsPlayer() )
						g_theConsole->Printf( "You pick up and equip the %s to your %s, storing away your %s.",
											  grabbedItem->GetName().c_str(),
											  GetStringForEquipmentSlot( validSlot ).c_str(),
											  itemBeingUnequipped->GetName().c_str() );
					else if ( IsCurrentlySeen() )
						g_theConsole->Printf( "You see %s pick up and equip the %s to its %s, storing away its %s.",
											  this->GetName().c_str(),
											  grabbedItem->GetName().c_str(),
											  GetStringForEquipmentSlot( validSlot ).c_str() );

					didEquip = true;
					break;
				}
			}
		}
	}

	return didEquip;
}


//--------------------------------------------------------------------------------------------------------------
void Agent::PickUpItemAndAutoEquip()
{
	Cell& cell = m_map->GetCellForPosition( GetPositionMins() );
	if ( cell.HasItems() )
	{
		Item* grabbedItem = cell.TakeItemFromCell();

		bool didEquip = AutoEquipItem( grabbedItem );

		if ( !didEquip )
		{
			if ( IsPlayer() )
				g_theConsole->Printf( "You pick up the %s and store it away.", grabbedItem->GetName().c_str() );
			else if ( IsCurrentlySeen() )
				g_theConsole->Printf( "You see %s pick up the %s and store it away.", this->GetName().c_str(), grabbedItem->GetName().c_str() );
		}

		g_theConsole->ShowConsole();
	}
	else if ( IsPlayer() )
	{
		g_theConsole->Printf( "Nothing to pick up." );
		g_theConsole->ShowConsole();
	}
}


//--------------------------------------------------------------------------------------------------------------
bool Agent::UseLastAcquiredPotion()
{

	bool hadPotion = false;

	std::vector< Item* > unequippedItems = m_unequippedItems->GetItems();
	if ( unequippedItems.size() == 0 )
		return hadPotion;

	for ( unsigned int itemIndex = unequippedItems.size() - 1; itemIndex > 0; itemIndex-- )
	{
		Item* item = unequippedItems[ itemIndex ];

		if ( item->GetItemType() != ITEM_TYPE_POTION )
			continue;

		hadPotion = true;

		if ( item->GetName() == "Potion of a Signal Dream" )
		{
			int delta = GetRandomIntInRange( 3, 5 );
			g_theConsole->Printf( "The signal dream gave you %d points of increased perception!", delta );
			m_viewRadius += delta;
			UpdateFieldOfView();
		}
		else if ( item->GetName() == "Potion of a Wellness Dream" )
		{
			int delta = GetRandomIntInRange( 5, 10 );
			g_theConsole->Printf( "The wellness dream healed you %d points!", delta );
			AddHealthDelta( delta );
		}
		else if ( item->GetName() == "Potion of a Nightmare" )
		{
			g_theConsole->Printf( "The nightmare granted you its power for +1 damage on all attacks!" );
			m_color = m_color + ( Rgba::RED * Rgba::GRAY );
			m_color.blue -= m_color.blue / 3;
			m_color.green -= m_color.green / 3;
			++m_damageBonus;
		}

		m_unequippedItems->EraseItem( item );

		break;
	}

	g_theConsole->ShowConsole();

	return hadPotion;
}


//--------------------------------------------------------------------------------------------------------------
bool Agent::DropFirstUnequippedItem( bool showMessages /*= true*/ )
{
	Cell& cell = m_map->GetCellForPosition( GetPositionMins() );
	
	Item* droppedItem = m_unequippedItems->PopItem( m_map );
	bool didDrop = ( droppedItem != nullptr );

	if ( didDrop )
	{
		if ( cell.m_cellType == CELL_TYPE_LAVA )
		{
			if ( IsPlayer() )
			{
				g_theConsole->Printf( "As you drop the %s, it burns away in the lava.", droppedItem->GetName().c_str() );
				g_theConsole->ShowConsole();
			}
		}
		else
		{
			droppedItem->SetPositionMins( GetPositionMins() );
			cell.PushItem( droppedItem );

			if ( showMessages )
			{
				if ( IsPlayer() )
				{
					g_theConsole->Printf( "You drop the %s.", droppedItem->GetName().c_str() );

					g_theConsole->ShowConsole();
				}
				else if ( IsCurrentlySeen() )
				{
					g_theConsole->Printf( "You see %s drop the %s.", this->GetName().c_str(), droppedItem->GetName().c_str() );

					g_theConsole->ShowConsole();
				}
			}
		}
	}
	else if ( IsPlayer() )
	{
		g_theConsole->Printf( "Nothing in unequipped items to drop." );
		g_theConsole->ShowConsole();
	}

	return didDrop;
}


//--------------------------------------------------------------------------------------------------------------
bool Agent::DropAllItems() //e.g. on death.
{
	bool didDropAnything = false;

	while ( m_unequippedItems->GetCount() > 0 )
		if ( DropFirstUnequippedItem( false ) )
			didDropAnything = true;

	return didDropAnything;
}


//--------------------------------------------------------------------------------------------------------------
void Agent::Die()
{
	DropAllItems();

	SetOccupiedCellsAgentTo( nullptr );
}


//--------------------------------------------------------------------------------------------------------------
void Agent::ToggleAdjacentFeatures()
{
	std::vector< Cell* > neighbors;
	m_map->GetAdjacentNeighborCells( GetPositionMins(), 1.f, false, neighbors );

	for ( Cell* cell : neighbors )
	{
		if ( !cell->IsOccupiedByFeature() )
			continue;

		cell->m_occupyingFeature->ToggleState();
	}
}
