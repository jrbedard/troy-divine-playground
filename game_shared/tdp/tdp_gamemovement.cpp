//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#include "cbase.h"
#include "gamemovement.h"
#include "tdp_gamerules.h"
#include "tdp_shareddefs.h"
#include "in_buttons.h"
#include "movevars_shared.h"


#ifdef CLIENT_DLL
	#include "c_tdp_player.h"
#else
	#include "tdp_player.h"
#endif


class CTDPGameMovement : public CGameMovement
{
public:
	DECLARE_CLASS( CTDPGameMovement, CGameMovement );

	CTDPGameMovement();
};


// Expose our interface.
static CTDPGameMovement g_GameMovement;
IGameMovement *g_pGameMovement = ( IGameMovement * )&g_GameMovement;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CGameMovement, IGameMovement,INTERFACENAME_GAMEMOVEMENT, g_GameMovement );


// ---------------------------------------------------------------------------------------- //
// CTDPGameMovement.
// ---------------------------------------------------------------------------------------- //

CTDPGameMovement::CTDPGameMovement()
{
	//m_vecViewOffsetNormal = TDP_PLAYER_VIEW_OFFSET;
}

