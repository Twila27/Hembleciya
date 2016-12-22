#include "Game/Cell.hpp"
#include "Game/Inventory.hpp"
#include "Game/Items/Item.hpp"
#include "Game/Features/Feature.hpp"
#include "Game/Agent.hpp"


//--------------------------------------------------------------------------------------------------------------
Cell::~Cell()
{
	if ( ( m_occupyingItems != nullptr ) && ( m_occupyingItems->GetCount() > 0 ) )
	{
		delete m_occupyingItems;
		m_occupyingItems = nullptr;
	}
}


//--------------------------------------------------------------------------------------------------------------
bool IsTypeSolid( CellType type ) //Move to GameCommon.
{
	return type == CELL_TYPE_STONE_WALL;
}


//--------------------------------------------------------------------------------------------------------------
bool Cell::DoesBlockLineOfSight() const
{
	bool doesFeatureBlock = IsOccupiedByFeature();
	if ( doesFeatureBlock )
		doesFeatureBlock = m_occupyingFeature->DoesCurrentlyBlockLineOfSight();

	return doesFeatureBlock || IsTypeSolid( m_cellType );
}


//--------------------------------------------------------------------------------------------------------------
bool Cell::DoesBlockMovement() const //Note this is now controlled by Map::DoesPositionSatisfyTraversalProperties.
{
	bool doesFeatureBlock = IsOccupiedByFeature();
	if ( doesFeatureBlock )
		doesFeatureBlock = m_occupyingFeature->DoesCurrentlyBlockMovement();

	return doesFeatureBlock || IsOccupiedByAgent() || IsTypeSolid( m_cellType );
}


//--------------------------------------------------------------------------------------------------------------
void Cell::SetSeen()
{
	m_hasBeenSeenBefore = m_isCurrentlySeen = true;
	if ( IsOccupiedByAgent() )
		m_occupyingAgent->SetSeen();
	if ( IsOccupiedByFeature() )
		m_occupyingFeature->SetSeen();
}


//--------------------------------------------------------------------------------------------------------------
void Cell::SetUnseen()
{
	m_isCurrentlySeen = false;
}


//--------------------------------------------------------------------------------------------------------------
void Cell::SetHasBeenSeenBefore()
{
	m_hasBeenSeenBefore = true;
	if ( IsOccupiedByAgent() )
		m_occupyingAgent->SetHasBeenSeenBefore();
	if ( IsOccupiedByFeature() )
		m_occupyingFeature->SetHasBeenSeenBefore();
}


//--------------------------------------------------------------------------------------------------------------
std::vector< Item* >& Cell::GetItems()
{
	return m_occupyingItems->GetItems();
}


//--------------------------------------------------------------------------------------------------------------
bool Cell::HasMoreItemsThan( unsigned int amount ) const
{
	if ( m_occupyingItems == nullptr )
		return 0 > amount;

	return m_occupyingItems->GetCount() > amount;
}


//--------------------------------------------------------------------------------------------------------------
void Cell::PushItem( Item* item )
{
	if ( m_occupyingItems == nullptr )
		m_occupyingItems = new Inventory();

	if ( IsCurrentlySeen() )
	{
		item->SetSeen();
	}
	else if ( HasBeenSeenBefore() )
	{
		item->SetSeen();
		item->SetUnseen();
	}

	m_occupyingItems->PushItem( item );
}


//--------------------------------------------------------------------------------------------------------------
Item* Cell::TakeItemFromCell()
{
	if ( m_occupyingItems != nullptr )
		return m_occupyingItems->PopItem( nullptr );
	else
		return nullptr;
}


//--------------------------------------------------------------------------------------------------------------
void Cell::EraseItem( Item* item )
{
	if ( m_occupyingItems != nullptr ) 
		m_occupyingItems->EraseItem( item );
}


//--------------------------------------------------------------------------------------------------------------
char Cell::GetGlyphForTopItem() const
{
	ASSERT_OR_DIE( m_occupyingItems != nullptr, nullptr );

	return m_occupyingItems->GetGlyphForTopItem();
}


//--------------------------------------------------------------------------------------------------------------
bool Cell::DoesOccupyingFeatureCurrentlyBlockMovement() const
{
	return m_occupyingFeature->DoesCurrentlyBlockMovement();
}
