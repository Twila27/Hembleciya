#include "Game/Items/Item.hpp"
#include "Game/Behaviors/Behavior.hpp"
#include "Game/FieldOfView/FieldOfView.hpp"
#include "Game/Map.hpp"
#include "Engine/Renderer/TheRenderer.hpp"


//--------------------------------------------------------------------------------------------------------------
Item::Item( const Item& other, Map* map /*= nullptr*/, const XMLNode& instanceDataNode /*= XMLNode::emptyNode()*/ )
	: GameEntity( other )
	, m_itemType( other.m_itemType )
	, m_weaponDamage( other.m_weaponDamage )
	, m_armorDefense( other.m_armorDefense )
	, m_validSlots( other.m_validSlots )
	, m_isHidden( other.m_isHidden )
{
	if ( !instanceDataNode.isEmpty() ) //XML library's IsContentEmpty() would be true for <Wander />, so I use IsEmpty().
		PopulateFromXMLNode( instanceDataNode, map );
}


//--------------------------------------------------------------------------------------------------------------
Item::~Item()
{
}


//--------------------------------------------------------------------------------------------------------------
inline bool Item::operator<( const Item& other )
{
	if ( m_weaponDamage < other.m_weaponDamage || m_armorDefense < other.m_armorDefense )
		return true;
}


//--------------------------------------------------------------------------------------------------------------
float g_alphaCounter = 0.f;
CooldownSeconds Item::Update( float deltaSeconds )
{
	static bool isAlphaBackwards = false;

	if ( isAlphaBackwards )
		g_alphaCounter -= deltaSeconds;
	else
		g_alphaCounter += deltaSeconds;

	if ( g_alphaCounter > 1.f )
		isAlphaBackwards = true;
	if ( g_alphaCounter < 0.f )
		isAlphaBackwards = false;

	return DEFAULT_TURN_COOLDOWN;
}


//--------------------------------------------------------------------------------------------------------------
void Item::PopulateFromXMLNode( const XMLNode& itemBlueprintNode, Map* map )
{
	GameEntity::PopulateFromXMLNode( itemBlueprintNode, map );

	if ( m_map != nullptr ) //Then we need to occupy a cell's inventory.
	{
		Cell& cell = m_map->GetCellForPosition( GetPositionMins() );
		cell.PushItem( this );
	}


	std::string fullSlotsString; 
	fullSlotsString = ReadXMLAttribute( itemBlueprintNode, "slot", fullSlotsString );
	if ( fullSlotsString != "" )
	{
		//Overwrite the template's string.
		m_validSlots.clear();

		std::vector< std::string > slotsAsStrings = SplitString( fullSlotsString.c_str(), ',', true, false );
		for ( std::string& slotString : slotsAsStrings )
			m_validSlots.push_back( GetEquipmentSlotForString( slotString ) );
	}

	//Weapon damage is set by the factory from its interval object.
	m_armorDefense = ReadXMLAttribute( itemBlueprintNode, "defense", m_armorDefense );
}


//--------------------------------------------------------------------------------------------------------------
bool Item::AttachToMapAtPosition( Map* map, const MapPosition& position )
{
	bool isValid = GameEntity::AttachToMapAtPosition( map, position );

	if ( isValid )
		SetOccupiedCellsItemTo( this );

	return isValid;
}


//--------------------------------------------------------------------------------------------------------------
void Item::ResolvePointersToEntities( std::map< EntityID, GameEntity* >& loadedEntities )
{
	UNREFERENCED( loadedEntities );
}


//--------------------------------------------------------------------------------------------------------------
bool Item::Render()
{
	Item::Update( .001f ); //Alpha update.

	bool didRender = false;

	if ( m_isHidden || m_map == nullptr )
		return didRender;

	//If we're not showing the full map, haven't been seen yet, don't render.
	if ( !g_showFullMap && !m_hasBeenSeenBefore )
		return didRender;

	Rgba entityColor = m_color;
	entityColor.alphaOpacity = static_cast<byte_t>( RangeMap( g_alphaCounter, 0.f, 1.f, 0.f, 255.f ) );
	if ( m_hasBeenSeenBefore && !m_isCurrentlySeen )
	{
		entityColor = entityColor * Rgba::GRAY;
		entityColor.alphaOpacity = 64;
	}

	std::string drawRequiredGlyphAsString;
	drawRequiredGlyphAsString = m_glyph;
	g_theRenderer->DrawTextProportional2D(
		GetScreenPositionForMapPosition( m_positionBounds->mins ),
		drawRequiredGlyphAsString,
		CELL_FONT_SCALE,
		CELL_FONT_OBJECT,
		entityColor
	);

	didRender = true;
	return didRender;

}


//--------------------------------------------------------------------------------------------------------------
void Item::WriteToXMLNode( XMLNode& out_entityDataNode )
{
	if ( m_map == nullptr )
		return; //Do not write out unless attached to a map.

	XMLNode itemNode = out_entityDataNode.addChild( "ItemBlueprint" );

	GameEntity::WriteToXMLNode( itemNode );

	WriteXMLAttribute( itemNode, "type", GetStringForItemType( m_itemType ), std::string() );

	std::string slotString;
	unsigned int numSlots = m_validSlots.size();
	for ( unsigned int slotIndex = 0; slotIndex < numSlots; slotIndex++ )
	{
		slotString += GetStringForEquipmentSlot( m_validSlots[ slotIndex ] );

		if ( slotIndex < numSlots - 1 )
			slotString += ',';
	}
	WriteXMLAttribute( itemNode, "slot", slotString, std::string() );

	WriteXMLAttribute( itemNode, "damage", m_weaponDamage, 0 );
	WriteXMLAttribute( itemNode, "defense", m_armorDefense, 0 );
}


//--------------------------------------------------------------------------------------------------------------
void Item::SetOccupiedCellsItemTo( Item* item )
{
	const MapPosition& positionMins = GetPositionMins();
	const MapPosition& positionMaxs = m_positionBounds->maxs;
	for ( int cellY = positionMins.y; cellY < positionMaxs.y; cellY++ )
		for ( int cellX = positionMins.x; cellX < positionMaxs.x; cellX++ )
		{
			Cell& cell = m_map->GetCellForPosition( MapPosition( cellX, cellY ) );
			if ( item == nullptr )
				cell.EraseItem( this );
			else
				cell.PushItem( item );
		}
}


//--------------------------------------------------------------------------------------------------------------
ItemType GetItemTypeForString( const std::string& name )
{
	const std::string& nameLower = GetAsLowercase( name );
	if ( nameLower == "weapon" )
		return ITEM_TYPE_WEAPON;
	if ( nameLower == "armor" )
		return ITEM_TYPE_ARMOR;
	if ( nameLower == "potion" )
		return ITEM_TYPE_POTION;

	ERROR_AND_DIE( "GetItemTypeForString: Found no match for argument!" );
}


//--------------------------------------------------------------------------------------------------------------
EquipmentSlot GetEquipmentSlotForString( const std::string& name )
{
	const std::string& nameLower = GetAsLowercase( name );
	if ( nameLower == "chest" )
		return EQUIPMENT_SLOT_CHEST;
	if ( nameLower == "head" )
		return EQUIPMENT_SLOT_HEAD;
	if ( nameLower == "legs" )
		return EQUIPMENT_SLOT_LEGS;
	if ( nameLower == "primaryhand" )
		return EQUIPMENT_SLOT_PRIMARY_HAND;
	if ( nameLower == "secondaryhand" )
		return EQUIPMENT_SLOT_SECONDARY_HAND;

	ERROR_AND_DIE( "GetItemTypeForString: Found no match for argument!" );
}


//--------------------------------------------------------------------------------------------------------------
std::string GetStringForEquipmentSlot( EquipmentSlot slot )
{
	switch ( slot )
	{
		case EQUIPMENT_SLOT_CHEST:			return "Chest";
		case EQUIPMENT_SLOT_HEAD:			return "Head";
		case EQUIPMENT_SLOT_LEGS:			return "Legs";
		case EQUIPMENT_SLOT_PRIMARY_HAND:	return "PrimaryHand";
		case EQUIPMENT_SLOT_SECONDARY_HAND:	return "SecondaryHand";
		default: ERROR_AND_DIE( "GetStringForEquipmentSlot: Found no match for argument!" );
	}
}


//--------------------------------------------------------------------------------------------------------------
std::string GetStringForItemType( ItemType type )
{
	switch ( type )
	{
		case ITEM_TYPE_WEAPON:	return "Weapon";
		case ITEM_TYPE_ARMOR:	return "Armor";
		case ITEM_TYPE_POTION:	return "Potion";
		default: ERROR_AND_DIE( "GetStringForItemType: Found no match for argument!" );
	}
}