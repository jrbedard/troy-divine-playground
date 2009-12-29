//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_JAVELIN_H
#define WEAPON_JAVELIN_H
#ifdef _WIN32
#pragma once
#endif


#include "tdp_weapon_projectile.h"
#include "tdp_baseprojectile.h"


#ifdef CLIENT_DLL
	#define CTDPJavelin C_TDPJavelin
	#define CTDPJavelinProjectile C_TDPJavelinProjectile
#else

	#define JAVELIN_MODEL "models/weapons/w_javelin1.mdl"
	#define JAVELIN_TIMER  2.0f		//Seconds
	#define JAVELIN_GRAVITY 0.4f
	#define JAVELIN_FRICTION 0.2f
	#define JAVELIN_ELASTICITY 0.45f

	ConVar jvlDmg( "jvl_dmg", "100", FCVAR_CHEAT, "Modify javelin damage" );
	ConVar jvlGravity( "jvl_gravity", "JAVELIN_GRAVITY", FCVAR_CHEAT, "Modify javelin gravity" );
	ConVar jvlFriction( "jvl_friction", "JAVELIN_FRICTION", FCVAR_CHEAT, "Modify javelin friction" );
	ConVar jvlElasticity( "jvl_elasticity", "JAVELIN_ELASTICITY", FCVAR_CHEAT, "Modify javelin elasticity" );

#endif




class CTDPJavelinProjectile : public CTDPBaseProjectile
{

	public:
		DECLARE_CLASS( CTDPJavelinProjectile, CTDPBaseProjectile );
		DECLARE_NETWORKCLASS();
		//DECLARE_PREDICTABLE();

		CTDPJavelinProjectile() {int t=0;}

#ifdef CLIENT_DLL

	// Overrides.
	public:

		int DrawModel( int flags );

		void PostDataUpdate( DataUpdateType_t type );

		void OnDataChanged( DataUpdateType_t updateType );

		RenderGroup_t GetRenderGroup( void )
		{
			// We want to draw translucent bits as well as our main model
			return RENDER_GROUP_TWOPASS;
		}

		void ClientThink( void );

		// motion blur
		Vector	m_vecLastOrigin;
		bool	m_bUpdated;

#else
		// Server Projectile

		//Constants for all TDP Projectile
		static const float GetProjectileGravity()  { return JAVELIN_GRAVITY; }
		static const float GetProjectileFriction() { return JAVELIN_FRICTION; }
		static const float GetProjectileElasticity() { return JAVELIN_ELASTICITY; }

		void ProjectileTouch( CBaseEntity *pOther );

		virtual void Spawn()
		{
			SetModel( JAVELIN_MODEL );
			BaseClass::Spawn();
		}

		virtual void Precache()
		{
			PrecacheModel( JAVELIN_MODEL );
			BaseClass::Precache();
		}

		// Javelin stuff.
	public:

		static CTDPJavelinProjectile* Create( 
			const Vector &position, 
			const QAngle &angles, 
			const Vector &velocity, 
			const AngularImpulse &angVelocity, 
			CBaseCombatCharacter *pOwner, 
			float timer )
		{
			//CTDPJavelinProjectile *pJavelin = (CTDPJavelinProjectile*)CBaseEntity::Create( "javelin_projectile", position, angles, pOwner );
			CTDPJavelinProjectile *pJavelin = (CTDPJavelinProjectile*)CBaseEntity::Create( "javelin_projectile", position, angles, pOwner );

			pJavelin->SetAbsVelocity( velocity );
			pJavelin->SetupInitialTransmittedProjectileVelocity( velocity );
			pJavelin->SetThrower( pOwner ); 

			pJavelin->SetGravity( GetProjectileGravity() );
			pJavelin->SetFriction( GetProjectileFriction() );
			pJavelin->SetElasticity( GetProjectileElasticity() );

			pJavelin->m_flDamage = jvlDmg.GetFloat();
			pJavelin->ChangeTeam( pOwner->GetTeamNumber() );
			pJavelin->ApplyLocalAngularVelocityImpulse( angVelocity );	

			// make NPCs afaid of it while in the air
			//pJavelin->SetThink( &CTDPJavelinProjectile::DangerSoundThink );
			//pJavelin->SetNextThink( gpGlobals->curtime );

			return pJavelin;
		}
#endif

	private:
		CTDPJavelinProjectile( const CTDPJavelinProjectile& ) {}

};



//-----------------------------------------------------------------------------
// Javelins
//-----------------------------------------------------------------------------
class CTDPJavelin : public CTDPWeaponProjectile
{
public:
	DECLARE_CLASS( CTDPJavelin, CTDPWeaponProjectile );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTDPJavelin() {}

	virtual TDPWeaponID GetWeaponID( void ) const { return WEAPON_JAVELIN; }

	void	StartProjectileThrow();
	void	ThrowProjectile();

#ifdef CLIENT_DLL

#else
	//DECLARE_DATADESC();

	virtual void EmitProjectile( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer );
	
#endif

	CTDPJavelin( const CTDPJavelin & ) {}
};


//BEGIN_DATADESC( CTDPJavelin )
//END_DATADESC()


IMPLEMENT_NETWORKCLASS_ALIASED( TDPJavelin, DT_CTDPJavelin )

BEGIN_NETWORK_TABLE(CTDPJavelin, DT_CTDPJavelin)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTDPJavelin )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_javelin, CTDPJavelin );
PRECACHE_WEAPON_REGISTER( weapon_javelin );



#endif // WEAPON_JAVELIN_H
