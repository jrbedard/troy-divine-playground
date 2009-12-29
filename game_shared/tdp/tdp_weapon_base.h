//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_TDPBASE_H
#define WEAPON_TDPBASE_H
#ifdef _WIN32
#pragma once
#endif

#include "tdp_playeranimstate.h"
#include "tdp_weapon_parse.h"

#if defined( CLIENT_DLL )
	#define CTDPWeaponBase C_TDPWeaponBase
#endif

class CTDPPlayer;

// These are the names of the ammo types that the weapon script files reference.
#define AMMO_JAVELIN			"AMMO_JAVELIN"

//--------------------------------------------------------------------------------------------------------
//
// Weapon IDs for all TDP Game weapons
//
typedef enum
{
	WEAPON_NONE = 0,

	WEAPON_SWORD_SHIELD,
	WEAPON_MAUL,
	WEAPON_JAVELIN,

	WEAPON_MAX,		// number of weapons weapon index
} TDPWeaponID;

typedef enum
{
	Primary_Mode = 0,
	Secondary_Mode,
} TDPWeaponMode;

const char *WeaponIDToAlias( int id );

class CTDPWeaponBase : public CBaseCombatWeapon
{
public:
	DECLARE_CLASS( CTDPWeaponBase, CBaseCombatWeapon );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTDPWeaponBase();

	#ifdef GAME_DLL
		DECLARE_DATADESC();
	#endif

	// All predicted weapons need to implement and return true
	virtual bool	IsPredicted() const { return true; }
	virtual TDPWeaponID GetWeaponID( void ) const { return WEAPON_NONE; }
	
	// Get TDP weapon specific weapon data.
	CTDPWeaponInfo const	&GetTDPWpnData() const;

	// Get a pointer to the player that owns this weapon
	CTDPPlayer* GetPlayerOwner() const;

	// override to play custom empty sounds
	virtual bool PlayEmptySound();

#ifdef GAME_DLL
	virtual void SendReloadEvents();
#endif

private:
	CTDPWeaponBase( const CTDPWeaponBase & );
};


#endif // WEAPON_TDPBASE_H
