//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Ram vehicule (T:DP)
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "vehicle_base.h"
#include "engine/IEngineSound.h"
#include "in_buttons.h"
#include "ammodef.h"  //TODO: should remove or maybe not for the javelins
#include "IEffects.h"
#include "beam_shared.h"
#include "soundenvelope.h"
#include "decals.h"
#include "soundent.h"
#include "te_effect_dispatch.h"
#include "ndebugoverlay.h"
#include "movevars_shared.h"
#include "bone_setup.h"
#include "ai_basenpc.h"
#include "ai_hint.h"
#include "globalstate.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	VEHICLE_HITBOX_DRIVER		1
#define LOCK_SPEED					10

//Ram has no cannon or gun
//#define RAM_GUN_YAW				"vehicle_weapon_yaw"
//#define RAM_GUN_PITCH				"vehicle_weapon_pitch"
//#define RAM_GUN_SPIN				"gun_spin"
//#define RAM_GUN_SPIN_RATE				20
//#define CANNON_MAX_UP_PITCH			20
//#define CANNON_MAX_DOWN_PITCH			20
//#define CANNON_MAX_LEFT_YAW			90
//#define CANNON_MAX_RIGHT_YAW			90

#define OVERTURNED_EXIT_WAITTIME	2.0f

//#define RAM_AMMOCRATE_HITGROUP		5 //ram doesn'T have ammo
#define RAM_WHEEL_COUNT			4

#define RAM_STEERING_SLOW_ANGLE	50.0f
#define RAM_STEERING_FAST_ANGLE	15.0f

//#define	RAM_AMMO_CRATE_CLOSE_DELAY	2.0f

#define RAM_DELTA_LENGTH_MAX	12.0f			// 1 foot
#define RAM_FRAMETIME_MIN		1e-6

ConVar	hud_jeephint_numentries( "hud_jeephint_numentries", "10", FCVAR_NONE ); // TODO: find what this is for, and change jeep for ram
ConVar	g_jeepexitspeed( "g_jeepexitspeed", "100", FCVAR_CHEAT );				// TODO: find what this is for, and change jeep for ram

//=============================================================================
//
// Ram water data.
//
struct RamWaterData_t
{
	bool		m_bWheelInWater[RAM_WHEEL_COUNT];
	bool		m_bWheelWasInWater[RAM_WHEEL_COUNT];
	Vector		m_vecWheelContactPoints[RAM_WHEEL_COUNT];
	float		m_flNextRippleTime[RAM_WHEEL_COUNT];
	bool		m_bBodyInWater;
	bool		m_bBodyWasInWater;

	DECLARE_SIMPLE_DATADESC();
};

BEGIN_SIMPLE_DATADESC( RamWaterData_t )
	DEFINE_ARRAY( m_bWheelInWater,				FIELD_BOOLEAN,	RAM_WHEEL_COUNT ),
	DEFINE_ARRAY( m_bWheelWasInWater,			FIELD_BOOLEAN,	RAM_WHEEL_COUNT ),
	DEFINE_ARRAY( m_vecWheelContactPoints,		FIELD_VECTOR,	RAM_WHEEL_COUNT ),
	DEFINE_ARRAY( m_flNextRippleTime,			FIELD_TIME,		RAM_WHEEL_COUNT ),
	DEFINE_FIELD( m_bBodyInWater,				FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bBodyWasInWater,			FIELD_BOOLEAN ),
END_DATADESC()	

//-----------------------------------------------------------------------------
// Purpose: Four wheel physics vehicle server vehicle with weaponry
//-----------------------------------------------------------------------------
class CRamFourWheelServerVehicle : public CFourWheelServerVehicle
{
	typedef CFourWheelServerVehicle BaseClass;

// IVehicle
	//void			InitSoundParams( vbs_sound_update_t &params );
	//void			SoundState_Update( vbs_sound_update_t &params );
	bool            HandlePassengerExit( CBasePlayer *pPlayer );
	bool            GetPassengerExitPoint( int nRole, Vector *pExitPoint, QAngle *pAngles );
	void			GetVehicleViewPosition( int nRole, Vector *pOrigin, QAngle *pAngles );
	CBasePlayer*	GetPassenger( int nRole = VEHICLE_DRIVER ); //Peho + Luc: added
	int				GetPassengerRole( CBasePlayer *pPassenger ); //Peho + Luc: added

// IServerVehicle
public:
	//bool		NPC_HasPrimaryWeapon( void ) { return true; } //TODO: me be used for AI
	//void		NPC_AimPrimaryWeapon( Vector vecTarget );//TODO: me be used for AI
	int			GetExitAnimToUse( Vector &vecEyeExitEndpoint, bool &bAllPointsBlocked );
	void		HandlePassengerEntry( CBasePlayer *pPlayer, bool bAllowEntryOutsideZone = false );
	bool		IsPassengerUsingStandardWeapons( int nRole = VEHICLE_DRIVER );
	void		SetPassenger( int nRole, CBasePlayer *pPassenger );
	void		GetPassengerStartPoint( int nRole, Vector *pPoint, QAngle *pAngles );
	bool		IsPassengerVisible( int nRole);
	bool		IsPassengerDamagable( int nRole  = VEHICLE_DRIVER ) { return true; } //Peho + Luc: added

// Player Driving
	CBaseEntity		*GetDriver( void ); //Peho + Luc: added

	CBaseEntity*    m_passenger;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CPropRam : public CPropVehicleDriveable
{
	DECLARE_CLASS( CPropRam, CPropVehicleDriveable );

public:

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CPropRam( void );

	// CPropVehicle
	virtual void	ProcessMovement( CBasePlayer *pPlayer, CMoveData *pMoveData );
	virtual void	DriveVehicle( float flFrameTime, CUserCmd *ucmd, int iButtonsDown, int iButtonsReleased );
	virtual void	SetupMove( CBasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move );
	virtual void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual void	DampenEyePosition( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles );
	virtual bool	AllowBlockedExit( CBasePlayer *pPlayer, int nRole ) { return false; }
	virtual bool	CanExitVehicle( CBaseEntity *pEntity );
	virtual bool	IsVehicleBodyInWater() { return m_WaterData.m_bBodyInWater; }
    //virtual bool		CanEnterVehicle( CBaseEntity *pEntity ); //TODO from Peho + Luc: add it...
	virtual void		SetVehicleEntryAnim( bool bOn ) { m_bEnterAnimOn = bOn; } //Peho + Luc:added
	virtual void		SetVehicleExitAnim( bool bOn, Vector vecEyeExitEndpoint ) { m_bExitAnimOn = bOn; if ( bOn ) m_vecEyeExitEndpoint = vecEyeExitEndpoint; }//Peho + Luc:added 

	// CBaseEntity
	void			Think(void);
	void			Precache( void );
	void			Spawn( void ); 

	virtual void	CreateServerVehicle( void );
	virtual Vector	BodyTarget( const Vector &posSrc, bool bNoisy = true );
	//virtual void	TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
	virtual int		OnTakeDamage( const CTakeDamageInfo &info );
	virtual void	EnterVehicle( CBasePlayer *pPlayer, int iRole );
	virtual void	ExitVehicle( int iRole );
	CBaseEntity		*GetDriver( void ); 
	CBaseEntity     *GetPassenger( int iRole ); //PEho+luc: added
/*
	void			AimGunAt( Vector *endPos, float flInterval );
	bool			TauCannonHasBeenCutOff( void ) { return m_bGunHasBeenCutOff; }
*/

	// NPC Driving, shouldn't happen
/*
	bool			NPC_HasPrimaryWeapon( void ) { return true; }
	void			NPC_AimPrimaryWeapon( Vector vecTarget );
*/

	const char		*GetTracerType( void ) { return "AR2Tracer"; }
	//void			DoImpactEffect( trace_t &tr, int nDamageType );
/*
	bool HeadlightIsOn( void ) { return m_bHeadlightIsOn; }
	void HeadlightTurnOn( void ) { m_bHeadlightIsOn = true; }
	void HeadlightTurnOff( void ) { m_bHeadlightIsOn = false; }
*/

private:

	/*
	void		FireCannon( void );
	void		ChargeCannon( void );
	void		FireChargedCannon( void );

	void		DrawBeam( const Vector &startPos, const Vector &endPos, float width );
	void		StopChargeSound( void );
	void		GetCannonAim( Vector *resultDir );
	*/

	void		InitWaterData( void );
	void		HandleWater( void );
	bool		CheckWater( void );
	void		CheckWaterLevel( void );
	void		CreateSplash( const Vector &vecPosition );
	void		CreateRipple( const Vector &vecPosition );

	void		UpdateSteeringAngle( void );
	void		CreateDangerSounds( void );

	void		ComputePDControllerCoefficients( float *pCoefficientsOut, float flFrequency, float flDampening, float flDeltaTime );
	void		DampenForwardMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime );
	void		DampenUpMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime );

	/*
	void		InputStartRemoveTauCannon( inputdata_t &inputdata );
	void		InputFinishRemoveTauCannon( inputdata_t &inputdata );
	*/

private:
	float			m_flDangerSoundTime;
/*
	bool			m_bGunHasBeenCutOff;
	int				m_nBulletType;
	bool			m_bCannonCharging;
	float			m_flCannonTime;
	float			m_flCannonChargeStartTime;
	Vector			m_vecGunOrigin;
	CSoundPatch		*m_sndCannonCharge;
	int				m_nSpinPos;
	float			m_aimYaw;
	float			m_aimPitch;
	*/
	float			m_throttleDisableTime;
	float			m_flAmmoCrateCloseTime;

	// handbrake after the fact to keep vehicles from rolling
	float			m_flHandbrakeTime;
	bool			m_bInitialHandbrake;
	
	float			m_flOverturnedTime;

	Vector			m_vecLastEyePos;
	Vector			m_vecLastEyeTarget;
	Vector			m_vecEyeSpeed;
	Vector			m_vecTargetSpeed;

	RamWaterData_t	m_WaterData;

	int				m_iNumberOfEntries;
	int				m_nAmmoType;

	// Seagull perching
	float			m_flPlayerExitedTime;	// Time at which the player last left this vehicle
	float			m_flLastSawPlayerAt;	// Time at which we last saw the player
	EHANDLE			m_hLastPlayerInVehicle;
	bool			m_bHasPoop;

	CNetworkVar( bool, m_bHeadlightIsOn );
//protected:
	//CHandle<C_BasePlayer>		m_hPassenger;		
};

BEGIN_DATADESC( CPropRam )

	//DEFINE_FIELD( m_bGunHasBeenCutOff, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flDangerSoundTime, FIELD_TIME ),
	//DEFINE_FIELD( m_nBulletType, FIELD_INTEGER ),
	//DEFINE_FIELD( m_bCannonCharging, FIELD_BOOLEAN ),
	//DEFINE_FIELD( m_flCannonTime, FIELD_TIME ),
	//DEFINE_FIELD( m_flCannonChargeStartTime, FIELD_TIME ),
	//DEFINE_FIELD( m_vecGunOrigin, FIELD_POSITION_VECTOR ),
	//DEFINE_SOUNDPATCH( m_sndCannonCharge ),
	//DEFINE_FIELD( m_nSpinPos, FIELD_INTEGER ),
	//DEFINE_FIELD( m_aimYaw, FIELD_FLOAT ),
	//DEFINE_FIELD( m_aimPitch, FIELD_FLOAT ),
	DEFINE_FIELD( m_throttleDisableTime, FIELD_TIME ),
	DEFINE_FIELD( m_flHandbrakeTime, FIELD_TIME ),
	DEFINE_FIELD( m_bInitialHandbrake, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flOverturnedTime, FIELD_TIME ),
	//DEFINE_FIELD( m_flAmmoCrateCloseTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_vecLastEyePos, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vecLastEyeTarget, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vecEyeSpeed, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vecTargetSpeed, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_bHeadlightIsOn, FIELD_BOOLEAN ),
	DEFINE_EMBEDDED( m_WaterData ),

	DEFINE_FIELD( m_iNumberOfEntries, FIELD_INTEGER ),
	//DEFINE_FIELD( m_nAmmoType, FIELD_INTEGER ),

	DEFINE_FIELD( m_flPlayerExitedTime, FIELD_TIME ),
	DEFINE_FIELD( m_flLastSawPlayerAt, FIELD_TIME ),
	DEFINE_FIELD( m_hLastPlayerInVehicle, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bHasPoop, FIELD_BOOLEAN ),

	//DEFINE_INPUTFUNC( FIELD_VOID, "StartRemoveTauCannon", InputStartRemoveTauCannon ),
	//DEFINE_INPUTFUNC( FIELD_VOID, "FinishRemoveTauCannon", InputFinishRemoveTauCannon ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CPropRam, DT_PropRam )
	//SendPropBool( SENDINFO( m_bHeadlightIsOn ) ),
END_SEND_TABLE();

LINK_ENTITY_TO_CLASS( prop_vehicle_ram, CPropRam );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CPropRam::CPropRam( void )
{
	m_bHasGun = false;
	/*
	m_bGunHasBeenCutOff = false;
	m_bCannonCharging = false;
	m_flCannonChargeStartTime = 0;
	m_flCannonTime = 0;
	m_nBulletType = -1;
	*/
	m_flOverturnedTime = 0.0f;
	m_iNumberOfEntries = 0;

	m_vecEyeSpeed.Init();

	InitWaterData();

	m_bUnableToFire = true; //TODO: check that value
	//m_flAmmoCrateCloseTime = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropRam::CreateServerVehicle( void )
{
	// Create our armed server vehicle
	m_pServerVehicle = new CRamFourWheelServerVehicle();
	m_pServerVehicle->SetVehicle( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropRam::Precache( void )
{
	UTIL_PrecacheOther( "npc_seagull" );
	
	/*
	PrecacheScriptSound( "PropRam.AmmoClose" );
	PrecacheScriptSound( "PropRam.FireCannon" );
	PrecacheScriptSound( "PropRam.FireChargedCannon" );
	PrecacheScriptSound( "PropRam.AmmoOpen" );
	*/

	PrecacheScriptSound( "Ram.GaussCharge" );

//	PrecacheModel( GAUSS_BEAM_SPRITE );

	BaseClass::Precache();
}

//------------------------------------------------
// Spawn
//------------------------------------------------
void CPropRam::Spawn( void )
{
	// Setup vehicle as a real-wheels car.
	SetVehicleType( VEHICLE_TYPE_CAR_WHEELS );

	BaseClass::Spawn();
	m_flHandbrakeTime = gpGlobals->curtime + 0.1;
	m_bInitialHandbrake = false;

	m_VehiclePhysics.SetHasBrakePedal( false );

	m_flMinimumSpeedToEnterExit = LOCK_SPEED;

	//m_nBulletType = GetAmmoDef()->Index("GaussEnergy");

	if ( m_bHasGun )
	{
		SetBodygroup( 1, true );
	}
	else
	{
		SetBodygroup( 1, false );
	}

	// Initialize pose parameters
	/*
	SetPoseParameter( RAM_GUN_YAW, 0 );
	SetPoseParameter( RAM_GUN_PITCH, 0 );
	m_nSpinPos = 0;
	SetPoseParameter( RAM_GUN_SPIN, m_nSpinPos );
	m_aimYaw = 0;
	m_aimPitch = 0;
	*/
	AddSolidFlags( FSOLID_NOT_STANDABLE ); //TODO: see what this can do
	/*
	CAmmoDef *pAmmoDef = GetAmmoDef();
	m_nAmmoType = pAmmoDef->Index("GaussEnergy");
	*/
}
/*
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &tr - 
//			nDamageType - 
//-----------------------------------------------------------------------------
void CPropRam::DoImpactEffect( trace_t &tr, int nDamageType )
{
	//Draw our beam
	//DrawBeam( tr.startpos, tr.endpos, 2.4 );

	if ( (tr.surface.flags & SURF_SKY) == false )
	{
		CPVSFilter filter( tr.endpos );
		te->GaussExplosion( filter, 0.0f, tr.endpos, tr.plane.normal, 0 );

		UTIL_ImpactTrace( &tr, m_nBulletType );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropRam::TraceAttack( const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr )
{
	CTakeDamageInfo info = inputInfo;
	if ( ptr->hitbox != VEHICLE_HITBOX_DRIVER )
	{
		if ( inputInfo.GetDamageType() & DMG_BULLET )
		{
			info.ScaleDamage( 0.0001 );
		}
	}

	BaseClass::TraceAttack( info, vecDir, ptr );
}
*/
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CPropRam::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	//Do scaled up physics damage to the car
	CTakeDamageInfo info = inputInfo;
	info.ScaleDamage( 25 );
	
	// HACKHACK: Scale up grenades until we get a better explosion/pressure damage system
	if ( inputInfo.GetDamageType() & DMG_BLAST )
	{
		info.SetDamageForce( inputInfo.GetDamageForce() * 10 );
	}
	VPhysicsTakeDamage( info );

	// reset the damage
	info.SetDamage( inputInfo.GetDamage() );

	// small amounts of shock damage disrupt the car, but aren't transferred to the player
	if ( info.GetDamageType() == DMG_SHOCK )
	{
		if ( info.GetDamage() <= 10 )
		{
			// take 10% damage and make the engine stall
			info.ScaleDamage( 0.1 );
			m_throttleDisableTime = gpGlobals->curtime + 2;
		}
	}

	//Check to do damage to driver
	if ( GetDriver() )
	{
		//Take no damage from physics damages
		if ( info.GetDamageType() & DMG_CRUSH )
			return 0;

		// Take the damage (strip out the DMG_BLAST)
		info.SetDamageType( info.GetDamageType() & (~DMG_BLAST) );
		GetDriver()->TakeDamage( info );
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Vector CPropRam::BodyTarget( const Vector &posSrc, bool bNoisy )
{
	Vector	shotPos;
	matrix3x4_t	matrix;

	int eyeAttachmentIndex = LookupAttachment("vehicle_driver_eyes");
	GetAttachment( eyeAttachmentIndex, matrix );
	MatrixGetColumn( matrix, 3, shotPos );

	if ( bNoisy )
	{
		shotPos[0] += random->RandomFloat( -8.0f, 8.0f );
		shotPos[1] += random->RandomFloat( -8.0f, 8.0f );
		shotPos[2] += random->RandomFloat( -8.0f, 8.0f );
	}

	return shotPos;
}

//-----------------------------------------------------------------------------
// Purpose: Aim Gun at a target
//-----------------------------------------------------------------------------
/*
void CPropRam::AimGunAt( Vector *endPos, float flInterval )
{
	Vector	aimPos = *endPos;

	// See if the gun should be allowed to aim
	if ( IsOverturned() || m_bEngineLocked )
	{
		SetPoseParameter( JEEP_GUN_YAW, 0 );
		SetPoseParameter( JEEP_GUN_PITCH, 0 );
		SetPoseParameter( JEEP_GUN_SPIN, 0 );
		return;

		// Make the gun go limp and look "down"
		Vector	v_forward, v_up;
		AngleVectors( GetLocalAngles(), NULL, &v_forward, &v_up );
		aimPos = WorldSpaceCenter() + ( v_forward * -32.0f ) - Vector( 0, 0, 128.0f );
	}

	matrix3x4_t gunMatrix;
	GetAttachment( LookupAttachment("gun_ref"), gunMatrix );

	// transform the enemy into gun space
	Vector localEnemyPosition;
	VectorITransform( aimPos, gunMatrix, localEnemyPosition );

	// do a look at in gun space (essentially a delta-lookat)
	QAngle localEnemyAngles;
	VectorAngles( localEnemyPosition, localEnemyAngles );
	
	// convert to +/- 180 degrees
	localEnemyAngles.x = UTIL_AngleDiff( localEnemyAngles.x, 0 );	
	localEnemyAngles.y = UTIL_AngleDiff( localEnemyAngles.y, 0 );

	float targetYaw = m_aimYaw + localEnemyAngles.y;
	float targetPitch = m_aimPitch + localEnemyAngles.x;
	
	// Constrain our angles
	float newTargetYaw	= clamp( targetYaw, -CANNON_MAX_LEFT_YAW, CANNON_MAX_RIGHT_YAW );
	float newTargetPitch = clamp( targetPitch, -CANNON_MAX_DOWN_PITCH, CANNON_MAX_UP_PITCH );

	// If the angles have been clamped, we're looking outside of our valid range
	if ( fabs(newTargetYaw-targetYaw) > 1e-4 || fabs(newTargetPitch-targetPitch) > 1e-4 )
	{
		m_bUnableToFire = true;
	}

	targetYaw = newTargetYaw;
	targetPitch = newTargetPitch;

	// Exponentially approach the target
	float yawSpeed = 8;
	float pitchSpeed = 8;

	m_aimYaw = UTIL_Approach( targetYaw, m_aimYaw, yawSpeed );
	m_aimPitch = UTIL_Approach( targetPitch, m_aimPitch, pitchSpeed );

	SetPoseParameter( JEEP_GUN_YAW, -m_aimYaw);
	SetPoseParameter( JEEP_GUN_PITCH, -m_aimPitch );

	InvalidateBoneCache();

	// read back to avoid drift when hitting limits
	// as long as the velocity is less than the delta between the limit and 180, this is fine.
	m_aimPitch = -GetPoseParameter( JEEP_GUN_PITCH );
	m_aimYaw = -GetPoseParameter( JEEP_GUN_YAW );

	// Now draw crosshair for actual aiming point
	Vector	vecMuzzle, vecMuzzleDir;
	QAngle	vecMuzzleAng;

	GetAttachment( "Muzzle", vecMuzzle, vecMuzzleAng );
	AngleVectors( vecMuzzleAng, &vecMuzzleDir );

	trace_t	tr;
	UTIL_TraceLine( vecMuzzle, vecMuzzle + (vecMuzzleDir * MAX_TRACE_LENGTH), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

		// see if we hit something, if so, adjust endPos to hit location
	if ( tr.fraction < 1.0 )
	{
		m_vecGunCrosshair = vecMuzzle + ( vecMuzzleDir * MAX_TRACE_LENGTH * tr.fraction );
	}
}
*/
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropRam::InitWaterData( void )
{
	m_WaterData.m_bBodyInWater = false;
	m_WaterData.m_bBodyWasInWater = false;

	for ( int iWheel = 0; iWheel < RAM_WHEEL_COUNT; ++iWheel )
	{
		m_WaterData.m_bWheelInWater[iWheel] = false;
		m_WaterData.m_bWheelWasInWater[iWheel] = false;
		m_WaterData.m_vecWheelContactPoints[iWheel].Init();
		m_WaterData.m_flNextRippleTime[iWheel] = 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropRam::HandleWater( void )
{
	// Only check the wheels and engine in water if we have a driver (player).
	if ( !GetDriver() )
		return;

	// Check to see if we are in water.
	if ( CheckWater() )
	{
		for ( int iWheel = 0; iWheel < RAM_WHEEL_COUNT; ++iWheel )
		{
			// Create an entry/exit splash!
			if ( m_WaterData.m_bWheelInWater[iWheel] != m_WaterData.m_bWheelWasInWater[iWheel] )
			{
				CreateSplash( m_WaterData.m_vecWheelContactPoints[iWheel] );
				CreateRipple( m_WaterData.m_vecWheelContactPoints[iWheel] );
			}
			
			// Create ripples.
			if ( m_WaterData.m_bWheelInWater[iWheel] && m_WaterData.m_bWheelWasInWater[iWheel] )
			{
				if ( m_WaterData.m_flNextRippleTime[iWheel] < gpGlobals->curtime )
				{
					// Stagger ripple times
					m_WaterData.m_flNextRippleTime[iWheel] = gpGlobals->curtime + RandomFloat( 0.1, 0.3 );
					CreateRipple( m_WaterData.m_vecWheelContactPoints[iWheel] );
				}
			}
		}
	}

	// Save of data from last think.
	for ( int iWheel = 0; iWheel < RAM_WHEEL_COUNT; ++iWheel )
	{
		m_WaterData.m_bWheelWasInWater[iWheel] = m_WaterData.m_bWheelInWater[iWheel];
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CPropRam::CheckWater( void )
{
	bool bInWater = false;

	// Check all four wheels.
	for ( int iWheel = 0; iWheel < RAM_WHEEL_COUNT; ++iWheel )
	{
		// Get the current wheel and get its contact point.
		IPhysicsObject *pWheel = m_VehiclePhysics.GetWheel( iWheel );
		if ( !pWheel )
			continue;

		// Check to see if we hit water.
		if ( pWheel->GetContactPoint( &m_WaterData.m_vecWheelContactPoints[iWheel], NULL ) )
		{
			m_WaterData.m_bWheelInWater[iWheel] = ( UTIL_PointContents( m_WaterData.m_vecWheelContactPoints[iWheel] ) & MASK_WATER ) ? true : false;
			if ( m_WaterData.m_bWheelInWater[iWheel] )
			{
				bInWater = true;
			}
		}
	}

	// Check the body and the BONNET.
	int iEngine = LookupAttachment( "vehicle_engine" );
	Vector vecEnginePoint;
	QAngle vecEngineAngles;
	GetAttachment( iEngine, vecEnginePoint, vecEngineAngles );

	m_WaterData.m_bBodyInWater = ( UTIL_PointContents( vecEnginePoint ) & MASK_WATER ) ? true : false;
	if ( m_WaterData.m_bBodyInWater )
	{
		if ( m_bHasPoop )
		{
			RemoveAllDecals();
			m_bHasPoop = false;
		}

		if ( !m_VehiclePhysics.IsEngineDisabled() )
		{
			m_VehiclePhysics.SetDisableEngine( true );
		}
	}
	else
	{
		if ( m_VehiclePhysics.IsEngineDisabled() )
		{
			m_VehiclePhysics.SetDisableEngine( false );
		}
	}

	if ( bInWater )
	{
		// Check the player's water level.
		CheckWaterLevel();
	}

	return bInWater;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropRam::CheckWaterLevel( void )
{
	CBaseEntity *pEntity = GetDriver();
	if ( pEntity && pEntity->IsPlayer() )
	{
		CBasePlayer *pPlayer = static_cast<CBasePlayer*>( pEntity );
		
		Vector vecAttachPoint;
		QAngle vecAttachAngles;
		
		// Check eyes. (vehicle_driver_eyes point)
		int iAttachment = LookupAttachment( "vehicle_driver_eyes" );
		GetAttachment( iAttachment, vecAttachPoint, vecAttachAngles );

		// Add the Ram's Z view offset
		Vector vecUp;
		AngleVectors( vecAttachAngles, NULL, NULL, &vecUp );
		vecUp.z = clamp( vecUp.z, 0.0f, vecUp.z );
		vecAttachPoint.z += r_RamViewZHeight.GetFloat() * vecUp.z;

		bool bEyes = ( UTIL_PointContents( vecAttachPoint ) & MASK_WATER ) ? true : false;
		if ( bEyes )
		{
			pPlayer->SetWaterLevel( WL_Eyes );
			return;
		}

		// Check waist.  (vehicle_engine point -- see parent function).
		if ( m_WaterData.m_bBodyInWater )
		{
			pPlayer->SetWaterLevel( WL_Waist );
			return;
		}

		// Check feet. (vehicle_feet_passenger0 point)
		iAttachment = LookupAttachment( "vehicle_feet_passenger0" );
		GetAttachment( iAttachment, vecAttachPoint, vecAttachAngles );
		bool bFeet = ( UTIL_PointContents( vecAttachPoint ) & MASK_WATER ) ? true : false;
		if ( bFeet )
		{
			pPlayer->SetWaterLevel( WL_Feet );
			return;
		}

		// Not in water.
		pPlayer->SetWaterLevel( WL_NotInWater );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropRam::CreateSplash( const Vector &vecPosition )
{
	// Splash data.
	CEffectData	data;
	data.m_fFlags = 0;
	data.m_vOrigin = vecPosition;
	data.m_vNormal.Init( 0.0f, 0.0f, 1.0f );
	VectorAngles( data.m_vNormal, data.m_vAngles );
	data.m_flScale = 10.0f + random->RandomFloat( 0, 2 );

	// Create the splash..
	DispatchEffect( "watersplash", data );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropRam::CreateRipple( const Vector &vecPosition )
{
	// Ripple data.
	CEffectData	data;
	data.m_fFlags = 0;
	data.m_vOrigin = vecPosition;
	data.m_vNormal.Init( 0.0f, 0.0f, 1.0f );
	VectorAngles( data.m_vNormal, data.m_vAngles );
	data.m_flScale = 10.0f + random->RandomFloat( 0, 2 );
	if ( GetWaterType() & CONTENTS_SLIME )
	{
		data.m_fFlags |= FX_WATER_IN_SLIME;
	}

	// Create the ripple.
	DispatchEffect( "waterripple", data );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropRam::Think(void)
{
	BaseClass::Think();

/*
	CBasePlayer	*pPlayer = UTIL_GetLocalPlayer();

	if ( m_bEngineLocked )
	{
		m_bUnableToFire = true;
		
		if ( pPlayer != NULL )
		{
			pPlayer->m_Local.m_iHideHUD |= HIDEHUD_VEHICLE_CROSSHAIR;
		}
	}
	else
	{
		// Start this as false and update it again each frame
		m_bUnableToFire = false;

		if ( pPlayer != NULL )
		{
			pPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_VEHICLE_CROSSHAIR;
		}
	}
*/

	// Water!?
	HandleWater();

	SetSimulationTime( gpGlobals->curtime );
	
	SetNextThink( gpGlobals->curtime );
	SetAnimatedEveryTick( true );

    if ( !m_bInitialHandbrake )	// after initial timer expires, set the handbrake
	{
		m_bInitialHandbrake = true;
		m_VehiclePhysics.SetHandbrake( true );
		m_VehiclePhysics.Think();
	}

	// Check overturned status.
	if ( !IsOverturned() )
	{
		m_flOverturnedTime = 0.0f;
	}
	else
	{
		m_flOverturnedTime += gpGlobals->frametime;
	}

	/*
	// spin gun if charging cannon
	//FIXME: Don't bother for E3
	if ( m_bCannonCharging )
	{
		m_nSpinPos += JEEP_GUN_SPIN_RATE;
		SetPoseParameter( JEEP_GUN_SPIN, m_nSpinPos );
	}
	*/

	/*
	// Aim gun based on the player view direction.
	if ( m_hPlayer && !m_bExitAnimOn && !m_bEnterAnimOn )
	{
		Vector vecEyeDir, vecEyePos;
		m_hPlayer->EyePositionAndVectors( &vecEyePos, &vecEyeDir, NULL, NULL );

		// Trace out from the player's eye point.
		Vector	vecEndPos = vecEyePos + ( vecEyeDir * MAX_TRACE_LENGTH );
		trace_t	trace;
		UTIL_TraceLine( vecEyePos, vecEndPos, MASK_SHOT, this, COLLISION_GROUP_NONE, &trace );

		// See if we hit something, if so, adjust end position to hit location.
		if ( trace.fraction < 1.0 )
		{
   			vecEndPos = vecEyePos + ( vecEyeDir * MAX_TRACE_LENGTH * trace.fraction );
		}

		//m_vecLookCrosshair = vecEndPos;
		AimGunAt( &vecEndPos, 0.1f );
	}
	*/

	StudioFrameAdvance();

	// If the enter or exit animation has finished, tell the server vehicle
	if ( IsSequenceFinished() && (m_bExitAnimOn || m_bEnterAnimOn) )
	{
		if ( m_bEnterAnimOn )
		{
			m_VehiclePhysics.ReleaseHandbrake();
			StartEngine();

			// HACKHACK: This forces the jeep to play a sound when it gets entered underwater
			if ( m_VehiclePhysics.IsEngineDisabled() )
			{
				CBaseServerVehicle *pServerVehicle = dynamic_cast<CBaseServerVehicle *>(GetServerVehicle());
				if ( pServerVehicle )
				{
					pServerVehicle->SoundStartDisabled();
				}
			}

			// The first few time we get into the jeep, print the jeep help
			if ( m_iNumberOfEntries < hud_jeephint_numentries.GetInt() )
			{
				UTIL_HudHintText( m_hPlayer, "#Valve_Hint_JeepKeys" );
				m_iNumberOfEntries++;
			}
		}
		
		// If we're exiting and have had the tau cannon removed, we don't want to reset the animation
		GetServerVehicle()->HandleEntryExitFinish( m_bExitAnimOn, true );
	}

	// See if the ammo crate needs to close
	if ( ( m_flAmmoCrateCloseTime < gpGlobals->curtime ) && ( GetSequence() == LookupSequence( "ammo_open" ) ) )
	{
		m_flAnimTime = gpGlobals->curtime;
		m_flPlaybackRate = 0.0;
		SetCycle( 0 );
		ResetSequence( LookupSequence( "ammo_close" ) );
	}
	else if ( ( GetSequence() == LookupSequence( "ammo_close" ) ) && IsSequenceFinished() )
	{
		m_flAnimTime = gpGlobals->curtime;
		m_flPlaybackRate = 0.0;
		SetCycle( 0 );
		ResetSequence( LookupSequence( "idle" ) );

		CPASAttenuationFilter sndFilter( this, "PropJeep.AmmoClose" );
		EmitSound( sndFilter, entindex(), "PropJeep.AmmoClose" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &startPos - 
//			&endPos - 
//			width - 
//			useMuzzle - 
//-----------------------------------------------------------------------------
/*
void CPropRam::DrawBeam( const Vector &startPos, const Vector &endPos, float width )
{	
	//Tracer down the middle
	UTIL_Tracer( startPos, endPos, 0, TRACER_DONT_USE_ATTACHMENT, 6500, false, "GaussTracer" );

	//Draw the main beam shaft
	CBeam *pBeam = CBeam::BeamCreate( GAUSS_BEAM_SPRITE, 0.5 );
	
	pBeam->SetStartPos( startPos );
	pBeam->PointEntInit( endPos, this );
	pBeam->SetEndAttachment( LookupAttachment("Muzzle") );
	pBeam->SetWidth( width );
	pBeam->SetEndWidth( 0.05f );
	pBeam->SetBrightness( 255 );
	pBeam->SetColor( 255, 185+random->RandomInt( -16, 16 ), 40 );
	pBeam->RelinkBeam();
	pBeam->LiveForTime( 0.1f );

	//Draw electric bolts along shaft
	pBeam = CBeam::BeamCreate( GAUSS_BEAM_SPRITE, 3.0f );
	
	pBeam->SetStartPos( startPos );
	pBeam->PointEntInit( endPos, this );
	pBeam->SetEndAttachment( LookupAttachment("Muzzle") );

	pBeam->SetBrightness( random->RandomInt( 64, 255 ) );
	pBeam->SetColor( 255, 255, 150+random->RandomInt( 0, 64 ) );
	pBeam->RelinkBeam();
	pBeam->LiveForTime( 0.1f );
	pBeam->SetNoise( 1.6f );
	pBeam->SetEndWidth( 0.1f ); 
}
*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*
void CPropRam::FireCannon( void )
{	
	//Don't fire again if it's been too soon
	if ( m_flCannonTime > gpGlobals->curtime )
		return;

	if ( m_bUnableToFire )
		return;

	m_flCannonTime = gpGlobals->curtime + 0.2f;
	m_bCannonCharging = false;

	//Find the direction the gun is pointing in
	Vector aimDir;
	GetCannonAim( &aimDir );

	FireBulletsInfo_t	info( 1, m_vecGunOrigin, aimDir, VECTOR_CONE_1DEGREES, MAX_TRACE_LENGTH, m_nAmmoType );

	info.m_nFlags = FIRE_BULLETS_ALLOW_WATER_SURFACE_IMPACTS;
	info.m_pAttacker = m_hPlayer;

	FireBullets( info );

	// Register a muzzleflash for the AI
	if ( m_hPlayer )
	{
		m_hPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );
	}

	CPASAttenuationFilter sndFilter( this, "PropJeep.FireCannon" );
	EmitSound( sndFilter, entindex(), "PropJeep.FireCannon" );
	
	// make cylinders of gun spin a bit
	m_nSpinPos += JEEP_GUN_SPIN_RATE;
	//SetPoseParameter( JEEP_GUN_SPIN, m_nSpinPos );	//FIXME: Don't bother with this for E3, won't look right
	
}
*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*
void CPropRam::FireChargedCannon( void )
{
	bool penetrated = false;

	m_bCannonCharging	= false;
	m_flCannonTime		= gpGlobals->curtime + 0.5f;

	StopChargeSound();

	CPASAttenuationFilter sndFilter( this, "PropJeep.FireChargedCannon" );
	EmitSound( sndFilter, entindex(), "PropJeep.FireChargedCannon" );

	//Find the direction the gun is pointing in
	Vector aimDir;
	GetCannonAim( &aimDir );

	Vector endPos = m_vecGunOrigin + ( aimDir * MAX_TRACE_LENGTH );
	
	//Shoot a shot straight out
	trace_t	tr;
	UTIL_TraceLine( m_vecGunOrigin, endPos, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
	
	ClearMultiDamage();

	//Find how much damage to do
	float flChargeAmount = ( gpGlobals->curtime - m_flCannonChargeStartTime ) / MAX_GAUSS_CHARGE_TIME;

	//Clamp this
	if ( flChargeAmount > 1.0f )
	{
		flChargeAmount = 1.0f;
	}

	//Determine the damage amount
	//FIXME: Use ConVars!
	float flDamage = 15 + ( ( 250 - 15 ) * flChargeAmount );

	CBaseEntity *pHit = tr.m_pEnt;
	
	//Look for wall penetration
	if ( tr.DidHitWorld() && !(tr.surface.flags & SURF_SKY) )
	{
		//Try wall penetration
		UTIL_ImpactTrace( &tr, m_nBulletType, "ImpactJeep" );
		UTIL_DecalTrace( &tr, "RedGlowFade" );

		CPVSFilter filter( tr.endpos );
		te->GaussExplosion( filter, 0.0f, tr.endpos, tr.plane.normal, 0 );
		
		Vector	testPos = tr.endpos + ( aimDir * 48.0f );

		UTIL_TraceLine( testPos, tr.endpos, MASK_SHOT, GetDriver(), COLLISION_GROUP_NONE, &tr );
			
		if ( tr.allsolid == false )
		{
			UTIL_DecalTrace( &tr, "RedGlowFade" );

			penetrated = true;
		}
	}
	else if ( pHit != NULL )
	{
		CTakeDamageInfo dmgInfo( this, GetDriver(), flDamage, DMG_SHOCK );
		CalculateBulletDamageForce( &dmgInfo, GetAmmoDef()->Index("GaussEnergy"), aimDir, tr.endpos, 1.0f + flChargeAmount * 4.0f );

		//Do direct damage to anything in our path
		pHit->DispatchTraceAttack( dmgInfo, aimDir, &tr );
	}

	ApplyMultiDamage();

	//Kick up an effect
	if ( !(tr.surface.flags & SURF_SKY) )
	{
  		UTIL_ImpactTrace( &tr, m_nBulletType, "ImpactJeep" );

		//Do a gauss explosion
		CPVSFilter filter( tr.endpos );
		te->GaussExplosion( filter, 0.0f, tr.endpos, tr.plane.normal, 0 );
	}

	//Show the effect
	DrawBeam( m_vecGunOrigin, tr.endpos, 9.6 );

	// Register a muzzleflash for the AI
	if ( m_hPlayer )
	{
		m_hPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5f );
	}

	//Rock the car
	IPhysicsObject *pObj = VPhysicsGetObject();

	if ( pObj != NULL )
	{
		Vector	shoveDir = aimDir * -( flDamage * 500.0f );

		pObj->ApplyForceOffset( shoveDir, m_vecGunOrigin );
	}

	//Do radius damage if we didn't penetrate the wall
	if ( penetrated == true )
	{
		RadiusDamage( CTakeDamageInfo( this, this, flDamage, DMG_SHOCK ), tr.endpos, 200.0f, CLASS_NONE, NULL );
	} 
}
*/
/*
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropJeep::ChargeCannon( void )
{
	//Don't fire again if it's been too soon
	if ( m_flCannonTime > gpGlobals->curtime )
		return;

	//See if we're starting a charge
	if ( m_bCannonCharging == false )
	{
		m_flCannonChargeStartTime = gpGlobals->curtime;
		m_bCannonCharging = true;

		//Start charging sound
		CPASAttenuationFilter filter( this );
		m_sndCannonCharge = (CSoundEnvelopeController::GetController()).SoundCreate( filter, entindex(), CHAN_STATIC, "Jeep.GaussCharge", ATTN_NORM );

		assert(m_sndCannonCharge!=NULL);
		if ( m_sndCannonCharge != NULL )
		{
			(CSoundEnvelopeController::GetController()).Play( m_sndCannonCharge, 1.0f, 50 );
			(CSoundEnvelopeController::GetController()).SoundChangePitch( m_sndCannonCharge, 250, 3.0f );
		}

		return;
	}

	//TODO: Add muzzle effect?

	//TODO: Check for overcharge and have the weapon simply fire or instead "decharge"?
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropJeep::StopChargeSound( void )
{
	if ( m_sndCannonCharge != NULL )
	{
		(CSoundEnvelopeController::GetController()).SoundFadeOut( m_sndCannonCharge, 0.1f );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Finds the true aiming position of the gun (looks at what player 
//			is looking at and adjusts)
// Input  : &resultDir - direction to be calculated
//-----------------------------------------------------------------------------
void CPropJeep::GetCannonAim( Vector *resultDir )
{
	Vector	muzzleOrigin;
	QAngle	muzzleAngles;

	GetAttachment( LookupAttachment("gun_ref"), muzzleOrigin, muzzleAngles );

	AngleVectors( muzzleAngles, resultDir );
}
*/
//-----------------------------------------------------------------------------
// Purpose: If the player uses the jeep while at the back, he gets ammo from the crate instead
//-----------------------------------------------------------------------------
void CPropRam::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = ToBasePlayer( pActivator );
	
	if ( pPlayer == NULL)
		return;

	// Find out if the player's looking at our ammocrate hitbox 
	Vector vecForward;
	pPlayer->EyeVectors( &vecForward, NULL, NULL );

	trace_t tr;
	Vector vecStart = pPlayer->EyePosition();
	UTIL_TraceLine( vecStart, vecStart + vecForward * 1024, MASK_SOLID | CONTENTS_DEBRIS | CONTENTS_HITBOX, pPlayer, COLLISION_GROUP_NONE, &tr );
	/*
	if ( tr.m_pEnt == this && tr.hitgroup == RAM_AMMOCRATE_HITGROUP )
	{
		// Player's using the crate.
		// Fill up his SMG ammo.
		pPlayer->GiveAmmo( 300, "SMG1");
		
		if ( ( GetSequence() != LookupSequence( "ammo_open" ) ) && ( GetSequence() != LookupSequence( "ammo_close" ) ) )
		{
			// Open the crate
			m_flAnimTime = gpGlobals->curtime;
			m_flPlaybackRate = 0.0;
			SetCycle( 0 );
			ResetSequence( LookupSequence( "ammo_open" ) );
			
			CPASAttenuationFilter sndFilter( this, "PropJeep.AmmoOpen" );
			EmitSound( sndFilter, entindex(), "PropJeep.AmmoOpen" );
		}

		m_flAmmoCrateCloseTime = gpGlobals->curtime + JEEP_AMMO_CRATE_CLOSE_DELAY;
		return;
	}
	*/
	// Fall back and get in the vehicle instead
	BaseClass::Use( pActivator, pCaller, useType, value );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CPropRam::CanExitVehicle( CBaseEntity *pEntity )
{
	return ( !m_bEnterAnimOn && !m_bExitAnimOn && !m_bLocked && (m_nSpeed <= g_jeepexitspeed.GetFloat() ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropRam::DampenEyePosition( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles )
{
	// Get the frametime. (Check to see if enough time has passed to warrent dampening).
	float flFrameTime = gpGlobals->frametime;
	if ( flFrameTime < RAM_FRAMETIME_MIN )
	{
		vecVehicleEyePos = m_vecLastEyePos;
		DampenUpMotion( vecVehicleEyePos, vecVehicleEyeAngles, 0.0f );
		return;
	}

	// Keep static the sideways motion.

	// Dampen forward/backward motion.
	DampenForwardMotion( vecVehicleEyePos, vecVehicleEyeAngles, flFrameTime );

	// Blend up/down motion.
	DampenUpMotion( vecVehicleEyePos, vecVehicleEyeAngles, flFrameTime );
}

//-----------------------------------------------------------------------------
// Use the controller as follows:
// speed += ( pCoefficientsOut[0] * ( targetPos - currentPos ) + pCoefficientsOut[1] * ( targetSpeed - currentSpeed ) ) * flDeltaTime;
//-----------------------------------------------------------------------------
void CPropRam::ComputePDControllerCoefficients( float *pCoefficientsOut,float flFrequency, float flDampening,float flDeltaTime )
{
	float flKs = 9.0f * flFrequency * flFrequency;
	float flKd = 4.5f * flFrequency * flDampening;

	float flScale = 1.0f / ( 1.0f + flKd * flDeltaTime + flKs * flDeltaTime * flDeltaTime );

	pCoefficientsOut[0] = flKs * flScale;
	pCoefficientsOut[1] = ( flKd + flKs * flDeltaTime ) * flScale;
}
 
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropRam::DampenForwardMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime )
{
	// Get forward vector.
	Vector vecForward;
	AngleVectors( vecVehicleEyeAngles, &vecForward);

	// Simulate the eye position forward based on the data from last frame
	// (assumes no acceleration - it will get that from the "spring").
	Vector vecCurrentEyePos = m_vecLastEyePos + m_vecEyeSpeed * flFrameTime;

	// Calculate target speed based on the current vehicle eye position and the last vehicle eye position and frametime.
	Vector vecVehicleEyeSpeed = ( vecVehicleEyePos - m_vecLastEyeTarget ) / flFrameTime;
	m_vecLastEyeTarget = vecVehicleEyePos;	

	// Calculate the speed and position deltas.
	Vector vecDeltaSpeed = vecVehicleEyeSpeed - m_vecEyeSpeed;
	Vector vecDeltaPos = vecVehicleEyePos - vecCurrentEyePos;

	// Clamp.
	if ( vecDeltaPos.Length() > RAM_DELTA_LENGTH_MAX )
	{
		float flSign = vecForward.Dot( vecVehicleEyeSpeed ) >= 0.0f ? -1.0f : 1.0f;
		vecVehicleEyePos += flSign * ( vecForward * RAM_DELTA_LENGTH_MAX );
		m_vecLastEyePos = vecVehicleEyePos;
		m_vecEyeSpeed = vecVehicleEyeSpeed;
		return;
	}

	// Generate an updated (dampening) speed for use in next frames position extrapolation.
	float flCoefficients[2];
	ComputePDControllerCoefficients( flCoefficients, r_RamViewDampenFreq.GetFloat(), r_RamViewDampenDamp.GetFloat(), flFrameTime );
	m_vecEyeSpeed += ( ( flCoefficients[0] * vecDeltaPos + flCoefficients[1] * vecDeltaSpeed ) * flFrameTime );

	// Save off data for next frame.
	m_vecLastEyePos = vecCurrentEyePos;

	// Move eye forward/backward.
	Vector vecForwardOffset = vecForward * ( vecForward.Dot( vecDeltaPos ) );
	vecVehicleEyePos -= vecForwardOffset;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropRam::DampenUpMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime )
{
	// Get up vector.
	Vector vecUp;
	AngleVectors( vecVehicleEyeAngles, NULL, NULL, &vecUp );
	vecUp.z = clamp( vecUp.z, 0.0f, vecUp.z );
	vecVehicleEyePos.z += r_JeepViewZHeight.GetFloat() * vecUp.z;

	// NOTE: Should probably use some damped equation here.
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropRam::SetupMove( CBasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move )
{
	// If we are overturned and hit any key - leave the vehicle (IN_USE is already handled!).
	if ( m_flOverturnedTime > OVERTURNED_EXIT_WAITTIME )
	{
		if ( (ucmd->buttons & (IN_FORWARD|IN_BACK|IN_MOVELEFT|IN_MOVERIGHT|IN_SPEED|IN_JUMP|IN_ATTACK|IN_ATTACK2) ) && !m_bExitAnimOn )
		{
			// Can't exit yet? We're probably still moving. Swallow the keys.
			if ( !CanExitVehicle(player) )
				return;

			if ( !GetServerVehicle()->HandlePassengerExit( m_hPlayer ) && ( m_hPlayer != NULL ) )
			{
				m_hPlayer->PlayUseDenySound();
			}
			return;
		}
	}

	// If the throttle is disabled or we're upside-down, don't allow throttling (including turbo)
	CUserCmd tmp;
	if ( ( m_throttleDisableTime > gpGlobals->curtime ) || ( IsOverturned() ) )
	{
		m_bUnableToFire = true;
		
		tmp = (*ucmd);
		tmp.buttons &= ~(IN_FORWARD|IN_BACK|IN_SPEED);
		ucmd = &tmp;
	}
	
	BaseClass::SetupMove( player, ucmd, pHelper, move );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropRam::DriveVehicle( float flFrameTime, CUserCmd *ucmd, int iButtonsDown, int iButtonsReleased )
{
	//int iButtons = ucmd->buttons;

	//Adrian: No headlights on Superfly.
/*	if ( ucmd->impulse == 100 )
	{
		if (HeadlightIsOn())
		{
			HeadlightTurnOff();
		}
        else 
		{
			HeadlightTurnOn();
		}
	}*/
		
	// If we're holding down an attack button, update our state
	/*
	if ( IsOverturned() == false )
	{
		if ( iButtons & IN_ATTACK )
		{
			if ( m_bCannonCharging )
			{
				FireChargedCannon();
			}
			else
			{
				FireCannon();
			}
		}
		else if ( iButtons & IN_ATTACK2 )
		{
			ChargeCannon();
		}
	}

	// If we've released our secondary button, fire off our cannon
	if ( ( iButtonsReleased & IN_ATTACK2 ) && ( m_bCannonCharging ) )
	{
		FireChargedCannon();
	}
	*/

	BaseClass::DriveVehicle( flFrameTime, ucmd, iButtonsDown, iButtonsReleased );	
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//			*pMoveData - 
//-----------------------------------------------------------------------------
void CPropRam::ProcessMovement( CBasePlayer *pPlayer, CMoveData *pMoveData )
{
	BaseClass::ProcessMovement( pPlayer, pMoveData );

	// Update the steering angles based on speed.
	UpdateSteeringAngle();

	// Create dangers sounds in front of the vehicle.
	CreateDangerSounds();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropRam::UpdateSteeringAngle( void )
{
	float flMaxSpeed = m_VehiclePhysics.GetMaxSpeed();
	float flSpeed = m_VehiclePhysics.GetSpeed();

	float flRatio = 1.0f - ( flSpeed / flMaxSpeed );
	float flSteeringDegrees = RAM_STEERING_FAST_ANGLE + ( ( RAM_STEERING_SLOW_ANGLE - RAM_STEERING_FAST_ANGLE ) * flRatio );
	flSteeringDegrees = clamp( flSteeringDegrees, RAM_STEERING_FAST_ANGLE, RAM_STEERING_SLOW_ANGLE );
	m_VehiclePhysics.SetSteeringDegrees( flSteeringDegrees );
}

//-----------------------------------------------------------------------------
// Purpose: Create danger sounds in front of the vehicle.
//-----------------------------------------------------------------------------
void CPropRam::CreateDangerSounds( void )
{
	QAngle dummy;
	//GetAttachment( "Muzzle", m_vecGunOrigin, dummy ); TODO: be sure this is about the canon

	if ( m_flDangerSoundTime > gpGlobals->curtime )
		return;

	QAngle vehicleAngles = GetLocalAngles();
	Vector vecStart = GetAbsOrigin();
	Vector vecDir, vecRight;

	GetVectors( &vecDir, &vecRight, NULL );

	const float soundDuration = 0.25;
	float speed = m_VehiclePhysics.GetHLSpeed();
	// Make danger sounds ahead of the jeep
	if ( fabs(speed) > 120 )
	{
		Vector	vecSpot;

		float steering = m_VehiclePhysics.GetSteering();
		if ( steering != 0 )
		{
			if ( speed > 0 )
			{
				vecDir += vecRight * steering * 0.5;
			}
			else
			{
				vecDir -= vecRight * steering * 0.5;
			}
			VectorNormalize(vecDir);
		}
		const float radius = speed * 0.4;
		// 0.3 seconds ahead of the jeep
		vecSpot = vecStart + vecDir * (speed * 0.3f);
		CSoundEnt::InsertSound( SOUND_DANGER, vecSpot, radius, soundDuration, this, 0 );
		CSoundEnt::InsertSound( SOUND_PHYSICS_DANGER, vecSpot, radius, soundDuration, this, 1 );
		//NDebugOverlay::Box(vecSpot, Vector(-radius,-radius,-radius),Vector(radius,radius,radius), 255, 0, 255, 0, soundDuration);

#if 0
		trace_t	tr;
		// put sounds a bit to left and right but slightly closer to Jeep to make a "cone" of sound 
		// in front of it
		vecSpot = vecStart + vecDir * (speed * 0.5f) - vecRight * speed * 0.5;
		UTIL_TraceLine( vecStart, vecSpot, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
		CSoundEnt::InsertSound( SOUND_DANGER, vecSpot, 400, soundDuration, this, 1 );

		vecSpot = vecStart + vecDir * (speed * 0.5f) + vecRight * speed * 0.5;
		UTIL_TraceLine( vecStart, vecSpot, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
		CSoundEnt::InsertSound( SOUND_DANGER, vecSpot, 400, soundDuration, this, 2);
#endif
	}

	m_flDangerSoundTime = gpGlobals->curtime + 0.1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropRam::EnterVehicle( CBasePlayer *pPlayer, int iRole )
{
	if ( !pPlayer )
		return;

	CheckWater();
	//Peho: adapt that code... 

	// Remove any player who may be in the vehicle at the moment
	if ( m_hPlayer )
	{
		ExitVehicle(VEHICLE_DRIVER);
	}

	m_hPlayer = pPlayer;
	m_playerOn.FireOutput( pPlayer, this, 0 );

	// Don't start the engine if the player's using an entry animation,
	// because we want to start the engine once the animation is done.
	if ( !m_bEnterAnimOn )
	{
		StartEngine();
	}

	// Start Thinking
	SetNextThink( gpGlobals->curtime );

	m_VehiclePhysics.GetVehicle()->OnVehicleEnter();

	
	BaseClass::EnterVehicle( pPlayer );


	// Start looking for seagulls to land
	m_hLastPlayerInVehicle = m_hPlayer;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropRam::ExitVehicle( int iRole )
{
	//	HeadlightTurnOff();

	BaseClass::ExitVehicle(iRole);
	
	/*************** Peho: replace the line above by that code 
	void CPropVehicleDriveable::ExitVehicle( int iRole )
{
	CBasePlayer *pPlayer = m_hPlayer;
	if ( !pPlayer )
		return;

	m_hPlayer = NULL;
	ResetUseKey( pPlayer );
	
	m_playerOff.FireOutput( pPlayer, this, 0 );

	// clear out the fire buttons
	m_attackaxis.Set( 0, pPlayer, this );
	m_attack2axis.Set( 0, pPlayer, this );

	m_nSpeed = 0;
	m_flThrottle = 0.0f;

	StopEngine();

	m_VehiclePhysics.GetVehicle()->OnVehicleExit();
}
	*/

	// Remember when we last saw the player
	m_flPlayerExitedTime = gpGlobals->curtime;
	m_flLastSawPlayerAt = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseEntity *CPropRam::GetDriver( void ) 
{ 
	//TODO from Peho: this should handle non driver passenger. 
	//NB: there's another def for that method, somewhere... be aware
	return m_hPlayer; 
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseEntity *CPropRam::GetPassenger( int iRole ) 
{ 
	//Peho: this now handle passenger	
	if (iRole == VEHICLE_DRIVER)
	{
		return m_hPlayer; 
	}
	else //if (iRole == VEHICLE_PASSENGER)
	{
		return m_hPlayer; 
	}		
}

/*
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropRam::InputStartRemoveTauCannon( inputdata_t &inputdata )
{
	// Start the gun removal animation
	m_flAnimTime = gpGlobals->curtime;
	m_flPlaybackRate = 0.0;
	SetCycle( 0 );
	ResetSequence( LookupSequence( "tau_levitate" ) );

	m_bGunHasBeenCutOff = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropJeep::InputFinishRemoveTauCannon( inputdata_t &inputdata )
{
	// Remove & hide the gun
	SetBodygroup( 1, false );
	m_bHasGun = false;
}
*/
//========================================================================================================================================
// JEEP FOUR WHEEL PHYSICS VEHICLE SERVER VEHICLE
//========================================================================================================================================

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*
void CRamFourWheelServerVehicle::NPC_AimPrimaryWeapon( Vector vecTarget )
{
	((CPropRam*)m_pVehicle)->AimGunAt( &vecTarget, 0.1f );
}
*/
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &vecEyeExitEndpoint - 
// Output : int
//-----------------------------------------------------------------------------
int CRamFourWheelServerVehicle::GetExitAnimToUse( Vector &vecEyeExitEndpoint, bool &bAllPointsBlocked )
{
	bAllPointsBlocked = false;

	if ( !m_bParsedAnimations )
	{
		// Load the entry/exit animations from the vehicle
		ParseEntryExitAnims();
		m_bParsedAnimations = true;
	}

	CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating *>(m_pVehicle);
	// If we don't have the gun anymore, we want to get out using the "gun-less" animation
	if ( pAnimating )
	{
		// HACK: We know the tau-cannon removed exit anim uses the first upright anim's exit details
		trace_t tr;
		Vector vehicleExitOrigin;
		QAngle vehicleExitAngles;

		// Ensure the endpoint is clear by dropping a point down from above
		pAnimating->GetAttachment( m_ExitAnimations[0].iAttachment, vehicleExitOrigin, vehicleExitAngles );
		vehicleExitOrigin -= VEC_VIEW;
		Vector vecMove = Vector(0,0,64);
		Vector vecStart = vehicleExitOrigin + vecMove;
		Vector vecEnd = vehicleExitOrigin - vecMove;
  		UTIL_TraceHull( vecStart, vecEnd, VEC_HULL_MIN, VEC_HULL_MAX, MASK_SOLID, NULL, COLLISION_GROUP_NONE, &tr );

		Assert( !tr.startsolid && tr.fraction < 1.0 );
		m_vecCurrentExitEndPoint = vecStart + ((vecEnd - vecStart) * tr.fraction);
		vecEyeExitEndpoint = m_vecCurrentExitEndPoint + VEC_VIEW;
		m_iCurrentExitAnim = 0;
		return pAnimating->LookupSequence( "exit_tauremoved" );
	}

	return BaseClass::GetExitAnimToUse( vecEyeExitEndpoint, bAllPointsBlocked );
}

void CRamFourWheelServerVehicle::HandlePassengerEntry( CBasePlayer *pPlayer, bool bAllowEntryOutsideZone )
{
	CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating *>(m_pVehicle);
	if ( !pAnimating )
		return;

	// Find out which hitbox the player's eyepoint is within
	int iEntryAnim = GetEntryAnimForPoint( pPlayer->EyePosition() );

	// Are we in an entrypoint zone? 
	if ( iEntryAnim == ACTIVITY_NOT_AVAILABLE )
	{
		// Normal get in refuses to allow entry
		if ( !bAllowEntryOutsideZone )
			return;

		// We failed to find a valid entry anim, but we've got to get back in because the player's
		// got stuck exiting the vehicle. For now, just use the first get in anim
		// UNDONE: We need a better solution for this.
		iEntryAnim = pAnimating->LookupSequence( m_EntryAnimations[0].szAnimName );
	}

	// Check to see if this vehicle can be controlled or if it's locked
	if ( GetDrivableVehicle()->CanEnterVehicle(pPlayer) )
	{
		if (this->GetPassenger( VEHICLE_DRIVER ))
			pPlayer->GetInVehicle( this, VEHICLE_PASSENGER);
		else
			pPlayer->GetInVehicle( this, VEHICLE_DRIVER);

		// Setup the "enter" vehicle sequence and skip the animation if it isn't present.
		pAnimating->SetCycle( 0 );
		pAnimating->m_flAnimTime = gpGlobals->curtime;
		pAnimating->ResetSequence( iEntryAnim );
		pAnimating->ResetClientsideFrame();
		GetDrivableVehicle()->SetVehicleEntryAnim( true );
	}
}

bool CRamFourWheelServerVehicle::HandlePassengerExit( CBasePlayer *pPlayer )
{
	// Clear hud hints
	UTIL_HudHintText( pPlayer, "" );

	vbs_sound_update_t params;
	//InitSoundParams(params); //TODO by Peho +luc: add this to the architecture, for now, it's disabled...
	params.bExitVehicle = true;
	//SoundState_Update( params ); //TODO by Peho +luc: add this to the architecture, for now, it's disabled...

	// Find the right exit anim to use based on available exit points.
	Vector vecExitPoint;
	bool bAllPointsBlocked;
	int iSequence = GetExitAnimToUse( vecExitPoint, bAllPointsBlocked );

	// If all exit points were blocked and this vehicle doesn't allow exiting in
	// these cases, bail.
	Vector vecNewPos = pPlayer->GetAbsOrigin();
	QAngle angNewAngles = pPlayer->GetAbsAngles();

	int nRole = GetPassengerRole( pPlayer );
	if ( ( bAllPointsBlocked ) || ( iSequence == ACTIVITY_NOT_AVAILABLE ) )
	{
		// Animation-driven exit points are all blocked, or we have none. Fall back to the more simple static exit points.
		if ( !GetPassengerExitPoint( nRole, &vecNewPos, &angNewAngles ) && !GetDrivableVehicle()->AllowBlockedExit( pPlayer, nRole ) )
		{
			return false;
		}
	}

	// Now we either have an exit sequence to play, a valid static exit position, or we don't care
	// whether we're blocked or not. We're getting out, one way or another.
	GetDrivableVehicle()->PreExitVehicle( pPlayer, nRole );

	if ( iSequence > ACTIVITY_NOT_AVAILABLE )
	{
		CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating *>(m_pVehicle);
		if ( pAnimating )
		{
			pAnimating->SetCycle( 0 );
			pAnimating->m_flAnimTime = gpGlobals->curtime;
			pAnimating->ResetSequence( iSequence );
			pAnimating->ResetClientsideFrame();
			GetDrivableVehicle()->SetVehicleExitAnim( true, vecExitPoint );

			// Re-deploy our weapon
			if ( pPlayer && pPlayer->IsAlive() )
			{
				if ( pPlayer->GetActiveWeapon() )
				{
					pPlayer->GetActiveWeapon()->Deploy();
					pPlayer->ShowCrosshair( true );
				}
			}

			// To prevent anything moving into the volume the player's going to occupy at the end of the exit
			Vector vecExitFeetPoint = vecExitPoint - VEC_VIEW;
			m_hExitBlocker = CEntityBlocker::Create( vecExitFeetPoint, VEC_HULL_MIN, VEC_HULL_MAX, NULL, true );
			return true;
		}
	}

	// Couldn't find an animation, so exit immediately
	pPlayer->LeaveVehicle( vecNewPos, angNewAngles );
	return true;
}
/*
void CRamFourWheelServerVehicle::InitSoundParams( vbs_sound_update_t &params )
{
	params.Defaults();
	params.bVehicleInWater = IsVehicleBodyInWater();
}
void CRamFourWheelServerVehicle::SoundState_Update( vbs_sound_update_t &params )
{
	sound_states newState = SoundState_ChooseState( params );
	if ( newState != m_soundState )
	{
		sound_states lastState = m_soundState;
		m_soundState = newState;
		SoundState_OnNewState( lastState );
	}

	switch( m_soundState )
	{
	case SS_SHUTDOWN:
	case SS_SHUTDOWN_WATER:
	case SS_START_WATER:
	case SS_START_IDLE:
	case SS_IDLE:
	case SS_REVERSE:
	case SS_GEAR_0:
	case SS_GEAR_4:
	case SS_SLOWDOWN_HIGHSPEED:
	case SS_SLOWDOWN:
	case SS_GEAR_0_RESUME:
	case SS_GEAR_4_RESUME:
		break;
	default:break;
	}
}
*/
bool CRamFourWheelServerVehicle::IsPassengerUsingStandardWeapons( int nRole )
{ 
	if (VEHICLE_PASSENGER == nRole)
		return true; 

	return false;
}

bool CRamFourWheelServerVehicle::IsPassengerVisible( int nRole) 
{ 
	return true; 
}


void CRamFourWheelServerVehicle::SetPassenger( int nRole, CBasePlayer *pPassenger )
{
	// Baseclass only handles vehicles with a single passenger //TODO peho: change this
	//Assert( nRole == VEHICLE_DRIVER ); //Peho: The vehicle should handle passenger now

	// Getting in? or out?
	if ( pPassenger )
	{
		m_savedViewOffset = pPassenger->GetViewOffset(); //Peho: seems to save the view of the player to restore it when he will leave.
		
		pPassenger->SetViewOffset( vec3_origin );
		pPassenger->ShowCrosshair( false );

		GetDrivableVehicle()->EnterVehicle( pPassenger ); //Peho: should set 2 types of how to enter the vehicle.
	}
	else
	{
		// Restore the exiting player's view offset
		CBasePlayer *pPlayer = dynamic_cast<CBasePlayer *>(GetPassenger(nRole)); //Luc: on devrait changer GetDriver() //Peho: changed for GetPassenger(nRole)
		if ( pPlayer )
		{
			pPlayer->SetViewOffset( m_savedViewOffset ); //Peho: restore the saved view
			pPlayer->ShowCrosshair( true );
		}

		GetDrivableVehicle()->ExitVehicle( nRole );
		GetDrivableVehicle()->SetVehicleEntryAnim( false );
		UTIL_Remove( m_hExitBlocker );
	}

}

//-----------------------------------------------------------------------------
// Purpose: Get a position in *world space* inside the vehicle for the player to start at
//-----------------------------------------------------------------------------
void CRamFourWheelServerVehicle::GetPassengerStartPoint( int nRole, Vector *pPoint, QAngle *pAngles )
{
	//Assert( nRole == VEHICLE_DRIVER ); //TODO peho: should handle other passenger then driver

	// NOTE: We don't set the angles, which causes them to remain the same
	// as they were before entering the vehicle

	CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating *>(m_pVehicle);
	if ( pAnimating )
	{
		char pAttachmentName[32];
		Q_snprintf( pAttachmentName, sizeof( pAttachmentName ), "vehicle_feet_passenger%d", nRole ); //Peho: here.
		int nFeetAttachmentIndex = pAnimating->LookupAttachment(pAttachmentName);
		if ( nFeetAttachmentIndex > 0 )
		{
			QAngle vecAngles;
			pAnimating->GetAttachment( nFeetAttachmentIndex, *pPoint, vecAngles );
			return;
		}
	}

	// Couldn't find the attachment point, so just use the origin
	*pPoint = m_pVehicle->GetAbsOrigin();
}

//-----------------------------------------------------------------------------
// Purpose: Where does this passenger exit the vehicle?
//-----------------------------------------------------------------------------
bool CRamFourWheelServerVehicle::GetPassengerExitPoint( int nRole, Vector *pExitPoint, QAngle *pAngles )
{ 
	//Assert( nRole == VEHICLE_DRIVER ); //TODO peho: this should handle passenger as well as driver.

	// First, see if we've got an attachment point
	CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating *>(m_pVehicle);
	if ( pAnimating )
	{
		Vector vehicleExitOrigin;
		QAngle vehicleExitAngles;

		//Peho: i think the next line is getting the driver, and here should be able to select the passenger also.
		//		So, technically, we should change that special line and everything will be ok.
		if ( pAnimating->GetAttachment( "vehicle_driver_exit", vehicleExitOrigin, vehicleExitAngles ) )
		{
			// Make sure it's clear
			trace_t tr;
			UTIL_TraceHull( vehicleExitOrigin + Vector(0, 0, 12), vehicleExitOrigin, VEC_HULL_MIN, VEC_HULL_MAX, MASK_PLAYERSOLID, m_pVehicle, COLLISION_GROUP_NONE, &tr );
			if ( !tr.startsolid )
			{
				*pAngles = vehicleExitAngles;
				*pExitPoint = tr.endpos;
				return true;
			}
		}
	}

	// left side 
	if( CheckExitPoint( 90, 90, pExitPoint ) )	// angle from car, distance from origin, actual exit point
		return true;

	// right side
	if( CheckExitPoint( -90, 90, pExitPoint ) )
		return true;

	// front
	if( CheckExitPoint( 0, 100, pExitPoint ) )
		return true;

	// back
	if( CheckExitPoint( 180, 170, pExitPoint ) )
		return true;

	// All else failed, try popping them out the top.
	Vector vecWorldMins, vecWorldMaxs;
	m_pVehicle->CollisionProp()->WorldSpaceAABB( &vecWorldMins, &vecWorldMaxs );
	pExitPoint->x = (vecWorldMins.x + vecWorldMaxs.x) * 0.5f;
	pExitPoint->y = (vecWorldMins.y + vecWorldMaxs.y) * 0.5f;
	pExitPoint->z = vecWorldMaxs.z + 50.0f;

	// Make sure it's clear
	trace_t tr;
	UTIL_TraceHull( m_pVehicle->CollisionProp()->WorldSpaceCenter(), *pExitPoint, VEC_HULL_MIN, VEC_HULL_MAX, MASK_PLAYERSOLID, m_pVehicle, COLLISION_GROUP_NONE, &tr );
	if ( !tr.startsolid )
	{
		return true;
	}

	// No clear exit point available!
	return false;
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBasePlayer	*CRamFourWheelServerVehicle::GetPassenger( int nRole ) 
{ 	
	return ToBasePlayer( GetDrivableVehicle()->GetPassenger(nRole) ); // Peho: GetDriver() was changed for GetPassenger(nRole)
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CRamFourWheelServerVehicle::GetPassengerRole( CBasePlayer *pPassenger )
{
	//Peho: GetDrivableVehicle() is actually fetching the PropRam
	if (pPassenger == GetDrivableVehicle()->GetPassenger(VEHICLE_DRIVER) ) //Peho: GetDriver() could be changed for GetPassenger(VEHICLE_DRIVER), should it be discontinued?
	{
		return VEHICLE_DRIVER;
	}
	else if (pPassenger == GetDrivableVehicle()->GetPassenger(VEHICLE_PASSENGER))
	{
		return VEHICLE_PASSENGER;
	}
	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: Where does the passenger see from?
//-----------------------------------------------------------------------------
void CRamFourWheelServerVehicle::GetVehicleViewPosition( int nRole, Vector *pAbsOrigin, QAngle *pAbsAngles )
{
	CBasePlayer *pPlayer = GetPassenger( nRole ); //Peho: VEHICLE_DRIVER changed for nRole
	Assert( pPlayer );

	*pAbsAngles = pPlayer->EyeAngles();
	*pAbsOrigin = m_pVehicle->GetAbsOrigin();
}



//-----------------------------------------------------------------------------
// Purpose: Returns the driver. Unlike GetPassenger(VEHICLE_DRIVER), it will return
//			the NPC driver if it has one.
//-----------------------------------------------------------------------------
CBaseEntity	*CRamFourWheelServerVehicle::GetDriver( void )
{
	return GetDrivableVehicle()->GetDriver(); //Peho: changed.
}
