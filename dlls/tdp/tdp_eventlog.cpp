//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "../EventLog.h"
#include "KeyValues.h"

class CTDPEventLog : public CEventLog
{
private:
	typedef CEventLog BaseClass;

public:
	virtual ~CTDPEventLog() {};

public:
	bool PrintEvent( KeyValues * event )	// override virtual function
	{
		if ( BaseClass::PrintEvent( event ) )
		{
			return true;
		}
	
		if ( Q_strcmp(event->GetName(), "cstrike_") == 0 )
		{
			return PrintCStrikeEvent( event );
		}

		return false;
	}

protected:

	bool PrintCStrikeEvent( KeyValues * event )	// print Mod specific logs
	{
	//	const char * name = event->GetName() + Q_strlen("cstrike_"); // remove prefix

		return false;
	}

};

CTDPEventLog g_TDPEventLog;

//-----------------------------------------------------------------------------
// Singleton access
//-----------------------------------------------------------------------------
IGameSystem* GameLogSystem()
{
	return &g_TDPEventLog;
}

