#pragma once


#include "Game/GameEntity.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"
#include "Engine/Math/Interval.hpp"
#include <vector>


//-----------------------------------------------------------------------------
enum EquipmentSlot
{
	EQUIPMENT_SLOT_HEAD,
	EQUIPMENT_SLOT_CHEST,
	EQUIPMENT_SLOT_LEGS,
	//	EQUIPMENT_SLOT_FINGER, or EQUIPMENT_SLOT_PRIMARY/SECONDARY_RING?
	EQUIPMENT_SLOT_PRIMARY_HAND,
	EQUIPMENT_SLOT_SECONDARY_HAND,
	NUM_EQUIPMENT_SLOTS
};
EquipmentSlot GetEquipmentSlotForString( const std::string& name );
std::string GetStringForEquipmentSlot( EquipmentSlot slot );


//-----------------------------------------------------------------------------
enum ItemType
{
	ITEM_TYPE_WEAPON,
	ITEM_TYPE_ARMOR,
	ITEM_TYPE_POTION,
	/*
	ITEM_TYPE_KEY,
	ITEM_TYPE_BOOK,
	ITEM_TYPE_SCROLL,
	ITEM_TYPE_MONEY,
	ITEM_TYPE_FOOD,
	ITEM_TYPE_CONTAINER,
	ITEM_TYPE_LIGHT,
	*/
	NUM_ITEM_TYPES
};
ItemType GetItemTypeForString( const std::string& name );
std::string GetStringForItemType( ItemType itemType );


//-----------------------------------------------------------------------------
class Item : public GameEntity
{
public:
	Item( const XMLNode& itemBlueprintNode, Map* map ) 
		: GameEntity( ENTITY_TYPE_ITEM ) 
		, m_weaponDamage( 0 )
		, m_armorDefense( 0 )
		, m_itemType( NUM_ITEM_TYPES )
		, m_isHidden( false )
	{
		m_color = Rgba::GREEN;
		PopulateFromXMLNode( itemBlueprintNode, map ); 
	}
	Item( const Item& other, Map* map = nullptr, const XMLNode& instanceDataNode = XMLNode::emptyNode() );
	virtual ~Item() override;

	inline bool operator<( const Item& other );
	bool IsEquippable() const { return m_validSlots.size() > 0; }
	const std::vector< EquipmentSlot >& GetValidSlots() const { return m_validSlots; }

	virtual CooldownSeconds Update( float deltaSeconds ) override;
	virtual bool Render() override;
	void Hide() { m_isHidden = true; }
	void Show() { m_isHidden = false; }

	virtual void WriteToXMLNode( XMLNode& out_entityDataNode ) override;
	int GetWeaponDamage() const { return m_weaponDamage; }
	int GetArmorDefense() const { return m_armorDefense; }
	void SetWeaponDamage( int newVal ) { m_weaponDamage = newVal; }
	void SetArmorDefense( int newVal ) { m_armorDefense = newVal; }

	void SetOccupiedCellsItemTo( Item* item );
	virtual bool AttachToMapAtPosition( Map* map, const MapPosition& position ) override;
	ItemType GetItemType() const { return m_itemType; }
	void SetItemType( ItemType type ) { m_itemType = type; }

	
private:
	virtual void PopulateFromXMLNode( const XMLNode& itemBlueprintNode, Map* map ) override;
	virtual void ResolvePointersToEntities( std::map< EntityID, GameEntity* >& loadedEntities ) override;

	ItemType m_itemType;
	std::vector< EquipmentSlot > m_validSlots; //e.g. none for potions.

	//Potentially monolithic list of all properties an item may have as XML attributes.
	int m_weaponDamage;
	int m_armorDefense;

	bool m_isHidden; //To override by the * of a cell if multiple items are on it.
};
