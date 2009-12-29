//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "tdp_team.h"
#include "entitylist.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


// Datatable
IMPLEMENT_SERVERCLASS_ST(CTDPTeam, DT_TDPTeam)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( tdp_team_manager, CTDPTeam );

//-----------------------------------------------------------------------------
// Purpose: Get a pointer to the specified TF team manager
//-----------------------------------------------------------------------------
CTDPTeam *GetGlobalTDPTeam( int iIndex )
{
	return (CTDPTeam*)GetGlobalTeam( iIndex );
}


//-----------------------------------------------------------------------------
// Purpose: Needed because this is an entity, but should never be used
//-----------------------------------------------------------------------------
void CTDPTeam::Init( const char *pName, int iNumber )
{
	BaseClass::Init( pName, iNumber );

	// Only detect changes every half-second.
	NetworkProp()->SetUpdateInterval( 0.75f );
}
