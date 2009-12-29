//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Weapon Implementation - HL2 Standard Crowbar
//
//=====================================================================================//

#include "cbase.h"
#include "tdp_weapon_base.h"
#include "tdp_fx_shared.h"
#include "in_buttons.h"

#if defined( CLIENT_DLL )
	#define CTDPWeaponMaul C_TDPWeaponMaul
	#include "c_tdp_player.h"
#else
	#include "tdp_player.h"
	#include "eventqueue.h"
#endif

#include "tdp_weapon_basebludgeon.h"

// The Crowbar has a longer range than the one defined in our base class
#define	MAUL_RANGE		150.0f
#define MAUL_KICK		7.0f  
#define MAUL_REFIRE		1.0f

//=====================================================================================//
// Purpose: Class definition. Deriving from our new class CBludgeonWeaponSDKBase
//=====================================================================================//
class CTDPWeaponMaul : public CTDPBludgeonWeaponBase
{
public:
	// Necessary definitions for a weapon in the MP SDK
	// Used weapon_mp5.cpp as a starting point
	DECLARE_CLASS( CTDPWeaponMaul, CTDPBludgeonWeaponBase );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	// Constructor. We just initialize m_fMaxRange1 with
	// the Crowbar range
	CTDPWeaponMaul() { m_fMaxRange1	= MAUL_RANGE; }

	TDPWeaponID GetWeaponID() const	{ return WEAPON_MAUL; }   // Definition of WEAPON_CROWBAR in weapon_sdkbase.h

	void AddViewKick();
	float GetFireRate();

	float GetDamageForActivity( Activity hitActivity );

	void ImpactEffect( trace_t &tr );

	void Swing( int bIsSecondary );

	// We define an empty SecondaryAttack() as it's not supported by the Crowbar
	void PrimaryAttack();
	void SecondaryAttack();
	void ThirdAttack();

private:

	// Hide the copy constructor
	CTDPWeaponMaul( const CTDPWeaponMaul & );
};

//=====================================================================================//
// Purpose: Some other necessary definitions. Pretty simple stuff
//			Based again on weapon_mp5.cpp
//=====================================================================================//
IMPLEMENT_NETWORKCLASS_ALIASED( TDPWeaponMaul, DT_TDPWeaponMaul )

BEGIN_NETWORK_TABLE( CTDPWeaponMaul, DT_TDPWeaponMaul )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTDPWeaponMaul )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_maul, CTDPWeaponMaul );
PRECACHE_WEAPON_REGISTER( weapon_maul);


//=====================================================================================//
// Purpose: Shake the player's screen a little
//=====================================================================================//
void CTDPWeaponMaul::AddViewKick()
{
	// Do we have a valid owner holding the weapon?
	CTDPPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	// Just shake the screen using some random numbers so it's
	// different every time
	pPlayer->ViewPunch( QAngle( random->RandomFloat( -MAUL_KICK, MAUL_KICK ),
								random->RandomFloat( -MAUL_KICK, MAUL_KICK ),
								0.0f ) );
	return;
}

//=====================================================================================//
// Purpose: How often can we attack using the weapon
//=====================================================================================//
float CTDPWeaponMaul::GetFireRate()
{
	// Return default value
	return ( MAUL_REFIRE );
}

//=====================================================================================//
// Purpose:
//=====================================================================================//
float CTDPWeaponMaul::GetDamageForActivity( Activity hitActivity )
{
	// The default damage is 100%
	return ( 80.0f );
}

//=====================================================================================//
// Purpose: Swing the weapon. bIsSecondary determines which Attack() function was used
//=====================================================================================//
void CTDPWeaponMaul::Swing( int bIsSecondary )
{

	// Do we have a valid owner holding the weapon?
	CTDPPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	// Get the player's position
	Vector vSwingStart = pPlayer->Weapon_ShootPosition( );
	Vector vForward;
	pPlayer->EyeVectors( &vForward, NULL, NULL );

	// Get the attack end position by using the weapon's range
	Vector vSwingEnd = vSwingStart + vForward * GetRange();

	trace_t tr;
	UTIL_TraceLine( vSwingStart, vSwingEnd, MASK_SHOT_HULL, pPlayer, COLLISION_GROUP_NONE, &tr );

	// Let's get our animation sequence depending on the attack type
	Activity nHitActivity = ( bIsSecondary ) ? GetSecondaryAttackActivity() : GetPrimaryAttackActivity();

#if defined( GAME_DLL )
	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo( GetOwner(), GetOwner(), GetDamageForActivity( nHitActivity ), DMG_CLUB );
	TraceAttackToTriggers( triggerInfo, tr.startpos, tr.endpos, vec3_origin );
#endif

	// We didn't hit anything. Let's check for hull trace now
	if ( tr.fraction == 1.0f )
	{
		// hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point
		float bludgeonHullRadius = 1.732f * BLUDGEON_HULL_DIM;

		// Back off by hull "radius"
		vSwingEnd -= vForward * bludgeonHullRadius;

		UTIL_TraceHull( vSwingStart, vSwingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pPlayer, COLLISION_GROUP_NONE, &tr );

		// Check if we hit something and if we have hit an entity
		if ( tr.fraction < 1.0 && tr.m_pEnt )
		{
			// Get the position of the entity we just hit
			Vector vToTarget = tr.m_pEnt->GetAbsOrigin() - vSwingStart;
			VectorNormalize( vToTarget );

			float dot = vToTarget.Dot( vForward );

			// Make sure they are sort of facing the guy at least...
			if ( dot < 0.70721f )
			{
				// Force a miss
				tr.fraction = 1.0f;
			}
			else
			{
				// We are sort of facing something. Let's see we should impact based on the melee swing trajectory
				BaseClass::ChooseIntersectionPointAndActivity( tr, g_bludgeonMins, g_bludgeonMaxs, pPlayer );
			}
		}
	}

	// See if we happened to hit water along the way
	BaseClass::ImpactWater( vSwingStart, tr.endpos );

	// Check for hit results
	if ( tr.fraction == 1.0f )
	{
		// We didn't hit anything so let's change the animation sequence to full swing
		nHitActivity = ( bIsSecondary ) ? GetSecondaryMissActivity() : GetPrimaryMissActivity();
	}
	else
	{
		// We hit something. We can keep the same animation sequence.
		BaseClass::Hit( tr, nHitActivity );
	}

	//Play swing sound
	WeaponSound( SINGLE );

	// Send the anim
	SendWeaponAnim( nHitActivity );

	// Setup our next attack times
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + GetFireRate();

	// Setup our next time to idle
	m_flTimeWeaponIdle = gpGlobals->curtime + GetTime2Idle();
}

//=====================================================================================//
// Purpose: Shows a decal where we just hit
//=====================================================================================//
void CTDPWeaponMaul::ImpactEffect( trace_t &tr )
{
	// FIXME: need new decals
	// Right now when hitting things the bullet decal is shown
	UTIL_ImpactTrace( &tr, DMG_SLASH );

	return;
}

void CTDPWeaponMaul::PrimaryAttack()
{
	Swing(false);
}

void CTDPWeaponMaul::SecondaryAttack()
{
	Swing(true);
}

void CTDPWeaponMaul::ThirdAttack()
{
	Swing(true);
}