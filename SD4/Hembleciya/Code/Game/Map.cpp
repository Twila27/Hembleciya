#include "Game/Map.hpp"
#include "Engine/Renderer/TheRenderer.hpp"
#include "Engine/Core/Command.hpp"
#include "Engine/String/StringUtils.hpp"
#include "Game/Pathfinding/PathNode.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC void Map::ShowMap( Command& /*args*/ )
{
	g_showFullMap = !g_showFullMap;
}


//--------------------------------------------------------------------------------------------------------------
void Map::GetAdjacentNeighborCells( const MapPosition& centerCellPos, float radiusFromCenterCell, bool includeDiagonals, std::vector< Cell* >& out_neighbors )
{
	//Starts at 1 to exclude the center itself.
	for ( int currentRadius = 1; currentRadius <= radiusFromCenterCell; currentRadius++  )
	{
		if ( includeDiagonals )
		{
			MapPosition topLeftPos = centerCellPos - ( MapPosition::UNIT_X + MapPosition::UNIT_Y ) * currentRadius;
			MapPosition topRightPos = centerCellPos + ( MapPosition::UNIT_X + MapPosition::UNIT_Y ) * currentRadius;
			MapPosition bottomLeftPos = centerCellPos - ( MapPosition::UNIT_X - MapPosition::UNIT_Y ) * currentRadius;
			MapPosition bottomRightPos = centerCellPos + ( MapPosition::UNIT_X - MapPosition::UNIT_Y ) * currentRadius;

			if ( IsPositionOnMap( topLeftPos ) )
				out_neighbors.push_back( &m_cells[ GetIndexForPosition( topLeftPos ) ] );
			if ( IsPositionOnMap( topRightPos ) )
				out_neighbors.push_back( &m_cells[ GetIndexForPosition( topRightPos ) ] );
			if ( IsPositionOnMap( bottomLeftPos ) )
				out_neighbors.push_back( &m_cells[ GetIndexForPosition( bottomLeftPos ) ] );
			if ( IsPositionOnMap( bottomRightPos ) )
				out_neighbors.push_back( &m_cells[ GetIndexForPosition( bottomRightPos ) ] );
		}

		MapPosition topPos = centerCellPos + MapPosition::UNIT_Y * currentRadius;
		MapPosition leftPos = centerCellPos - MapPosition::UNIT_X * currentRadius;
		MapPosition rightPos = centerCellPos + MapPosition::UNIT_X * currentRadius;
		MapPosition bottomPos = centerCellPos - MapPosition::UNIT_Y * currentRadius;

		if ( IsPositionOnMap( topPos ) )
			out_neighbors.push_back( &m_cells[ GetIndexForPosition( topPos ) ] );
		if ( IsPositionOnMap( leftPos ) )
			out_neighbors.push_back( &m_cells[ GetIndexForPosition( leftPos ) ] );
		if ( IsPositionOnMap( rightPos ) )
			out_neighbors.push_back( &m_cells[ GetIndexForPosition( rightPos ) ] );
		if ( IsPositionOnMap( bottomPos ) )
			out_neighbors.push_back( &m_cells[ GetIndexForPosition( bottomPos ) ] );
	}
}


//--------------------------------------------------------------------------------------------------------------
void Map::BuildPathNodesForTraversableNeighbors( PathNode* activeNode, const MapPosition& goalPos, std::vector<PathNode*>& out_neighbors, bitfield_int traversalProperties )
{
	const MapPosition& centerCellPos = activeNode->m_position;

	const MapPosition adjPositions[8] =
	{
		centerCellPos - MapPosition::UNIT_X + MapPosition::UNIT_Y,
		centerCellPos + MapPosition::UNIT_X + MapPosition::UNIT_Y,
		centerCellPos - MapPosition::UNIT_X - MapPosition::UNIT_Y,
		centerCellPos + MapPosition::UNIT_X - MapPosition::UNIT_Y,
		centerCellPos + MapPosition::UNIT_Y,
		centerCellPos - MapPosition::UNIT_X,
		centerCellPos + MapPosition::UNIT_X,
		centerCellPos - MapPosition::UNIT_Y
	};

	//Build and add a PathNode for each valid (non-solid) neighbor.
		//1. Increase g-cost if bitfield shows they are slowed by the cell type present.
		//2. Don't push altogether when blocked.

	for ( int adjPosIndex = 0; adjPosIndex < 8; adjPosIndex++ )
	{
		const MapPosition& currentPos = adjPositions[ adjPosIndex ];

		if ( DoesPositionSatisfyTraversalProperties( currentPos, traversalProperties ) )
		{
			bool wouldBeSlowed = IsSlowedAtPosition( currentPos, traversalProperties ) ? 1 : 0;

			float localStepCostG = CalcManhattanDistBetweenPoints( goalPos, currentPos ) * ( wouldBeSlowed ? 2.f : 1.f );

			out_neighbors.push_back( new PathNode( currentPos, 
												   activeNode, 
												   CalcDistBetweenPoints( centerCellPos, currentPos ), 
												   activeNode->GetTotalCostG(), 
												   localStepCostG ) );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
bool Map::DoesPositionSatisfyTraversalProperties( const MapPosition& position, bitfield_int traversalProperties )
{
	if ( !IsPositionOnMap( position ) )
		return false;

	Cell& queriedCell = GetCellForPosition( position );

	TODO( "Create a for-loop structure << 1 each iteration, and a GetCellTypeForMovementProperty() to plug in for macro's 2nd arg below." );
	//And then just check by-agents outside the loop.

	if ( GET_BIT_WITHOUT_INDEX_MASKED( traversalProperties, TraversalProperties::BLOCKED_BY_SOLIDS ) != 0 )
		if ( IsTypeSolid( queriedCell.m_cellType ) )
			return false;

	if ( GET_BIT_WITHOUT_INDEX_MASKED( traversalProperties, TraversalProperties::BLOCKED_BY_AIR ) != 0 )
		if ( queriedCell.m_cellType == CELL_TYPE_AIR )
			return false;

	if ( GET_BIT_WITHOUT_INDEX_MASKED( traversalProperties, TraversalProperties::BLOCKED_BY_AGENTS ) != 0 )
		if ( queriedCell.IsOccupiedByAgent() )
			return false;

	if ( GET_BIT_WITHOUT_INDEX_MASKED( traversalProperties, TraversalProperties::BLOCKED_BY_LAVA ) != 0 )
		if ( queriedCell.m_cellType == CELL_TYPE_LAVA )
			return false;

	if ( GET_BIT_WITHOUT_INDEX_MASKED( traversalProperties, TraversalProperties::BLOCKED_BY_WATER ) != 0 )
		if ( queriedCell.m_cellType == CELL_TYPE_WATER )
			return false;

	if ( queriedCell.IsOccupiedByFeature() )
		if ( queriedCell.DoesOccupyingFeatureCurrentlyBlockMovement() )
			return false;

	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool Map::IsSlowedAtPosition( const MapPosition& position, bitfield_int traversalProperties )
{
	if ( !IsPositionOnMap( position ) )
		return true;

	Cell& queriedCell = GetCellForPosition( position );

	TODO( "Create a for-loop structure << 1 each iteration, and a GetCellTypeForMovementProperty() to plug in for macro's 2nd arg below." );
	//And then just check by-agents outside the loop.

	if ( GET_BIT_WITHOUT_INDEX_MASKED( traversalProperties, TraversalProperties::SLOWED_BY_LAVA ) != 0 )
		if ( queriedCell.m_cellType == CELL_TYPE_LAVA )
			return true;

	if ( GET_BIT_WITHOUT_INDEX_MASKED( traversalProperties, TraversalProperties::SLOWED_BY_WATER ) != 0 )
		if ( queriedCell.m_cellType == CELL_TYPE_WATER )
			return true;

	return false;
}


//--------------------------------------------------------------------------------------------------------------
MapPosition Map::GetRandomMapPosition( bitfield_int blockedProperties )
{
	static const int MAX_ITERATIONS = INT_MAX;
	int numIteration = 0;
	MapPosition candidatePos;

	//We have a list of non-solid cells, may as well use it to filter out all the surrounding perimeter!
	bool includeSolid = ( GET_BIT_WITHOUT_INDEX_MASKED( blockedProperties, TraversalProperties::BLOCKED_BY_SOLIDS ) != 0 );

	while ( numIteration < MAX_ITERATIONS )
	{
		int randomIndex = GetRandomIntInRange( 0, ( includeSolid ? m_cells.size() : m_traversableCells.size() ) - 1 );

		candidatePos = includeSolid ? GetPositionForIndex( randomIndex ) : m_traversableCells.at( randomIndex );
		
		if ( DoesPositionSatisfyTraversalProperties( candidatePos, blockedProperties ) )
			return candidatePos;

		++numIteration;
	}

	ERROR_AND_DIE( "Failed to find a random cell grab compatible with argument bitfield within MAX_ITERATIONS." );
}


//--------------------------------------------------------------------------------------------------------------
void Map::WriteToXMLNode( XMLNode& out_mapNode )
{
	WriteXMLAttribute( out_mapNode, "mapName", m_mapName, std::string() );
	WriteXMLAttribute( out_mapNode, "mapSize", Stringf( "%d,%d", m_size.x, m_size.y ), std::string() );

	XMLNode tileDataNode = out_mapNode.addChild( "TileData" );
	tileDataNode.addText( Map::GetAsString( this, false, 2 ).c_str() );

	XMLNode visibilityDataNode = out_mapNode.addChild( "VisibilityData" );
	visibilityDataNode.addText( Map::GetAsString( this, false, 2, true, true ).c_str() );
}


//--------------------------------------------------------------------------------------------------------------
STATIC std::string Map::GetAsString( Map* map, bool wasMapFromXML, int numTabs /*= 0*/, bool includeNewlines /*= true */, bool onlyShowKnownCells /*= false*/ )
{
	std::string mapAsString = "\n"; //Start off on the next line after the data element.
	std::string tabString = "";
	for ( int numTab = 0; numTab < numTabs; ++numTab )
		tabString += '\t';

	//Iterate strings' rows in reverse, else comes out upside down.
	for ( int y = map->m_size.y - 1; y >= 0; y-- )
	{
		mapAsString += tabString; //Start newline with tabbing.

		for ( int x = 0; x < map->m_size.x; x++ )
		{
			Cell& cell = map->GetCellForPosition( MapPosition( x, y ) );
			char candidateGlyph = ( wasMapFromXML ) ? cell.m_parsedMapGlyph : GetGlyphForCell( cell, true );
			mapAsString += ( onlyShowKnownCells && !cell.HasBeenSeenBefore() ) ? '_' : candidateGlyph;
			//e.g. dream maps parse their glyphs, but generated maps do not.
		}

		if ( includeNewlines )
			mapAsString += '\n';
	}

	if ( numTabs > 0 )
		for ( int numTab = 0; numTab < numTabs - 1; ++numTab )
		mapAsString += '\t'; //Indent next line normally else it seems to override.

	return mapAsString;
}


//--------------------------------------------------------------------------------------------------------------
void Map::RefreshCellOccupantVisibility()
{
	for ( Cell& cell : m_cells )
	{
		if ( cell.IsCurrentlySeen() )
			cell.SetSeen();
		else if ( cell.HasBeenSeenBefore() )
			cell.SetHasBeenSeenBefore();
	}
}


//--------------------------------------------------------------------------------------------------------------
void Map::HideOccludedCells()
{
	//See whether any tile has non - solid neighbors.If any neighbor has a non - solid neighbor, it could be seen by someone.
	// 		Might be a good chance to use TileDefinition for it...
	// 		But if none of my neighbors are non - solid, or all of my neighbors are solid, that tile will never be "seen".
	
	for ( unsigned int cellIndex = 0; cellIndex < m_cells.size(); cellIndex++ )
	{
		//if you're solid first here?
		if ( GetNumNeighborsAroundCellOfType( GetPositionForIndex( cellIndex ), CELL_TYPE_STONE_WALL, 1.f, true ) == 8 )
			m_cells[ cellIndex ].m_isHidden = true;
	}
}


//--------------------------------------------------------------------------------------------------------------
static CellType GetCellTypeForGlyph( char glyph )
{
	unsigned int numGlyphs = _countof( NON_DREAM_GLYPHS );
	for ( unsigned int glyphIndex = 0; glyphIndex < numGlyphs; glyphIndex++ )
		if ( NON_DREAM_GLYPHS[ glyphIndex ] == glyph )
			return (CellType)NON_DREAM_GLYPHS[ glyphIndex ];

	return CELL_TYPE_DREAM;
}


//--------------------------------------------------------------------------------------------------------------
Map::Map( const XMLNode& mapNode, const std::string& mapName /*= "" */ )
{
	const char* tileDataString = mapNode.getChildNode( "TileData" ).getText();


	m_mapName = ( mapName == "" ) ? ReadXMLAttribute( mapNode, "mapName", m_mapName ) : mapName;
	Map* result = CreateFromTileDataStringWithNewlines( tileDataString, m_mapName );
	*this = *result; //Will work if m_cells is by value.

	const XMLNode& visibilityDataNode = mapNode.getChildNode( "VisibilityData" );
	if ( !visibilityDataNode.isEmpty() )
	{
		const char* visibilityDataString = visibilityDataNode.getText();
		InitializeFromVisibilityDataStringWithNewlines( visibilityDataString, this );
	}

	RefreshTraversableCells();

	delete result; //Delete what we copied in above line.
}


//--------------------------------------------------------------------------------------------------------------
Map::Map( const std::string& xmlPath, const std::string& mapName )
{
	XMLNode root = XMLNode::openFileHelper( xmlPath.c_str(), "MapData" );
	*this = Map( root, mapName );
}


//--------------------------------------------------------------------------------------------------------------
Map::Map( const Vector2i& size, const std::string& mapName )
	: m_size( size )
	, m_mapName( mapName )
	, m_generationStepCount( 1 )
{
	for ( int y = 0; y < size.y; y++ )
		for ( int x = 0; x < size.x; x++ )
			m_cells.push_back( Vector2i( x, y ) );
}


//--------------------------------------------------------------------------------------------------------------
void Map::RefreshTraversableCells()
{
	for ( int x = 0; x < m_size.x; x++ )
	{
		for ( int y = 0; y < m_size.y; y++ )
		{
			MapPosition currentPos = Vector2i( x, y );
			if ( IsTraversableAtPosition( currentPos ) )
				m_traversableCells.push_back( currentPos );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
void Map::RefreshCellColors() //Right now flops for dream maps because they can be tinted wildly and do multiple tints in a map.
{
	for ( int x = 0; x < m_size.x; x++ )
	{
		for ( int y = 0; y < m_size.y; y++ )
		{
			Cell& cell = GetCellForPosition( Vector2i( x, y ) );
			cell.m_color = GetColorForCellType( cell.m_cellType ); 
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
STATIC Map* Map::CreateFromTileDataStringWithNewlines( const char* tileDataString, const std::string& mapName )
{
	std::vector< std::string > mapRowsStrings = SplitString( tileDataString, '\n', true );

	int mapWidth = mapRowsStrings[ 0 ].size();
	int mapHeight = mapRowsStrings.size();

	Map* newMap = new Map( Vector2i( mapWidth, mapHeight ), mapName );

	for ( int y = 0; y < mapHeight; y++ )
	{
		//Iterate strings' rows in reverse, else comes out upside down.
		const std::string& currentMapRowString = mapRowsStrings[ ( mapHeight - 1 ) - y ];
		for ( int x = 0; x < mapWidth; x++ )
		{
			Cell& currentCell = newMap->m_cells[ newMap->GetIndexForPosition( Vector2i( x, y ) ) ];

			currentCell.m_parsedMapGlyph = currentMapRowString[ x ];
			currentCell.m_cellType = GetCellTypeForGlyph( currentMapRowString[ x ] );
			currentCell.m_color = GetColorForCellType( currentCell.m_cellType );
		}
	}

	newMap->HideOccludedCells();

	return newMap;
}


//--------------------------------------------------------------------------------------------------------------
STATIC Map* Map::InitializeFromVisibilityDataStringWithNewlines( const char* visibilityDataString, Map* tilesReadiedMap )
{
	std::vector< std::string > mapRowsStrings = SplitString( visibilityDataString, '\n', true );

	int mapWidth = mapRowsStrings[ 0 ].size();
	int mapHeight = mapRowsStrings.size();

	for ( int y = 0; y < mapHeight; y++ )
	{
		//Could probably iterate forward here, just being consistent:
		const std::string& currentMapRowString = mapRowsStrings[ ( mapHeight - 1 ) - y ];
		for ( int x = 0; x < mapWidth; x++ )
		{
			Cell& currentCell = tilesReadiedMap->m_cells[ tilesReadiedMap->GetIndexForPosition( Vector2i( x, y ) ) ];
			if ( currentMapRowString[x] != '_' )
				currentCell.SetHasBeenSeenBefore();
		}
	}

	return tilesReadiedMap;
}


//--------------------------------------------------------------------------------------------------------------
void Map::Render()
{ 
	for ( unsigned int cellIndex = 0; cellIndex < m_cells.size(); cellIndex++ )
	{
		const Cell& currentCell = m_cells[ cellIndex ];

		if ( currentCell.m_isHidden )
			continue;

		if ( !g_showFullMap && !currentCell.HasBeenSeenBefore() )
			continue;

		Rgba cellColor = currentCell.m_color;
		if ( ( currentCell.HasBeenSeenBefore() || g_showFullMap ) && !currentCell.IsCurrentlySeen() )
		{
			cellColor = cellColor * Rgba::GRAY;
			cellColor.alphaOpacity = 64;
		}

		std::string glyph;
		glyph = GetGlyphForCell( currentCell, false ); //Will draw as item(s) on top of it instead. Looks nicer.
		g_theRenderer->DrawTextProportional2D
		(
			GetScreenPositionForMapPosition( currentCell.m_position ),
			glyph,
			CELL_FONT_SCALE,
			CELL_FONT_OBJECT,
			cellColor,
			false
		);
	}
}


//--------------------------------------------------------------------------------------------------------------
MapDirection Map::IsCellAdjacentToType( const Vector2i& centerCellPos, CellType queriedType, bool considerDiagonals )
{
	//Neighbors == all 8 cells around a given cell. Any results off the edges of the map are considered solid.

	if ( considerDiagonals )
	{
		int topLeftCellIndex = GetIndexForPosition( centerCellPos - Vector2i::UNIT_X + Vector2i::UNIT_Y );
		int topRightCellIndex = GetIndexForPosition( centerCellPos + Vector2i::UNIT_X + Vector2i::UNIT_Y );
		int bottomLeftCellIndex = GetIndexForPosition( centerCellPos - Vector2i::UNIT_X - Vector2i::UNIT_Y );
		int bottomRightCellIndex = GetIndexForPosition( centerCellPos + Vector2i::UNIT_X - Vector2i::UNIT_Y );

		if ( DoesCellMatchType( topLeftCellIndex, queriedType ) ) return DIRECTION_UP_LEFT;
		if ( DoesCellMatchType( topRightCellIndex, queriedType ) ) return DIRECTION_UP_RIGHT;
		if ( DoesCellMatchType( bottomLeftCellIndex, queriedType ) ) return DIRECTION_DOWN_LEFT;
		if ( DoesCellMatchType( bottomRightCellIndex, queriedType ) ) return DIRECTION_DOWN_RIGHT;
	}

	int topCellIndex = GetIndexForPosition( centerCellPos + Vector2i::UNIT_Y );
	int leftCellIndex = GetIndexForPosition( centerCellPos - Vector2i::UNIT_X );
	int rightCellIndex = GetIndexForPosition( centerCellPos + Vector2i::UNIT_X );
	int bottomCellIndex = GetIndexForPosition( centerCellPos - Vector2i::UNIT_Y );

	if ( DoesCellMatchType( topCellIndex, queriedType ) ) return DIRECTION_UP;
	if ( DoesCellMatchType( leftCellIndex, queriedType ) ) return DIRECTION_LEFT;
	if ( DoesCellMatchType( rightCellIndex, queriedType ) ) return DIRECTION_RIGHT;
	if ( DoesCellMatchType( bottomCellIndex, queriedType ) ) return DIRECTION_DOWN;

	return DIRECTION_NONE;
}


//--------------------------------------------------------------------------------------------------------------
bool Map::IsPositionOnMap( const MapPosition& position ) const
{
	if ( position.x < 0 || position.y < 0 )
		return false;

	Vector2i mapSize = GetDimensions();
	if ( position.x >= mapSize.x || position.y >= mapSize.y )
		return false;

	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool Map::IsTraversableAtPosition( const MapPosition& position ) const
{
	if ( !IsPositionOnMap( position ) )
		return false;

	const Cell& queriedCell = m_cells[ GetIndexForPosition( position ) ];
	return !queriedCell.DoesBlockMovement();
}


//--------------------------------------------------------------------------------------------------------------
bool Map::IsSolidAtPosition( const MapPosition& position ) const
{
	if ( !IsPositionOnMap( position ) )
		return false;

	const Cell& queriedCell = m_cells[ GetIndexForPosition( position ) ]; 
	return queriedCell.DoesBlockMovement();
}


//--------------------------------------------------------------------------------------------------------------
bool Map::IsSolidAtPosition( const Vector2f& position ) const
{
	return IsSolidAtPosition( MapPosition(
		static_cast<int>( position.x ),
		static_cast<int>( position.y ) ) );
}


//--------------------------------------------------------------------------------------------------------------
bool Map::DoesBlockLineOfSightAtPosition( const Vector2f& position ) const
{
	return DoesBlockLineOfSightAtPosition( MapPosition(
		static_cast<int>( position.x ),
		static_cast<int>( position.y ) ) );
}


//--------------------------------------------------------------------------------------------------------------
bool Map::DoesBlockLineOfSightAtPosition( const MapPosition& position ) const
{
	if ( !IsPositionOnMap( position ) )
		return false;

	const Cell& queriedCell = m_cells[ GetIndexForPosition( position ) ];
	return queriedCell.DoesBlockLineOfSight();
}


//--------------------------------------------------------------------------------------------------------------
inline MapPosition Map::GetPositionForIndex( unsigned int cellIndex ) const
{
	MapPosition position = MapPosition( cellIndex % m_size.x, cellIndex / m_size.x );

	if ( position.x < 0 || position.y < 0 || position.x > m_size.x || position.y > m_size.y )
		position = MapPosition( -1, -1 );

	return position;
}


//--------------------------------------------------------------------------------------------------------------
inline int Map::GetIndexForPosition( const MapPosition& position ) const
{
	int index = position.x + ( position.y * m_size.x );

	if ( index < 0 || ( index >= ( m_size.x * m_size.y ) ) )
		index = -1;

	return index;
}


//--------------------------------------------------------------------------------------------------------------
void Map::CopyCellsFromMap( Map* sourceMap )
{
	m_cells = sourceMap->GetCells();
	m_traversableCells = sourceMap->GetTraversableCells();
	m_size = sourceMap->GetDimensions();
}


//--------------------------------------------------------------------------------------------------------------
inline bool Map::DoesCellMatchType( unsigned int cellIndex, CellType type )
{
	if ( cellIndex < 0 || cellIndex >( m_cells.size() - 1 ) )
		return ( type == CELL_TYPE_STONE_WALL ); //Assumes cells off-map are solid.

	return ( m_cells[ cellIndex ].m_cellType == type );
}


//--------------------------------------------------------------------------------------------------------------
unsigned int Map::CountCellsWithFeaturesAroundCenter( const MapPosition& centerCellPos, float radiusFromCenterCell, bool considerDiagonals /*= true*/ )
{
	//Neighbors == all 4 or 8 cells around a given cell. Any results off the edges of the map are considered solid.

	std::vector< Cell* > neighbors;
	GetAdjacentNeighborCells( centerCellPos, radiusFromCenterCell, considerDiagonals, neighbors );

	unsigned int numFeatures = 0;

	for ( Cell* cell : neighbors )
		if ( cell->IsOccupiedByFeature() )
			++numFeatures;

	return numFeatures;
}


//--------------------------------------------------------------------------------------------------------------
unsigned int Map::GetNumNeighborsAroundCellOfType( const Vector2i& centerCellPos, CellType queriedType, float radiusFromCenterCell, bool considerDiagonals /*= true*/ )
{
	//Neighbors == all 4 or 8 cells around a given cell. Any results off the edges of the map are considered solid.

	unsigned int numMatches = 0;
	
	std::vector< Cell* > neighbors;
	GetAdjacentNeighborCells( centerCellPos, radiusFromCenterCell, considerDiagonals, neighbors );
	for ( Cell* cell : neighbors )
		if ( cell->m_cellType == queriedType )
			++numMatches;

	return numMatches;
}