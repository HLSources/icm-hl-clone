//=============================================================================
// observer.cpp
//
#include        "extdll.h"
#include        "util.h"
#include        "cbase.h"
#include        "player.h"
#include        "weapons.h"
#define OBS_NONE				0
#define OBS_CHASE_LOCKED		1
#define OBS_CHASE_FREE			2
#define OBS_ROAMING				3		
#define OBS_IN_EYE				4
#define OBS_MAP_FREE			5
#define OBS_MAP_CHASE			6

extern int gmsgCurWeapon;
extern int gmsgSetFOV;
extern int gmsgTeamInfo;
extern int gmsgSpectator;

// Spieler wurde zum Spectator, erstmal alles einrichten
// Dies wurde von der player.cpp hierher verschoben.
void CBasePlayer::StartObserver( Vector vecPosition, Vector vecViewAngle )
{
        // Alle Client-Side-Entities, die dem Spieler
        // zugewiesen wurden, abschalten
        MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
                WRITE_BYTE( TE_KILLPLAYERATTACHMENTS );
                WRITE_BYTE( (BYTE)entindex() );
        MESSAGE_END();

        // Alle Waffen wegstecken
        if (m_pActiveItem)
                m_pActiveItem->Holster( );

        if ( m_pTank != NULL )
        {
                m_pTank->Use( this, this, USE_OFF, 0 );
                m_pTank = NULL;
        }

        // Message Cache des Anzugs löschen,
        // damit wir nicht weiterlabern ;-)
        SetSuitUpdate(NULL, FALSE, 0);

        // Mitteilung an Ammo-HUD, dass der Spieler tot ist
        MESSAGE_BEGIN( MSG_ONE, gmsgCurWeapon, NULL, pev );
                WRITE_BYTE(0);
                WRITE_BYTE(0XFF);
                WRITE_BYTE(0xFF);
        MESSAGE_END();

        // Zoom zurücksetzen
        m_iFOV = m_iClientFOV = 0;
        pev->fov = m_iFOV;
        MESSAGE_BEGIN( MSG_ONE, gmsgSetFOV, NULL, pev );
                WRITE_BYTE(0);
        MESSAGE_END();

        // Ein paar Flags setzen
        m_iHideHUD = (HIDEHUD_HEALTH | HIDEHUD_WEAPONS);
        m_afPhysicsFlags |= PFLAG_OBSERVER;
        pev->effects = EF_NODRAW;
        pev->view_ofs = g_vecZero;
        pev->angles = pev->v_angle = vecViewAngle;
        pev->fixangle = TRUE;
        pev->solid = SOLID_NOT;
        pev->takedamage = DAMAGE_YES;
        pev->movetype = MOVETYPE_NONE;
        ClearBits( m_afPhysicsFlags, PFLAG_DUCKING );
        ClearBits( pev->flags, FL_DUCKING );
        pev->deadflag = DEAD_RESPAWNABLE;
        pev->health = 1;

        // Status-Bar ausschalten
        m_fInitHUD = TRUE;

        // Team Status updaten
        pev->team = 0;
        MESSAGE_BEGIN( MSG_ALL, gmsgTeamInfo );
                WRITE_BYTE( ENTINDEX(edict()) );
                WRITE_STRING( "" );
        MESSAGE_END();

        // Dem Spieler alles wegnehmen
        RemoveAllItems( FALSE );

        // Zur neuen Position bewegen
        UTIL_SetOrigin( pev, vecPosition );

        // Einen Spieler finden, den wir angucken
        m_flNextObserverInput = 0;
        Observer_SetMode(OBS_CHASE_LOCKED);

        // Allen Clients mitteilen, dass der Spieler jetzt Spectator ist
        MESSAGE_BEGIN( MSG_ALL, gmsgSpectator );
                WRITE_BYTE( ENTINDEX( edict() ) );
                WRITE_BYTE( 1 );
        MESSAGE_END();
}

// Observermodus verlassen
void CBasePlayer::StopObserver( void )
{
        // Spectator-Modus abschalten
        if ( pev->iuser1 || pev->iuser2 )
        {
                // Allen Clients mitteilen, dass der Spieler kein Spectator
                // mehr ist.
                MESSAGE_BEGIN( MSG_ALL, gmsgSpectator );
                        WRITE_BYTE( ENTINDEX( edict() ) );
                        WRITE_BYTE( 0 );
                MESSAGE_END();

                pev->iuser1 = pev->iuser2 = 0;
                m_hObserverTarget = NULL;
				m_iHideHUD = NULL;
		}
}

// Den nächsten Client finden, den der Spieler anschaut
void CBasePlayer::Observer_FindNextPlayer( bool bReverse )
{
        // MOD AUTOREN: Hier die Logik ändern, wenn ihr den Observer nur
        //              bestimmte Spieler beobachten lassen wollt, beispielsweise
        //                nur Teammitglieder.

        int                iStart;
        if ( m_hObserverTarget )
                iStart = ENTINDEX( m_hObserverTarget->edict() );
        else
                iStart = ENTINDEX( edict() );
        int            iCurrent = iStart;
        m_hObserverTarget = NULL;
        int iDir = bReverse ? -1 : 1;

        do
        {
                iCurrent += iDir;

                // Durch alle Clients loopen
                if (iCurrent > gpGlobals->maxClients)
                        iCurrent = 1;
                if (iCurrent < 1)
                        iCurrent = gpGlobals->maxClients;

                CBaseEntity *pEnt = UTIL_PlayerByIndex( iCurrent );
                if ( !pEnt )
                        continue;
				
				if ( !strcmp( STRING( pEnt->pev->netname ), "") )
					continue;

                if ( pEnt == this )
                        continue;
                // Keine unsichtbaren Spieler oder andere Observer beobachten
                if ( ((CBasePlayer*)pEnt)->IsObserver() || (pEnt->pev->effects & EF_NODRAW) )
                        continue;

                // MOD AUTOREN: Hier die Überprüfungen einfügen

                m_hObserverTarget = pEnt;
                break;

        } while ( iCurrent != iStart );

        // Ziel gefunden?
        if ( m_hObserverTarget )
        {
                // Ziel in pev speichern damit die Bewegungs-DLL dran kommt
                pev->iuser2 = ENTINDEX( m_hObserverTarget->edict() );
                // Zum Ziel bewegen
                UTIL_SetOrigin( pev, m_hObserverTarget->pev->origin );

                //ALERT( at_console, "Now Tracking \"%s\"\n", STRING( m_hObserverTarget->pev->netname ) );
        }
        else
        {
                ALERT( at_console, "No observer targets.\n" );
        }
}

// Handle buttons in observer mode
void CBasePlayer::Observer_HandleButtons()
{
  // Slow down mouse clicks
   if ( m_flNextObserverInput > gpGlobals->time )
     return;

   // Jump changes from modes: Chase to Roaming
   if ( m_afButtonPressed & IN_JUMP )
   {
     if ( pev->iuser1 == OBS_CHASE_LOCKED )
       Observer_SetMode( OBS_CHASE_FREE );

     else if ( pev->iuser1 == OBS_CHASE_FREE )
       Observer_SetMode( OBS_ROAMING );

     else if ( pev->iuser1 == OBS_ROAMING )
       Observer_SetMode( OBS_IN_EYE );

     else if ( pev->iuser1 == OBS_IN_EYE )
       Observer_SetMode( OBS_MAP_FREE );

     else if ( pev->iuser1 == OBS_MAP_FREE )
       Observer_SetMode( OBS_MAP_CHASE );

    else
       Observer_SetMode( OBS_CHASE_LOCKED ); // don't use OBS_CHASE_LOCKED anymore

     m_flNextObserverInput = gpGlobals->time + 0.2;
   }

   // Attack moves to the next player
   if ( m_afButtonPressed & IN_ATTACK && pev->iuser1 != OBS_ROAMING )
   {
     Observer_FindNextPlayer( false );

     m_flNextObserverInput = gpGlobals->time + 0.2;
   }

   // Attack2 moves to the prev player
   if ( m_afButtonPressed & IN_ATTACK2 && pev->iuser1 != OBS_ROAMING )
   {
     Observer_FindNextPlayer( true );

     m_flNextObserverInput = gpGlobals->time + 0.2;
   }
}

// Attempt to change the observer mode
void CBasePlayer::Observer_SetMode( int iMode )
{
  // Just abort if we're changing to the mode we're already in
   if ( iMode == pev->iuser1 )
     return;

   // is valid mode ?
   if ( iMode < OBS_CHASE_LOCKED || iMode > OBS_MAP_CHASE )
     iMode = OBS_IN_EYE; // now it is

   // if we are not roaming, we need a valid target to track
   if ( (iMode != OBS_ROAMING && iMode != OBS_MAP_FREE) && (m_hObserverTarget == NULL) )
   {
     Observer_FindNextPlayer( false );

   // if we didn't find a valid target switch to roaming
   if (m_hObserverTarget == NULL)
   {
     ClientPrint( pev, HUD_PRINTCENTER, "#Spec_NoTarget" );
     iMode = OBS_ROAMING;
   }
   }

   // set spectator mode
   pev->iuser1 = iMode;

   // set target if not roaming
   if (iMode == OBS_ROAMING || iMode == OBS_MAP_FREE)
     pev->iuser2 = 0;
   else
    pev->iuser2 = ENTINDEX( m_hObserverTarget->edict() );
}

