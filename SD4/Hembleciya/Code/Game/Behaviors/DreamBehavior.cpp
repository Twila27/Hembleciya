#include "Game/Behaviors/DreamBehavior.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Agent.hpp"
#include "Game/Pathfinding/Pathfinder.hpp"
#include "Game/Map.hpp"
#include "Engine/Audio/TheAudio.hpp"
#include "Engine/Core/TheConsole.hpp"


//--------------------------------------------------------------------------------------------------------------
BehaviorRegistration DreamBehavior::s_DreamBehavior = BehaviorRegistration( "Dream", DreamBehavior::CreateDreamBehavior );
STATIC const std::string	DreamBehavior::s_DEFAULT_DREAM_NAME = "Unnamed Dream";
STATIC const Rgba			DreamBehavior::s_DEFAULT_DREAM_TINT = Rgba();
STATIC const float			DreamBehavior::s_DEFAULT_MAX_HEALTH_FRACTION_NEEDED_TO_ACTIVATE = .25f;


//--------------------------------------------------------------------------------------------------------------
DreamBehavior::DreamBehavior( const XMLNode& behaviorNode )
	: Behavior( behaviorNode )
	, m_isDreaming( false )
	, m_dreamMap( nullptr )
	, m_dreamTint( s_DEFAULT_DREAM_TINT )
	, m_maxHealthFractionNeededToActivate( s_DEFAULT_MAX_HEALTH_FRACTION_NEEDED_TO_ACTIVATE )
	, m_dreamMapSpawnPositionInRealMap( MapPosition::ZERO )
{
	const char* mapString = behaviorNode.getText();

	std::string dreamName = s_DEFAULT_DREAM_NAME;
	dreamName = ReadXMLAttribute( behaviorNode, "name", dreamName );
	m_maxHealthFractionNeededToActivate = ReadXMLAttribute( behaviorNode, "maxHealthFractionNeededToActivate", m_maxHealthFractionNeededToActivate );

	m_isDreaming = ( ReadXMLAttribute( behaviorNode, "isDreaming", 0 ) > 0 ) ? true : false;
	m_dreamMap = Map::CreateFromTileDataStringWithNewlines( mapString, dreamName );
	m_dreamMapSpawnPositionInRealMap = ReadXMLAttribute( behaviorNode, "dreamMapSpawnPositionInRealMap", MapPosition::ZERO );

	//Recolorize cells. Maybe make more robust later?
	std::string colorString = s_DEFAULT_DREAM_TINT.ToString();
	colorString = ReadXMLAttribute( behaviorNode, "color", colorString );
	sscanf_s( colorString.c_str(), "%hhu,%hhu,%hhu", &m_dreamTint.red, &m_dreamTint.green, &m_dreamTint.blue );

	if ( !m_isDreaming )
	{
		std::vector< Cell >& dreamCells = m_dreamMap->GetCells();
		for ( Cell& dreamCell : dreamCells )
		{
			if ( dreamCell.m_cellType == CELL_TYPE_AIR )
				dreamCell.m_color = Rgba::BLACK;
			else
			{
				Vector4i modulation = Vector4i( GetRandomIntLessThan( 256 ), 255 );
				dreamCell.m_color = m_dreamTint * Rgba( modulation );
			}
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
DreamBehavior::DreamBehavior( const DreamBehavior& other ) //Has a pointer, needs deep copy.
	: Behavior( other )
	, m_dreamMap( new Map( *other.m_dreamMap ) )
	, m_dreamTint( other.m_dreamTint )
	, m_isDreaming( other.m_isDreaming )
	, m_maxHealthFractionNeededToActivate( other.m_maxHealthFractionNeededToActivate )
	, m_dreamMapSpawnPositionInRealMap( other.m_dreamMapSpawnPositionInRealMap )
{
}


//--------------------------------------------------------------------------------------------------------------
inline void DreamBehavior::operator=( const Behavior& copyFrom )
{
	Behavior::operator=( copyFrom );

	const Behavior* asPointer = &copyFrom;
	const DreamBehavior* other = dynamic_cast<const DreamBehavior*>( asPointer );

	m_dreamMap = new Map( *other->m_dreamMap );
	m_dreamTint = other->m_dreamTint;
	m_isDreaming = other->m_isDreaming;
	m_maxHealthFractionNeededToActivate = other->m_maxHealthFractionNeededToActivate;
	m_dreamMapSpawnPositionInRealMap = other->m_dreamMapSpawnPositionInRealMap;
}


//--------------------------------------------------------------------------------------------------------------
DreamBehavior::~DreamBehavior()
{
	if ( m_isDreaming && m_agent != nullptr )
	{
		if ( !m_agent->IsAlive() )
		{
			m_isDreaming = false;
			OverwriteMap(); //Agent has died, restore world.
			TODO( "Drop Corresponding Item @ m_agent's Pos, Read ItemFactory Name From XML!" );
		}
	}
	if ( m_dreamMap != nullptr )
	{
		delete m_dreamMap;
		m_dreamMap = nullptr;
	}
}


//--------------------------------------------------------------------------------------------------------------
UtilityValue DreamBehavior::CalcUtility()
{
	if ( m_isDreaming && m_agent->IsAlive() )
		return NO_UTILITY_VALUE; //No need to overwrite again until dead.

	const Agent* targetEnemy = m_agent->GetTargetEnemy();

	if ( targetEnemy == nullptr || targetEnemy->IsPlayer() == false )
		return NO_UTILITY_VALUE;

	Vector2i displacement = m_agent->GetTargetEnemy()->GetPositionMins() - m_agent->GetPositionMins();
	Vector2i myAgentDreamMapSpawnPos = GetSpawnInDreamMap();
	Vector2i playerDreamMapPos = myAgentDreamMapSpawnPos + displacement;
	if ( !m_dreamMap->IsPositionOnMap( playerDreamMapPos ) )
		return NO_UTILITY_VALUE; //Player not yet in range.

	if ( m_dreamMap->IsSolidAtPosition( playerDreamMapPos ) )
		return NO_UTILITY_VALUE; //Don't spawn player in on a wall.

	//By this point we know we have the player in range and can bring the dream map around them without getting them stuck.
	//	if ( m_agent->GetHealth() > ( m_agent->GetMaxHealth() * m_maxHealthFractionNeededToActivate ) )
	//		return 0.f; //Agent still too healthy. Letting it take damage first makes it slightly likelier the player's in close.

	//First, at least for now--if we're starting a swap--test to be sure a dream doesn't already exist there.
	
	//-----------------------------------------------------------------------------
	//Check for other dreams already happening around the NPC.
	//If one exists, don't create a new one. 
	//(Breaks the swapping/saving systems to have 2+ dreams going at once).

	Map* map = m_agent->GetMap();
	Vector2i spawnInDreamMap = GetSpawnInDreamMap();
	for ( Cell& dreamCell : m_dreamMap->GetCells() )
	{
		Vector2i displacementInDreamAndRealMap = dreamCell.m_position - spawnInDreamMap;
		Vector2i dreamCellPositionInRealMap = m_dreamMapSpawnPositionInRealMap + displacementInDreamAndRealMap;

		if ( !map->IsPositionOnMap( dreamCellPositionInRealMap ) //Will be on map for hidden ones, hence below check.
			 || map->GetCellForPosition( dreamCellPositionInRealMap ).m_isHidden )
			continue;

		Cell& realCell = map->GetCellForPosition( dreamCellPositionInRealMap );
		if ( realCell.m_cellType == CELL_TYPE_DREAM )
			return NO_UTILITY_VALUE; //Stop before we make a big mistake swapping maps with another dream.
	}

	return MIN_UTILITY_VALUE + HIGH_UTILITY_VALUE;
}


//--------------------------------------------------------------------------------------------------------------
CooldownSeconds DreamBehavior::Run()
{
	m_isDreaming = true;
	m_isDreaming = OverwriteMap(); //May return false if the dream doesn't actually occur.
		//Otherwise CalcUtility's one-shot use of isDreaming wouldn't allow a retry if we fail to overwrite (e.g. already dream there).

	if ( m_isDreaming )
	{
		g_theConsole->SetTextColor( Rgba::YELLOW );
		g_theConsole->Printf( "%s draws you into its dream: \"%s\"!",
							  m_agent->GetName().c_str(),
							  m_dreamMap->GetMapName().c_str() );
		g_theConsole->SetTextColor();
		g_theConsole->ShowConsole();
	}

	return DEFAULT_TURN_COOLDOWN * 2.f;
}


//--------------------------------------------------------------------------------------------------------------
bool DreamBehavior::OverwriteMap()
{
	static SoundID dreamOpenedSoundID = g_theAudio->CreateOrGetSound( "Data/Audio/DreamOpened.wav" );
	static SoundID dreamBrokenSoundID = g_theAudio->CreateOrGetSound( "Data/Audio/DreamBroken.wav" );

	if ( m_isDreaming )
		m_dreamMapSpawnPositionInRealMap = m_agent->GetPositionMins(); //Where the agent was when the dream behavior ran the first time.

	int numItemsLostToDream = 0;
	Cell tempCell( -Vector2i::ONE );
	Map* map = m_agent->GetMap();
	Vector2i spawnInDreamMap = GetSpawnInDreamMap();

	//First, at least for now--if we're starting a swap--test to be sure a dream doesn't already exist there.
	if ( m_isDreaming )
	{
		for ( Cell& dreamCell : m_dreamMap->GetCells() )
		{
			Vector2i displacementInDreamAndRealMap = dreamCell.m_position - spawnInDreamMap;
			Vector2i dreamCellPositionInRealMap = m_dreamMapSpawnPositionInRealMap + displacementInDreamAndRealMap;

			if ( !map->IsPositionOnMap( dreamCellPositionInRealMap ) //Will be on map for hidden ones, hence below check.
				 || map->GetCellForPosition( dreamCellPositionInRealMap ).m_isHidden )
				continue;

			Cell& realCell = map->GetCellForPosition( dreamCellPositionInRealMap );
			if ( realCell.m_cellType == CELL_TYPE_DREAM )
				return false; //Stop before we make a big mistake swapping maps with another dream.
		}
	}

	//Actual overwriting loop.
	for ( Cell& dreamCell : m_dreamMap->GetCells() )
	{
		Vector2i displacementInDreamAndRealMap = dreamCell.m_position - spawnInDreamMap;
		Vector2i dreamCellPositionInRealMap = m_dreamMapSpawnPositionInRealMap + displacementInDreamAndRealMap;

		if ( !map->IsPositionOnMap( dreamCellPositionInRealMap ) //Will be on map for hidden ones, hence below check.
			 || map->GetCellForPosition( dreamCellPositionInRealMap ).m_isHidden )
			continue;

		Cell& realCell = map->GetCellForPosition( dreamCellPositionInRealMap );

		if ( !m_isDreaming ) //Agent dead, all items swept up by the dream vanish.
		{
			while ( realCell.HasItems() )
			{
				realCell.TakeItemFromCell();
				++numItemsLostToDream;
			}
		}

		tempCell = realCell;
		if ( m_isDreaming )
			tempCell.m_parsedMapGlyph = GetGlyphForCellType( realCell.m_cellType ); //If we save while a dream is laid down, we want to write the correct value out.
		tempCell.m_position = dreamCell.m_position; //Need it like this so that, when we break the dream, same math above works.
		realCell.m_cellType = dreamCell.m_cellType;
		realCell.m_color = dreamCell.m_color;
		realCell.m_parsedMapGlyph = dreamCell.m_parsedMapGlyph;
		realCell.m_occupyingFeature = dreamCell.m_occupyingFeature; //?
		dreamCell = tempCell;
	}

	if ( numItemsLostToDream > 0 )
	{
		g_theConsole->Printf( "The shattered dream took away %d items with it!", numItemsLostToDream );
		g_theConsole->ShowConsole();
	}

	if ( m_isDreaming ) //NOTE: True on initial call, false on agent death.
		g_theAudio->PlaySound( dreamOpenedSoundID, VOLUME_MULTIPLIER );
	else
		g_theAudio->PlaySound( dreamBrokenSoundID, VOLUME_MULTIPLIER * .5f );

	return m_isDreaming;
}

//--------------------------------------------------------------------------------------------------------------
Vector2i DreamBehavior::GetSpawnInDreamMap() const
{
	TODO( "Substitute this for an actual given (S)tart tile." );
		//At that point, just return that location instead of this temp one.
	
	return m_dreamMap->GetDimensions() / 2; 
}


//--------------------------------------------------------------------------------------------------------------
void DreamBehavior::WriteToXMLNode( XMLNode& behaviorsNode )
{
	Behavior::WriteToXMLNode( behaviorsNode );
	XMLNode behaviorNode = behaviorsNode.getChildNode( m_name.c_str() );

	WriteXMLAttribute( behaviorNode, "name", m_dreamMap->GetMapName(), s_DEFAULT_DREAM_NAME );
	WriteXMLAttribute( behaviorNode, "isDreaming", m_isDreaming ? 1 : 0, 0 );
	WriteXMLAttribute( behaviorNode, "dreamMapSpawnPositionInRealMap", m_dreamMapSpawnPositionInRealMap, MapPosition::ZERO );
	WriteXMLAttribute( behaviorNode, "maxHealthFractionNeededToActivate", m_maxHealthFractionNeededToActivate, s_DEFAULT_MAX_HEALTH_FRACTION_NEEDED_TO_ACTIVATE );
	WriteXMLAttribute( behaviorNode, "color", m_dreamTint, s_DEFAULT_DREAM_TINT );
	std::string dreamMapAsString = Map::GetAsString( m_dreamMap, true, 5 );
	behaviorNode.addText( dreamMapAsString.c_str() );
}