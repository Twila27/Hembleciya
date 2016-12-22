#pragma once

#include "Game/GameCommon.hpp"
#include "Engine/Core/Entity.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"


//-----------------------------------------------------------------------------
class Map;
struct XMLNode;


//-----------------------------------------------------------------------------
class GameEntity : public Entity
{
public:
	GameEntity( EntityType entityType );
	GameEntity( const GameEntity& other ); //Else all will copy pointers to same position heap data, etc.

	virtual ~GameEntity();

	virtual bool Render();
	virtual bool IsReadyToUpdate() const { return true; }
	virtual CooldownSeconds Update( float deltaSeconds ) = 0;
	virtual bool IsPlayer() const { return false; }
	virtual bool IsInvincible() const { return false; }
	bool IsAlive() const { return m_health > 0; }

	void SetCurrentMap( Map* map ) { m_map = map; }
	virtual bool AttachToMapAtPosition( Map* map, const MapPosition& newPosMins );
	MapPosition GetPositionMins() const { return m_positionBounds->mins; }
	void SetPositionMins( const MapPosition& newMins );
	Map* GetMap() const { return m_map; }
	std::string GetName() const { return m_name; }
	EntityID GetSavedID() const { return m_savedID; }
	EntityID GetEntityID() const { return m_entityID; }
	int GetHealth() const { return m_health; }
	int GetMaxHealth() const { return m_maxHealth; }
	void SetMaxHealth( int newMaxHealth ) { m_maxHealth = newMaxHealth; }
	Rgba GetColor() const { return m_color; }
	void SetColor( const Rgba& newColor ) { m_color = newColor; }

	virtual char GetGlyph() const { return m_glyph; } //Overridden for objects that change glyphs.
	void SetGlyph( char newGlyph ) { m_glyph = newGlyph; }

	bool AddHealthDelta( int delta );
	bool SubtractHealthDelta( int delta );

	void SetSeen() { m_hasBeenSeenBefore = m_isCurrentlySeen = true; }
	void SetUnseen() { m_isCurrentlySeen = false; }
	void SetHasBeenSeenBefore() { m_hasBeenSeenBefore = true; }
	bool IsCurrentlySeen() const { return m_isCurrentlySeen; }
	bool HasBeenSeenBefore() const { return m_hasBeenSeenBefore; }

	virtual void WriteToXMLNode( XMLNode& out_gameEntityNode );	
	virtual void PopulateFromXMLNode( const XMLNode& instanceDataNode, Map* map );
	virtual void ResolvePointersToEntities( std::map< EntityID, GameEntity* >& loadedEntities ) = 0;

	static const EntityID s_INVALID_ID;


protected:
	EntityType m_entityType;
	Map* m_map;
	AABB2i* m_positionBounds;
	char m_glyph;
	Rgba m_color;
//	Rgba m_backgroundColor;
	std::string m_name;
	EntityID m_entityID;
	EntityID m_savedID;
	int m_health; //Float for use as an expiration timer -= deltaSeconds.
	int m_maxHealth;
	bool m_isCurrentlySeen;
	bool m_hasBeenSeenBefore;


private:
	static EntityID s_BASE_ENTITY_ID; //Incremented and given to instances in .cpp.
	static const int s_BASE_HEALTH;
};