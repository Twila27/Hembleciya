#pragma once

class Agent;
class Item;


//--------------------------------------------------------------------------------------------------------------
struct AttackData
{
	AttackData() 
		: chanceToHit( 1.f )
	{
		target = instigator = nullptr;
		instigatorWeapon = targetArmor = nullptr;
	}

	int baseDamage;
	float chanceToHit;
	Agent* target;
	Agent* instigator;
	Item* instigatorWeapon;
	Item* targetArmor;

	bool didAttackHit;
	int damageDealt;
	//Reason reason; //Enum result of outcome: REASON_TARGET_MISSED, REASON_TARGET_IMMUNE, REASON_TARGET_BLOCKED/ARMORED etc.
};


//--------------------------------------------------------------------------------------------------------------
class CombatSystem
{
public:
	static void PerformAttack( AttackData& attackData );
	static void ReportAttackResult( AttackData &attackData );
	static void PlayAttackSound( Agent* instigator );
	static void PlayHurtSound( Agent* victim );
};