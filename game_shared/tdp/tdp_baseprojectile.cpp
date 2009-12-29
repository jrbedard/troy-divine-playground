//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "tdp_baseprojectile.h"


#include "gamevars_shared.h"

extern ConVar sv_gravity;


#ifdef CLIENT_DLL

	#include "c_tdp_player.h"

#else

	#include "soundent.h"

	BEGIN_DATADESC( CTDPBaseProjectile )
		DEFINE_FUNCTION( ProjectileTouch ),
	END_DATADESC()

#endif




#ifdef CLIENT_DLL

	void CTDPBaseProjectile::PostDataUpdate( DataUpdateType_t type )
	{
		BaseClass::PostDataUpdate( type );
	}

	void CTDPBaseProjectile::OnDataChanged( DataUpdateType_t updateType )
	{
		BaseClass::OnDataChanged( updateType );
	}

	int CTDPBaseProjectile::DrawModel( int flags )
	{
		return BaseClass::DrawModel( flags );
	}


	void CTDPBaseProjectile::Spawn()
	{
		m_flSpawnTime = gpGlobals->curtime;
		BaseClass::Spawn();
	}

#else

	//-----------------------------------------------------------------------------
	// Purpose: 
	// Output : Returns true on success, false on failure.
	//-----------------------------------------------------------------------------
	bool CTDPBaseProjectile::CreateVPhysics( void )
	{
		// Create the object in the physics system
		VPhysicsInitNormal( SOLID_BBOX, FSOLID_NOT_STANDABLE, false );

		return true;
	}

	void CTDPBaseProjectile::Spawn( void )
	{
		BaseClass::Spawn();

		//SetSolidFlags( FSOLID_NOT_STANDABLE );
		SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
		SetSolid( SOLID_BBOX );	// So it will collide with physics props!

		SetTouch( &CTDPBaseProjectile::ProjectileTouch );

		// smaller, cube bounding box so we rest on the ground
		SetSize( Vector ( -2, -2, -2 ), Vector ( 2, 2, 2 ) );
	}

/*	void CBaseGrenadeProjectile::DangerSoundThink( void )
	{
		if (!IsInWorld())
		{
			Remove( );
			return;
		}

		if( gpGlobals->curtime > m_flDetonateTime )
		{
			Detonate();
			return;
		}

		CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin() + GetAbsVelocity() * 0.5, GetAbsVelocity().Length( ), 0.2 );

		SetNextThink( gpGlobals->curtime + 0.2 );

		if (GetWaterLevel() != 0)
		{
			SetAbsVelocity( GetAbsVelocity() * 0.5 );
		}
	}
*/

	void CTDPBaseProjectile::SetupInitialTransmittedProjectileVelocity( const Vector &velocity )
	{
		m_vInitialVelocity = velocity;
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	// Input  : *pOther - 
	//-----------------------------------------------------------------------------
	void CTDPBaseProjectile::ProjectileTouch( CBaseEntity *pOther )
	{

	}


#endif
