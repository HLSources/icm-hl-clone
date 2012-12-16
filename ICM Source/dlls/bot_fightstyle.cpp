
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
#include "weapons.h"
#include "bot.h"

///////////////////////////////////////////////////////////////////////////////
// Constructors/Destructors
///////////////////////////////////////////////////////////////////////////////

CBaseBotFightStyle::CBaseBotFightStyle( CBasePlayer *pPlayer ):
AimAt( AIM_BODY ),
bHoldDownAttack( FALSE ),
bSecondaryFire( FALSE ),
pOwner( pPlayer )
{
}

CBaseBotFightStyle::~CBaseBotFightStyle()
{
}

///////////////////////////////////////////////////////////////////////////////
// DispatchWeaponUse
///////////////////////////////////////////////////////////////////////////////

void CBaseBotFightStyle::DispatchWeaponUse( CBasePlayerItem *ActiveItem )
{
	SetHoldDownAttack( FALSE ); // unless the particular weapon sets this TRUE we want it false

	if ( FClassnameIs( ActiveItem->pev, "weapon_9mmAR" ) )
	{
		UseMP5();
	}
	else if ( FClassnameIs( ActiveItem->pev, "weapon_rpg" ) )
	{
		UseRPG();
	}
	else if ( FClassnameIs( ActiveItem->pev, "weapon_crossbow" ) )
	{
		UseCrossbow();
	}
	else if ( FClassnameIs( ActiveItem->pev, "weapon_crowbar" ) )
	{
		UseCrowbar();
	}
	else if ( FClassnameIs( ActiveItem->pev, "weapon_duebel" ) )
	{
		UseDuebel();
	}
	else if ( FClassnameIs( ActiveItem->pev, "weapon_handgrenade" ) )
	{
		UseHandGrenade();
	}
	else
	{
		ALERT( at_aiconsole, "Unidentified Weapon!\n" );
	}
}

///////////////////////////////////////////////////////////////////////////////
// RandomizeAimAtHead
///////////////////////////////////////////////////////////////////////////////

void CBaseBotFightStyle::RandomizeAimAtHead( const int AimAtHeadPropensity )
{
	CBaseBot *pBotOwner = (CBaseBot *)pOwner;

	if ( ( RANDOM_LONG(0,100) < AimAtHeadPropensity ) || ( pBotOwner->Stats.GetTraitAccuracy() >= 99 ) )
		SetAimAt( AIM_HEAD );
	else
		SetAimAt( AIM_BODY );
}

///////////////////////////////////////////////////////////////////////////////
// RandomizeSecondaryFire
///////////////////////////////////////////////////////////////////////////////

void CBaseBotFightStyle::RandomizeSecondaryFire( const int SecondaryFirePropensity )
{
	if ( RANDOM_LONG(0,100) < SecondaryFirePropensity )
	{
		SetSecondaryFire( TRUE );
	}
	else
	{
		SetSecondaryFire( FALSE );
	}
}

///////////////////////////////////////////////////////////////////////////////
// UseCrossbow
///////////////////////////////////////////////////////////////////////////////

inline void CBaseBotFightStyle::UseCrossbow( void )
{
	CBaseBot *pBotOwner = (CBaseBot *)pOwner;

	float DistanceToEnemy = ( pBotOwner->GetEnemy()->pev->origin - pBotOwner->pev->origin ).Length();

	if (  DistanceToEnemy > DISTANCE_FAR )
	{
		RandomizeSecondaryFire( 90 );
	}
	else if ( DistanceToEnemy > DISTANCE_NEAR )
	{
		RandomizeSecondaryFire( 60 );
	}
	else
	{
		RandomizeSecondaryFire( 20 );
	}

	if ( GetSecondaryFire() )
	{
		RandomizeAimAtHead( 60 );
	}
	else
	{
		SetAimAt( AIM_SPLASH );
	}
}

///////////////////////////////////////////////////////////////////////////////
// UseCrowbar
///////////////////////////////////////////////////////////////////////////////

inline void CBaseBotFightStyle::UseCrowbar( void )
{
	SetSecondaryFire( FALSE );

	RandomizeAimAtHead( 80 );

	SetHoldDownAttack( TRUE );
}

inline void CBaseBotFightStyle::UseDuebel( void )
{
	SetSecondaryFire( FALSE );


	SetHoldDownAttack( TRUE );
}

///////////////////////////////////////////////////////////////////////////////
// UseHandGrenade
///////////////////////////////////////////////////////////////////////////////

inline void CBaseBotFightStyle::UseHandGrenade( void )
{ // TODO: use grenadeToss ai (valve), also needs to do timing as well
	SetSecondaryFire( FALSE );

	SetAimAt( AIM_SPLASH );
}

///////////////////////////////////////////////////////////////////////////////
// UseMP5
///////////////////////////////////////////////////////////////////////////////

inline void CBaseBotFightStyle::UseMP5( void )
{ // TODO: should mixup ATTACK1/ATTACK2 mid-attack
	CBaseBot *pBotOwner = (CBaseBot *)pOwner;

	SetSecondaryFire( FALSE );

	if ( GetSecondaryFire() )
	{
		SetAimAt( AIM_SPLASH );
	}
	else
	{
		RandomizeAimAtHead( 30 );
		SetHoldDownAttack( TRUE );
	}
}

///////////////////////////////////////////////////////////////////////////////
// UseRPG
///////////////////////////////////////////////////////////////////////////////

inline void CBaseBotFightStyle::UseRPG( void )
{ // TODO: should use dot sometimes
	SetSecondaryFire( FALSE );

	SetAimAt( AIM_SPLASH );
}

