#pragma once


#include <map>
#include <string>
#include "Engine/FileUtils/XMLUtils.hpp"
#include "Engine/Math/Interval.hpp"
#include "Game/Items/Item.hpp"


//-----------------------------------------------------------------------------
class Map;


//-----------------------------------------------------------------------------
class ItemFactory;
typedef std::map< std::string, ItemFactory* > ItemFactoryCategory;


//-----------------------------------------------------------------------------
class ItemFactory
{
public:
	ItemFactory( const XMLNode& itemBlueprintNode ) { PopulateFromXMLNode( itemBlueprintNode ); }
	static void LoadAllItemBlueprints();
	Item* CreateItem( Map* map = nullptr, const XMLNode& itemInstanceNode = XMLNode::emptyNode() );
	static ItemFactoryCategory& GetRegistryForItemType( ItemType type ) { return s_itemFactoryRegistry[ type ]; }


private:
	void PopulateFromXMLNode( const XMLNode& itemBlueprintNode = XMLNode::emptyNode() );
	void SetItemType( ItemType type );
	//Constructor doesn't parse XML directly, instead calling this from constructor.
	//Public if we have to call it anywhere else as needed, can be protected/private hitherto.

	ItemType m_factoryItemType;
	static ItemFactoryCategory s_itemFactoryRegistry[ NUM_ITEM_TYPES ]; //Index via ItemType enum.
	std::string m_name;
	Item* m_templateItem;
	Interval<int> m_damageRange;
};
