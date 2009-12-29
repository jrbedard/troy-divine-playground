//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side CTFTeam class
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_TDP_TEAM_H
#define C_TDP_TEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "c_team.h"
#include "shareddefs.h"

class C_BaseEntity;
class C_BaseObject;
class CBaseTechnology;

//-----------------------------------------------------------------------------
// Purpose: TF's Team manager
//-----------------------------------------------------------------------------
class C_TDPTeam : public C_Team
{
	DECLARE_CLASS( C_TDPTeam, C_Team );
	DECLARE_CLIENTCLASS();

public:

					C_TDPTeam();
	virtual			~C_TDPTeam();
};


#endif // C_TDP_TEAM_H
