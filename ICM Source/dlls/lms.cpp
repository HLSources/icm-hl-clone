#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"weapons.h"
#include	"gamerules.h"
#include	"game.h"
#include	"lms.h"

extern void respawn( entvars_t* pev, BOOL fCopyCorpse );

CRulesLMS :: CRulesLMS( ) {
	m_flWaitCheckLMS=0.0;
	m_flWaitResetRound=0.0;
	m_flWaitEndLife=0.0;
	m_iEndLife=0;	
}

BOOL bTON(float *fZeit, float fVerzoegerung) {
	if (*fZeit == 0)
		*fZeit = gpGlobals->time + fVerzoegerung;
	if (gpGlobals->time > *fZeit)
	{
		*fZeit = 0;	
		return TRUE;
	}
	return FALSE;
}

void CRulesLMS :: CheckLMS( )
{
	CBaseEntity *pEnt = NULL;
	CBasePlayer *pPlayer = NULL;
	int iConnected = 0;
	int iAlive = 0;
	int iHumansConnected = 0;
	int iHumansAlive = 0;

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
    	{
		pEnt = UTIL_PlayerByIndex( i );
		pPlayer = (CBasePlayer *)pEnt;

		if ( !pEnt )
			continue;

		if ( !strcmp( STRING( pEnt->pev->netname ), "") )
			continue;

		iConnected++;
		if (!pPlayer->IsBot())
			iHumansConnected++;
		if( pEnt->IsAlive() ) 
			{
				iAlive++;
				if (!pPlayer->IsBot())
					iHumansAlive++;
			}
	}

	if(iAlive == 0 && iConnected > 0)
	{
		if (bTON(&m_flWaitResetRound, 3.0))		
			ResetRound( );
	}
	else if( (iConnected >= 2 && iAlive == 1) || (iHumansConnected > 0 && iHumansAlive == 0) )
	{
		if (bTON(&m_flWaitEndLife, 4.0))		
			EndLife(iAlive);
	}
}

void CRulesLMS :: Think ( ) 
{
	CHalfLifeMultiplay :: Think ( );
	if (bTON(&m_flWaitCheckLMS, 1.0))	
		CheckLMS( );
}

void CRulesLMS :: EndLife(int iAlive)
{
	CBaseEntity *pEnt = NULL;
	CBasePlayer *pPlayer = NULL;
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
			if( m_iEndLife >= 3)
			{
				pPlayer->KamikazeTouch (NULL);
			}
			else
			{
				if (iAlive == 1)				
					pPlayer->pev->frags += 11;
				pPlayer->KamikazeEnd( );
				m_iEndLife++;
			}
			break;
		}
	}
}

void CRulesLMS :: ResetRound( )
{
	CBaseEntity *pEnt = NULL;
	CBasePlayer *pPlayer = NULL;
	m_iEndLife=0; //Reset
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
		}
	}
}
