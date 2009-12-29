//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef TDP_PLAYERANIMSTATE_H
#define TDP_PLAYERANIMSTATE_H
#ifdef _WIN32
#pragma once
#endif


#include "convar.h"
#include "iplayeranimstate.h"
#include "base_playeranimstate.h"


#ifdef CLIENT_DLL
	class C_BaseAnimatingOverlay;
	class C_TDPWeaponBase;
	#define CBaseAnimatingOverlay C_BaseAnimatingOverlay
	#define CTDPWeaponBase C_TDPWeaponBase
	#define CTDPPlayer C_TDPPlayer
#else
	class CBaseAnimatingOverlay;
	class CTDPWeaponBase; 
	class CTDPPlayer;
#endif


// When moving this fast, he plays run anim.
#define ARBITRARY_RUN_SPEED		175.0f


enum PlayerAnimEvent_t
{
	PLAYERANIMEVENT_FIRE_GUN_PRIMARY=0,
	PLAYERANIMEVENT_FIRE_GUN_SECONDARY,
	PLAYERANIMEVENT_THROW_GRENADE,
	PLAYERANIMEVENT_JUMP,
	PLAYERANIMEVENT_RELOAD,
	
	PLAYERANIMEVENT_COUNT
};


class ITDPPlayerAnimState : virtual public IPlayerAnimState
{
public:
	// This is called by both the client and the server in the same way to trigger events for
	// players firing, jumping, throwing grenades, etc.
	virtual void DoAnimationEvent( PlayerAnimEvent_t event ) = 0;
	
	// Returns true if we're playing the grenade prime or throw animation.
	virtual bool IsThrowingGrenade() = 0;
};


// This abstracts the differences between TDP players and hostages.
class ITDPPlayerAnimStateHelpers
{
public:
	virtual CTDPWeaponBase* TDPAnim_GetActiveWeapon() = 0;
	virtual bool TDPAnim_CanMove() = 0;
};


ITDPPlayerAnimState* CreatePlayerAnimState( CBaseAnimatingOverlay *pEntity, ITDPPlayerAnimStateHelpers *pHelpers, LegAnimType_t legAnimType, bool bUseAimSequences );

// If this is set, then the game code needs to make sure to send player animation events
// to the local player if he's the one being watched.
extern ConVar cl_showanimstate;


#endif // TDP_PLAYERANIMSTATE_H
