#include "Game/CombatSystem.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Agent.hpp"
#include "Game/Items/Item.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Audio/TheAudio.hpp"
#include "Game/FactionSystem.hpp"
#include "Engine/Core/TheConsole.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC void CombatSystem::PlayAttackSound( Agent* instigator )
{
	static SoundID enemyAttackSoundID = g_theAudio->CreateOrGetSound( "Data/Audio/EnemyAttack.wav" );
	static SoundID playerAttackSoundID = g_theAudio->CreateOrGetSound( "Data/Audio/PlayerAttack.wav" );

	if ( instigator->IsPlayer() )
		g_theAudio->PlaySound( playerAttackSoundID, VOLUME_MULTIPLIER * .5f );
	else
		g_theAudio->PlaySound( enemyAttackSoundID, VOLUME_MULTIPLIER * .5f );

	g_theConsole->ShowConsole();
}


//--------------------------------------------------------------------------------------------------------------
STATIC void CombatSystem::PlayHurtSound( Agent* victim )
{
	const int NUM_PLAYER_HURT_SOUNDS = 4;
	const int NUM_NPC_HURT_SOUNDS = 3;

	static SoundID playerHurtSoundIDs[ NUM_PLAYER_HURT_SOUNDS ] = {
		g_theAudio->CreateOrGetSound( "Data/Audio/PlayerHurt1.wav" ),
		g_theAudio->CreateOrGetSound( "Data/Audio/PlayerHurt2.wav" ),
		g_theAudio->CreateOrGetSound( "Data/Audio/PlayerHurt3.wav" ),
		g_theAudio->CreateOrGetSound( "Data/Audio/PlayerHurt4.wav" )
	};

	static SoundID npcHurtSoundIDs[ NUM_NPC_HURT_SOUNDS ] = {
		g_theAudio->CreateOrGetSound( "Data/Audio/Hurt1.wav" ),
		g_theAudio->CreateOrGetSound( "Data/Audio/Hurt2.wav" ),
		g_theAudio->CreateOrGetSound( "Data/Audio/Hurt3.wav" )
	};


	if ( victim->IsPlayer() )
		g_theAudio->PlaySound( playerHurtSoundIDs[ GetRandomIntInRange( 0, NUM_PLAYER_HURT_SOUNDS-1 ) ], VOLUME_MULTIPLIER );
	else
		g_theAudio->PlaySound( npcHurtSoundIDs[ GetRandomIntInRange( 0, NUM_NPC_HURT_SOUNDS-1 ) ], VOLUME_MULTIPLIER );

	g_theConsole->ShowConsole();
}

//--------------------------------------------------------------------------------------------------------------
STATIC void CombatSystem::PerformAttack( AttackData& attackData )
{
	if ( GetRandomChance( attackData.chanceToHit ) == false )
	{
		if ( attackData.target->IsCurrentlySeen() || attackData.instigator->IsCurrentlySeen() )
			if ( attackData.instigator->IsPlayer() )
				g_theConsole->Printf( "Your attack missed %s!", attackData.target->GetName().c_str() );
			else
				g_theConsole->Printf( "The attack by %s missed!", attackData.instigator->GetName().c_str() );
		attackData.didAttackHit = false;
		attackData.damageDealt = 0;
		return;
	}

	if ( attackData.target->IsPlayer() && attackData.target->IsInvincible() )
	{
		g_theConsole->Printf( "The attack by %s bounces off you like it was nothing.",
							  attackData.instigator->GetName().c_str() );
		g_theConsole->ShowConsole();

		attackData.didAttackHit = false;
		return;
	}
	else attackData.didAttackHit = true;

	int weaponDamage = 0;
	if ( attackData.instigatorWeapon != nullptr )
		weaponDamage = attackData.instigatorWeapon->GetWeaponDamage();

	int armorDefense = 0;
	if ( attackData.targetArmor != nullptr )
		armorDefense = attackData.targetArmor->GetArmorDefense();

	//Damage equation:
	attackData.damageDealt =
		static_cast<int>( ( attackData.baseDamage + weaponDamage ) * GetRandomFloatInRange( .75f, 1.f ) )
		- armorDefense
		+ attackData.instigator->GetDamageBonus();
	if ( attackData.damageDealt < 0 )
		attackData.damageDealt = 0;


	//Roll for surviving lethal strike. Always give player one last shot.
	//Motivation: can specify a behavior triggering below 10% health even for a 2 HP enemy.
	bool wouldAttackBeFatal = ( attackData.target->GetHealth() - attackData.damageDealt ) <= 0;
	bool isPlayerAndHealthy = attackData.target->IsPlayer() && ( attackData.target->GetHealth() > 1 );
	if ( wouldAttackBeFatal && ( isPlayerAndHealthy || GetRandomChance( .1f ) ) )
	{
		attackData.damageDealt = static_cast<int>( attackData.target->GetHealth() ) - 1;
		if ( isPlayerAndHealthy || attackData.target->IsCurrentlySeen() )
			g_theConsole->Printf( "Fatal attack survived, 1 health left!" );
	}

	//Actually apply damage.
	attackData.target->SubtractHealthDelta( attackData.damageDealt );

	//Handle outcome.
	ReportAttackResult( attackData );
	//attackData.instigator->AdjustFactionStatus(); //Do we do one for the instigator?
	attackData.target->AdjustFactionStatus( attackData.instigator, FACTION_ACTION_ATTACKS );

	if ( !attackData.target->IsAlive() ) //Note death sound/message displayed in TheGame::DestroyDeadGameplayEntities().
		attackData.instigator->AddKill();
}


//--------------------------------------------------------------------------------------------------------------
void CombatSystem::ReportAttackResult( AttackData &attackData )
{
	//Note the player is not marked visible.
	if ( attackData.instigator->IsCurrentlySeen() && attackData.target->IsCurrentlySeen() )
	{
		g_theConsole->Printf( "%s hits %s for %d damage!",
								  attackData.instigator->GetName().c_str(),
								  attackData.target->GetName().c_str(),
								  attackData.damageDealt );

		PlayAttackSound( attackData.instigator );
		PlayHurtSound( attackData.target );
	}
	else //Handle player or only-saw-one-side cases.
	{
		if ( attackData.instigator->IsCurrentlySeen() || attackData.instigator->IsPlayer() )
		{
			if ( attackData.instigator->IsPlayer() )
			{
				g_theConsole->Printf( "You hit %s for %d damage!",
												  attackData.target->GetName().c_str(),
												  attackData.damageDealt );
			}
			else if ( !attackData.target->IsPlayer() ) //Player message handled below.
			{
				g_theConsole->Printf( "%s hits for %d damage!",
													attackData.instigator->GetName().c_str(),
													attackData.damageDealt );
			}
			PlayAttackSound( attackData.instigator );
		}
		if ( attackData.target->IsCurrentlySeen() || attackData.target->IsPlayer() )
		{
			if ( attackData.target->IsPlayer() )
			{
				g_theConsole->Printf( "You were hit by %s for %d damage!",
									  attackData.instigator->GetName().c_str(),
									  attackData.damageDealt );
			}
			else if ( !attackData.instigator->IsPlayer() ) //Player message handled above.
			{
				g_theConsole->Printf( "%s was hit for %d damage!",
											attackData.target->GetName().c_str(),
											attackData.damageDealt );
			}
			PlayHurtSound( attackData.target );
		}
		else //Neither seen, do not show console.
		{

		}
	}
}
