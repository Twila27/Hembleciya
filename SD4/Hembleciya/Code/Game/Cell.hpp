#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/Rgba.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------
class Inventory;
class Item;
class Feature;


//-----------------------------------------------------------------------------
extern bool IsTypeSolid( CellType type );


//-----------------------------------------------------------------------------
class Cell
{
public:
	Cell( const Vector2i& position )
		: m_position( position )
		, m_isHidden( false )
		, m_isCurrentlySeen( false )
		, m_hasBeenSeenBefore( false )
		, m_occupyingAgent( nullptr )
		, m_occupyingFeature( nullptr )
		, m_occupyingItems( nullptr )
	{
	}
	~Cell();

	Rgba m_color;
	Agent* m_occupyingAgent;
	Feature* m_occupyingFeature;
	Inventory* m_occupyingItems;
	Vector2i m_position;
	CellType m_cellType;
	CellType m_nextCellType;
	char m_parsedMapGlyph;

	bool DoesBlockLineOfSight() const;
	bool DoesBlockMovement() const;

	void SetSeen();
	void SetUnseen();
	void SetHasBeenSeenBefore();
	bool IsCurrentlySeen() const { return m_isCurrentlySeen; }
	bool HasBeenSeenBefore() const { return m_hasBeenSeenBefore; }
	bool m_isHidden; //Will never be seen, e.g. because it's surrounded by solid cells.

	std::vector< Item* >& GetItems();
	bool HasItems() const { return HasMoreItemsThan( 0 ); }
	bool HasMoreItemsThan( unsigned int amount ) const;
	void PushItem( Item* item ); //Forbids duplicates.
	Item* TakeItemFromCell();
	void EraseItem( Item* item );
	char GetGlyphForTopItem() const;

	bool IsOccupiedByAgent() const { return m_occupyingAgent != nullptr; }	
	Agent* GetAgent() const { return m_occupyingAgent; }
	
	bool IsOccupiedByFeature() const { return m_occupyingFeature != nullptr; }
	bool DoesOccupyingFeatureCurrentlyBlockMovement() const;
	Feature* GetFeature() const { return m_occupyingFeature; }

private:
	bool m_isCurrentlySeen;
	bool m_hasBeenSeenBefore;
};
