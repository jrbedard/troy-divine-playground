//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include <cdll_client_int.h>

#include "tdpteammenu.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>

#include <vgui_controls/RichText.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/HTML.h>

#include "IGameUIFuncs.h" // for key bindings
#include <igameresources.h>
extern IGameUIFuncs *gameuifuncs; // for key binding details

#include <cl_dll/iviewport.h>

#include <stdlib.h> // MAX_PATH define
#include <stdio.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

void UpdateCursorState();
// void DuckMessage(const char *str);



CON_COMMAND( chooseteam, "Opens a menu for teamchoose" )
{
	if ( !gViewPortInterface )
		return;
	gViewPortInterface->ShowPanel( "team", true );
}

CON_COMMAND( joingreeks, "Join the Troyan team" )
{
	if ( !gViewPortInterface )
		return;
	engine->ClientCmd( const_cast<char *>( "jointeam 2" ) );	// see define in sharedef.h
}

CON_COMMAND( jointroyans, "Join the Troyan team" )
{
	if ( !gViewPortInterface )
		return;
	engine->ClientCmd( const_cast<char *>( "jointeam 3" ) );
}


CON_COMMAND( autojoin, "Auto joins a team" )
{
	if ( !gViewPortInterface )
		return;
	engine->ClientCmd( const_cast<char *>( "jointeam 0" ) );
}


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_TDPTeamMenu::C_TDPTeamMenu(IViewPort *pViewPort) : Frame(NULL, PANEL_TEAM )
{
	m_pViewPort = pViewPort;
	m_iJumpKey = KEY_NONE; // this is looked up in Activate()
	m_iScoreBoardKey = KEY_NONE; // this is looked up in Activate()

	// initialize dialog
	SetTitle("", true);

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);

	// hide the system buttons
	SetTitleBarVisible( false );
	SetProportional(true);

	// info window about this map
	//m_pMapInfo = new RichText( this, "MapInfo" );

	LoadControlSettings("Resource/UI/TDPTeamMenu.res");
	InvalidateLayout();

	m_szMapName[0] = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
C_TDPTeamMenu::~C_TDPTeamMenu()
{
}

//-----------------------------------------------------------------------------
// Purpose: sets the text color of the map description field
//-----------------------------------------------------------------------------
void C_TDPTeamMenu::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	//m_pMapInfo->SetFgColor( pScheme->GetColor("MapDescriptionText", Color(255, 255, 255, 0)) );

	if ( *m_szMapName )
	{
		//LoadMapPage( m_szMapName ); // reload the map description to pick up the color
	}
}

//-----------------------------------------------------------------------------
// Purpose: makes the user choose the auto assign option
//-----------------------------------------------------------------------------
void C_TDPTeamMenu::AutoAssign()
{
	engine->ClientCmd("jointeam 0");
}


//-----------------------------------------------------------------------------
// Purpose: shows the team menu
//-----------------------------------------------------------------------------
void C_TDPTeamMenu::ShowPanel(bool bShow)
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
	{
		Activate();

		SetMouseInputEnabled( true );

		// get key bindings if shown

		if( m_iJumpKey == KEY_NONE ) // you need to lookup the jump key AFTER the engine has loaded
		{
			m_iJumpKey = gameuifuncs->GetVGUI2KeyCodeForBind( "jump" );
		}

		if ( m_iScoreBoardKey == KEY_NONE ) 
		{
			m_iScoreBoardKey = gameuifuncs->GetVGUI2KeyCodeForBind( "showscores" );
		}
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
	}

	m_pViewPort->ShowBackGround( bShow );
}


//-----------------------------------------------------------------------------
// Purpose: updates the UI with a new map name and map html page, and sets up the team buttons
//-----------------------------------------------------------------------------
void C_TDPTeamMenu::Update()
{
	char mapname[MAX_MAP_NAME];

	Q_FileBase( engine->GetLevelName(), mapname, sizeof(mapname) );

	SetLabelText( "mapname", mapname );

	LoadMapPage( mapname );
}

//-----------------------------------------------------------------------------
// Purpose: chooses and loads the text page to display that describes mapName map
//-----------------------------------------------------------------------------
void C_TDPTeamMenu::LoadMapPage( const char *mapName )
{
	// Save off the map name so we can re-load the page in ApplySchemeSettings().
	Q_strncpy( m_szMapName, mapName, strlen( mapName ) + 1 );
	
	char mapRES[ MAX_PATH ];

	Q_snprintf( mapRES, sizeof( mapRES ), "maps/%s.txt", mapName);

	// if no map specific description exists, load default text
	if( !vgui::filesystem()->FileExists( mapRES ) )
	{
		if ( vgui::filesystem()->FileExists( "maps/default.txt" ) )
		{
			Q_snprintf ( mapRES, sizeof( mapRES ), "maps/default.txt");
		}
		else
		{
			return; 
		}
	}

	FileHandle_t f = vgui::filesystem()->Open( mapRES, "r" );

	// read into a memory block
	int fileSize = vgui::filesystem()->Size(f) ;
	wchar_t *memBlock = (wchar_t *)malloc(fileSize + sizeof(wchar_t));
	memset( memBlock, 0x0, fileSize + sizeof(wchar_t));
	vgui::filesystem()->Read(memBlock, fileSize, f);

	// null-terminate the stream
	memBlock[fileSize / sizeof(wchar_t)] = 0x0000;

	// check the first character, make sure this a little-endian unicode file
	if (memBlock[0] != 0xFEFF)
	{
		// its a ascii char file
		//m_pMapInfo->SetText( reinterpret_cast<char *>( memBlock ) );
		
	}
	else
	{
		//m_pMapInfo->SetText( memBlock+1 );
	}
	// go back to the top of the text buffer
	//m_pMapInfo->GotoTextStart();

	vgui::filesystem()->Close( f );
	free(memBlock);

	InvalidateLayout();
	Repaint();

}

/*
//-----------------------------------------------------------------------------
// Purpose: sets the text on and displays the team buttons
//-----------------------------------------------------------------------------
void C_TDPTeamMenu::MakeTeamButtons(void)
{
	int i = 0;

	for( i = 0; i< m_pTeamButtons.Count(); i++ )
	{
		m_pTeamButtons[i]->SetVisible(false);
	}

	i = 0;

	while( true )
	{
		const char *teamname = GameResources()->GetTeamName( i );

		if ( !teamname || !teamname[0] )
			return; // no more teams
	
		char buttonText[32];
		Q_snprintf( buttonText, sizeof(buttonText), "&%i %s", i +1, teamname ); 
		m_pTeamButtons[i]->SetText( buttonText );

		m_pTeamButtons[i]->SetCommand( new KeyValues("TeamButton", "team", i ) );	
		IScheme *pScheme = scheme()->GetIScheme( GetScheme() );
		m_pTeamButtons[i]->SetArmedColor(pScheme->GetColor(GetStringTeamColor(i), Color(255, 255, 255, 255))  ,  pScheme->GetColor("SelectionBG", Color(255, 255, 255, 0)) );
		m_pTeamButtons[i]->SetDepressedColor( pScheme->GetColor(GetStringTeamColor(i), Color(255, 255, 255, 255)), pScheme->GetColor("ButtonArmedBgColor", Color(255, 255, 255, 0)) );
		m_pTeamButtons[i]->SetDefaultColor( pScheme->GetColor(GetStringTeamColor(i), Color(255, 255, 255, 255)), pScheme->GetColor("ButtonDepressedBgColor", Color(255, 255, 255, 0)) );
		m_pTeamButtons[i]->SetVisible(true);

		i++;
	}
} 


//-----------------------------------------------------------------------------
// Purpose: When a team button is pressed it triggers this function to cause the player to join a team
//-----------------------------------------------------------------------------
void C_TDPTeamMenu::OnTeamButton( int team )
{
	char cmd[64];
	if( team >= m_iNumTeams )  // its a special button
	{
		if( team == m_iNumTeams ) // first extra team is auto assign	
		{
			Q_snprintf( cmd, sizeof( cmd ), "jointeam 5" );
		}
		else // next is spectate
		{
			// DuckMessage( "#Spec_Duck" );
			gViewPortInterface->ShowBackGround( false );
		}
	}
	else
	{
		Q_snprintf( cmd, sizeof( cmd ), "jointeam %i", team + 1 );
		//g_iTeamNumber = team + 1;
	}

	engine->ClientCmd(cmd);
	SetVisible( false );
	OnClose();
} */

//-----------------------------------------------------------------------------
// Purpose: Sets the text of a control by name
//-----------------------------------------------------------------------------
void C_TDPTeamMenu::SetLabelText(const char *textEntryName, const char *text)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));
	if (entry)
	{
		entry->SetText(text);
	}
}

void C_TDPTeamMenu::OnKeyCodePressed(KeyCode code)
{
	if( m_iJumpKey!=KEY_NONE && m_iJumpKey == code )
	{
		AutoAssign();
	}
	else if ( m_iScoreBoardKey!=KEY_NONE && m_iScoreBoardKey == code )
	{
		gViewPortInterface->ShowPanel( PANEL_SCOREBOARD, true );
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}

/**
* Description of the function.
* @param arguments
* @return arguments
*/ 
void C_TDPTeamMenu::OnCommand( const char *command )
{
	if ( Q_stricmp( command, "vguicancel" ) )
	{
		engine->ClientCmd( const_cast<char *>( command ) );
	}
	Close();
	gViewPortInterface->ShowBackGround( false );
	BaseClass::OnCommand(command);
}
