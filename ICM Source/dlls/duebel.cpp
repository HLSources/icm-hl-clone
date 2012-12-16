#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"

//Hier fängt der spannende Teil an: Meine Flamme besteht aus (bis zu 15) "Feuerbällen". Die werden hier "gemacht".
class CFireBall : public CBaseEntity
{
  void Spawn( void );
  void Precache( void );
  void EXPORT AnimateThink( void );
  void EXPORT ExplodeTouch( CBaseEntity *pOther );

EHANDLE m_hOwner;

public:
  static CFireBall *BallCreate( void );

};

LINK_ENTITY_TO_CLASS( fire_ball, CFireBall );

//Hiermit wird ein neuer Feuerball erstellt...
CFireBall *CFireBall::BallCreate( void )
{
  CFireBall *pBall = GetClassPtr( (CFireBall *)NULL );
  pBall->pev->classname = MAKE_STRING("fireball");
  pBall->Spawn();

  return pBall;
}


void CFireBall :: Spawn( void )
{
  Precache( );
  pev->movetype = MOVETYPE_FLY;
  pev->solid = SOLID_BBOX;

  SET_MODEL(ENT(pev), "sprites/smokepuff.spr");
  pev->rendermode = kRenderTransAdd;
  pev->rendercolor.x = 255;
  pev->rendercolor.y = 255;
  pev->rendercolor.z = 255;
  pev->renderamt = 255;
  pev->scale = 0.2;

  UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));
  UTIL_SetOrigin( pev, pev->origin );

  SetThink( &CFireBall::AnimateThink ); //Das wird jetzt immer wieder ausgeführt...

//Wenn man beim Auftreffen der Bälle irgendwas machen will (Sound, anderes Sprite),
//muss man die nächste Zeile wieder ent-kommentieren
  SetTouch( &CFireBall::ExplodeTouch );

#ifndef CLIENT_DLL
  m_hOwner = Instance( pev->owner );
#endif  
  pev->dmgtime = gpGlobals->time; // keep track of when ball spawned
  pev->nextthink = gpGlobals->time + 0.03;
}


void CFireBall :: Precache( void )
{
  PRECACHE_MODEL("sprites/smokepuff.spr"); //wir brauchen bloß ein Sprite
}


void CFireBall :: AnimateThink( void  )
{
  pev->nextthink = gpGlobals->time + 0.05; //das nächste Mal in einer zwanzigstel Sekunde ausführen

  pev->frame = ((int)pev->frame + 1) % 10; //Das Sprite ein bischen animieren...
  pev->scale = pev->scale * 1.5;   //Ganz wichtig: hier wird das Sprite vergrößert
          //Am Anfang ist es ganz klein und wird dann immer größer

  entvars_t *pevOwner;
  if ( pev->owner )
    pevOwner = VARS( pev->owner );
  else
    pevOwner = NULL;
  pev->owner = NULL; // can't traceline attack owner if this is set

  //Hier richten wir dann den Schaden an. Der Ausmaß orientiert sich auch ander Größe des Sprites
  //Pro Feuerball ziehe ich hier einen Lebenspunkt ab. Bei 30 Feuerbällen pro Sekunde ist das nicht wenig. Man kann es natürlich noch erhöhen...

  //Wenn das Sprite länger als eine halbe Sekunde lebt, wird es hier zerstört.
  if (gpGlobals->time - pev->dmgtime > 0.5 || pev->velocity.Length() < 10)
  {
    SetTouch( NULL );
    UTIL_Remove( this );
  }
}


void CFireBall::ExplodeTouch( CBaseEntity *pOther )
{
	//Wenn man das weiter oben aktiviert hat kann man hier seinen Code reinschreiben...
	pOther->TakeHealth( 1, DMG_GENERIC );
	UTIL_Remove( this );
}

enum Duebel_e
{
	DUEBEL_IDLE = 0,
};

LINK_ENTITY_TO_CLASS(weapon_duebel, CDuebel);

void CDuebel::Spawn(void)
{
    pev->classname = MAKE_STRING("weapon_duebel"); // hack to allow for old names
    Precache();
    m_iId = WEAPON_DUEBEL;
    m_iClip = -1;
	SET_MODEL(ENT(pev), "models/w_duebel.mdl");
    FallInit();
}

void CDuebel::Precache(void)
{
    PRECACHE_MODEL("models/p_duebel.mdl");
    PRECACHE_MODEL("models/v_duebel.mdl");
    PRECACHE_MODEL("models/w_duebel.mdl");
	UTIL_PrecacheOther( "fire_ball" );
}

int CDuebel::GetItemInfo(ItemInfo *p)
{
	p->pszName      = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 1;
    p->iId = WEAPON_DUEBEL;
    p->iWeight = DUEBEL_WEIGHT;
    return 1;
}

int CDuebel::AddToPlayer( CBasePlayer *pPlayer )
{
    if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
        MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
            WRITE_BYTE(m_iId);
        MESSAGE_END();
        return TRUE;
    }
    return FALSE;
}

BOOL CDuebel::Deploy (void)
{
    return DefaultDeploy("models/v_duebel.mdl", "models/p_duebel.mdl", DUEBEL_IDLE, "rpg");
}

void CDuebel::Holster(void)
{
    m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5;
    SendWeaponAnim(DUEBEL_IDLE);
}

void CDuebel::WeaponIdle(void)
{
}

void CDuebel::PrimaryAttack(void)
{
    if (m_pPlayer->pev->waterlevel == 3)
    {
        PlayEmptySound();
        m_flNextPrimaryAttack = gpGlobals->time + 0.15;
        return;
    }

#ifndef CLIENT_DLL

	if (m_pPlayer->pev->armorvalue <= 0) 
	{
		//ClientPrint( this->pev, HUD_PRINTCENTER, "collect dübelz tu smoke!"  );
		return; //Pech, Ammo ist leer...
	}

	SendWeaponAnim( DUEBEL_IDLE ); //sucht euch eine aus..

	Vector anglesAim = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
	UTIL_MakeVectors( anglesAim );
	anglesAim.x    = -anglesAim.x;
	Vector vecSrc   = m_pPlayer->GetGunPosition( ) - gpGlobals->v_up * 2; //Wo wir sind..
	Vector vecDir   = gpGlobals->v_forward; // ..und wo wir hinwollen!

	float delta = 0.06976; //8 Grad Abweichung..

	//Jetzt werden drei Feuerbälle ertellt. Alle mit zufälliger Abweichung und Geschwindigkeit

	vecDir = vecDir + Vector( RANDOM_FLOAT( -delta, delta ), RANDOM_FLOAT( -delta, delta ), RANDOM_FLOAT( -delta, delta ) );
	CFireBall *pBall1 = CFireBall::BallCreate();
	pBall1->pev->origin = vecSrc;
	pBall1->pev->angles = anglesAim;
	pBall1->pev->owner = m_pPlayer->edict();
	pBall1->pev->velocity = vecDir * RANDOM_FLOAT( 400, 600 );

	vecDir = vecDir + Vector( RANDOM_FLOAT( -delta, delta ), RANDOM_FLOAT( -delta, delta ), RANDOM_FLOAT( -delta, delta ) );
	CFireBall *pBall2 = CFireBall::BallCreate();
	pBall2->pev->origin = vecSrc;
	pBall2->pev->angles = anglesAim;
	pBall2->pev->owner = m_pPlayer->edict();
	pBall2->pev->velocity = vecDir * RANDOM_FLOAT( 400, 600 );

	vecDir = vecDir + Vector( RANDOM_FLOAT( -delta, delta ), RANDOM_FLOAT( -delta, delta ), RANDOM_FLOAT( -delta, delta ) );
	CFireBall *pBall3 = CFireBall::BallCreate();
	pBall3->pev->origin = vecSrc;
	pBall3->pev->angles = anglesAim;
	pBall3->pev->owner = m_pPlayer->edict();
	pBall3->pev->velocity = vecDir * RANDOM_FLOAT( 400, 600 );

	vecDir = vecDir + Vector( RANDOM_FLOAT( -delta, delta ), RANDOM_FLOAT( -delta, delta ), RANDOM_FLOAT( -delta, delta ) );
	CFireBall *pBall4 = CFireBall::BallCreate();
	pBall1->pev->origin = vecSrc;
	pBall1->pev->angles = anglesAim;
	pBall1->pev->owner = m_pPlayer->edict();
	pBall1->pev->velocity = vecDir * RANDOM_FLOAT( 400, 600 );

	vecDir = vecDir + Vector( RANDOM_FLOAT( -delta, delta ), RANDOM_FLOAT( -delta, delta ), RANDOM_FLOAT( -delta, delta ) );
	CFireBall *pBall5 = CFireBall::BallCreate();
	pBall2->pev->origin = vecSrc;
	pBall2->pev->angles = anglesAim;
	pBall2->pev->owner = m_pPlayer->edict();
	pBall2->pev->velocity = vecDir * RANDOM_FLOAT( 400, 600 );

	m_flTimeWeaponIdle = gpGlobals->time + 0.1;
	m_flNextPrimaryAttack = gpGlobals->time + 0.15;
	//m_flNextSecondaryAttack = UTIL_WeaponTimeBase(); //+ 0.1;

	m_pPlayer->pev->armorvalue--;
#endif
}

void CDuebel::SecondaryAttack(void)
{
}
