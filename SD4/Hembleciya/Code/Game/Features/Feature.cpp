#include "Game/Features/Feature.hpp"
#include "Game/Behaviors/Behavior.hpp"
#include "Game/FieldOfView/FieldOfView.hpp"
#include "Game/Map.hpp"
#include "Engine/Renderer/TheRenderer.hpp"
#include "Engine/Core/TheConsole.hpp"


//--------------------------------------------------------------------------------------------------------------
Feature::Feature( const Feature& other, Map* map /*= nullptr*/, const XMLNode& instanceDataNode /*= XMLNode::emptyNode()*/ )
	: GameEntity( other )
	, m_featureType( other.m_featureType )
	, m_featureState( other.m_featureState )
	, m_isLineOfSightBlockedWhenActive( other.m_isLineOfSightBlockedWhenActive )
	, m_isMovementBlockedWhenActive( other.m_isMovementBlockedWhenActive )
	, m_isMovementBlockedWhenInactive( other.m_isMovementBlockedWhenInactive )
	, m_isLineOfSightBlockedWhenInactive( other.m_isLineOfSightBlockedWhenInactive )
	, m_zeroGlyph( other.m_zeroGlyph )
	, m_oneGlyph( other.m_oneGlyph )
{
	if ( !instanceDataNode.isEmpty() ) //IsContentEmpty() would be true for <Wander />.
		PopulateFromXMLNode( instanceDataNode, map );
}


//--------------------------------------------------------------------------------------------------------------
void Feature::PopulateFromXMLNode( const XMLNode& featureBlueprintNode, Map* map )
{
	GameEntity::PopulateFromXMLNode( featureBlueprintNode, map );

	if ( m_map != nullptr ) //Then we need to occupy a cell's inventory.
	{
		Cell& cell = m_map->GetCellForPosition( GetPositionMins() );
		cell.m_occupyingFeature = this;
	}

	std::string featureTypeAsString;
	featureTypeAsString = ReadXMLAttribute( featureBlueprintNode, "type", featureTypeAsString );
	m_featureType = GetFeatureTypeForString( featureTypeAsString );

	m_featureState = (FeatureState)ReadXMLAttribute( featureBlueprintNode, "currentState", (int)m_featureState );

	m_isLineOfSightBlockedWhenActive = ( ReadXMLAttribute( featureBlueprintNode, "isLineOfSightBlockedWhenActive", 0 ) > 0 ) ? true : false;
	m_isLineOfSightBlockedWhenInactive = ( ReadXMLAttribute( featureBlueprintNode, "isLineOfSightBlockedWhenInactive", 0 ) > 0 ) ? true : false;
	m_isMovementBlockedWhenActive = ( ReadXMLAttribute( featureBlueprintNode, "isMovementBlockedWhenActive", 0 ) > 0 ) ? true : false;
	m_isMovementBlockedWhenInactive = ( ReadXMLAttribute( featureBlueprintNode, "isMovementBlockedWhenInactive", 0 ) > 0 ) ? true : false;

	m_zeroGlyph = ReadXMLAttribute( featureBlueprintNode, "zeroGlyph", 'Y' );
	m_oneGlyph = ReadXMLAttribute( featureBlueprintNode, "oneGlyph", 'N' );
}


//--------------------------------------------------------------------------------------------------------------
bool Feature::AttachToMapAtPosition( Map* map, const MapPosition& position )
{
	bool isValid = GameEntity::AttachToMapAtPosition( map, position );

	if ( isValid )
		SetOccupiedCellsFeatureTo( this );

	return isValid;
}


void Feature::ToggleState()
{
	m_featureState = (FeatureState)( ( m_featureState > 0 ) ? 0 : 1 );

	if ( m_featureType == FEATURE_TYPE_DOOR )
	{
		if ( MatchesState( FEATURE_STATE_OPEN ) )
			g_theConsole->Printf( "The %s opens softly with a quiet creak...", this->GetName().c_str() );
		else
			g_theConsole->Printf( "You seal the %s shut behind you.", this->GetName().c_str() );
		g_theConsole->ShowConsole();
	}
}

//--------------------------------------------------------------------------------------------------------------
bool Feature::DoesCurrentlyBlockLineOfSight()
{
	return ( (int)m_featureState > 0 ) ? m_isLineOfSightBlockedWhenActive : m_isLineOfSightBlockedWhenInactive;
}


//--------------------------------------------------------------------------------------------------------------
bool Feature::DoesCurrentlyBlockMovement()
{
	return ( (int)m_featureState > 0 ) ? m_isMovementBlockedWhenActive : m_isMovementBlockedWhenInactive;
}


//--------------------------------------------------------------------------------------------------------------
void Feature::WriteToXMLNode( XMLNode& out_entityDataNode )
{
	if ( m_map == nullptr )
		return; //Do not write out unless attached to a map.

	XMLNode featureNode = out_entityDataNode.addChild( "FeatureBlueprint" );

	GameEntity::WriteToXMLNode( featureNode );

	WriteXMLAttribute( featureNode, "type", GetStringForFeatureType( m_featureType ), std::string() );
	WriteXMLAttribute( featureNode, "currentState", (int)m_featureState, -1 );

	WriteXMLAttribute( featureNode, "isLineOfSightBlockedWhenActive", m_isLineOfSightBlockedWhenActive ? 1 : 0, 0 );
	WriteXMLAttribute( featureNode, "isLineOfSightBlockedWhenInactive", m_isLineOfSightBlockedWhenInactive ? 1 : 0, 0 );
	WriteXMLAttribute( featureNode, "isMovementBlockedWhenActive", m_isMovementBlockedWhenActive ? 1 : 0, 0 );
	WriteXMLAttribute( featureNode, "isMovementBlockedWhenInactive", m_isMovementBlockedWhenInactive ? 1 : 0, 0 );

	WriteXMLAttribute( featureNode, "zeroGlyph", m_zeroGlyph, 'Y' );
	WriteXMLAttribute( featureNode, "oneGlyph", m_oneGlyph, 'N' );
}


//--------------------------------------------------------------------------------------------------------------
char Feature::GetGlyph() const
{
	return ( (int)m_featureState > 0 ) ? m_oneGlyph : m_zeroGlyph;
}


//--------------------------------------------------------------------------------------------------------------
void Feature::SetOccupiedCellsFeatureTo( Feature* feature )
{
	const MapPosition& positionMins = GetPositionMins();
	const MapPosition& positionMaxs = m_positionBounds->maxs;
	for ( int cellY = positionMins.y; cellY < positionMaxs.y; cellY++ )
	{
		for ( int cellX = positionMins.x; cellX < positionMaxs.x; cellX++ )
		{
			Cell& cell = m_map->GetCellForPosition( MapPosition( cellX, cellY ) );
			cell.m_occupyingFeature = feature;
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
FeatureType GetFeatureTypeForString( const std::string& name )
{
	const std::string& nameLower = GetAsLowercase( name );
	if ( nameLower == "door" )
		return FEATURE_TYPE_DOOR;

	ERROR_AND_DIE( "GetFeatureTypeForString: Found no match for argument!" );
}


//--------------------------------------------------------------------------------------------------------------
std::string GetStringForFeatureType( FeatureType type )
{
	switch ( type )
	{
		case FEATURE_TYPE_DOOR:	return "Door";
		default: ERROR_AND_DIE( "GetStringForItemType: Found no match for argument!" );
	}
}