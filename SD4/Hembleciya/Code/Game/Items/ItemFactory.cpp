#include "Game/Items/ItemFactory.hpp"
#include "Game/GameCommon.hpp"
#include <vector>
#include "Engine/FileUtils/FileUtils.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/String/StringUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
ItemFactoryCategory ItemFactory::s_itemFactoryRegistry[ NUM_ITEM_TYPES ]; //Do we need to assign the maps?


//--------------------------------------------------------------------------------------------------------------
void ItemFactory::PopulateFromXMLNode( const XMLNode& itemBlueprintNode )
{
	m_damageRange = ReadXMLAttribute( itemBlueprintNode, "damage", m_damageRange );

	m_templateItem = new Item( itemBlueprintNode, nullptr );
}


//--------------------------------------------------------------------------------------------------------------
void ItemFactory::SetItemType( ItemType type )
{
	m_factoryItemType = type;
	m_templateItem->SetItemType( type );
	switch ( type )
	{
		case ITEM_TYPE_WEAPON: m_templateItem->SetGlyph( '(' ); return;
		case ITEM_TYPE_ARMOR: m_templateItem->SetGlyph( '[' ); return;
		case ITEM_TYPE_POTION: m_templateItem->SetGlyph( '!' ); return;
	}
}


//--------------------------------------------------------------------------------------------------------------
STATIC void ItemFactory::LoadAllItemBlueprints()
{
	//Note we may have more than one Item in a file.
	std::vector< std::string > m_itemFiles = EnumerateFilesInDirectory( "Data/XML/Items", "*.Item.xml" );

	for ( unsigned int itemFileIndex = 0; itemFileIndex < m_itemFiles.size(); itemFileIndex++ )
	{
		const char* xmlFilename = m_itemFiles[ itemFileIndex ].c_str();
		XMLNode blueprintsRoot = XMLNode::openFileHelper( xmlFilename, "ItemBlueprints" );

		for ( int itemBlueprintIndex = 0; itemBlueprintIndex < blueprintsRoot.nChildNode(); itemBlueprintIndex++ )
		{
			XMLNode itemBlueprintNode = blueprintsRoot.getChildNode( itemBlueprintIndex );

			ItemFactory* newFactory = new ItemFactory( itemBlueprintNode ); //Each ItemFactory corresponds to a ItemBlueprint element.
			newFactory->m_name = ReadXMLAttribute( itemBlueprintNode, "name", newFactory->m_name );

			std::string itemTypeAsString;
			itemTypeAsString = ReadXMLAttribute( itemBlueprintNode, "type", itemTypeAsString );
			newFactory->SetItemType( GetItemTypeForString( itemTypeAsString ) );

			ItemFactoryCategory& categoryRegistry = s_itemFactoryRegistry[ newFactory->m_factoryItemType ];

			if ( categoryRegistry.find( newFactory->m_name ) != categoryRegistry.end() )
				ERROR_AND_DIE( Stringf( "Found duplicate Item factory type/name %s in LoadAllItemBlueprints!", newFactory->m_name.c_str() ) );

			categoryRegistry.insert( std::pair< std::string, ItemFactory* >( newFactory->m_name, newFactory ) );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
Item* ItemFactory::CreateItem( Map* map /*= nullptr*/, const XMLNode& ItemInstanceNode /*= XMLNode::emptyNode()*/ )
{
	Item* newItem = new Item( *m_templateItem, map, ItemInstanceNode ); //Note overridden copy constructor at work to perform deep copy.

	//Factory stores the range's Interval, gives instances to Items:
	newItem->SetWeaponDamage( m_damageRange.GetRandomElement() );
	return newItem;
}
