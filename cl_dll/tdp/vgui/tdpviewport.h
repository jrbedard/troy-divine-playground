//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef TDPVIEWPORT_H
#define TDPVIEWPORT_H


#include "tdp_shareddefs.h"
#include "baseviewport.h"



using namespace vgui;

namespace vgui 
{
	class Panel;
}

class C_TDPViewport : public CBaseViewport
{

private:
	DECLARE_CLASS_SIMPLE( C_TDPViewport, CBaseViewport );

public:

	void CreateDefaultPanels( void );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
		
	int GetDeathMessageStartHeight( void );
};


#endif // TDPViewport_H
