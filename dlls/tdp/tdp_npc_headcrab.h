//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef TDP_NPC_HEADCRAB_H
#define TDP_NPC_HEADCRAB_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//	>> CTDP_NPC_Headcrab
//=========================================================
class CTDPHeadcrab : public CAI_BaseNPC
{
	DECLARE_CLASS( CTDPHeadcrab, CAI_BaseNPC ); 

public:

	void Spawn( void );
	void Precache( void );

	virtual int   TranslateSchedule( int scheduleType );
	virtual Activity NPC_TranslateActivity( Activity eNewActivity );
	void  HandleAnimEvent( animevent_t *pEvent );

	DEFINE_CUSTOM_AI; // set this class up, so it can do custom AI interaction, 
	// in our case we only declare some animation events
	DECLARE_DATADESC();
};

#endif