#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"weapons.h"
#include	"gamerules.h"
#include	"game.h"
#include	"lms.h"

extern void respawn( entvars_t* pev, BOOL fCopyCorpse );

void CRulesLMS :: CheckLMS( )
{
	CBaseEntity *pEnt = NULL;
	CBasePlayer *pPlayer = NULL;

	int iConnected = 0;
	int iAlive = 0;

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
    {
		pEnt = UTIL_PlayerByIndex( i );
		pPlayer = (CBasePlayer *)pEnt;

		if ( !pEnt )
			continue;

		if ( !strcmp( STRING( pEnt->pev->netname ), "") )
			continue;

		iConnected++;
		if( pEnt->IsAlive() ) 
			{
				iAlive++;
//				if( CVAR_GET_FLOAT("icm_yousuck") == 1 )
//					pPlayer->KamikazeTouch (pPlayer);
			}
	}

	//ALERT(at_console, "%i : %i\n", iConnected, iAlive );

//	if( CVAR_GET_FLOAT("icm_yousuck") == 1 )
//		CVAR_SET_FLOAT("icm_yousuck", 0);
	
	if(iAlive == 0)
	{
		if (m_flWaitResetRound == 0)
			m_flWaitResetRound = gpGlobals->time + 2.0;

		if(gpGlobals->time > m_flWaitResetRound)
		{
			ResetRound( );
		}
	}

	if( iConnected >= 2 && iAlive == 1)
	{
		if (m_flWaitEndLife == 0)
			m_flWaitEndLife = gpGlobals->time + 3.0;

		if(gpGlobals->time > m_flWaitEndLife)
		{
			EndLife( );
		}
	}

}

void CRulesLMS :: Think ( ) 
{
	CHalfLifeMultiplay :: Think ( );
	CheckLMS( );
}

void CRulesLMS :: EndLife( )
{
	CBaseEntity *pEnt = NULL;
	CBasePlayer *pPlayer = NULL;

	ALERT(at_console, "ENDLIFE\n" );

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
    {
		pEnt = UTIL_PlayerByIndex( i );
		pPlayer = (CBasePlayer *)pEnt;

		if ( !pEnt )
			continue;

		if ( !strcmp( STRING( pEnt->pev->netname ), "") )
			continue;

		if( pEnt->IsAlive() ) 
		{
			if( m_flEndLife == 1)
			{
				pPlayer->KamikazeTouch (pPlayer);
				m_flWaitEndLife = 0;
			}
			if( m_flEndLife == 0)
			{
				pPlayer->pev->frags += 11;
				pPlayer->KamikazeEnd( );
				m_flEndLife = 1;
				m_flWaitEndLife = 0;
			}
		}
	}
}

void CRulesLMS :: ResetRound( )
{
	CBaseEntity *pEnt = NULL;
	CBasePlayer *pPlayer = NULL;

	ALERT(at_console, "RESETROUND\n" );

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
    {
		pEnt = UTIL_PlayerByIndex( i );
		pPlayer = (CBasePlayer *)pEnt;

		if ( !pEnt )
			continue;

		if ( !strcmp( STRING( pEnt->pev->netname ), "") )
			continue;

		if( pEnt->IsPlayer() ) 
		{
			pPlayer->RemoveAllItems( TRUE );
			respawn( pPlayer->pev, !(PFLAG_OBSERVER) );// don't copy a corpse if we're in deathcam.
			//respawn( pPlayer->pev, FALSE);
			//pPlayer->Respawn();
			//pPlayer->Spawn();
			m_flWaitResetRound = 0;
		}
	}
}