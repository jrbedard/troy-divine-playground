//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for TDP Game
//
// $NoKeywords: $
//=============================================================================//

#ifndef TDP_PLAYER_H
#define TDP_PLAYER_H
#pragma once


#include "player.h"
#include "server_class.h"
#include "tdp_playeranimstate.h"
#include "tdp_shareddefs.h"


//=============================================================================
// >> TDP Game player
//=============================================================================
class CTDPPlayer : public CBasePlayer, public ITDPPlayerAnimStateHelpers
{
public:
	DECLARE_CLASS( CTDPPlayer, CBasePlayer );
	DECLARE_SERVERCLASS();
	DECLARE_PREDICTABLE();

	CTDPPlayer();
	~CTDPPlayer();

	static CTDPPlayer *CreatePlayer( const char *className, edict_t *ed );
	static CTDPPlayer* Instance( int iEnt );

	// This passes the event to the client's and server's CPlayerAnimState.
	void DoAnimationEvent( PlayerAnimEvent_t event );

	virtual void FlashlightTurnOn( void );
	virtual void FlashlightTurnOff( void );
	virtual int FlashlightIsOn( void );

	virtual void PreThink();
	virtual void PostThink();
	virtual void Spawn();
	virtual void InitialSpawn();
	virtual void Precache();
	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual void LeaveVehicle( const Vector &vecExitPoint, const QAngle &vecExitAngles );
	
	CTDPWeaponBase* GetActiveTDPWeapon() const;
	virtual void	CreateViewModel( int viewmodelindex = 0 );

	virtual void	CheatImpulseCommands( int iImpulse );

	CNetworkVar( int, m_iThrowGrenadeCounter );	// used to trigger grenade throw animations.
	CNetworkQAngle( m_angEyeAngles );	// Copied from EyeAngles() so we can send it to the client.
	CNetworkVar( int, m_iShotsFired );	// number of shots fired recently

	// Tracks our ragdoll entity.
	CNetworkHandle( CBaseEntity, m_hRagdoll );	// networked entity handle 

// In shared code.
public:
	// ITDPPlayerAnimState overrides.
	virtual CTDPWeaponBase* TDPAnim_GetActiveWeapon();
	virtual bool TDPAnim_CanMove();
	

	void FireBullet( 
		Vector vecSrc, 
		const QAngle &shootAngles, 
		float vecSpread, 
		int iDamage, 
		int iBulletType,
		CBaseEntity *pevAttacker,
		bool bDoEffects,
		float x,
		float y );

private:

	void CreateRagdollEntity();

	ITDPPlayerAnimState *m_PlayerAnimState;
};


inline CTDPPlayer *ToTDPPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

#ifdef _DEBUG
	Assert( dynamic_cast<CTDPPlayer*>( pEntity ) != 0 );
#endif
	return static_cast< CTDPPlayer* >( pEntity );
}


#endif	// TDP_PLAYER_H
