//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef TDP_NPC_COMBINES_H
#define TDP_NPC_COMBINES_H
#ifdef _WIN32
#pragma once
#endif

#include "tdp_npc_combine.h"

//=========================================================
//	>> CTDP_NPC_CombineS
//=========================================================
class CTDP_NPC_CombineS : public CTDP_NPC_Combine
{
	DECLARE_CLASS( CTDP_NPC_CombineS, CTDP_NPC_Combine );

public: 
	void		Spawn( void );
	void		Precache( void );
	void		DeathSound( void );
	void		PrescheduleThink( void );
	void		BuildScheduleTestBits( void );
	int			SelectSchedule ( void );
	float		GetHitgroupDamageMultiplier( int iHitGroup, const CTakeDamageInfo &info );
	int			TakeDamage( const CTakeDamageInfo &info );
	void		HandleAnimEvent( animevent_t *pEvent );
	void		OnChangeActivity( Activity eNewActivity );
	void		Event_Killed( const CTakeDamageInfo &info );
	void		OnListened();

	void		ClearAttackConditions( void );

	bool		m_fIsBlocking;

	bool		IsLightDamage( const CTakeDamageInfo &info );
	bool		IsHeavyDamage( const CTakeDamageInfo &info );

	virtual	bool		AllowedToIgnite( void ) { return true; }

private:
	bool		ShouldHitPlayer( const Vector &targetDir, float targetDist );

};

#endif // TDP_NPC_COMBINES_H
