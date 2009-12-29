//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "tdp_weapon_base.h"
#include "gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "tdp_weapon_javelin.h"




#ifdef CLIENT_DLL
	
	#include "c_tdp_player.h"

	// for the motion blur FX
	#include "model_types.h"
	#include "ClientEffectPrecacheSystem.h"
	#include "fx.h"
	#include "c_te_effect_dispatch.h"

	extern void DrawHalo( IMaterial* pMaterial, const Vector &source, float scale, float const *color );

	CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectCrossbow )
	CLIENTEFFECT_MATERIAL( "effects/muzzleflash1" )
	CLIENTEFFECT_REGISTER_END()

#else

	#include "tdp_player.h"
	#include "items.h"

#endif

/*

IMPLEMENT_NETWORKCLASS_ALIASED( TDPJavelinProjectile, DT_TDPJavelinProjectile )

BEGIN_NETWORK_TABLE( CTDPJavelinProjectile, DT_TDPJavelinProjectile )
#ifdef CLIENT_DLL
RecvPropVector( RECVINFO( m_vInitialVelocity ) )
#else
SendPropVector( SENDINFO( m_vInitialVelocity ), 
			   20,		// nbits
			   0,		// flags
			   -3000,	// low value
			   3000	// high value
			   )
#endif
END_NETWORK_TABLE()
*/



IMPLEMENT_NETWORKCLASS_ALIASED( TDPJavelinProjectile, DT_TDPJavelinProjectile )

BEGIN_NETWORK_TABLE( CTDPJavelinProjectile, DT_TDPJavelinProjectile )
#ifdef CLIENT_DLL
RecvPropVector( RECVINFO( m_vInitialVelocity ) )
#else
SendPropVector( SENDINFO( m_vInitialVelocity ), 
			   20,		// nbits
			   0,		// flags
			   -3000,	// low value
			   3000	// high value
			   )
#endif
			   END_NETWORK_TABLE()



LINK_ENTITY_TO_CLASS( javelin_projectile, CTDPJavelinProjectile );
PRECACHE_WEAPON_REGISTER( javelin_projectile );



#ifdef CLIENT_DLL   // Server Projectile

	
	void CTDPJavelinProjectile::PostDataUpdate( DataUpdateType_t type )
	{
		BaseClass::PostDataUpdate( type );

		if ( type == DATA_UPDATE_CREATED )
		{
			// Now stick our initial velocity into the interpolation history 
			CInterpolatedVar< Vector > &interpolator = GetOriginInterpolator();

			interpolator.ClearHistory();
			float changeTime = GetLastChangeTime( LATCH_SIMULATION_VAR );

			// Add a sample 1 second back.
			Vector vCurOrigin = GetLocalOrigin() - m_vInitialVelocity;
			interpolator.AddToHead( changeTime - 1.0, &vCurOrigin, false );

			// Add the current sample.
			vCurOrigin = GetLocalOrigin();
			interpolator.AddToHead( changeTime, &vCurOrigin, false );
		}
	}


	void CTDPJavelinProjectile::OnDataChanged( DataUpdateType_t updateType )
	{
		BaseClass::OnDataChanged( updateType );


		if ( updateType == DATA_UPDATE_CREATED )
		{
			m_bUpdated = false;
			m_vecLastOrigin = GetAbsOrigin();
			SetNextClientThink( CLIENT_THINK_ALWAYS );
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CTDPJavelinProjectile::ClientThink( void )
	{
		m_bUpdated = false;
	}


	int CTDPJavelinProjectile::DrawModel( int flags )
	{
		// During the first half-second of our life, don't draw ourselves if he's
		// still playing his throw animation.
		// (better yet, we could draw ourselves in his hand).
		if ( GetThrower() != C_BasePlayer::GetLocalPlayer() )
		{
			if ( gpGlobals->curtime - m_flSpawnTime < 0.5 )
			{
				C_TDPPlayer *pPlayer = dynamic_cast<C_TDPPlayer*>( GetThrower() );
				if ( pPlayer && pPlayer->m_PlayerAnimState->IsThrowingGrenade() )
				{
					return 0;
				}
			}
		}

		// FX by JRB
		// See if we're drawing the motion blur
		if ( flags & STUDIO_TRANSPARENCY )
		{
			float		color[3];
			IMaterial	*pBlurMaterial = materials->FindMaterial( "effects/muzzleflash1", NULL, false );

			Vector	vecDir = GetAbsOrigin() - m_vecLastOrigin;
			float	speed = VectorNormalize( vecDir );

			speed = clamp( speed, 0, 32 );

			if ( speed > 0 )
			{
				float	stepSize = min( ( speed * 0.5f ), 4.0f );

				Vector	spawnPos = GetAbsOrigin() + ( vecDir * 24.0f );
				Vector	spawnStep = -vecDir * stepSize;

				materials->Bind( pBlurMaterial );

				float	alpha;

				// Draw the motion blurred trail
				for ( int i = 0; i < 100; i++ )
				{
					spawnPos += spawnStep;

					alpha = RemapValClamped( i, 5, 11, 0.25f, 0.05f );

					color[0] = color[1] = color[2] = alpha;

					DrawHalo( pBlurMaterial, spawnPos, 5.0f, color );
				}
			}

			if ( gpGlobals->frametime > 0.0f && !m_bUpdated)
			{
				m_bUpdated = true;
				m_vecLastOrigin = GetAbsOrigin();
			}

			return 1;
		}


		return BaseClass::DrawModel( flags );
	}


#else   // TDP JAVELIN PROJECTILE SERVER


	//-----------------------------------------------------------------------------
	// Purpose: 
	// Input  : *pOther - 
	//-----------------------------------------------------------------------------
	void CTDPJavelinProjectile::ProjectileTouch( CBaseEntity *pOther )
	{
		if ( !pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS) )
			return;

		if ( pOther->m_takedamage != DAMAGE_NO )
		{
			trace_t	tr, tr2;
			tr = BaseClass::GetTouchTrace();
			Vector	vecNormalizedVel = GetAbsVelocity();

			ClearMultiDamage();
			VectorNormalize( vecNormalizedVel );

			if( GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && pOther->IsNPC() )
			{
				CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), 100.0f, DMG_NEVERGIB );
				dmgInfo.AdjustPlayerDamageInflictedForSkillLevel();
				CalculateMeleeDamageForce( &dmgInfo, vecNormalizedVel, tr.endpos, 0.7f );
				dmgInfo.SetDamagePosition( tr.endpos );
				pOther->DispatchTraceAttack( dmgInfo, vecNormalizedVel, &tr );
			}
			else
			{
				CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), 100.0f, DMG_BULLET | DMG_NEVERGIB );
				//CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), 100.0f, DMG_BULLET | DMG_NEVERGIB );
				CalculateMeleeDamageForce( &dmgInfo, vecNormalizedVel, tr.endpos, 0.7f );
				dmgInfo.SetDamagePosition( tr.endpos );
				pOther->DispatchTraceAttack( dmgInfo, vecNormalizedVel, &tr );
			}

			ApplyMultiDamage();

			//Adrian: keep going through the glass.
			if ( pOther->GetCollisionGroup() == COLLISION_GROUP_BREAKABLE_GLASS )
				return;

			SetAbsVelocity( Vector( 0, 0, 0 ) );

			// play body "thwack" sound
			EmitSound( "Weapon_Crossbow.BoltHitBody" );

			Vector vForward;

			AngleVectors( GetAbsAngles(), &vForward );
			VectorNormalize ( vForward );

			UTIL_TraceLine( GetAbsOrigin(),	GetAbsOrigin() + vForward * 128, MASK_OPAQUE, pOther, COLLISION_GROUP_NONE, &tr2 );

			if ( tr2.fraction != 1.0f )
			{
				//			NDebugOverlay::Box( tr2.endpos, Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 255, 0, 0, 10 );
				//			NDebugOverlay::Box( GetAbsOrigin(), Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 0, 255, 0, 10 );

				if ( tr2.m_pEnt == NULL || ( tr2.m_pEnt && tr2.m_pEnt->GetMoveType() == MOVETYPE_NONE ) )
				{
					//CEffectData	data;

					//data.m_vOrigin = tr2.endpos;
					//data.m_vNormal = vForward;
					//data.m_nEntIndex = tr2.fraction != 1.0f;

					//DispatchEffect( "BoltImpact", data );
				}
			}

			SetTouch( NULL );
			SetThink( NULL );

			if ( !g_pGameRules->IsMultiplayer() )
			{
				UTIL_Remove( this );
			}
		}
		else
		{
			trace_t	tr;
			tr = BaseClass::GetTouchTrace();


			// See if we struck the world
			if ( pOther->GetMoveType() == MOVETYPE_NONE && !( tr.surface.flags & SURF_SKY ) )
			{
				EmitSound( "Weapon_Crossbow.BoltHitWorld" );

				// if what we hit is static architecture, can stay around for a while.
				Vector vecDir = GetAbsVelocity();
				float speed = VectorNormalize( vecDir );

				// See if we should reflect off this surface
				float hitDot = DotProduct( tr.plane.normal, -vecDir );

				if ( ( hitDot < 0.5f ) && ( speed > 100 ) )
				{
					Vector vReflection = 2.0f * tr.plane.normal * hitDot + vecDir;

					QAngle reflectAngles;

					VectorAngles( vReflection, reflectAngles );

					SetLocalAngles( reflectAngles );

					SetAbsVelocity( vReflection * speed * 0.75f );

					// Start to sink faster
					SetGravity( 1.0f );
				}
				else
				{
					//SetThink( SUB_Remove );
					SetNextThink( gpGlobals->curtime + 2.0f );

					//FIXME: We actually want to stick (with hierarchy) to what we've hit
					SetMoveType( MOVETYPE_NONE );

					Vector vForward;

					AngleVectors( GetAbsAngles(), &vForward );
					VectorNormalize ( vForward );

					//CEffectData	data;
					//data.m_vOrigin = tr.endpos;
					//data.m_vNormal = vForward;
					//data.m_nEntIndex = 0;
					//DispatchEffect( "BoltImpact", data );

					UTIL_ImpactTrace( &tr, DMG_BULLET );

					//AddEffects( EF_NODRAW );
					SetTouch( NULL );
					//SetThink( SUB_Remove );
					SetNextThink( gpGlobals->curtime + 2.0f );

					//if ( m_pGlowSprite != NULL )
					//{
					//	m_pGlowSprite->TurnOn();
					//	m_pGlowSprite->FadeAndDie( 3.0f );
					//}
				}

				// Shoot some sparks
				if ( UTIL_PointContents( GetAbsOrigin() ) != CONTENTS_WATER)
				{
					//g_pEffects->Sparks( GetAbsOrigin() );
				}
			}
			else
			{
				// Put a mark unless we've hit the sky
				if ( ( tr.surface.flags & SURF_SKY ) == false )
				{
					UTIL_ImpactTrace( &tr, DMG_BULLET );
				}

				UTIL_Remove( this );
			}
		}

		if ( g_pGameRules->IsMultiplayer() )
		{
			//		SetThink( &CCrossbowBolt::ExplodeThink );
			//		SetNextThink( gpGlobals->curtime + 0.1f );
		}
	}
	
#endif


#ifdef GAME_DLL   // Server Projectile

	void CTDPJavelin::EmitProjectile( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer )
	{
		CTDPJavelinProjectile::Create( vecSrc, vecAngles, vecVel, angImpulse, pPlayer, JAVELIN_TIMER );
	}

	void CTDPJavelin::StartProjectileThrow()
	{
		m_fThrowTime = gpGlobals->curtime + 0.1f;
	}


	void CTDPJavelin::ThrowProjectile()
	{
		CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
		if ( !pPlayer )
		{
			Assert( false );
			return;
		}

		QAngle angThrow = pPlayer->LocalEyeAngles();

		// Override here :
		Vector vecAiming = pPlayer->GetAutoaimVector( 0 );	

		QAngle angAiming;
		VectorAngles( vecAiming, angAiming );

		Vector vecSrc = pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset();

		Vector vecThrow = vecAiming * 1000;

		//EmitProjectile( vecSrc, vec3_angle, vecThrow, AngularImpulse(600,random->RandomInt(-1200,1200),0), pPlayer );

		EmitProjectile( vecSrc, angAiming, vecThrow, AngularImpulse(random->RandomInt(-10,10),random->RandomInt(-10,10),random->RandomInt(-10,10)), pPlayer );

		m_bRedraw = true;
		m_fThrowTime = 0.0f;

		//pPlayer->ViewPunch( QAngle( -5, 5, 0 ) );
	}

#else

	void CTDPJavelin::StartProjectileThrow()
	{
		m_fThrowTime = gpGlobals->curtime + 0.1f;
	}

	void CTDPJavelin::ThrowProjectile()
	{
		m_bRedraw = true;
		m_fThrowTime = 0.0f;
	}


#endif

