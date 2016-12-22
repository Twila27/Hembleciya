#include "Game/Inventory.hpp"
#include "Game/Items/Item.hpp"
#include "Game/Items/ItemFactory.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
Item* Inventory::PopItem( Map* newMapToAttachTo )
{
	if ( GetCount() == 0 )
		return nullptr;

	Item* item = m_items.back();

	m_items.pop_back(); 

	item->SetCurrentMap( newMapToAttachTo );

	return item;
}


//--------------------------------------------------------------------------------------------------------------
void Inventory::PushItem( Item* item )
{
	for ( Item* existingItem : m_items )
		if ( existingItem == item ) //May be trouble down the line doing shallow compare.
			return;

	m_items.push_back( item );
}


//--------------------------------------------------------------------------------------------------------------
void Inventory::ShowItem()
{
	for ( Item* item : m_items )
		item->Show();
}


//--------------------------------------------------------------------------------------------------------------
void Inventory::HideItems()
{
	for ( Item* item : m_items )
		item->Hide();
}


//--------------------------------------------------------------------------------------------------------------
void Inventory::EraseItem( Item* item )
{
	for ( std::vector< Item* >::iterator itemIter = m_items.begin(); itemIter != m_items.end(); )
	{
		if ( *itemIter == item )
		{
			m_items.erase( itemIter );
			return;
		}
		else ++itemIter;
	}
}


//--------------------------------------------------------------------------------------------------------------
char Inventory::GetGlyphForTopItem() const
{
	return m_items.front()->GetGlyph();
}


//--------------------------------------------------------------------------------------------------------------
void Inventory::WriteToXMLNode( XMLNode& inventoryNode )
{
	for ( Item* item : m_items )
	{
		std::string itemTypeAsString = GetStringForItemType( item->GetItemType() );
		XMLNode inventoryItemNode = inventoryNode.addChild( itemTypeAsString.c_str() );
		WriteXMLAttribute( inventoryItemNode, GetAsLowercase( itemTypeAsString ), item->GetName(), std::string() );
	}
}


//--------------------------------------------------------------------------------------------------------------
void Inventory::PopulateFromXMLNode( const XMLNode& inventoryNode )
{
	for ( int inventoryItemIndex = 0; inventoryItemIndex < inventoryNode.nChildNode(); inventoryItemIndex++ )
	{
		XMLNode inventoryItemNode = inventoryNode.getChildNode( inventoryItemIndex );
		std::string itemTypeAsString = inventoryItemNode.getName();
		ItemType itemType = GetItemTypeForString( itemTypeAsString );
		ItemFactoryCategory& registry = ItemFactory::GetRegistryForItemType( itemType );
		ItemFactory* factory = registry.at( inventoryItemNode.getAttribute( GetAsLowercase( itemTypeAsString ).c_str() ) );
		PushItem( factory->CreateItem() );
	}
}