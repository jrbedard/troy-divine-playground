// ======================================================================
// Purpose: Simple headcrab that follows paths and charges at a player
// do a 'npc_create tdp_headcrab' in console to spawn this critter
// it will only follow paths, if it can find path nodes !
// ======================================================================

// didn't take the time to sort out which of these are actually needed
#include "cbase.h"
#include "game.h"
#include "ai_basenpc.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "ai_hint.h"
#include "ai_motor.h"
#include "ai_navigator.h"
#include "ai_route.h"
#include "npcevent.h"
#include "gib.h"
#include "ai_interactions.h"
#include "ndebugoverlay.h"
#include "soundent.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "movevars_shared.h"
#include "ai_moveprobe.h"
#include "tdp_npc_headcrab.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Animation events.
//-----------------------------------------------------------------------------
int AE_HEADCRAB_JUMPATTACK;
int AE_HEADCRAB_JUMP_TELEGRAPH;

// you can refer to this critter as tdp_headcrab in Hammer or the console
LINK_ENTITY_TO_CLASS( tdp_headcrab, CTDPHeadcrab ); 

// no additinal data
BEGIN_DATADESC( CTDPHeadcrab )
END_DATADESC()

// Name of our entity's model
#define ENTITY_MODEL  "models/headcrabclassic.mdl"

//-----------------------------------------------------------------------------
// Purpose: Precache assets used by the entity
//-----------------------------------------------------------------------------
void CTDPHeadcrab::Precache( void )
{
	// we ignore all the sounds for now
	PrecacheModel( ENTITY_MODEL );
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
void CTDPHeadcrab::Spawn( void )
{
	Precache(); // precache resources
	SetModel( ENTITY_MODEL ); // set the display model

	BaseClass::Spawn(); // init CAI_BaseNPC's member vars

	m_iHealth = 100.0; // set health of your crab; you'll want to make this a convar

	SetHullType(HULL_TINY);
	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	// we don't want to allow others to stand on this critter
	AddSolidFlags( FSOLID_NOT_STANDABLE );  
	SetMoveType( MOVETYPE_STEP );

	// have to look into this, the original crab has it's own collision
	SetCollisionGroup( COLLISION_GROUP_NPC ); 
	// model defined in hl2 specific code

	SetViewOffset( Vector(6, 0, 11) ) ;   // Position of the eyes relative to NPC's origin.

	SetBloodColor( BLOOD_COLOR_GREEN );   // guess ;-)
	m_flFieldOfView   = 0.5;
	m_NPCState    = NPC_STATE_NONE;

	CapabilitiesClear();
	// crab can move around and start melee attack
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_INNATE_MELEE_ATTACK1 );

	NPCInit();  // initialize our NPC

	// Crab hates player, set to D_FR to see it run away from player
	AddClassRelationship( CLASS_PLAYER, D_HT, 0 );
	AddClassRelationship( CLASS_COMBINE, D_HT, 0 );

}

//-----------------------------------------------------------------------------
// Purpose: Override schedules
//
//-----------------------------------------------------------------------------

int CTDPHeadcrab::TranslateSchedule( int scheduleType )
{
	switch ( scheduleType )
	{
	case SCHED_IDLE_STAND: // make the crab walk around instead of just standing there idly
		return SCHED_PATROL_WALK;
		break;
	default:
		return BaseClass::TranslateSchedule( scheduleType );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Override activities
//
//-----------------------------------------------------------------------------

Activity CTDPHeadcrab::NPC_TranslateActivity( Activity eNewActivity )
{
	if ( eNewActivity == ACT_MELEE_ATTACK1) {
		// the regular headcrab actually does a range attack by jumping at you, so the mdl
		// doesn't have a melee attack, so we fudge the attack activity
		return ACT_RANGE_ATTACK1;
	} 
	else
	{
		return eNewActivity;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handle animation events
//
//-----------------------------------------------------------------------------

void CTDPHeadcrab::HandleAnimEvent( animevent_t *pEvent )
{
	// Animation events are triggered by the mdl;
	// we handle the attack related events to prevent errors about unhandled events
	if ( pEvent->event == AE_HEADCRAB_JUMPATTACK )
	{
		DevMsg("JumpAttack!"); // no damage for now, have to look into melee damage some more
		return;
	}

	if ( pEvent->event == AE_HEADCRAB_JUMP_TELEGRAPH ) 
	{
		return;
	}

	BaseClass::HandleAnimEvent( pEvent );
}

// Declare all your custom activities, tasks, schedules, and animation events here 
AI_BEGIN_CUSTOM_NPC( tdp_headcrab, CTDPHeadcrab )

DECLARE_ANIMEVENT( AE_HEADCRAB_JUMPATTACK )
DECLARE_ANIMEVENT( AE_HEADCRAB_JUMP_TELEGRAPH )

AI_END_CUSTOM_NPC()