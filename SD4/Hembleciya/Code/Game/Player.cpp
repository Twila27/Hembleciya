#include "Game/Player.hpp"
#include "Game/Map.hpp"
#include "Engine/Input/TheInput.hpp"
#include "Game/FieldOfView/FieldOfView.hpp"
#include "Game/Pathfinding/Pathfinder.hpp"
#include "Engine/Core/TheConsole.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Audio/TheAudio.hpp"
#include "Game/CombatSystem.hpp"
#include "Game/Items/Item.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC const Interval<int> Player::s_PLAYER_BASE_DAMAGE_RANGE = Interval<int>( 5, 8 );


//--------------------------------------------------------------------------------------------------------------
bool Player::ProcessInput()
{
	bool didFindInput = false;

	m_nextAction = PLAYER_ACTION_UNSPECIFIED;

	bool pressedLeft, pressedRight, pressedUp, pressedDown;
	didFindInput = ProcessMovementKeyboard( pressedLeft, pressedRight, pressedUp, pressedDown );

	bool pressedUsePotion = g_theInput->WasKeyPressedOnce( KEY_TO_USE_POTION );
	bool pressedToggleFeature = g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_FEATURE );
	bool pressedRest = g_theInput->WasKeyPressedOnce( KEY_TO_REST );

	ROADMAP( "Use bool[] instead!" );
	if ( PredictNextAction( pressedLeft, pressedRight, pressedUp, pressedDown, 
							pressedUsePotion, pressedToggleFeature, pressedRest ) )
		didFindInput = true;


	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TEST_PATHING_ONESTEP ) || g_theInput->IsKeyDown( KEY_TO_TEST_PATHING_MULTISTEP ) )
	{
		didFindInput = true;

		if ( m_currentPath == nullptr || m_currentPath->m_currentActiveNode->m_position == GetPositionMins() )
		{
			m_currentPath = PathFactory::StartPathfinding( m_currentPath, m_map, m_positionBounds->mins, m_map->GetRandomMapPosition( false ), m_traversalProperties );
			m_currentPath->Pathfind();
			m_currentPath->GetNextPositionAlongPath(); //Burn through "moving" to the first start node that doesn't move, we're already there.
		}
		else if ( m_currentPath->m_currentActiveNode != nullptr )
		{
			m_goalDirection = GetDirectionBetweenMapPositions( GetPositionMins(), m_currentPath->GetNextPositionAlongPath() );
			m_nextAction = PLAYER_ACTION_MOVE;
		}
	}

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_GODMODE ) )
	{
		m_isInvincible = !m_isInvincible;
		g_theConsole->Printf( "Invincibility toggled %s.", m_isInvincible ? "on" : "off" );
		g_theConsole->ShowConsole();
	}
	if ( g_theInput->WasKeyPressedOnce( VK_PAGEUP ) )
		g_theConsole->RaiseShownLines();
	if ( g_theInput->WasKeyPressedOnce( VK_PAGEDOWN ) )
		g_theConsole->LowerShownLines();

	//Note these actions here also don't cost a turn though directly play-related, since the inventory has no UI.
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_PICK_UP_ITEM ) )
		PickUpItemAndAutoEquip();
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_DROP_FIRST_UNEQUIPPED_ITEM ) )
		DropFirstUnequippedItem();

	if ( didFindInput )
		g_theConsole->HideConsole();

	return didFindInput;
}


//--------------------------------------------------------------------------------------------------------------
bool Player::ProcessMovementKeyboard( bool& out_pressedLeft, bool& out_pressedRight, bool& out_pressedUp, bool& out_pressedDown )
{
	out_pressedLeft =
		g_theInput->WasKeyPressedOnce( KEY_TO_MOVE_LEFT ) ||
		g_theInput->WasKeyPressedOnce( KEY_TO_MOVE_LEFT2 ) ||
		g_theInput->WasKeyPressedOnce( KEY_TO_MOVE_LEFT3 );
	out_pressedRight =
		g_theInput->WasKeyPressedOnce( KEY_TO_MOVE_RIGHT ) ||
		g_theInput->WasKeyPressedOnce( KEY_TO_MOVE_RIGHT2 ) ||
		g_theInput->WasKeyPressedOnce( KEY_TO_MOVE_RIGHT3 );
	out_pressedUp =
		g_theInput->WasKeyPressedOnce( KEY_TO_MOVE_UP ) ||
		g_theInput->WasKeyPressedOnce( KEY_TO_MOVE_UP2 ) ||
		g_theInput->WasKeyPressedOnce( KEY_TO_MOVE_UP3 );
	out_pressedDown =
		g_theInput->WasKeyPressedOnce( KEY_TO_MOVE_DOWN ) ||
		g_theInput->WasKeyPressedOnce( KEY_TO_MOVE_DOWN2 ) ||
		g_theInput->WasKeyPressedOnce( KEY_TO_MOVE_DOWN3 );

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_MOVE_UP_LEFT ) ||
		 g_theInput->WasKeyPressedOnce( KEY_TO_MOVE_UP_LEFT2 ) )
	{
		out_pressedUp = true;
		out_pressedLeft = true;
	}
	else if ( g_theInput->WasKeyPressedOnce( KEY_TO_MOVE_UP_RIGHT ) ||
		 g_theInput->WasKeyPressedOnce( KEY_TO_MOVE_UP_RIGHT2 ) )
	{
		out_pressedUp = true;
		out_pressedRight = true;
	}

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_MOVE_DOWN_LEFT ) ||
		 g_theInput->WasKeyPressedOnce( KEY_TO_MOVE_DOWN_LEFT2 ) )
	{
		out_pressedDown = true;
		out_pressedLeft = true;
	}
	else if ( g_theInput->WasKeyPressedOnce( KEY_TO_MOVE_DOWN_RIGHT ) ||
		 g_theInput->WasKeyPressedOnce( KEY_TO_MOVE_DOWN_RIGHT2 ) )
	{
		out_pressedDown = true;
		out_pressedRight = true;
	}


	return ( out_pressedDown || out_pressedLeft || out_pressedRight || out_pressedUp );
}


//--------------------------------------------------------------------------------------------------------------
bool Player::PredictNextAction( bool pressedLeft, bool pressedRight, bool pressedUp, bool pressedDown, 
								bool pressedUsePotion, bool pressedToggleFeature, bool pressedRest )
{
	if ( pressedLeft )
	{
		m_nextAction = PLAYER_ACTION_MOVE;
		m_goalDirection = DIRECTION_LEFT;
	}
	if ( pressedRight )
	{
		m_nextAction = PLAYER_ACTION_MOVE;
		m_goalDirection = DIRECTION_RIGHT;
	}
	if ( pressedUp )
	{
		m_nextAction = PLAYER_ACTION_MOVE;
		m_goalDirection = DIRECTION_UP;
	}
	if ( pressedDown )
	{
		m_nextAction = PLAYER_ACTION_MOVE;
		m_goalDirection = DIRECTION_DOWN;
	}

	static const bool considerDiagonals = true;
	if ( considerDiagonals )
	{
		if ( pressedUp && pressedLeft )
		{
			m_nextAction = PLAYER_ACTION_MOVE;
			m_goalDirection = DIRECTION_UP_LEFT;
		}
		if ( pressedUp && pressedRight )
		{
			m_nextAction = PLAYER_ACTION_MOVE;
			m_goalDirection = DIRECTION_UP_RIGHT;
		}
		if ( pressedDown && pressedLeft )
		{
			m_nextAction = PLAYER_ACTION_MOVE;
			m_goalDirection = DIRECTION_DOWN_LEFT;
		}
		if ( pressedDown && pressedRight )
		{
			m_nextAction = PLAYER_ACTION_MOVE;
			m_goalDirection = DIRECTION_DOWN_RIGHT;
		}
	}

	if ( pressedToggleFeature )
		m_nextAction = PLAYER_ACTION_TOGGLE_FEATURE;

	if ( pressedUsePotion )
		m_nextAction = PLAYER_ACTION_USE_ITEM;

	if ( pressedRest )
		m_nextAction = PLAYER_ACTION_REST;

	return ( m_nextAction != PLAYER_ACTION_UNSPECIFIED );
}


//--------------------------------------------------------------------------------------------------------------
void Player::CheckForItems()
{
	Cell& cell = m_map->GetCellForPosition( GetPositionMins() );
	if ( cell.HasItems() )
	{
		for ( const Item* item : cell.GetItems() )
		{
			switch ( item->GetItemType() )
			{
			case ITEM_TYPE_WEAPON:
				g_theConsole->Printf( "You find a %s (+%d) here.", item->GetName().c_str(), item->GetWeaponDamage() );
				break;
			case ITEM_TYPE_ARMOR:
				g_theConsole->Printf( "You find a %s (+%d) here.", item->GetName().c_str(), item->GetArmorDefense() );
				break;
			case ITEM_TYPE_POTION:
				g_theConsole->Printf( "You find a %s here.", item->GetName().c_str() );
				break;
			default:
				break;
			}
		}

		g_theConsole->ShowConsole();
	}
}


//--------------------------------------------------------------------------------------------------------------
CooldownSeconds Player::Update( float deltaSeconds )
{
	++m_numTurnsTaken;

	const int NUM_PLAYER_MOVE_SOUNDS = 4;
	static SoundID stepSoundIDs[ NUM_PLAYER_MOVE_SOUNDS ] = 
	{
		g_theAudio->CreateOrGetSound( "Data/Audio/PlayerWalk1.wav" ),
		g_theAudio->CreateOrGetSound( "Data/Audio/PlayerWalk2.wav" ),
		g_theAudio->CreateOrGetSound( "Data/Audio/PlayerWalk3.wav" ),
		g_theAudio->CreateOrGetSound( "Data/Audio/PlayerWalk4.wav" ),
	};

	CooldownSeconds secondsUntilNextTurn = 0.f;

	UNREFERENCED( deltaSeconds ); //How to make frame independent, then?
	
	MapPosition goalPosition;
	Vector2i positionDelta = Vector2i::ZERO;

	switch ( m_nextAction )
	{
		case PLAYER_ACTION_UNSPECIFIED: secondsUntilNextTurn = DEFAULT_TURN_COOLDOWN; break;
		case PLAYER_ACTION_MOVE:
		{
			goalPosition = m_positionBounds->mins;
			positionDelta = GetPositionDeltaForMapDirection( m_goalDirection );
			goalPosition += positionDelta;

			if ( TestOneStep( goalPosition ) )
			{
				MoveOneStep( positionDelta );
				g_theAudio->PlaySound( stepSoundIDs[ GetRandomIntInRange( 0, NUM_PLAYER_MOVE_SOUNDS - 1 ) ], VOLUME_MULTIPLIER * .25f );
				//Later could check on current cell type to play a corresponding sound in a subfunction.
				CheckForItems();
			}
			else if ( m_map->IsPositionOnMap( goalPosition ) ) //Did we hit an NPC?
			{
				Cell& goalCell = m_map->GetCellForPosition( goalPosition );
				if ( goalCell.IsOccupiedByAgent() )
				{
					AttackData attackData;
					Agent* target = goalCell.GetAgent();

					attackData.baseDamage = s_PLAYER_BASE_DAMAGE_RANGE.GetRandomElement();
					attackData.chanceToHit = .75f;
					attackData.instigator = this;
					attackData.target = target;
					attackData.instigatorWeapon = GetBestOfEquippedItemType( ITEM_TYPE_WEAPON );
					attackData.targetArmor = target->GetBestOfEquippedItemType( ITEM_TYPE_ARMOR );

					CombatSystem::PerformAttack( attackData );
				}
			}

			secondsUntilNextTurn = DEFAULT_TURN_COOLDOWN;
			break;
		}
		case PLAYER_ACTION_REST:
		{
			if ( AddHealthDelta( 1 ) )
			{
				g_theConsole->Printf( "You gain +1 health from resting." );
				g_theConsole->ShowConsole();
			}
			secondsUntilNextTurn = DEFAULT_TURN_COOLDOWN * 2.f;
			break;
		}
		case PLAYER_ACTION_USE_ITEM:
		{
			bool usedPotion = UseLastAcquiredPotion();
			if ( !usedPotion )
			{
				g_theConsole->Printf( "No potions to use in inventory." );
				g_theConsole->ShowConsole();
			}
			secondsUntilNextTurn = DEFAULT_TURN_COOLDOWN;
			break;
		}
		case PLAYER_ACTION_TOGGLE_FEATURE:
		{
			ToggleAdjacentFeatures();
			secondsUntilNextTurn = DEFAULT_TURN_COOLDOWN;
			break;
		}
		default: DebuggerPrintf( "Unsupported Player Action in Player::Update!" ); break;
	}

	UpdateFieldOfView();

	Agent::Update( deltaSeconds );

	return secondsUntilNextTurn;
}


//--------------------------------------------------------------------------------------------------------------
void Player::WriteToXMLNode( XMLNode& out_entityDataNode )
{
	XMLNode playerNode = out_entityDataNode.addChild( "PlayerBlueprint" );

	Agent::WriteToXMLNode( playerNode );

	WriteXMLAttribute( playerNode, "turnsTaken", m_numTurnsTaken, 0 );
	WriteXMLAttribute( playerNode, "isInvincible", m_isInvincible ? 1 : 0, 0 );
}
