
/**************************************************
 * Rho-Bot Source Code                            *
 * by Eric Bieschke (rhobot@botepidemic.com)      *
 * http://www.botepidemic.com/rhobot/             *
 **************************************************
 * You are free to do whatever you want with this *
 * source. All I ask is that (A) you tell me what *
 * you do with it so that I can check it out, (B) *
 * you credit me for any of my code that you use  *
 * in your project, and (C) if you are now, or    *
 * later become involved in the gaming industry   *
 * and think I would be a valuable contribution   *
 * to the team, contact me.   =]                  *
 **************************************************
 * If you have questions about the source, please *
 * don't hesitate to give me a ring. If you make  *
 * improvements to this source that you'd like to *
 * share with others, please let me know and I'll *
 * release your changes with the next source code *
 * release (you will be fully credited for all of *
 * your work). If I inadvertantly did not credit  *
 * either botman or Rich Whitehouse for code taken*
 * from their respective open-source bots, I      *
 * apologize (I did my best to comment what was   *
 * theirs), and if you let me know, I will credit *
 * them in the next source release.               *
 **************************************************/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "client.h"
#include "gamerules.h"
#include "effects.h"
#include "items.h"
#include "soundent.h"
#include "weapons.h"
#include "nodes.h"
#include "bot.h"
#include <string.h>

extern DLL_GLOBAL BOOL g_fGameOver;

AmmoCheck ammo_check[] = {
	{"ammo_9mmclip", "9mm", _9MM_MAX_CARRY},
	{"ammo_9mmAR", "9mm", _9MM_MAX_CARRY},
	{"ammo_ARgrenades", "ARgrenades", M203_GRENADE_MAX_CARRY},
	{"ammo_buckshot", "buckshot", BUCKSHOT_MAX_CARRY},
	{"ammo_crossbow", "bolts", BOLT_MAX_CARRY},
	{"ammo_357", "357", _357_MAX_CARRY},
	{"ammo_rpgclip", "rockets", ROCKET_MAX_CARRY},
	{"ammo_egonclip", "uranium", URANIUM_MAX_CARRY},
	{"ammo_gaussclip", "uranium", URANIUM_MAX_CARRY},
	{"", 0, 0}};

///////////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
///////////////////////////////////////////////////////////////////////////////

CBaseBot::CBaseBot():
bCalledAimThisFrame( FALSE ),
DesiredVelocity( Vector(0,0,0) ),
FightStyle( this ),
bGoUpOnLadder( TRUE ),
LookAtBiasVec( Vector(0,0,0) ),
LookAtVec( Vector(0,0,0) ),
Memory(),
MovedDistance( 0.0 ),
MoveForward( 0.0 ),
MoveStrafe( 0.0 ),
MoveVertical( 0.0 ),
MSec( 0.0 ),
MSecCounter( 0 ),
//Vacindak:  Memory Leak Fix
//pEnemy( NULL ),
//pGoal( NULL ),
//End Memory Leak Fix
RoamDirection( UNKNOWN ),
Stats(),
TimeGoalCheck( gpGlobals->time ),
TimeGoalCheckDelay( 0.3 ),
TimeMSecCheck( gpGlobals->time ),
TurningDirection( NONE ),
bWantToBeInCombat( FALSE )
{
	pEnemy = (CBaseEntity *)NULL;
	pGoal = (CBaseEntity *)NULL;
}

CBaseBot::~CBaseBot()
{
}

///////////////////////////////////////////////////////////////////////////////
// ActionOpenFire
///////////////////////////////////////////////////////////////////////////////

void CBaseBot::ActionOpenFire( void )
{ // TODO: bots should fire at different rates depending on skill and weapon
	if ( !m_pActiveItem->CanDeploy()
		/*|| ( ( int(Memory.GetEnemyFirstSeenTime()) )%( RANDOM_LONG(1,9) ) == 0 )*/
		)
	{
		ActionChooseWeapon();
	}
	else
	{
		CBasePlayerWeapon *pActiveWeapon = (CBasePlayerWeapon *)m_pActiveItem;

		if ( pActiveWeapon->pszAmmo1()
			&& ( pActiveWeapon->m_iClip <= 0 )
			&& ( pActiveWeapon->iMaxClip() != WEAPON_NOCLIP )
			)
		{
			ActionReload();
		}
		else if ( ( RANDOM_LONG(1,100) <= 20 || FightStyle.GetHoldDownAttack() )
				&& !pActiveWeapon->m_fInReload
				) 
		{ // fire approx 20% of frames, and don't interrupt a reload
			if ( FightStyle.GetSecondaryFire() )
			{
				if ( !pActiveWeapon->pszAmmo1()
					|| ( pActiveWeapon->iMaxClip() == WEAPON_NOCLIP )
					|| ( m_rgAmmo[pActiveWeapon->m_iSecondaryAmmoType] != 0 )
					)
				{
					pev->button |= IN_ATTACK2;
				}
				else
				{
					pev->button |= IN_ATTACK;
				}
			}
			else
			{
				if ( !pActiveWeapon->pszAmmo2()
					|| ( pActiveWeapon->iMaxClip() == WEAPON_NOCLIP )
					|| ( m_rgAmmo[pActiveWeapon->m_iPrimaryAmmoType] != 0 )
					)
				{
					pev->button |= IN_ATTACK;
				}
				else
				{
					pev->button |= IN_ATTACK2;
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// ActionChooseGoal - sets pGoal by searching through m_pLink's list
///////////////////////////////////////////////////////////////////////////////

inline void CBaseBot::ActionChooseGoal( void )
{
	float		BestPickupDesire = 0;
	float		DistanceToEntity;
	float		PickupDesire; // [0,100]
	CBaseEntity	*pNextEnt = m_pLink;

	if ( (pev->health < 10) && (RANDOM_LONG(0, 23) == 0) )
	{
		KamikazeEnd();
	}

	while ( pNextEnt != NULL ) // pNextEnt is already NULL if we don't want a new pGoal this frame
	{
		PickupDesire = 0.0;

		if ( !CheckHasDecentWeapon() )
		{
			if ( !(pNextEnt->pev->effects & EF_NODRAW) && CheckVisible( pNextEnt ) )
			{
				if ( strncmp( "weapon", STRING(pNextEnt->pev->classname), 6 ) == 0 )
				{ // this strncmp matches all weapon_ and also weaponbox
					CBasePlayerWeapon *pWeapon = (CBasePlayerWeapon *)pNextEnt;

					if ( pWeapon->m_pPlayer )
					{
						continue; // someone owns this weapon
					}

					if ( g_pGameRules->CanHavePlayerItem( this, pWeapon ) )
					{
						PickupDesire = 100;
					}
				}
			}
		}
		else if ( !(pNextEnt->pev->effects & EF_NODRAW) && CheckVisible( pNextEnt ) )
		{
			if ( strncmp( "ammo_", STRING(pNextEnt->pev->classname), 5 ) == 0 )
			{
				CBasePlayerAmmo *pAmmo = (CBasePlayerAmmo *)pNextEnt;
				BOOL ammo_found = FALSE;

				for ( int i = 0; ammo_check[i].ammo_name[0]; i++ )
				{
					if ( strcmp(ammo_check[i].ammo_name, STRING(pNextEnt->pev->classname)) == 0 )
					{
						if ( g_pGameRules->CanHaveAmmo( this, ammo_check[i].weapon_name, ammo_check[i].max_carry ) )
						{
							PickupDesire = 1; // FIXME: need to pass CBaseWeapon not ammo
//							CBasePlayerItem *NextItem = (CBasePlayerItem *)pNextEnt;
//							PickupDesire = Stats.FindWeaponDesire( NextItem, DISTANCE_MAX ) / 5;
							break;
						}
					}
				}
			}
			else if ( strncmp( "weapon_", STRING(pNextEnt->pev->classname), 7 ) == 0 )
			{
				CBasePlayerWeapon *pWeapon = (CBasePlayerWeapon *)pNextEnt;

				if ( pWeapon->m_pPlayer )
				{
					continue; // someone owns this weapon
				}

				if ( g_pGameRules->CanHavePlayerItem( this, pWeapon ) )
				{
					CBasePlayerItem *NextItem = (CBasePlayerItem *)pNextEnt;
					PickupDesire = Stats.FindWeaponDesire( NextItem, DISTANCE_MAX );
				}
			}
			else if ( FClassnameIs( pNextEnt->pev, "item_healthkit" ) )
			{
				PickupDesire = 100.0 - pev->health; // we want health proportional to how much we need
			}
			else if ( FClassnameIs( pNextEnt->pev, "weaponbox" ) ) // weapon box
			{
				PickupDesire = 80;
			}
			else if ( FClassnameIs( pNextEnt->pev, "item_battery" ) )
			{
				PickupDesire = MAX_NORMAL_BATTERY - pev->armorvalue; // we want armor proportional to how much we need
			}
			else if ( FClassnameIs( pNextEnt->pev, "item_longjump" ) )
			{
				PickupDesire = 1; // NOTE: arbitrary PickupDesire
			}
		}

		if ( PickupDesire > 0 ) // desire this at all?
		{
			DistanceToEntity = ( pNextEnt->pev->origin - pev->origin ).Length();

			float diff = SEARCH_DISTANCE - DistanceToEntity;

			//Scott: fix.  Using diff * diff results in bots possibly wanting things
			//       that are far away.  now using diff*diff*diff
			PickupDesire *= (diff * diff * diff); // want things more the closer they are

			if ( PickupDesire > BestPickupDesire ) // is this the most desirable item so far?
			{
				BestPickupDesire = PickupDesire;
				SetGoal( pNextEnt );
			}
		}

		pNextEnt = pNextEnt->m_pLink;
	}
}

///////////////////////////////////////////////////////////////////////////////
// ActionChooseWeapon
///////////////////////////////////////////////////////////////////////////////

void CBaseBot::ActionChooseWeapon( void )
{
	CBasePlayerItem *pCheckWeapon;
	CBasePlayerItem *pBestWeapon = NULL;
	int BestWeaponDesire = 0; // no weapon lower than -1 can be autoswitched to

	if ( m_pActiveItem == NULL || !m_pActiveItem->CanHolster() )
	{
//		ActionSpeak( "I can't put away my current weapon." );
		return;
	}

	for ( int i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		pCheckWeapon = m_rgpPlayerItems[ i ];

		while ( pCheckWeapon )
		{
			// this randomizes what weapon a given bot will choose at any particular moment
			// and yet maintains preferencial weapon bias

			float DistanceToEnemy = DISTANCE_MAX;
			if ( GetEnemy() )
			{
				DistanceToEnemy = ( GetEnemy()->pev->origin - pev->origin ).Length();
			}

			float CheckWeaponDesire = Stats.FindWeaponDesire( pCheckWeapon, DistanceToEnemy ) * RANDOM_FLOAT(0,1);

			if ( ( CheckWeaponDesire > BestWeaponDesire ) && pCheckWeapon->CanDeploy() )
			{
				BestWeaponDesire = CheckWeaponDesire;
				pBestWeapon = pCheckWeapon;
			}

			pCheckWeapon = pCheckWeapon->m_pNext;
		}
	}

	if ( pBestWeapon && this->pev )
	{ // this->pev check is protecting against us trying to call SwitchWeapon on a dead man's weapon
		SwitchWeapon( pBestWeapon );
	}

	FightStyle.DispatchWeaponUse( m_pActiveItem );
}

///////////////////////////////////////////////////////////////////////////////
// ActionLook - reacts to immediate threats, and set m_pLink as head of item list
///////////////////////////////////////////////////////////////////////////////

inline void CBaseBot::ActionLook( int SearchDistance )
{
	// section one: handle client entitites

	float	DistanceToEntity;
	float	NearestEnemyDistance = DISTANCE_MAX;

	//Vacindak: changed "!GetEnemy" to "GetEnemy == NULL"
	if ( GetEnemy() == NULL 
		|| ( ( gpGlobals->time - Memory.GetEnemyLastSeenTime() ) > 3 ) )
	{
		for ( int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBaseEntity *pPlayerEntity = UTIL_PlayerByIndex( i );
			CBasePlayer *pPlayer = (CBasePlayer *)pPlayerEntity;

			if ( pPlayer // UTIL_PlayerByIndex can return NULL
				&& pPlayer != this
				&& pPlayer->IsAlive()
				&& CheckVisible( pPlayer )
				&& FInViewCone( pPlayer ) // Scott: must be in sight cone
				// the next set of logic uses cvars
				&& (bot_dontshoot.value == 0) // can shoot
				&& (!pPlayer->IsNetClient() || (bot_observer.value == 0)) //bot or not observer mode
				)
			{
				if ( g_pGameRules->PlayerRelationship( this, pPlayer ) == GR_NOTTEAMMATE
					&& CheckNotice( pPlayer )
					)
				{
					DistanceToEntity = ( pPlayer->pev->origin - pev->origin ).Length();

					if ( DistanceToEntity < NearestEnemyDistance )
					{
						NearestEnemyDistance = DistanceToEntity;
						SetEnemy( pPlayer );
					}
				}
				else if ( !pPlayer->IsBot()
						&& g_pGameRules->PlayerRelationship( this, pPlayer ) != GR_NOTTEAMMATE
						)
				{
					DistanceToEntity = ( pPlayer->pev->origin - pev->origin ).Length();

					if ( DistanceToEntity < GROUPING_DISTANCE )
					{
						SteerSafeGroupFollow( pPlayer );
					}
					else
					{
						SteerSafePursue( pPlayer, STEER_GROUP_ASSEMBLE );
					}
				}
				else if ( pPlayer->IsBot()
						&& g_pGameRules->PlayerRelationship( this, pPlayer ) != GR_NOTTEAMMATE
						)
				{
					DistanceToEntity = ( pPlayer->pev->origin - pev->origin ).Length();

					if ( DistanceToEntity < GROUPING_DISTANCE )
					{
						SteerSafeGroupFlock();
					}
					else
					{
						SteerSafePursue( pPlayer, STEER_GROUP_ASSEMBLE );
					}
				}
			}
		}

		if ( GetEnemy() )
		{
//			ActionSpeak("I've found an enemy.");

			ActionChooseWeapon();

			SetLookAtBiasVec( Vector( RANDOM_FLOAT(-1,1), RANDOM_FLOAT(-1,1), RANDOM_FLOAT(-1,1) ) );

			Memory.EnemySighted( GetEnemy() );
		}
	}

	// section two: sets m_pLink as head of a linked list containing all possible pGoal ents

	m_pLink = NULL;
	CBaseEntity	*pNextEnt = NULL; // the current visible entity that we're dealing with
	CBaseEntity *pList[100];
	Vector delta = Vector( SearchDistance, SearchDistance, SearchDistance );
	BOOL CheckGoalThisFrame = FALSE;
	BOOL NoGoalAtStartOfFrame = FALSE;

	if ( GetGoal() == NULL )
	{
		NoGoalAtStartOfFrame = TRUE;
	}

	if ( gpGlobals->time > GetTimeGoalCheck() )
	{
		CheckGoalThisFrame = TRUE;
		SetTimeGoalCheck( gpGlobals->time + GetTimeGoalCheckDelay() );
	}

	int count = UTIL_EntitiesInBox( pList, 100, pev->origin - delta, pev->origin + delta, 0 );
	for ( int i = 0; i < count; i++ )
	{
		pNextEnt = pList[i];

		if ( pNextEnt && pNextEnt != this && CheckVisible( pNextEnt ) )
		{
			if ( CheckNotice( pNextEnt ) )
			{
				DistanceToEntity = ( pNextEnt->pev->origin - pev->origin ).Length();

				if (	FClassnameIs( pNextEnt->pev, "monster_satchel" )	||
						FClassnameIs( pNextEnt->pev, "monster_snark" )		||
						FClassnameIs( pNextEnt->pev, "hornet" )				||
						FClassnameIs( pNextEnt->pev, "rpg_rocket" )			||
						FClassnameIs( pNextEnt->pev, "grenade" )
					)
				{
					SteerSafeEvade( pNextEnt );
				}
				else if ( FClassnameIs( pNextEnt->pev, "laser_spot" ) ) // rpg laser spot
				{
					SteerSafeFlee( pNextEnt );
				}
				else if ( FClassnameIs( pNextEnt->pev, "beam" ) ) // tripmine beam
				{
					CBeam *pBeam = (CBeam *)pNextEnt;
					Vector BeamStart = pBeam->GetStartPos();
					if ( (BeamStart - pev->origin).Length() < 375 )
					{
						SteerSafeFlee( BeamStart ); // run away from the tripmine if within range
					}
				}
				else if ( FClassnameIs( pNextEnt->pev, "monster_tripmine" ) )
				{
					if ( DistanceToEntity > 375 ) // safe to shoot tripmine?
					{
						if ( GetEnemy() )
						{
							if ( (pNextEnt->pev->origin - GetEnemy()->pev->origin).Length() < 375 )
							{
								AimAtEntity( pNextEnt );
							}
						}
						else
						{
							AimAtEntity( pNextEnt );
						}
					}
					else
					{
						SteerSafeFlee( pNextEnt );
					}
				}
				else if ( FClassnameIs( pNextEnt->pev, "func_button" ) ) // hit any button within range
				{
					if ( DistanceToEntity < 64 ) // 64 is the usual maximum range of a use command
					{
						pev->button |= IN_USE;
					}
				}
				else if ( NoGoalAtStartOfFrame && CheckGoalThisFrame )
				{
					pNextEnt->m_pLink = m_pLink; // linked list: link new list head to previous list head
					m_pLink = pNextEnt; // linked list: link rho-bot to list head
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// ActionReload
///////////////////////////////////////////////////////////////////////////////

BOOL CBaseBot::ActionReload( void )
{
	CBasePlayerWeapon *pActiveWeapon = (CBasePlayerWeapon *)m_pActiveItem;

	if ( pActiveWeapon->pszAmmo1()
		&& pActiveWeapon->iMaxClip() != WEAPON_NOCLIP
		&& pActiveWeapon->m_iClip != pActiveWeapon->iMaxClip()
		)
	{
		pev->button |= IN_RELOAD;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

///////////////////////////////////////////////////////////////////////////////
// ActionSpeak
///////////////////////////////////////////////////////////////////////////////

void CBaseBot::ActionSpeak( char *pText )
{
	char buffer[256];
	sprintf( buffer, "%s: %s\n", STRING(pev->netname), pText );
	UTIL_SayTextAll( buffer, this );
}

///////////////////////////////////////////////////////////////////////////////
// CheckBotKicked (Scott - adapted from botman's bot10)
///////////////////////////////////////////////////////////////////////////////

BOOL CBaseBot::CheckBotKick()
{
	// check if someone kicked the bot off of the server (DON'T RESPAWN!)...
	if ((pev->takedamage == DAMAGE_NO) && (SpawnIndex > -1))
	{
		pev->health = 0;
		pev->deadflag = DEAD_DEAD;  // make the kicked bot be dead

		bot_respawn[SpawnIndex].is_used = FALSE;  // this slot is now free
		SpawnIndex = -1;

		// fall through to next if statement
		// SpawnIndex is -1
	}

	// is the round over (time/frag limit) or has the bot been removed?
	if ((g_fGameOver) || (SpawnIndex == -1))
	{
		CSound *pSound;

		// keep resetting the sound entity until the bot is respawned...
		pSound = CSoundEnt::SoundPointerForIndex( CSoundEnt::ClientSoundIndex( edict() ) );
		if ( pSound )
		{
			pSound->Reset();
		}

		ThinkDead();

		return TRUE;
	}

	pev->button = 0;  // make sure no buttons are pressed

	// if the bot is dead, randomly press fire to respawn...
	if ((pev->health < 1) || (pev->deadflag != DEAD_NO))
	{
		if (RANDOM_LONG(1, 100) > 50)
		pev->button = IN_ATTACK;

		g_engfuncs.pfnRunPlayerMove( edict(), pev->v_angle, GetMoveForward(), GetMoveStrafe(), GetMoveVertical(), pev->button, 0, GetMSec() );
		return TRUE;
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// BotThink() - the main loop
///////////////////////////////////////////////////////////////////////////////

void CBaseBot::BotThink( void )
{	

	if (CheckBotKick()) return;  // Exit if bot has been kicked

	ThinkStart();

	if ( IsAlive() )
	{
		if ( GetEnemy() )
		{
			ThinkValidateEnemy();
		}

		if ( GetGoal() )
		{
			ThinkValidateGoal();
		}
		
		ActionLook();

		ActionChooseGoal();

		ThinkMood();

		ThinkSteering();
		
		if ( GetCalledAimThisFrame() == FALSE )
		{
			AimWander();
		}

		HandleMovement();
	}
	else
	{
		ThinkDead();
	}

	Memory.EndFrameUpdate( this );

	g_engfuncs.pfnRunPlayerMove( edict(), pev->v_angle, GetMoveForward(), GetMoveStrafe(), GetMoveVertical(), pev->button, 0, GetMSec() );

}

///////////////////////////////////////////////////////////////////////////////
// CheckHasDecentWeapon
///////////////////////////////////////////////////////////////////////////////

BOOL CBaseBot::CheckHasDecentWeapon( void )
{
	for ( int i = 2; i < 4; i++ ) // have weapon from groups 3 or 4?
	{
		if ( m_rgpPlayerItems[ i ] )
		{
			return TRUE;
		}
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// CheckNotice
///////////////////////////////////////////////////////////////////////////////

BOOL CBaseBot::CheckNotice( CBaseEntity *pEntity )
{
	if ( FInViewCone( pEntity ) )
//Scott: Whoa, Nellie!  The code as it was allowed the bots to 'cheat' by seeing where humans cannot.
//		These changes force the bots to see only in their view cone (still better than humans, but more acceptable).
//		Ability to notice items falls of as 1/distance^2, linear TraitPerception combinatory affects
//		The item is more noticable as a function of speed. If the item is not moving, it is 4 times
//		less likely to be noticed than if it is moving at sv_maxspeed.
//	{
//		return TRUE;
//	}
//	else // exponentially more likely to notice at closer distances, linear TraitPerception combinatory affects
	{
		float distance_helper = ( pev->origin - pEntity->pev->origin ).Length(); // raw distance

		if ( distance_helper > DISTANCE_MAX ) // just in case
		{
			distance_helper = DISTANCE_MAX;
		}

		if ( distance_helper < 256 ) // below this is negligible
		{
			distance_helper = 256;
		}

		distance_helper = 8*( DISTANCE_MAX / distance_helper ); // [8,256]

		distance_helper *= distance_helper; // [64,65536]

		float velocity_helper = (pEntity->pev->velocity.Length())/CVAR_GET_FLOAT("sv_maxspeed"); // velocity ratio

		velocity_helper = (velocity_helper*(2.*velocity_helper+1.)+1.); 

		float render_amt = (pEntity->pev->renderamt +1.)/256.; // render ratio

		float perceptTweak = Stats.GetTraitPerception() / 100; // [0,1]

//01-Aug-2001:  Made sure perceptTweak >= 0
//		perceptTweak -= .5; // [-.5,.5]
//
//		perceptTweak *= 2; // [-1,1]
//
//		perceptTweak *= 65536; // [-65536,65536]

		perceptTweak *= 65536; // [0,65536]
//01-Aug-2001: end

		if ( RANDOM_FLOAT(1,65536) < ( distance_helper*velocity_helper*render_amt + perceptTweak ) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// CheckVisible
///////////////////////////////////////////////////////////////////////////////

BOOL CBaseBot::CheckVisible( CBaseEntity *pEntity )
{
	if ( !pEntity || pEntity == this || !(pEntity->pev) ) // 
	{
		return FALSE; // intuition dictates this should return TRUE for this, but our Validate functions will fix things if this returns FALSE
	}

	return FVisible( pEntity );
}

///////////////////////////////////////////////////////////////////////////////
// HandleTime - from Rich Whitehouse's Advanced Bot Frame
///////////////////////////////////////////////////////////////////////////////

inline void CBaseBot::HandleTime( void )
{
	if ( GetTimeMSecCheck() <= gpGlobals->time )
	{
		SetTimeMSecCheck( gpGlobals->time + 0.5 );  // Scott:  was + 1

		if ( GetMSecCounter() > 0 )
		{
			SetMSec( 450 / GetMSecCounter() ); // Scott:  was 1000
		}

		SetMSecCounter( 0 );
	}
	else
	{
		SetMSecCounter( GetMSecCounter() + 1 );
	}

	if ( GetMSec() < 1 ) // was 10
	{
		SetMSec( 1 );
	}

	if ( GetMSec() > 100 ) // was 75
	{
		SetMSec( 100 );
	}
}

///////////////////////////////////////////////////////////////////////////////
// ThinkDead
///////////////////////////////////////////////////////////////////////////////

inline void CBaseBot::ThinkDead( void )
{
	SetDesiredVelocity( Vector(0,0,0) );
	SetGoUpOnLadder( TRUE );
	SetLookAtVec( Vector(0,0,0) );
	SetMovedDistance( 0.0 );
	SetMoveForward( 0.0 );
	SetMoveStrafe( 0.0 );
	SetMoveVertical( 0.0 );
	SetEnemy( NULL );
	SetGoal( NULL );
	SetRoamDirection( UNKNOWN );
	SetTimeGoalCheck( gpGlobals->time );
	SetTurningDirection( NONE );
	SetWantToBeInCombat( FALSE );
}

///////////////////////////////////////////////////////////////////////////////
// ThinkMood
///////////////////////////////////////////////////////////////////////////////

inline void CBaseBot::ThinkMood( void )
{

	CBasePlayerWeapon *pActiveWeapon = (CBasePlayerWeapon *)m_pActiveItem;

	if ( pActiveWeapon->m_fInReload )
	{
		SetWantToBeInCombat( FALSE );
	}
	else if ( FClassnameIs( pActiveWeapon->pev, "weapon_crowbar" ) )
	{
		SetWantToBeInCombat( TRUE );
	}
	else if ( GetEnemy() )
	{
		//Vacindak: Messy, but i did this late at night and errors about with the single line alternative for some reason
		CBaseEntity *pEnemy = GetEnemy();
		CBasePlayer *pEnemyPlayer = (CBasePlayer *)pEnemy;	

		if ( pEnemyPlayer->m_pActiveItem != NULL && //Scott: Make sure the pointer is non-zero!
			FClassnameIs( pEnemyPlayer->m_pActiveItem->pev, "weapon_crowbar" ) )
		{
			SetWantToBeInCombat( FALSE );
		}
		else if ( (pev->health + pev->armorvalue) + (Stats.GetTraitAggression() - 50) > (pEnemyPlayer->pev->health + pEnemyPlayer->pev->armorvalue) )
		{
			SetWantToBeInCombat( TRUE );
		}
		else
		{
			SetWantToBeInCombat( FALSE );
		}
	}
	else
	{
		if ( ((pev->health + pev->armorvalue) > (140 - Stats.GetTraitAggression())) && CheckHasDecentWeapon() )
		{
			SetWantToBeInCombat( TRUE );
		}
		else
		{
			SetWantToBeInCombat( FALSE );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// ThinkStart
///////////////////////////////////////////////////////////////////////////////

inline void CBaseBot::ThinkStart( void )
{
	HandleTime();

	SetCalledAimThisFrame( FALSE );

	SetSteerCallCounter( 0 );

	SetSteerCallPrecedence( STEER_WANDER );

	pev->button = 0;

	SetMovedDistance( ( Memory.GetPrevOrigin() - pev->origin ).Length() );

	if ( GetMovedDistance() <= 1.0 )
	{
		WorldGraph.MarkLocationUnfavorable( pev->origin );
	}
}

///////////////////////////////////////////////////////////////////////////////
// ThinkSteering
///////////////////////////////////////////////////////////////////////////////

inline void CBaseBot::ThinkSteering( void )
{
	//Vacindak: changed "GetEnemy()" to "GetEnemy() != NULL"
	if ( GetEnemy() != NULL && GetEnemy()->IsAlive() )
	{
		if ( CheckVisible( GetEnemy() ) )
		{
			AimAtEnemy();
			ActionOpenFire();

			Memory.EnemyInSight( GetEnemy() );
		}
		else
		{
			if ( Memory.GetEnemyInSight() != FALSE ) // i.e. this is the first time through
			{
				if ( !ActionReload() ) // change weapons if we're not going to reload
				{
					ActionChooseWeapon();
				}
			}

			Memory.EnemyOutOfSight();
		}

		if ( CheckHasDecentWeapon() )
		{
			if ( GetWantToBeInCombat() )
			{
				if ( RANDOM_FLOAT(1,4000) < Stats.GetTraitJumpPropensity() ) 
				{ // jump a lil bit during a duel
					pev->button |= IN_JUMP;
				}

				if ( Memory.GetEnemyInSight() )
				{
					SteerSafePursue( GetEnemy() );
				}
				else
				{
					SteerSafeSeek( Memory.GetEnemyLastSeenPosition(), STEER_WANT_ENEMY );
				}
			}
			else
			{
				if ( RANDOM_FLOAT(1,2000) < Stats.GetTraitJumpPropensity() )
				{ // jump a bit more if you don't want to be in combat
					pev->button |= IN_JUMP;
				}

				if ( Memory.GetEnemyInSight() )
				{
					SteerSafeEvade( GetEnemy() );
				}
				else
				{
					SteerSafeFlee( Memory.GetEnemyLastSeenPosition() );
				}
			}
		}
		else
		{
			if ( RANDOM_FLOAT(1,1000) < Stats.GetTraitJumpPropensity() )
			{ // jump alot if you don't have a decent weapon
				pev->button |= IN_JUMP;
			}

			if ( GetGoal() )
			{
				SteerSafeSeek( GetGoal() );
			}
			else
			{
				SteerSafeWanderImproved();
			}
		}
	}
	else
	{
		ActionReload();

		if ( GetGoal() )
		{
			SteerSafeSeek( GetGoal() );
		}
		else
		{
			SteerSafeWanderImproved();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// ThinkValidateEnemy
///////////////////////////////////////////////////////////////////////////////

inline void CBaseBot::ThinkValidateEnemy( void )
{
	//Vacindak: changed "GetEnemy()" to "GetEnemy() != NULL"
	if ( GetEnemy() != NULL && CheckVisible( GetEnemy() ) && FInViewCone( GetEnemy() ) )
	{
		Memory.EnemyInSight( GetEnemy() );
	}

	//Vacindak: changed "!GetEnemy()" to "GetEnemy() == NULL"
	if ( GetEnemy() == NULL
		|| GetEnemy()->pev->deadflag != DEAD_NO
		|| ( gpGlobals->time - Memory.GetEnemyLastSeenTime() ) > (10 + ((Stats.GetTraitAggression() - 50) / 10))
		)
	{
//		ActionSpeak("I'm giving up on my current target.");
		SetEnemy( NULL );
	}
}

///////////////////////////////////////////////////////////////////////////////
// ThinkValidateGoal
///////////////////////////////////////////////////////////////////////////////

inline void CBaseBot::ThinkValidateGoal( void )
{ // TODO: eventually rather than check visibility we should should check to make sure there's still a valid path
	//Vacindak: changed "!GetGoal()" to "GetGoal() == NULL"
	if ( GetGoal() == NULL || !CheckVisible( GetGoal() ) || ( GetGoal()->pev->effects & EF_NODRAW ) )
	{
		SetGoal( NULL );
//		ActionSpeak( "Giving up on goal." );
	}
}

