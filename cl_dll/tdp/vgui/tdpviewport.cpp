//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client DLL VGUI2 Viewport
//
// $Workfile:     $
// $Date: 2005/02/09 19:48:51 $
//
//-----------------------------------------------------------------------------
// $Log: tdpviewport.cpp,v $
// Revision 1.2  2005/02/09 19:48:51  godg2002
// *** empty log message ***
//
// Revision 1.1  2005/01/27 17:16:22  bonf1901
// *** empty log message ***
//
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#pragma warning( disable : 4800  )  // disable forcing int to bool performance warning

// VGUI panel includes
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui/Cursor.h>
#include <vgui/IScheme.h>
#include <vgui/IVGUI.h>
#include <vgui/ILocalize.h>
#include <vgui/VGUI.h>

// client dll/engine defines
#include "hud.h"
#include <voice_status.h>

// viewport definitions
#include <baseviewport.h>
#include "tdpviewport.h"

#include "vguicenterprint.h"
#include "text_message.h"

#include "clientscoreboarddialog.h"
#include "vguitextwindow.h"
#include "spectatorgui.h"
#include "NavProgress.h"
#include "tdpteammenu.h"


void C_TDPViewport::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	gHUD.InitColors( pScheme );

	SetPaintBackgroundEnabled( false );
}


void C_TDPViewport::CreateDefaultPanels( void )
{
	AddNewPanel( new CClientScoreBoardDialog( this ) );
	AddNewPanel( new CTextWindow( this ) );
	AddNewPanel( new CSpectatorGUI( this ) );
	AddNewPanel( new CSpectatorMenu( this ) );
	AddNewPanel( new CNavProgress( this ) );
	AddNewPanel( new C_TDPTeamMenu( this ) );
}

int C_TDPViewport::GetDeathMessageStartHeight( void )
{
	int x = YRES(2);

	IViewPortPanel *spectator = gViewPortInterface->FindPanelByName( PANEL_SPECGUI );

	//TODO: Link to actual height of spectator bar
	if ( spectator && spectator->IsVisible() )
	{
		x += YRES(52);
	}

	return x;
}

