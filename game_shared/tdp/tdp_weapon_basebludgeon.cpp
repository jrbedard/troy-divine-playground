//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of a bludgeon (melee) weapon base class
//
//=====================================================================================//

#include "cbase.h"
#include "tdp_weapon_base.h"
#include "in_buttons.h"
#include "takedamageinfo.h"

// Include our header file
#include "TDP_weapon_basebludgeon.h"


// ----------------------------------------------------------------------------- 
//
// CBludgeonWeaponSDKBase tables.
// ----------------------------------------------------------------------------- 
IMPLEMENT_NETWORKCLASS_ALIASED( TDPBludgeonWeaponBase, 
DT_TDPBludgeonWeaponBase )

BEGIN_NETWORK_TABLE( CTDPBludgeonWeaponBase, DT_TDPBludgeonWeaponBase )
#if defined( GAME_DLL )
	// world weapon models have no animations
  	SendPropExclude( "DT_AnimTimeMustBeFirst", "m_flAnimTime" ),
	SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTDPBludgeonWeaponBase )
	DEFINE_PRED_FIELD( m_flTimeWeaponIdle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_NOERRORCHECK ),
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( bludgeonweapon_sdk_base, CTDPBludgeonWeaponBase );


#if defined( GAME_DLL )
	BEGIN_DATADESC( CTDPBludgeonWeaponBase )
		// New weapon Think and Touch Functions go here..
	END_DATADESC()
#endif



//=====================================================================================//
// Purpose: Weapon constructor
//=====================================================================================//
CTDPBludgeonWeaponBase::CTDPBludgeonWeaponBase()
{
	// Initialization of some internal values
	m_fMinRange1		= 0.0f;
	m_fMaxRange1		= BLUDGEON_RANGE;
	m_fMinRange2		= 0.0f;
	m_fMaxRange2		= BLUDGEON_RANGE;

	// By default any melee weapon "works" under the water
	m_bFiresUnderwater	= true;
}


//=====================================================================================//
// Purpose: Resource pre-cache
//=====================================================================================//
void CTDPBludgeonWeaponBase::Precache()
{
	// Let the base class handle this
	BaseClass::Precache();

	return;
}


//=====================================================================================//
// Purpose: Spawn the weapon
//=====================================================================================//
void CTDPBludgeonWeaponBase::Spawn()
{
	// Let the base class handle this
	BaseClass::Spawn();

	return;
}


//=====================================================================================//
// Purpose: Deploy the weapon
//=====================================================================================//
bool CTDPBludgeonWeaponBase::Deploy()
{
	// Let the base class handle this
	return ( BaseClass::Deploy() );
}


//=====================================================================================//
// Purpose: Hoslter the weapon
//=====================================================================================//
bool CTDPBludgeonWeaponBase::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	// Let the base class handle this
	return ( BaseClass::Holster( pSwitchingTo ) );
}

void CTDPBludgeonWeaponBase::AddViewKick()
{
	// Do we have a valid owner holding the weapon?
	CTDPPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	// Just shake the screen using some random numbers so it's
	// different every time
	pPlayer->ViewPunch( QAngle( random->RandomFloat( -1.0f, 1.0f),
								random->RandomFloat( -1.0f, 1.0f ),
								0.0f ) );
	return;
}

//=====================================================================================//
// Purpose: Controls the attacks and the call of WeaponIdle() method
//=====================================================================================//
void CTDPBludgeonWeaponBase::ItemPostFrame()
{
	// Do we have a valid owner holding the weapon?
	CTDPPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	static int lastButton = 0;

	//TDP_TODO
	if ( pPlayer->m_nButtons & IN_ATTACK3 && m_flNextSecondaryAttack <= gpGlobals->curtime && (lastButton != pPlayer->m_afButtonPressed) )
	{	
		ThirdAttack();
	}
	lastButton = pPlayer->m_afButtonPressed;

	// Is the player pressing the secondary attack button and is it time for the next
	// secondary attack?
	if ( pPlayer->m_nButtons & IN_ATTACK2 && m_flNextSecondaryAttack <= gpGlobals->curtime )
		SecondaryAttack();

	// Is the player pressing the primary attack button and is it time for the next
	// primary attack?
	if ( pPlayer->m_nButtons & IN_ATTACK && m_flNextPrimaryAttack <= gpGlobals->curtime )
		PrimaryAttack();

	// If the player is not pressing any of the attack buttons let's try to idle
	if ( !( pPlayer->m_nButtons & IN_ATTACK3 || pPlayer->m_nButtons & IN_ATTACK2 || pPlayer->m_nButtons & IN_ATTACK ) )
		WeaponIdle();

	//BaseClass::ItemPostFrame(); // Make this conditionnal

	return;
}

//=====================================================================================//
// Purpose: Returns which animation sequence we should use when we hit using the
//			PrimaryAttack() method
//=====================================================================================//
Activity CTDPBludgeonWeaponBase::GetPrimaryAttackActivity()
{
	// If the melee weapon supports a primary attack it should have a
	// sequence animation with the following activity assigned to it
	return ( ACT_VM_HITCENTER );
}


//=====================================================================================//
// Purpose: Returns which animation sequence we should use when we hit using the
//			SecondaryAttack() method
//=====================================================================================//
Activity CTDPBludgeonWeaponBase::GetSecondaryAttackActivity()
{
	// If the melee weapon supports a secondary attack it should have a
	// sequence animation with the following activity assigned to it
	return ( ACT_VM_HITCENTER2 );
}

//=====================================================================================//
// Purpose: Returns which animation sequence we should use when we hit using the
//			SecondaryAttack() method
//=====================================================================================//
Activity CTDPBludgeonWeaponBase::GetThirdAttackActivity()
{
	// If the melee weapon supports a secondary attack it should have a
	// sequence animation with the following activity assigned to it
	return ( ACT_VM_BLOCK );
}


//=====================================================================================//
// Purpose: Returns which animation sequence we should use when we miss using the
//			PrimaryAttack() method
//=====================================================================================//
Activity CTDPBludgeonWeaponBase::GetPrimaryMissActivity()
{
	// If the melee weapon supports a primary attack it should have a
	// sequence animation with the following activity assigned to it
	return ( ACT_VM_MISSCENTER );
}


//=====================================================================================//
// Purpose: Returns which animation sequence we should use when we miss using the
//			SecondaryAttack() method
//=====================================================================================//
Activity CTDPBludgeonWeaponBase::GetSecondaryMissActivity()
{
	// If the melee weapon supports a secondary attack it should have a
	// sequence animation with the following activity assigned to it
	return ( ACT_VM_MISSCENTER2 );
}


//=====================================================================================//
// Purpose: How often can we attack using the weapon
//=====================================================================================//
float CTDPBludgeonWeaponBase::GetFireRate()
{
	// Return default value
	return ( BLUDGEON_REFIRE );
}


//=====================================================================================//
// Purpose: How long until idle after an attack?
//=====================================================================================//
float CTDPBludgeonWeaponBase::GetTime2Idle()
{
	// Return default value
	return ( BLUDGEON_TIME2IDLE );
}


//=====================================================================================//
// Purpose: Returns the maximum range for the melee attack
//			Should we define a GetRangeSecondary() to get m_flMaxRange2?
//=====================================================================================//
float CTDPBludgeonWeaponBase::GetRange()
{
	// Just returns the primary melee range
	return ( m_fMaxRange1 );
}


//=====================================================================================//
// Purpose:
//=====================================================================================//
float CTDPBludgeonWeaponBase::GetDamageForActivity( Activity hitActivity )
{
	// The default damage is 100%
	return ( 20.0f );
}



//=====================================================================================//
// Purpose: This function will check if hit something along the swing animation
// I'm not sure how it does it so I'll leave the explanation of this to VALVe  ;)
//=====================================================================================//
void CTDPBludgeonWeaponBase::ChooseIntersectionPointAndActivity( trace_t &tr, const Vector &mins, const Vector &maxs, CTDPPlayer *pPlayer )
{
	int			i, j, k;
	float		lowerDistance = MAX_TRACE_LENGTH;
	const float	*minmaxs[2] = {mins.Base(), maxs.Base()};
	trace_t		tmptr;
	Vector		vecHullEnd = tr.endpos;
	Vector		vecEnd;

	Vector vecSrc = tr.startpos;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc)*2);
	UTIL_TraceLine( vecSrc, vecHullEnd, MASK_SHOT_HULL, pPlayer, COLLISION_GROUP_NONE, &tmptr );
	if ( tmptr.fraction == 1.0 )
	{
		for ( i = 0; i < 2; i++ )
		{
			for ( j = 0; j < 2; j++ )
			{
				for ( k = 0; k < 2; k++ )
				{
					vecEnd.x = vecHullEnd.x + minmaxs[i][0];
					vecEnd.y = vecHullEnd.y + minmaxs[j][1];
					vecEnd.z = vecHullEnd.z + minmaxs[k][2];

					UTIL_TraceLine( vecSrc, vecEnd, MASK_SHOT_HULL, pPlayer, COLLISION_GROUP_NONE, &tmptr );
					if ( tmptr.fraction < 1.0 )
					{
						float thisDistance = (tmptr.endpos - vecSrc).Length();
						if ( thisDistance < lowerDistance )
						{
							tr = tmptr;
							lowerDistance = thisDistance;
						}
					}
				}
			}
		}
	}
	else
	{
		tr = tmptr;
	}

	return;
}


//=====================================================================================//
// Purpose: Performs the screen shake and it checks to see if we hit an entity to
//          handle the proper damage
//			An entity here can be another player or a wood plank
//=====================================================================================//
void CTDPBludgeonWeaponBase::Hit( trace_t &tr, Activity nHitActivity )
{
	// Do we have a valid owner holding the weapon?
	CTDPPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	// Let's shake the screen a little
	AddViewKick();

	// if tr.m_pEnt is not NULL it means we have hit a target
	if ( tr.m_pEnt != NULL )
	{
		Vector vForward;
		pPlayer->EyeVectors( &vForward, NULL, NULL );
		VectorNormalize( vForward );

		// Process the damage and send it to the entity we just hit
		CTakeDamageInfo dmgInfo( GetOwner(), GetOwner(), GetDamageForActivity( nHitActivity ), DMG_CLUB );
		CalculateMeleeDamageForce( &dmgInfo, vForward, tr.endpos );
		tr.m_pEnt->DispatchTraceAttack( dmgInfo, vForward, &tr );
		ApplyMultiDamage();

#if defined( GAME_DLL )
		// Now hit all triggers along the ray that...
		TraceAttackToTriggers( dmgInfo, tr.startpos, tr.endpos, vForward );
#endif
	}

	// Apply an impact effect
	ImpactEffect( tr );
}


//=====================================================================================//
// Purpose: Checks for water or slime contents along the attack line to dispatch
//          a splash effect
//=====================================================================================//
void CTDPBludgeonWeaponBase::ImpactWater( const Vector &vStart, const Vector &vEnd )
{
	// We must start outside the water
	if ( UTIL_PointContents( vStart ) & ( CONTENTS_WATER|CONTENTS_SLIME ) )
		return;

	trace_t	tr;
	// Let's check for water all along the swing
	UTIL_TraceLine( vStart, vEnd, ( CONTENTS_WATER | CONTENTS_SLIME ), GetOwner(), COLLISION_GROUP_NONE, &tr );

	// If tr.fraction < 1.0f it means that we had hit water or slime along our swing
	if ( tr.fraction < 1.0f )
	{
		// tr.endpos contains the water level
		CEffectData	effectData;
		effectData.m_fFlags  = 0;
		effectData.m_vOrigin = tr.endpos;
		effectData.m_vNormal = tr.plane.normal;
		effectData.m_flScale = 8.0f;

		// See if we hit slime
		if ( tr.contents & CONTENTS_SLIME )
		{
			// By adding this flag the splash will be different
			effectData.m_fFlags |= FX_WATER_IN_SLIME;
		}
		// Just show the water/slime splash
		DispatchEffect( "watersplash", effectData );
	}
	return;
}


//=====================================================================================//
// Purpose: Shows a decal where we just hit
//=====================================================================================//
void CTDPBludgeonWeaponBase::ImpactEffect( trace_t &tr )
{
	// FIXME: need new decals
	// Right now when hitting things the bullet decal is shown
	UTIL_ImpactTrace( &tr, DMG_CLUB );

	return;
}

bool Reload(void) 
{
	return true;
}