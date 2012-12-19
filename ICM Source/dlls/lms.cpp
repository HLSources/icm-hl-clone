#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"weapons.h"
#include	"gamerules.h"
#include	"game.h"
#include	"lms.h"
#include <fstream>
extern void respawn( entvars_t* pev, BOOL fCopyCorpse );

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

void writeLog(char *msg) {
	ofstream file;
	char line[200];
	file.open("icm/lms.log", ios_base::app);
	sprintf(line, "%08.0f: %s\n", gpGlobals->time, msg);
	file << line;
	file.close();
}

void CRulesLMS :: CheckLMS( )
{
	CBaseEntity *pEnt = NULL;
	CBasePlayer *pPlayer = NULL;
	char msg[100];
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
	//ALERT(at_console, "%.1f CheckLMS: iConnected=%i, iAlive=%i, iHumansConnected=%i, iHumansAlive=%i\n", 
	//	gpGlobals->time, iConnected, iAlive, iHumansConnected, iHumansAlive );
	if ( (iConnected != iConnected_old) || (iAlive != iAlive_old) 
		|| (iHumansConnected != iHumansConnected_old) || (iHumansAlive != iHumansAlive_old) ) {
		iConnected_old = iConnected;
		iAlive_old = iAlive;
		iHumansConnected_old = iHumansConnected;
		iHumansAlive_old = iHumansAlive;
		sprintf(msg, "CheckLMS: iConnected=%i, iAlive=%i, iHumansConnected=%i, iHumansAlive=%i",
			iConnected, iAlive, iHumansConnected, iHumansAlive);
		writeLog(msg);	
	}

	if(iAlive == 0 && iConnected > 0)
	{
		if (bTON(&m_flWaitResetRound, 2.0))		
			ResetRound( );
	}

	if( (iConnected >= 2 && iAlive == 1) || (iHumansConnected > 0 && iHumansAlive == 0) )
	{
		if (bTON(&m_flWaitEndLife, 3.0))		
			EndLife( );
	}
}

void CRulesLMS :: Think ( ) 
{
	CHalfLifeMultiplay :: Think ( );
	if (bTON(&m_flWaitCheckLMS, 1.0))	
		CheckLMS( );
}

void CRulesLMS :: EndLife( )
{
	CBaseEntity *pEnt = NULL;
	CBasePlayer *pPlayer = NULL;
	char msg[100];
	//ALERT(at_console, "EndLife\n" );
	sprintf(msg, "EndLife: m_iEndLife=%i", m_iEndLife);
	writeLog(msg);
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
			else//if( m_iEndLife == 0)
			{
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

	//ALERT(at_console, "ResetRound\n" );
	writeLog("ResetRound");	
	m_iEndLife = 0;
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
		}
	}
}
