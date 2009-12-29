//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef TDP_CLIENTMODE_H
#define TDP_CLIENTMODE_H
#ifdef _WIN32
#pragma once
#endif

#include "clientmode_shared.h"
#include "tdpviewport.h"

class ClientModeTDPNormal : public ClientModeShared 
{
DECLARE_CLASS( ClientModeTDPNormal, ClientModeShared );

private:

// IClientMode overrides.
public:

					ClientModeTDPNormal();
	virtual			~ClientModeTDPNormal();

	virtual void	InitViewport();

	virtual float	GetViewModelFOV( void );

	int				GetDeathMessageStartHeight( void );

	virtual void	FireGameEvent( KeyValues * event);
	virtual void	PostRenderVGui();

	
private:
	
	//	void	UpdateSpectatorMode( void );

};


extern IClientMode *GetClientModeNormal();
extern ClientModeTDPNormal* GetClientModeTDPNormal();


#endif // TDP_CLIENTMODE_H
