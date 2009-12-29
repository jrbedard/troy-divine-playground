//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_TDP_PLAYER_H
#define C_TDP_PLAYER_H
#ifdef _WIN32
#pragma once
#endif


#include "tdp_playeranimstate.h"
#include "c_baseplayer.h"
#include "tdp_shareddefs.h"
#include "baseparticleentity.h"


class C_TDPPlayer : public C_BasePlayer, public ITDPPlayerAnimStateHelpers
{
public:
	DECLARE_CLASS( C_TDPPlayer, C_BasePlayer );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();

	C_TDPPlayer();
	~C_TDPPlayer();

	static C_TDPPlayer* GetLocalTDPPlayer();

	virtual const QAngle& GetRenderAngles();
	virtual void UpdateClientSideAnimation();
	virtual void PostDataUpdate( DataUpdateType_t updateType );
	virtual void OnDataChanged( DataUpdateType_t updateType );


// Called by shared code.
public:
	
	// ITDPPlayerAnimState overrides.
	virtual CTDPWeaponBase* TDPAnim_GetActiveWeapon();
	virtual bool TDPAnim_CanMove();

	void DoAnimationEvent( PlayerAnimEvent_t event );
	bool ShouldDraw();

	ITDPPlayerAnimState *m_PlayerAnimState;

	QAngle	m_angEyeAngles;
	CInterpolatedVar< QAngle >	m_iv_angEyeAngles;

	CNetworkVar( int, m_iThrowGrenadeCounter );	// used to trigger grenade throw animations.
	CNetworkVar( int, m_iShotsFired );	// number of shots fired recently

	EHANDLE	m_hRagdoll;

	CTDPWeaponBase *GetActiveTDPWeapon() const;

	C_BaseAnimating *BecomeRagdollOnClient( bool bCopyEntity);
	IRagdoll* C_TDPPlayer::GetRepresentativeRagdoll() const;

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
	C_TDPPlayer( const C_TDPPlayer & );
};


inline C_TDPPlayer* ToTDPPlayer( CBaseEntity *pPlayer )
{
	Assert( dynamic_cast< C_TDPPlayer* >( pPlayer ) != NULL );
	return static_cast< C_TDPPlayer* >( pPlayer );
}


#endif // C_TDP_PLAYER_H
