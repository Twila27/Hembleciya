#pragma once

#include "Game/GameCommon.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Game/Cell.hpp"
#include <vector>
#include <string>
class Command;
struct PathNode;
struct XMLNode;


class Map
{
public:

	inline Map( const Vector2i& size, const std::string& mapName ); //Bare-bones map.
	Map( const std::string& xmlPath, const std::string& mapName ); //Data-driven map.
	Map( const XMLNode& mapNode, const std::string& mapName = "" );

	void RefreshCellColors();
	void RefreshTraversableCells();
	void HideOccludedCells();
	static Map* CreateFromTileDataStringWithNewlines( const char* tileDataString, const std::string& mapName );
	static Map* InitializeFromVisibilityDataStringWithNewlines( const char* visibilityDataString, Map* tilesReadiedMap );

	inline void SetCellTypeForIndex( int index, CellType newType );
	inline void SetCellTypeForIndex( const MapPosition& position, CellType newType );

	Vector2i GetDimensions() const { return m_size; }
	int& GetCurrentGeneratorStepNum() { return m_generationStepCount; }
	std::string GetMapName() const { return m_mapName; }

	inline MapPosition GetPositionForIndex( unsigned int cellIndex ) const;
	inline int GetIndexForPosition( const MapPosition& position ) const;
	inline int GetIndexForPosition( const Vector2f& position ) const;
	Cell& GetCellForPosition( const MapPosition& position ) { return m_cells[ GetIndexForPosition( position ) ]; }
	Cell& GetCellForPosition( const Vector2f& position ) { return m_cells[ GetIndexForPosition( position ) ]; }

	std::vector< Cell >& GetCells() { return m_cells; }
	std::vector< MapPosition >& GetTraversableCells() { return m_traversableCells; }
	void CopyCellsFromMap( Map* sourceMap );

	inline bool DoesCellMatchType( unsigned int cellIndex, CellType type );
	unsigned int CountCellsWithFeaturesAroundCenter( const MapPosition& centerCellPos, float radiusFromCenterCell, bool considerDiagonals = true );
	unsigned int GetNumNeighborsAroundCellOfType( const MapPosition& centerCellPos, CellType queriedType, float radiusFromCenterCell, bool considerDiagonals = true );
	MapDirection IsCellAdjacentToType( const MapPosition& centerCellPos, CellType queriedType, bool considerDiagonals );
	bool IsPositionOnMap( const MapPosition& position ) const;
	bool IsTraversableAtPosition( const MapPosition& position ) const;
	bool IsSolidAtPosition( const MapPosition& position ) const;
	bool IsSolidAtPosition( const Vector2f& position ) const;
	bool DoesBlockLineOfSightAtPosition( const MapPosition& position ) const;
	bool DoesBlockLineOfSightAtPosition( const Vector2f& position ) const;

	void Render();
	static void ShowMap( Command& args );

	void GetAdjacentNeighborCells( const MapPosition& centerCellPos, float radiusFromCenterCell, bool includeDiagonals, std::vector< Cell* >& out_neighbors );
	void BuildPathNodesForTraversableNeighbors( PathNode* activeNode, const MapPosition& goalPos, std::vector<PathNode*>& out_neighbors, bitfield_int traversalProperties );

	bool DoesPositionSatisfyTraversalProperties( const MapPosition& position, bitfield_int traversalProperties );
	bool IsSlowedAtPosition( const MapPosition& position, bitfield_int traversalProperties );
	MapPosition GetRandomMapPosition( bitfield_int blockedProperties );

	void WriteToXMLNode( XMLNode& out_mapNode );
	static std::string GetAsString( Map* map, bool wasMapFromXML, int numTabs = 0, bool includeNewlines = true, bool onlyShowKnownCells = false );
	void RefreshCellOccupantVisibility();

private:
	std::string m_mapName;
	int m_generationStepCount;  //Reset after each m_process in a BiomeBlueprint completes.

	std::vector< Cell > m_cells;
	std::vector< Vector2i > m_traversableCells;

	Vector2i m_size;
};


//--------------------------------------------------------------------------------------------------------------
inline int Map::GetIndexForPosition( const Vector2f& position ) const
{
	return GetIndexForPosition( MapPosition(
		static_cast<int>( position.x ),
		static_cast<int>( position.y )
		) );
}


//--------------------------------------------------------------------------------------------------------------
inline void Map::SetCellTypeForIndex( const MapPosition& position, CellType newType )
{
	int index = GetIndexForPosition( position );
	SetCellTypeForIndex( index, newType );
}


//--------------------------------------------------------------------------------------------------------------
inline void Map::SetCellTypeForIndex( int index, CellType newType )
{

	if ( index < 0 || index >( int )m_cells.size() )
		return;

	m_cells[ index ].m_cellType = newType;
}
