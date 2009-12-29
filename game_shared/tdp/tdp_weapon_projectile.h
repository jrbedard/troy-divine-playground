//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef TDP_WEAPON_PROJECTILE_H
#define TDP_WEAPON_PROJECTILE_H
#ifdef _WIN32
#pragma once
#endif
	

#include "tdp_weapon_base.h"


#ifdef CLIENT_DLL	
	#define CTDPWeaponProjectile C_TDPWeaponProjectile
#endif


class CTDPWeaponProjectile : public CTDPWeaponBase
{
public:
	DECLARE_CLASS( CTDPWeaponProjectile, CTDPWeaponBase );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTDPWeaponProjectile();

	virtual void	Precache();

	bool			Deploy();
	bool			Holster( CBaseCombatWeapon *pSwitchingTo );

	virtual	void	PrimaryAttackPull();
	virtual	void	PrimaryAttackThrow();

	virtual void	SecondaryAttackPull();
	virtual void	SecondaryAttackThrow();

	virtual void	ThirdAttack();

	//bool			Reload();

	virtual void	ItemPostFrame();

	virtual void	DecrementAmmo( CBaseCombatCharacter *pOwner );

	virtual void	StartProjectileThrow();
	virtual void	ThrowProjectile();
	virtual void	DropProjectile();

	virtual bool	IsPulled() const;

#ifndef CLIENT_DLL
	DECLARE_DATADESC();

	virtual bool AllowsAutoSwitchFrom( void ) const;

	int		CapabilitiesGet();

	// Each derived projectile class implements this.
	virtual void EmitProjectile( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer );
#endif

protected:
	CNetworkVar( bool, m_bRedraw );	// Draw the weapon again after throwing a projectile
	CNetworkVar( bool, m_bPulled );	// Set to true when the weapon projectile is pulled.
	CNetworkVar( float, m_fThrowTime ); // the time at which the projectile will be thrown.  If this value is 0 then the time hasn't been set yet.

private:
	CTDPWeaponProjectile( const CTDPWeaponProjectile & ) {}
};


inline bool CTDPWeaponProjectile::IsPulled() const
{
	return m_bPulled;
}


#endif // TDP_WEAPON_PROJECTILE_H
