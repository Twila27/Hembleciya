#pragma once


#include "Game/GameEntity.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"
#include "Engine/Math/Interval.hpp"
#include <vector>


//-----------------------------------------------------------------------------
enum FeatureState
{
	FEATURE_STATE_DEACTIVATED = 0,
	FEATURE_STATE_CLOSED = 0,
	FEATURE_STATE_OFF = 0,

	FEATURE_STATE_ACTIVATED = 1,
	FEATURE_STATE_OPEN = 1,
	FEATURE_STATE_ON = 1
};


//-----------------------------------------------------------------------------
enum FeatureType
{
	FEATURE_TYPE_DOOR,
// 	FEATURE_TYPE_CONTAINER, //e.g. chest but more broadly, would have an Inventory*.
// 	FEATURE_TYPE_LEVER,
// 	FEATURE_TYPE_BONFIRE,
// 	FEATURE_TYPE_PLANT,
// 	FEATURE_TYPE_WINDOW,
// 	FEATURE_TYPE_TRAP,
// 	FEATURE_TYPE_RAIL,
// 	FEATURE_TYPE_FOUNTAIN,
	NUM_FEATURE_TYPES
};
FeatureType GetFeatureTypeForString( const std::string& name );
std::string GetStringForFeatureType( FeatureType featureType );


//-----------------------------------------------------------------------------
class Feature : public GameEntity
{
public:
	Feature( const XMLNode& featureBlueprintNode, Map* map ) 
		: GameEntity( ENTITY_TYPE_FEATURE ) 
		, m_featureState( (FeatureState)0 )
		, m_featureType( NUM_FEATURE_TYPES )
		, m_isLineOfSightBlockedWhenActive( false )
		, m_isMovementBlockedWhenActive( false )
		, m_isMovementBlockedWhenInactive( false )
		, m_isLineOfSightBlockedWhenInactive( false )
		, m_zeroGlyph( 'Y' )
		, m_oneGlyph( 'N' )
	{ 
		m_color = ( Rgba::YELLOW * Rgba::GRAY ) + Rgba::RED;
		PopulateFromXMLNode( featureBlueprintNode, map ); 
	}
	Feature( const Feature& other, Map* map = nullptr, const XMLNode& instanceDataNode = XMLNode::emptyNode() );
	virtual void WriteToXMLNode( XMLNode& out_entityDataNode ) override;
	void ResolvePointersToEntities( std::map< EntityID, GameEntity* >& ) override {}
	void SetOccupiedCellsFeatureTo( Feature* feature );
	virtual bool AttachToMapAtPosition( Map* map, const MapPosition& position ) override;

	virtual char GetGlyph() const override;
	FeatureType GetFeatureType() const { return m_featureType; }
	void SetFeatureType( FeatureType type ) { m_featureType = type; }
	
	virtual CooldownSeconds Update( float deltaSeconds ) override { UNREFERENCED( deltaSeconds ); return NO_TURN_COOLDOWN; }
	void ToggleState();
	bool DoesCurrentlyBlockLineOfSight();
	bool DoesCurrentlyBlockMovement();

private:
	virtual void PopulateFromXMLNode( const XMLNode& featureBlueprintNode, Map* map ) override;

	FeatureType m_featureType;
	FeatureState m_featureState;
	bool MatchesState( FeatureState state ) { return m_featureState == state; }

	bool m_isLineOfSightBlockedWhenActive;
	bool m_isMovementBlockedWhenActive;
	bool m_isMovementBlockedWhenInactive;
	bool m_isLineOfSightBlockedWhenInactive;

	char m_zeroGlyph;
	char m_oneGlyph;
};
