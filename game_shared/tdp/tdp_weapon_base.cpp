//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "in_buttons.h"
#include "takedamageinfo.h"
#include "tdp_weapon_base.h"
#include "ammodef.h"


#if defined( CLIENT_DLL )

	#include "c_tdp_player.h"

#else

	#include "tdp_player.h"

#endif


// ----------------------------------------------------------------------------- //
// Global functions.
// ----------------------------------------------------------------------------- //

//--------------------------------------------------------------------------------------------------------
static const char * s_WeaponAliasInfo[] = 
{
	"none",		// WEAPON_NONE
	"maul"		// WEAPON_MAUL
	"sword_shield"		// WEAPON_SWORD
	"javelin",	// WEAPON_JAVELIN
	NULL,		// WEAPON_NONE
};

//--------------------------------------------------------------------------------------------------------
//
// Given an alias, return the associated weapon ID
//
int AliasToWeaponID( const char *alias )
{
	if (alias)
	{
		for( int i=0; s_WeaponAliasInfo[i] != NULL; ++i )
			if (!Q_stricmp( s_WeaponAliasInfo[i], alias ))
				return i;
	}

	return WEAPON_NONE;
}

//--------------------------------------------------------------------------------------------------------
//
// Given a weapon ID, return its alias
//
const char *WeaponIDToAlias( int id )
{
	if ( (id >= WEAPON_MAX) || (id < 0) )
		return NULL;

	return s_WeaponAliasInfo[id];
}

// ----------------------------------------------------------------------------- //
// CTDPWeaponBase tables.
// ----------------------------------------------------------------------------- //

IMPLEMENT_NETWORKCLASS_ALIASED( TDPWeaponBase, DT_TDPWeaponBase )

BEGIN_NETWORK_TABLE( CTDPWeaponBase, DT_TDPWeaponBase )
#ifdef CLIENT_DLL
  
#else
	// world weapon models have no animations
  	SendPropExclude( "DT_AnimTimeMustBeFirst", "m_flAnimTime" ),
	SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTDPWeaponBase )
	DEFINE_PRED_FIELD( m_flTimeWeaponIdle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_NOERRORCHECK ),
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tdp_weapon_base, CTDPWeaponBase );


#ifdef GAME_DLL

	BEGIN_DATADESC( CTDPWeaponBase )

		// New weapon Think and Touch Functions go here..

	END_DATADESC()

#endif

// ----------------------------------------------------------------------------- //
// CWeaponCSBase implementation. 
// ----------------------------------------------------------------------------- //
CTDPWeaponBase::CTDPWeaponBase()
{
	SetPredictionEligible( true );

	AddSolidFlags( FSOLID_TRIGGER ); // Nothing collides with these but it gets touches.
}

const CTDPWeaponInfo &CTDPWeaponBase::GetTDPWpnData() const
{
	const FileWeaponInfo_t *pWeaponInfo = &GetWpnData();
	const CTDPWeaponInfo *pTDPInfo;

	#ifdef _DEBUG
		pTDPInfo = dynamic_cast< const CTDPWeaponInfo* >( pWeaponInfo );
		Assert( pTDPInfo );
	#else
		pTDPInfo = static_cast< const CTDPWeaponInfo* >( pWeaponInfo );
	#endif

	return *pTDPInfo;
}

bool CTDPWeaponBase::PlayEmptySound()
{
	CPASAttenuationFilter filter( this );
	filter.UsePredictionRules();

	EmitSound( filter, entindex(), "Default.ClipEmpty_Rifle" );
	
	return 0;
}

CTDPPlayer* CTDPWeaponBase::GetPlayerOwner() const
{
	return dynamic_cast< CTDPPlayer* >( GetOwner() );
}

#ifdef GAME_DLL

void CTDPWeaponBase::SendReloadEvents()
{
	CTDPPlayer *pPlayer = dynamic_cast< CTDPPlayer* >( GetOwner() );
	if ( !pPlayer )
		return;

	// Send a message to any clients that have this entity to play the reload.
	CPASFilter filter( pPlayer->GetAbsOrigin() );
	filter.RemoveRecipient( pPlayer );

	UserMessageBegin( filter, "ReloadEffect" );
	WRITE_SHORT( pPlayer->entindex() );
	MessageEnd();

	// Make the player play his reload animation.
	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD );
}

#endif