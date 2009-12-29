/*
	Base projectile for Troy : Divine Playground

	Author : Jean-René
*/

#include "cbase.h"
//#include "decals.h"
#include "baseprojectile_shared.h"
//#include "shake.h"
#include "engine/IEngineSound.h"

#if !defined( CLIENT_DLL ) // server
	#include "soundent.h"
	#include "entitylist.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern short	g_sModelIndexFireball;		// (in combatweapon.cpp) holds the index for the fireball 
extern short	g_sModelIndexWExplosion;	// (in combatweapon.cpp) holds the index for the underwater explosion
extern short	g_sModelIndexSmoke;			// (in combatweapon.cpp) holds the index for the smoke cloud
extern ConVar    sk_plr_dmg_grenade;

#if !defined( CLIENT_DLL )

// Global Savedata for friction modifier
BEGIN_DATADESC( CBaseProjectile )
	//					nextGrenade
	DEFINE_FIELD( m_hThrower, FIELD_EHANDLE ),
	//					m_fRegisteredSound ???
	DEFINE_FIELD( m_bIsLive, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flDamage, FIELD_FLOAT ),
END_DATADESC()

void SendProxy_CropFlagsToPlayerFlagBitsLength( const SendProp *pProp, const void *pStruct, const void *pVarData, DVariant *pOut, int iElement, int objectID);

#endif

IMPLEMENT_NETWORKCLASS_ALIASED( BaseProjectile, DT_BaseProjectile ) // declare data table

BEGIN_NETWORK_TABLE( CBaseProjectile, DT_BaseProjectile )
#if !defined( CLIENT_DLL ) // server
	SendPropFloat( SENDINFO( m_flDamage ), 10, SPROP_ROUNDDOWN, 0.0, 256.0f ),
	SendPropInt( SENDINFO( m_bIsLive ), 1, SPROP_UNSIGNED ),
	SendPropEHandle( SENDINFO( m_hThrower ) ),
	SendPropVector( SENDINFO( m_vecVelocity ), 0, SPROP_NOSCALE ), 
	
	// HACK: Use same flag bits as player for now
	SendPropInt			( SENDINFO(m_fFlags), PLAYER_FLAG_BITS, SPROP_UNSIGNED, SendProxy_CropFlagsToPlayerFlagBitsLength ),

	SendPropTime( SENDINFO( m_flNextAttack ) ),

#else // client
	
	RecvPropFloat( RECVINFO( m_flDamage ) ),
	RecvPropInt( RECVINFO( m_bIsLive ) ),
	RecvPropEHandle( RECVINFO( m_hThrower ) ),

	// Need velocity from projectiles to make animation system work correctly when running
	RecvPropVector( RECVINFO(m_vecVelocity), 0, RecvProxy_LocalVelocity ),

	RecvPropInt( RECVINFO( m_fFlags ) ),
	RecvPropTime( RECVINFO( m_flNextAttack ) ),
#endif

END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( projectile, CBaseProjectile );

BEGIN_PREDICTION_DATA( CBaseProjectile  )

	DEFINE_PRED_FIELD( m_hThrower, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bIsLive, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flDamage, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),

	DEFINE_PRED_FIELD_TOL( m_vecVelocity, FIELD_VECTOR, FTYPEDESC_INSENDTABLE, 0.5f ),
	DEFINE_PRED_FIELD_TOL( m_flNextAttack, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ),

END_PREDICTION_DATA()


void CBaseProjectile::Event_Killed( const CTakeDamageInfo &info )
{
	//Detonate( );
	// fade-out here...
}


#if !defined( CLIENT_DLL )
	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CBaseProjectile::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
	{
		// Support player pickup
		if ( useType == USE_TOGGLE )
		{
			CBasePlayer *pPlayer = ToBasePlayer( pActivator );
			if ( pPlayer )
			{
				pPlayer->PickupObject( this );
				return;
			}
		}

		// Pass up so we still call any custom Use function
		BaseClass::Use( pActivator, pCaller, useType, value );
	}
#endif



void CBaseProjectile::Precache( void )
{
	BaseClass::Precache( );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : CBaseCombatCharacter
//-----------------------------------------------------------------------------
CBaseCombatCharacter *CBaseProjectile::GetThrower( void )
{
	CBaseCombatCharacter *pResult = ToBaseCombatCharacter( m_hThrower );
	if ( !pResult && GetOwnerEntity() != NULL )
	{
		pResult = ToBaseCombatCharacter( GetOwnerEntity() );
	}
	return pResult;
}

//-----------------------------------------------------------------------------

void CBaseProjectile::SetThrower( CBaseCombatCharacter *pThrower )
{
	m_hThrower = pThrower;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
// Input  :
// Output :
//-----------------------------------------------------------------------------
CBaseProjectile::~CBaseProjectile(void)
{
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
// Input  :
// Output :
//-----------------------------------------------------------------------------
CBaseProjectile::CBaseProjectile(void)
{
	m_hThrower			= NULL;
	m_bIsLive			= false;

	SetSimulatedEveryTick( true );
};














