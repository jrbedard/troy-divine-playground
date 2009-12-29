/*
	Base projectile for Troy : Divine Playground

	Author : Jean-René
*/


#ifndef BASEPROJECTILE_SHARED_H
#define BASEPROJECTILE_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#if defined( CLIENT_DLL )
	#define CBaseProjectile C_BaseProjectile // The client classes are declared with C_
	#include "c_basecombatcharacter.h"		 // This contain all of the combat entry points / functionality that are common between NPCs and players
#else
	#include "basecombatcharacter.h"
	#include "player_pickup.h"
#endif


//#define BASEGRENADE_EXPLOSION_VOLUME	1024

class CTakeDamageInfo;


#if !defined( CLIENT_DLL )
	class CBaseProjectile : public CBaseCombatCharacter//, public CDefaultPlayerPickupVPhysics // server 
#else
	#define CBaseCombatCharacter C_BaseCombatCharacter
	class CBaseProjectile : public CBaseCombatCharacter
#endif
{
	DECLARE_CLASS( CBaseProjectile, CBaseCombatCharacter );


public:

	CBaseProjectile(void);
	~CBaseProjectile(void);

	DECLARE_PREDICTABLE();
	DECLARE_NETWORKCLASS();


#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
#endif

	virtual void		Precache( void );

	virtual int			BloodColor( void ) { return DONT_BLEED; }
	virtual void		Event_Killed( const CTakeDamageInfo &info );

	// Damage accessors.
	virtual float GetDamage()
	{
		return m_flDamage;
	}
	
	
	virtual void SetDamage(float flDamage)
	{
		m_flDamage = flDamage;
	}

	CBaseCombatCharacter *GetThrower( void );
	void				  SetThrower( CBaseCombatCharacter *pThrower );

#if !defined( CLIENT_DLL )
	// Allow +USE pickup
	int ObjectCaps() 
	{ 
		return (BaseClass::ObjectCaps() | FCAP_IMPULSE_USE | FCAP_USE_IN_RADIUS);
	}

	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );


#endif

public:
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_vecVelocity );
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_fFlags );

	CNetworkVar( bool, m_bIsLive );					// Is this grenade live, or can it be picked up?
	
protected:

	CNetworkVar( float, m_flDamage );		// Damage to inflict.
	
private:
	CNetworkHandle( CBaseEntity, m_hThrower );	// Who threw this grenade

	CBaseProjectile( const CBaseProjectile & ); // not defined, not accessible

};

#endif // BASEPROJECTILE_SHARED_H
