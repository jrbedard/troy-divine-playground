//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: This is the soldier version of the combine, analogous to the HL1 grunt.
//
//=============================================================================//

#include "cbase.h"
#include "ai_hull.h"
#include "ai_motor.h"
#include "tdp_npc_combines.h"
#include "bitstring.h"
#include "engine/IEngineSound.h"
#include "soundent.h"
#include "ndebugoverlay.h"
#include "npcevent.h"
#include "tdp_player.h"
#include "game.h"
#include "ammodef.h"
#include "explode.h"
#include "ai_memory.h"
#include "Sprite.h"
#include "soundenvelope.h"
//#include "weapon_physcannon.h"
#include "tdp_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar	sk_combine_s_health( "sk_combine_s_health","0");
ConVar	sk_combine_s_kick( "sk_combine_s_kick","0");

ConVar sk_combine_guard_health( "sk_combine_guard_health", "0");
ConVar sk_combine_guard_kick( "sk_combine_guard_kick", "0");
 
// Whether or not the combine guard should spawn health on death
ConVar combine_guard_spawn_health( "combine_guard_spawn_health", "1" );

ConVar	sk_plr_dmg_buckshot		( "sk_plr_dmg_buckshot","0", FCVAR_REPLICATED);	
ConVar	sk_plr_num_shotgun_pellets( "sk_plr_num_shotgun_pellets","7", FCVAR_REPLICATED);


//Whether or not the combine should spawn health on death
ConVar	combine_spawn_health( "combine_spawn_health", "1" );

LINK_ENTITY_TO_CLASS( tdp_npc_combine_s, CTDP_NPC_CombineS );


#define AE_SOLDIER_BLOCK_PHYSICS		20 // trying to block an incoming physics object


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTDP_NPC_CombineS::Spawn( void )
{
	Precache();
	SetModel( STRING( GetModelName() ) );

	if( IsElite() )
	{
		// Stronger, tougher.
		SetHealth( sk_combine_guard_health.GetFloat() );
		SetMaxHealth( sk_combine_guard_health.GetFloat() );
		SetKickDamage( sk_combine_guard_kick.GetFloat() );
	}
	else
	{
		SetHealth( sk_combine_s_health.GetFloat() );
		SetMaxHealth( sk_combine_s_health.GetFloat() );
		SetKickDamage( sk_combine_s_kick.GetFloat() );
	}

	CapabilitiesAdd( bits_CAP_ANIMATEDFACE );
	CapabilitiesAdd( bits_CAP_MOVE_SHOOT );
	CapabilitiesAdd( bits_CAP_DOORS_GROUP );

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CTDP_NPC_CombineS::Precache()
{
	const char *pModelName = STRING( GetModelName() );

	if( !stricmp( pModelName, "models/combine_super_soldier.mdl" ) )
	{
		m_fIsElite = true;
	}
	else
	{
		m_fIsElite = false;
	}

	if( !GetModelName() )
	{
		SetModelName( MAKE_STRING( "models/combine_soldier.mdl" ) );
	}

	PrecacheModel( STRING( GetModelName() ) );

	UTIL_PrecacheOther( "item_healthvial" );
	UTIL_PrecacheOther( "weapon_frag" );
	UTIL_PrecacheOther( "item_ammo_ar2_altfire" );

	BaseClass::Precache();
}


void CTDP_NPC_CombineS::DeathSound( void )
{
	// NOTE: The response system deals with this at the moment
	if ( GetFlags() & FL_DISSOLVING )
		return;

	GetSentences()->Speak( "COMBINE_DIE", SENTENCE_PRIORITY_INVALID, SENTENCE_CRITERIA_ALWAYS ); 
}


//-----------------------------------------------------------------------------
// Purpose: Soldiers use CAN_RANGE_ATTACK2 to indicate whether they can throw
//			a grenade. Because they check only every half-second or so, this
//			condition must persist until it is updated again by the code
//			that determines whether a grenade can be thrown, so prevent the 
//			base class from clearing it out. (sjb)
//-----------------------------------------------------------------------------
void CTDP_NPC_CombineS::ClearAttackConditions( )
{
	bool fCanRangeAttack2 = HasCondition( COND_CAN_RANGE_ATTACK2 );

	// Call the base class.
	BaseClass::ClearAttackConditions();

	if( fCanRangeAttack2 )
	{
		// We don't allow the base class to clear this condition because we
		// don't sense for it every frame.
		SetCondition( COND_CAN_RANGE_ATTACK2 );
	}
}

void CTDP_NPC_CombineS::PrescheduleThink( void )
{
	/*//FIXME: This doesn't need to be in here, it's all debug info
	if( HasCondition( COND_HEAR_PHYSICS_DANGER ) )
	{
		// Don't react unless we see the item!!
		CSound *pSound = NULL;

		pSound = GetLoudestSoundOfType( SOUND_PHYSICS_DANGER );

		if( pSound )
		{
			if( FInViewCone( pSound->GetSoundReactOrigin() ) )
			{
				DevMsg( "OH CRAP!\n" );
				NDebugOverlay::Line( EyePosition(), pSound->GetSoundReactOrigin(), 0, 0, 255, false, 2.0f );
			}
		}
	}
	*/

	BaseClass::PrescheduleThink();
}

//-----------------------------------------------------------------------------
// Purpose: Allows for modification of the interrupt mask for the current schedule.
//			In the most cases the base implementation should be called first.
//-----------------------------------------------------------------------------
void CTDP_NPC_CombineS::BuildScheduleTestBits( void )
{
	BaseClass::BuildScheduleTestBits();

	//Interrupt any schedule with physics danger (as long as I'm not moving or already trying to block)
	if ( m_flGroundSpeed == 0.0 && !IsCurSchedule( SCHED_FLINCH_PHYSICS ) )
	{
		SetCustomInterruptCondition( COND_HEAR_PHYSICS_DANGER );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CTDP_NPC_CombineS::SelectSchedule ( void )
{
	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float CTDP_NPC_CombineS::GetHitgroupDamageMultiplier( int iHitGroup, const CTakeDamageInfo &info )
{
	switch( iHitGroup )
	{
	case HITGROUP_HEAD:
		{
			// Soldiers take double headshot damage
			return 2.0f;
		}
	}

	return BaseClass::GetHitgroupDamageMultiplier( iHitGroup, info );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CTDP_NPC_CombineS::TakeDamage( const CTakeDamageInfo &info )
{
	if( info.GetInflictor() && info.GetInflictor()->VPhysicsGetObject() )
	{
		// Hit by a physics object! Was I blocking?
		if( m_fIsBlocking )
		{
			IPhysicsObject *pPhysObject;

			pPhysObject = info.GetInflictor()->VPhysicsGetObject();

			if( pPhysObject )
			{
				// Only deflect objects of relatively low mass
				//DevMsg( "MASS: %f\n", pPhysObject->GetMass() );

				if( pPhysObject->GetMass() <= 30.0 )
				{
					// No damage from light objects (tuned for melons)
					return 0;
				}
			}
		}
	}

	BaseClass::TakeDamage( info );
	return 0;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CTDP_NPC_CombineS::HandleAnimEvent( animevent_t *pEvent )
{
	switch( pEvent->event )
	{
	case AE_SOLDIER_BLOCK_PHYSICS:
		DevMsg( "BLOCKING!\n" );
		m_fIsBlocking = true;
		break;

	default:
		BaseClass::HandleAnimEvent( pEvent );
		break;
	}
}

void CTDP_NPC_CombineS::OnChangeActivity( Activity eNewActivity )
{
	// Any new sequence stops us blocking.
	m_fIsBlocking = false;
	BaseClass::OnChangeActivity( eNewActivity );
}

void CTDP_NPC_CombineS::OnListened()
{
	BaseClass::OnListened();

	if ( HasCondition( COND_HEAR_DANGER ) && HasCondition( COND_HEAR_PHYSICS_DANGER ) )
	{
		if ( HasInterruptCondition( COND_HEAR_DANGER ) )
		{
			ClearCondition( COND_HEAR_PHYSICS_DANGER );
		}
	}

	// debugging to find missed schedules
#if 0
	if ( HasCondition( COND_HEAR_DANGER ) && !HasInterruptCondition( COND_HEAR_DANGER ) )
	{
		DevMsg("Ignore danger in %s\n", GetCurSchedule()->GetName() );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
void CTDP_NPC_CombineS::Event_Killed( const CTakeDamageInfo &info )
{
	// Don't bother if we've been told not to, or the player has a megaphyscannon
/*	if ( combine_spawn_health.GetBool() == false || PlayerHasMegaPhysCannon() )
	{
		BaseClass::Event_Killed( info );
		return;
	}*/

	CBasePlayer *pPlayer = ToBasePlayer( info.GetAttacker() );

	if ( pPlayer != NULL )
	{
		// Elites drop alt-fire ammo, so long as they weren't killed by dissolving.
		if( IsElite() )
		{
			CBaseEntity *pItem = DropItem( "item_ammo_ar2_altfire", WorldSpaceCenter()+RandomVector(-4,4), RandomAngle(0,360) );

			if ( pItem )
			{
				IPhysicsObject *pObj = pItem->VPhysicsGetObject();

				if ( pObj )
				{
					Vector			vel		= RandomVector( -64.0f, 64.0f );
					AngularImpulse	angImp	= RandomAngularImpulse( -300.0f, 300.0f );

					vel[2] = 0.0f;
					pObj->AddVelocity( &vel, &angImp );
				}

				if( info.GetDamageType() & DMG_DISSOLVE )
				{
					CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating*>(pItem);

					if( pAnimating )
					{
						pAnimating->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
					}
				}
			}
		}

		//CTDPGameRules *pTDPGameRules = static_cast<CTDPGameRules *>(g_pGameRules);

		// Attempt to drop health
		/*if ( pTDPGameRules->NPC_ShouldDropHealth( pPlayer ) )
		{
			DropItem( "item_healthvial", WorldSpaceCenter()+RandomVector(-4,4), RandomAngle(0,360) );
			pTDPGameRules->NPC_DroppedHealth();
		}*/
		
		// Attempt to drop a grenade
		/*if ( pTDPGameRules->NPC_ShouldDropGrenade( pPlayer ) )
		{
			DropItem( "weapon_frag", WorldSpaceCenter()+RandomVector(-4,4), RandomAngle(0,360) );
			pTDPGameRules->NPC_DroppedGrenade();
		}*/
	}

	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CTDP_NPC_CombineS::IsLightDamage( const CTakeDamageInfo &info )
{
	return BaseClass::IsLightDamage( info );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CTDP_NPC_CombineS::IsHeavyDamage( const CTakeDamageInfo &info )
{
	// Combine considers AR2 fire to be heavy damage
	if ( info.GetAmmoType() == GetAmmoDef()->Index("AR2") )
		return true;

	// 357 rounds are heavy damage
	if ( info.GetAmmoType() == GetAmmoDef()->Index("357") )
		return true;

	// Shotgun blasts where at least half the pellets hit me are heavy damage
	if ( info.GetDamageType() & DMG_BUCKSHOT )
	{
		int iHalfMax = sk_plr_dmg_buckshot.GetFloat() * sk_plr_num_shotgun_pellets.GetInt() * 0.5;
		if ( info.GetDamage() >= iHalfMax )
			return true;
	}

	return BaseClass::IsHeavyDamage( info );
}
