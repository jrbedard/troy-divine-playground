//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "tdp_weapon_base.h"
#include "gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "tdp_weapon_projectile.h"
#include "in_buttons.h"	


#ifdef CLIENT_DLL

	#include "c_tdp_player.h"

#else
	
	#include "tdp_player.h"
	#include "items.h"

#endif



IMPLEMENT_NETWORKCLASS_ALIASED( TDPWeaponProjectile, DT_TDPWeaponProjectile )

BEGIN_NETWORK_TABLE(CTDPWeaponProjectile, DT_TDPWeaponProjectile)

#ifndef CLIENT_DLL
	SendPropBool( SENDINFO(m_bRedraw) ),
	SendPropBool( SENDINFO(m_bPulled) ),
	SendPropFloat( SENDINFO(m_fThrowTime), 0, SPROP_NOSCALE ),
#else
	RecvPropBool( RECVINFO(m_bRedraw) ),
	RecvPropBool( RECVINFO(m_bPulled) ),
	RecvPropFloat( RECVINFO(m_fThrowTime) ),
#endif

END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTDPWeaponProjectile )
	DEFINE_PRED_FIELD( m_bRedraw, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	//DEFINE_PRED_FIELD( m_bRedraw, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tdp_weapon_projectile, CTDPWeaponProjectile );


CTDPWeaponProjectile::CTDPWeaponProjectile()
{
	m_bRedraw = false;
	m_bPulled = false;
	m_fThrowTime = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTDPWeaponProjectile::Precache()
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTDPWeaponProjectile::Deploy()
{
	m_bRedraw = false;
	m_bPulled = false;
	m_fThrowTime = 0;

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CTDPWeaponProjectile::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_bRedraw = false;
	m_bPulled = false; // when this is holstered make sure the pin isn’t pulled.
	m_fThrowTime = 0;

#ifndef CLIENT_DLL
	// If they attempt to switch weapons before the throw animation is done, 
	// allow it, but kill the weapon if we have to.
	CTDPPlayer *pPlayer = GetPlayerOwner();

	if( pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 )
	{
		CBaseCombatCharacter *pOwner = (CBaseCombatCharacter *)pPlayer;
		pOwner->Weapon_Drop( this );
		UTIL_Remove(this);
	}
#endif

	return BaseClass::Holster( pSwitchingTo );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTDPWeaponProjectile::PrimaryAttackPull()
{
	if ( m_bRedraw || m_bPulled )
		return;

	CTDPPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer || pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		return;

	// The pull pin animation has to finish, then we wait until they aren't holding the primary
	// attack button, then throw the grenade.
	SendWeaponAnim( ACT_VM_PULLPIN );
	m_bPulled = true;

	// Don't let weapon idle interfere in the middle of a throw!
	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}



void CTDPWeaponProjectile::PrimaryAttackThrow()
{

	CTDPPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	CBaseViewModel *vm = pPlayer->GetViewModel( m_nViewModelIndex );
	if ( !vm )
		return;

	// If they let go of the fire button, they want to throw the grenade.
	if ( m_bPulled && !(pPlayer->m_nButtons & IN_ATTACK) ) 
	{
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_THROW_GRENADE );

		StartProjectileThrow();

		DecrementAmmo( pPlayer );

		m_bPulled = false;
		SendWeaponAnim( ACT_VM_THROW );	
		SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
	}
	else if ((m_fThrowTime > 0) && (m_fThrowTime < gpGlobals->curtime))
	{
		ThrowProjectile();
	}
	else if( m_bRedraw )
	{
		// Has the throw animation finished playing
		if( m_flTimeWeaponIdle < gpGlobals->curtime )
		{
#ifdef GAME_DLL
			// if we're officially out of grenades, ditch this weapon
			if( pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 )
			{
				pPlayer->Weapon_Drop( this, NULL, NULL );
				UTIL_Remove(this);
			}
			else
			{
				m_bRedraw = false;
				//m_bPulled = false;
				//m_fThrowTime = 0;
				//UTIL_Remove(this);
				//pPlayer->SwitchToNextBestWeapon( this );
			}
#endif
			return;	//don't animate this grenade any more!
		}	
	}
	else if( !m_bRedraw )
	{
		//BaseClass::ItemPostFrame();
	}
}


/*
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTDPWeaponProjectile::SecondaryAttack()
{
	if ( m_bRedraw )
		return;

	CTDPPlayer *pPlayer = GetPlayerOwner();
	
	if ( pPlayer == NULL )
		return;

	//See if we're ducking
	if ( pPlayer->GetFlags() & FL_DUCKING )
	{
		//Send the weapon animation
		SendWeaponAnim( ACT_VM_SECONDARYATTACK );
	}
	else
	{
		//Send the weapon animation
		SendWeaponAnim( ACT_VM_HAULBACK );
	}

	// Don't let weapon idle interfere in the middle of a throw!
	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

	m_flNextSecondaryAttack	= gpGlobals->curtime + SequenceDuration();
}

*/



void CTDPWeaponProjectile::SecondaryAttackThrow()
{


}



void CTDPWeaponProjectile::SecondaryAttackPull()
{


}



void CTDPWeaponProjectile::ThirdAttack()
{

	
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
/*
bool CTDPWeaponProjectile::Reload()
{
	if ( ( m_bRedraw ) && ( m_flNextPrimaryAttack <= gpGlobals->curtime ) && ( m_flNextSecondaryAttack <= gpGlobals->curtime ) )
	{
		//Redraw the weapon
		SendWeaponAnim( ACT_VM_DRAW );

		//Update our times
		m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration();
		m_flNextSecondaryAttack	= gpGlobals->curtime + SequenceDuration();

		SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
	}

	return true;
}
*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTDPWeaponProjectile::ItemPostFrame()
{
	CTDPPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	// Is the player pressing the primary attack button and is it time for the next
	// primary attack?
	if ( pPlayer->m_nButtons & IN_ATTACK && m_flNextPrimaryAttack <= gpGlobals->curtime )
	{
		PrimaryAttackPull();
	}
	PrimaryAttackThrow();

	// Is the player pressing the secondary attack button and is it time for the next
	// secondary attack?
	if ( pPlayer->m_nButtons & IN_ATTACK2 && m_flNextSecondaryAttack <= gpGlobals->curtime )
		SecondaryAttack();


	// If the player is not pressing any of the attack buttons let's try to idle
	if ( !( pPlayer->m_nButtons & IN_ATTACK3 || pPlayer->m_nButtons & IN_ATTACK2 || pPlayer->m_nButtons & IN_ATTACK ) )
		WeaponIdle();

}




#ifdef CLIENT_DLL

	void CTDPWeaponProjectile::DecrementAmmo( CBaseCombatCharacter *pOwner )
	{
		
	}

	void CTDPWeaponProjectile::DropProjectile()
	{
		m_bRedraw = true;
		m_fThrowTime = 0.0f;
	}

	void CTDPWeaponProjectile::ThrowProjectile()
	{
		
	}

	void CTDPWeaponProjectile::StartProjectileThrow()
	{
		
	}

#else

	BEGIN_DATADESC( CTDPWeaponProjectile )
		DEFINE_FIELD( m_bRedraw, FIELD_BOOLEAN ),
	END_DATADESC()

	int CTDPWeaponProjectile::CapabilitiesGet()
	{
		return bits_CAP_WEAPON_RANGE_ATTACK1; 
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	// Input  : *pOwner - 
	//-----------------------------------------------------------------------------
	void CTDPWeaponProjectile::DecrementAmmo( CBaseCombatCharacter *pOwner )
	{
		pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
	}

	void CTDPWeaponProjectile::StartProjectileThrow()
	{
		
	}

	void CTDPWeaponProjectile::ThrowProjectile()
	{
		

	}

	void CTDPWeaponProjectile::DropProjectile()
	{
		CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
		if ( !pPlayer )
		{
			Assert( false );
			return;
		}

		Vector vForward;
		pPlayer->EyeVectors( &vForward );
		Vector vecSrc = pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset() + vForward * 16; 

		Vector vecVel = pPlayer->GetAbsVelocity();

		EmitProjectile( vecSrc, vec3_angle, vecVel, AngularImpulse(600,random->RandomInt(-1200,1200),0), pPlayer );

		m_bRedraw = true;
		m_fThrowTime = 0.0f;
	}

	void CTDPWeaponProjectile::EmitProjectile( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer )
	{
		Assert( 0 && "CBaseTDPGrenade::EmitGrenade should not be called. Make sure to implement this in your subclass!\n" );
	}

	bool CTDPWeaponProjectile::AllowsAutoSwitchFrom( void ) const
	{
		return !m_bPulled;
	}


	

#endif

