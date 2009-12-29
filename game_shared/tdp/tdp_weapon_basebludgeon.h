//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Base class for melee type weapons
//
//=====================================================================================//

#ifndef TDP_BASEBLUDGEONWEAPON_H
#define TDP_BASEBLUDGEONWEAPON_H

#ifdef _WIN32
#pragma once
#endif

//#include "weapon_sdkbase.h"
#include "tdp_weapon_base.h"
#include "effect_dispatch_data.h"

#if defined( CLIENT_DLL )
	#define CTDPBludgeonWeaponBase C_TDPBludgeonWeaponBase
	#include "c_TDP_player.h"
#else
	#include "TDP_player.h"
#endif

void DispatchEffect( const char *pName, const CEffectData &data );

// Some default values we implement in our base class
#define	BLUDGEON_REFIRE		0.5f
#define	BLUDGEON_RANGE		64.0f
#define	BLUDGEON_TIME2IDLE	3.0f

// Some VALVe definitions we'll use in some methods
#define BLUDGEON_HULL_DIM	16
static const Vector g_bludgeonMins( -BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM );
static const Vector g_bludgeonMaxs(  BLUDGEON_HULL_DIM,  BLUDGEON_HULL_DIM,  BLUDGEON_HULL_DIM );

class CTDPBludgeonWeaponBase : public CTDPWeaponBase
{
public:
	DECLARE_CLASS( CTDPBludgeonWeaponBase, CTDPWeaponBase );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTDPBludgeonWeaponBase();

#if defined( GAME_DLL )
	DECLARE_DATADESC();
#endif

	// Nothing really much to see here. Everything is in the .cpp file!

	virtual void		Precache();
	virtual void		Spawn();
	virtual bool		Deploy();
	virtual bool		Holster( CBaseCombatWeapon *pSwitchingTo );

	virtual void		PrimaryAttack(){return;}
	virtual void		SecondaryAttack(){return;}
	virtual void		ThirdAttack(){return;}

	virtual void		AddViewKick();

	virtual void		ItemPostFrame();

	virtual Activity	GetPrimaryAttackActivity();
	virtual Activity	GetSecondaryAttackActivity();
	virtual Activity	GetThirdAttackActivity();

	virtual Activity	GetPrimaryMissActivity();
	virtual Activity	GetSecondaryMissActivity();	

	// TDP_TODO : drop a bludgeon weapon			
	// virtual void		DropWeapon();

	virtual float		GetFireRate();
	virtual float		GetTime2Idle();
	virtual float		GetRange();
	virtual float		GetDamageForActivity( Activity hitActivity );

	void				ChooseIntersectionPointAndActivity( trace_t &tr, const Vector &mins,
															const Vector &maxs, CTDPPlayer *pPlayer );

	void				Hit( trace_t &tr, Activity hitActivity );
	void				ImpactWater( const Vector &vStart, const Vector &vEnd );

	virtual				bool Reload(void) {return true;}

private:
	virtual				void Swing( int bIsSecondary ) { return; }

	virtual void		ImpactEffect( trace_t &tr );

	// Hide copy constructor
	CTDPBludgeonWeaponBase( const CTDPBludgeonWeaponBase & );
};

#endif
