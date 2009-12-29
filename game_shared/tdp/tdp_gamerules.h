//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: The TF Game rules object
//
// $Workfile:     $
// $Date: 2005/01/27 17:39:16 $
// $NoKeywords: $
//=============================================================================//

#ifndef TDP_GAMERULES_H
#define TDP_GAMERULES_H

#ifdef _WIN32
#pragma once
#endif


#include "teamplay_gamerules.h"
#include "convar.h"
#include "gamevars_shared.h"

#ifdef CLIENT_DLL
	#include "c_baseplayer.h"
#else
	#include "player.h"
#endif


#ifdef CLIENT_DLL
	#define CTDPGameRules C_TDPGameRules
	#define CTDPGameRulesProxy C_TDPGameRulesProxy
#endif


class CTDPGameRulesProxy : public CGameRulesProxy
{
public:
	DECLARE_CLASS( CTDPGameRulesProxy, CGameRulesProxy );
	DECLARE_NETWORKCLASS();
};


class CTDPGameRules : public CTeamplayRules
{
public:
	DECLARE_CLASS( CTDPGameRules, CTeamplayRules );

	virtual bool	ShouldCollide( int collisionGroup0, int collisionGroup1 );

	virtual int		PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget );
	virtual bool	IsTeamplay( void ) { return false;	}

#ifdef CLIENT_DLL

	DECLARE_CLIENTCLASS_NOBASE(); // This makes datatables able to access our private vars.

#else

	DECLARE_SERVERCLASS_NOBASE(); // This makes datatables able to access our private vars.
	
	CTDPGameRules();
	virtual ~CTDPGameRules();

	virtual bool ClientCommand( const char *pcmd, CBaseEntity *pEdict );
	virtual void RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore );
	virtual void Think();

	virtual const char *GetChatPrefix( bool bTeamOnly, CBasePlayer *pPlayer );

private:

	void RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore, bool bIgnoreWorld );


#endif
};

//-----------------------------------------------------------------------------
// Gets us at the team fortress game rules
//-----------------------------------------------------------------------------

inline CTDPGameRules* TDPGameRules()
{
	return static_cast<CTDPGameRules*>(g_pGameRules);
}


#endif // TDP_GAMERULES_H
