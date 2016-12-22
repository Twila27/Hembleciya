#pragma once
#include <vector>

//-----------------------------------------------------------------------------
class Item;
class Map;
struct XMLNode;


//-----------------------------------------------------------------------------
class Inventory
{
	std::vector< Item* > m_items;

public:
	Inventory() {}
	Inventory( const XMLNode& inventoryNode ) { PopulateFromXMLNode( inventoryNode ); }
	Inventory( const Inventory& other )
	{
		for ( Item* item : other.m_items )
			PushItem( item );
	}

	std::vector< Item* >& GetItems() { return m_items; }
	Item* PopItem( Map* newMapToAttachTo );
	void PushItem( Item* item ); //Forbids duplicates.
	unsigned int GetCount() const { return m_items.size(); }

	void ShowItem();
	void HideItems();
	void EraseItem( Item* item );
	char GetGlyphForTopItem() const;

	void WriteToXMLNode( XMLNode& inventoryNode );
	void PopulateFromXMLNode( const XMLNode& inventoryNode );
};