//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef TDP_BASE_PROJECTILE_H
#define TDP_BASE_PROJECTILE_H
#ifdef _WIN32
#pragma once
#endif


#include "baseprojectile_shared.h"


#ifdef CLIENT_DLL
	#define CTDPBaseProjectile C_TDPBaseProjectile
#endif


class CTDPBaseProjectile : public CBaseProjectile
{
public:
	DECLARE_CLASS( CTDPBaseProjectile, CBaseProjectile );
	//DECLARE_NETWORKCLASS();

	virtual void Spawn();

	// This gets sent to the client and placed in the client's interpolation history
	// so the projectile starts out moving right off the bat.
	CNetworkVector( m_vInitialVelocity );

	CTDPBaseProjectile() {int t=3;}
	CTDPBaseProjectile( const CTDPBaseProjectile& ) {}

#ifdef CLIENT_DLL

	virtual int DrawModel( int flags );

	virtual void PostDataUpdate( DataUpdateType_t type );

	virtual void OnDataChanged( DataUpdateType_t updateType );

	virtual RenderGroup_t GetRenderGroup( void )
	{
		// We want to draw translucent bits as well as our main model
		return RENDER_GROUP_TWOPASS;
	}

	virtual void ClientThink( void ){return;}

	float m_flSpawnTime;

#else

	DECLARE_DATADESC();

	Class_T Classify( void ) { return CLASS_NONE; }

	//Constants overidden in derived classes
	virtual float GetProjectileGravity() const { return 0.4f; }
	virtual float GetProjectileFriction() const { return 0.2f; }
	virtual float GetProjectileElasticity() const { return 0.45f; }

	virtual void ProjectileTouch( CBaseEntity *pOther );
	
	bool CreateVPhysics( void );

	//Think function to emit danger sounds for the AI
	//void DangerSoundThink( void );

	// Specify what velocity we want the grenade to have on the client immediately.
	// Without this, the entity wouldn't have an interpolation history initially, so it would
	// sit still until it had gotten a few updates from the server.
	void SetupInitialTransmittedProjectileVelocity( const Vector &velocity );

private:


#endif
};


#endif // TDP_BASE_PROJECTILE_H
