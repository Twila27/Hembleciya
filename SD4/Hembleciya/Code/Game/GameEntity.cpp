#include "Game/GameEntity.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/TheRenderer.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"
#include "Engine/String/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC const EntityID GameEntity::s_INVALID_ID = -1;
STATIC EntityID GameEntity::s_BASE_ENTITY_ID = 1;
STATIC const int GameEntity::s_BASE_HEALTH = 20;


//--------------------------------------------------------------------------------------------------------------
GameEntity::GameEntity( EntityType entityType )
	: Entity()
	, m_entityType( entityType )
	, m_map( nullptr )
	, m_entityID( s_BASE_ENTITY_ID++ )
	, m_positionBounds( new AABB2i( 0, 0, 1, 1 ) )
	, m_isCurrentlySeen( false )
	, m_hasBeenSeenBefore( false )
	, m_health( s_BASE_HEALTH )
	, m_maxHealth( s_BASE_HEALTH )
{
}


//--------------------------------------------------------------------------------------------------------------
GameEntity::GameEntity( const GameEntity& other )
	: Entity( other )
	, m_entityType( other.m_entityType )
	, m_map( other.m_map )
	, m_entityID( s_BASE_ENTITY_ID++ )
	, m_positionBounds( new AABB2i( *other.m_positionBounds ) )
	, m_glyph( other.m_glyph )
	, m_health( other.m_health )
	, m_maxHealth( other.m_maxHealth )
	, m_color( other.m_color )
	, m_name( other.m_name )
	, m_isCurrentlySeen( other.m_isCurrentlySeen )
	, m_hasBeenSeenBefore( other.m_hasBeenSeenBefore )
	, m_savedID( s_INVALID_ID )
{
}


//------------------------------------------
void GameEntity::PopulateFromXMLNode( const XMLNode& instanceDataNode, Map* map )
{
	m_map = map;

	m_name = ReadXMLAttribute( instanceDataNode, "name", m_name );
	m_glyph = ReadXMLAttribute( instanceDataNode, "glyph", m_glyph );
	m_maxHealth = ReadXMLAttribute( instanceDataNode, "maxHealth", m_maxHealth );
	m_health = ReadXMLAttribute( instanceDataNode, "health", m_maxHealth ); //Defaults to the max.
	
	std::string colorString = ReadXMLAttribute( instanceDataNode, "color", std::string() );
	sscanf_s( colorString.c_str(), "%hhu,%hhu,%hhu", &m_color.red, &m_color.green, &m_color.blue );

	MapPosition savedPosition = ReadXMLAttribute( instanceDataNode, "position", GetPositionMins() );
	m_positionBounds->AddOffset( savedPosition );

	m_savedID = ReadXMLAttribute( instanceDataNode, "savedId", m_savedID );
}

//--------------------------------------------------------------------------------------------------------------
bool GameEntity::AttachToMapAtPosition( Map* map, const MapPosition& newPosMins )
{
	SetCurrentMap( map );

	bool isValid = m_map->IsPositionOnMap( newPosMins );
	if ( isValid )
		SetPositionMins( newPosMins );
	else 
		DebuggerPrintf( "GameEntity::AttachToMapAtPosition ignored invocation for illegal map position!" );

	return isValid;
}


//--------------------------------------------------------------------------------------------------------------
void GameEntity::SetPositionMins( const MapPosition& newMins )
{
	Vector2i size = Vector2i( m_positionBounds->GetWidth(), m_positionBounds->GetHeight() );

	m_positionBounds->mins = newMins;
	m_positionBounds->maxs = m_positionBounds->mins + size;
}


//--------------------------------------------------------------------------------------------------------------
GameEntity::~GameEntity()
{
	if ( m_positionBounds != nullptr )
	{
		delete m_positionBounds;
		m_positionBounds = nullptr;
	}
}


//--------------------------------------------------------------------------------------------------------------
bool GameEntity::AddHealthDelta( int delta ) //Boolean to show prompt message when healed on PLAYER_ACTION_REST.
{
	bool didHeal = true;

	m_health += delta;
	if ( m_health > m_maxHealth )
	{
		m_health = m_maxHealth;
		didHeal = false;
	}

	return didHeal;
}


//--------------------------------------------------------------------------------------------------------------
bool GameEntity::SubtractHealthDelta( int delta )
{
	bool didHeal = true;

	m_health -= delta;
	if ( m_health < 0 )
	{
		m_health = 0;
		didHeal = false;
	}

	return didHeal;
}

//--------------------------------------------------------------------------------------------------------------
bool GameEntity::Render()
{
	bool didRender = false;

	if ( m_map == nullptr )
		return didRender;
	
	//If we're not showing the full map, haven't been seen yet, and aren't the player, don't render.
	if ( !g_showFullMap && !m_hasBeenSeenBefore && !IsPlayer() )
		return didRender;

	Rgba entityColor = m_color;
	if ( m_hasBeenSeenBefore && !m_isCurrentlySeen )
	{
		entityColor = entityColor * Rgba::GRAY;
		entityColor.alphaOpacity = m_color.alphaOpacity / 4;
	}

	std::string drawRequiredGlyphAsString;
	drawRequiredGlyphAsString = GetGlyph();
	g_theRenderer->DrawTextProportional2D(
		GetScreenPositionForMapPosition( m_positionBounds->mins ),
		drawRequiredGlyphAsString,
		CELL_FONT_SCALE,
		CELL_FONT_OBJECT,
		entityColor
		);

	didRender = true;
	return didRender;
}


//--------------------------------------------------------------------------------------------------------------
void GameEntity::WriteToXMLNode( XMLNode& out_gameEntityNode )
{
	const std::string default = "";

	WriteXMLAttribute( out_gameEntityNode, "name", m_name, default );
	WriteXMLAttribute( out_gameEntityNode, "health", m_health, m_maxHealth );
	WriteXMLAttribute( out_gameEntityNode, "color", m_color, Rgba() );

	Vector2i pos = GetPositionMins();
	WriteXMLAttribute( out_gameEntityNode, "position", Stringf( "%d,%d", pos.x, pos.y ), default );

	WriteXMLAttribute( out_gameEntityNode, "savedId", m_entityID, s_INVALID_ID );

	//Visibility taken care of by the player's <VisibilityData> map, and NPCs won't use has-seen.
}
