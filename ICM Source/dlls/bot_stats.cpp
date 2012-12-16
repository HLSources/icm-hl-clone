
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
#include "game.h"
#include <fstream.h>

///////////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
///////////////////////////////////////////////////////////////////////////////

CBaseBotStats::CBaseBotStats():
PrefCrossbow( 70 + RANDOM_FLOAT(0,20) ),
PrefCrowbar( 10 + RANDOM_FLOAT(0,20) ),
PrefHandGrenade( 10 + RANDOM_FLOAT(0,20) ),
PrefMP5( 80 + RANDOM_FLOAT(0,20) ),
PrefRPG( 60 + RANDOM_FLOAT(0,20) ),
TraitAccuracy( 75 + RANDOM_FLOAT(0,25) ),
TraitAggression( 10 + RANDOM_FLOAT(0,70) ),
//TraitChatPropensity( 0 + RANDOM_FLOAT(0,40) ),
TraitJumpPropensity( 5 + RANDOM_FLOAT(0,94) ),
TraitPerception( 50 + RANDOM_FLOAT(0,50) ),
TraitReflexes( 30 + RANDOM_FLOAT(0,40) )
{
	strcpy( Name, "icmbot" );
	strcpy( Model, "gordon" ),
//	strcpy( ChatFile, "default" );
	strcpy( TopColor, "255" );
	strcpy( BottomColor, "255" );
}

CBaseBotStats::~CBaseBotStats()
{
}

///////////////////////////////////////////////////////////////////////////////
// FDifficultyAdjusted - tweaks a Trait to adjust for BotGlobals.DifficultyLevel
///////////////////////////////////////////////////////////////////////////////

float CBaseBotStats::FDifficultyAdjusted( float currentTrait )
{
	float tweakedDiffLevel = bot_skill.value - 50; // [-50,50]

	BOOL TweakEasier;
	if ( tweakedDiffLevel < 0 )
	{
		TweakEasier = TRUE;
	}
	else
	{
		TweakEasier = FALSE;
	}

	tweakedDiffLevel = abs( tweakedDiffLevel ); // [0,50]
	tweakedDiffLevel /= 50; // [0,1]

	float adjustedTrait = currentTrait;
	if ( TweakEasier )
	{
		adjustedTrait *= ( 1 - tweakedDiffLevel );
	}
	else
	{
		adjustedTrait += ( ( 100 - adjustedTrait ) * tweakedDiffLevel );
	}

	return adjustedTrait;
}

///////////////////////////////////////////////////////////////////////////////
// GetWeaponDesire - pass DISTANCE_MAX to DistanceToEnemy to use this like DispatchWeaponDesire
///////////////////////////////////////////////////////////////////////////////

float CBaseBotStats::FindWeaponDesire( CBasePlayerItem *pWeapon, float DistanceToEnemy )
{ // this function is tweaking the bot's base weapon preferences to suit the situation

/*	if ( FClassnameIs( pWeapon->pev, "weapon_duebel" ) )
	{
		if ( pPlayer->pev->armorvalue > 0 )
		{
			return 100;
		}
		else
		{
			return 0;
		}
	}
*/	if ( FClassnameIs( pWeapon->pev, "weapon_9mmAR" ) )
	{
		return GetPrefMP5();
	}
	else if ( FClassnameIs( pWeapon->pev, "weapon_rpg" ) )
	{
		if ( DistanceToEnemy > DISTANCE_FAR )
		{
			return GetPrefRPG();
		}
		else
		{
			return 0;
		}
	}
	else if ( FClassnameIs( pWeapon->pev, "weapon_crossbow" ) )
	{
		if ( DistanceToEnemy > DISTANCE_NEAR )
		{
			return GetPrefCrossbow();
		}
		else
		{
			return 0;
		}
	}
	else if ( FClassnameIs( pWeapon->pev, "weapon_crowbar" ) )
	{
		if ( DistanceToEnemy < DISTANCE_NEAR )
		{
			return GetPrefCrowbar();
		}
		else
		{
			return 0;
		}
	}
	else if ( FClassnameIs( pWeapon->pev, "weapon_handgrenade" ) )
	{
		if ( DistanceToEnemy > DISTANCE_FAR )
		{
			return GetPrefHandGrenade();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
// ReadBotFile (Ben)
///////////////////////////////////////////////////////////////////////////////

BOOL CBaseBotStats::ReadBotFile( char *filename ) {
	/* 
	reads the contents of filename into the CBaseBotStats object
	*/
	char line[MAX_LINE_LENGTH];
	//start arsh0r
    struct {
        char botName[32];
        char botModel[32];
        int botTopColor;
        int botBottomColor;
        int botJumpPropensity;
        int botAccuracy;
        int botAggression;
        int botPerception;
        int botReflexes;
        int botSnipermac;
        int botLightsaber;
        int botGrumbern;
        int botMac10;
        int botRocketmac;
    } botData;

    FILE *botFile;

    if (botFile = fopen( filename, "rb" ))
    {
   		fread( &botData, sizeof(botData), 1, botFile);
        fclose( botFile );        
		
		SetName (botData.botName);
		SetModel (botData.botModel);
		sprintf(line, "%i", botData.botTopColor);
	    SetTopColor(line);
		sprintf(line, "%i", botData.botBottomColor);
		SetBottomColor(line);
	    
		SetTraitJumpPropensity(botData.botJumpPropensity);
	    SetTraitAccuracy(botData.botAccuracy);
		SetTraitAggression(botData.botAggression);
	    SetTraitPerception(botData.botPerception);
	    SetTraitReflexes(botData.botReflexes);

	    SetPrefCrossbow(botData.botSnipermac);
	    SetPrefCrowbar(botData.botLightsaber);
		SetPrefHandGrenade(botData.botGrumbern);
		SetPrefMP5(botData.botMac10);
		SetPrefRPG(botData.botRocketmac);
		return TRUE;
    }
	else
	{
		return FALSE;
	}
	//end arsh0r
	
	//	ifstream botFile(filename);
	//	char line[MAX_LINE_LENGTH];

	/*
	we only need to read in every other line of the file.  
	we skip the first.  all of these will come in groups of two.  
	note that this will not work if we get the wrong file, 
	or the wrong format

	if (botFile.bad()) {
	return FALSE;
	} else {
	botFile.getline(line, sizeof(line));
	botFile.getline(line, sizeof(line));//name
	SetName(line);

	botFile.getline(line, sizeof(line));
	botFile.getline(line, sizeof(line));//model
	SetModel(line);

    botFile.getline(line, sizeof(line));
    botFile.getline(line, sizeof(line));//top color
    SetTopColor(line);

    botFile.getline(line, sizeof(line));
    botFile.getline(line, sizeof(line));//bottom color
    SetBottomColor(line);

    botFile.getline(line, sizeof(line));
    botFile.getline(line, sizeof(line));//jump propensity
    SetTraitJumpPropensity(atof(line));

    botFile.getline(line, sizeof(line));
    botFile.getline(line, sizeof(line));//accuracy
    SetTraitAccuracy(atof(line));

    botFile.getline(line, sizeof(line));
    botFile.getline(line, sizeof(line));//aggression
    SetTraitAggression(atof(line));

    botFile.getline(line, sizeof(line));
    botFile.getline(line, sizeof(line));//perception
    SetTraitPerception(atof(line));

    botFile.getline(line, sizeof(line));
    botFile.getline(line, sizeof(line));//reflexes
    SetTraitReflexes(atof(line));

    botFile.getline(line, sizeof(line));
    botFile.getline(line, sizeof(line));//preferences - crossbow
    SetPrefCrossbow(atof(line));

    botFile.getline(line, sizeof(line));
    botFile.getline(line, sizeof(line));//preferences - crowbar
    SetPrefCrowbar(atof(line));

    botFile.getline(line, sizeof(line));
    botFile.getline(line, sizeof(line));//preferences - egon
    SetPrefEgon(atof(line));

    botFile.getline(line, sizeof(line));
    botFile.getline(line, sizeof(line));//preferences - gauss
    SetPrefGauss(atof(line));

    botFile.getline(line, sizeof(line));
    botFile.getline(line, sizeof(line));//preferences - glock
    SetPrefGlock(atof(line));

    botFile.getline(line, sizeof(line));
    botFile.getline(line, sizeof(line));//preferences - handgrenade
    SetPrefHandGrenade(atof(line));

    botFile.getline(line, sizeof(line));
    botFile.getline(line, sizeof(line));//preferences - hornetgun
    SetPrefHornetGun(atof(line));

    botFile.getline(line, sizeof(line));
    botFile.getline(line, sizeof(line));//preferences - mp5
    SetPrefMP5(atof(line));

    botFile.getline(line, sizeof(line));
    botFile.getline(line, sizeof(line));//preferences - python
    SetPrefPython(atof(line));

    botFile.getline(line, sizeof(line));
    botFile.getline(line, sizeof(line));//preferences - rpg
    SetPrefRPG(atof(line));

    botFile.getline(line, sizeof(line));
    botFile.getline(line, sizeof(line));//preferences - satchel
    SetPrefSatchel(atof(line));

    botFile.getline(line, sizeof(line));
    botFile.getline(line, sizeof(line));//preferences - shotgun
	SetPrefShotgun(atof(line));

	botFile.getline(line, sizeof(line));
	botFile.getline(line, sizeof(line));//preferences - snark
	SetPrefSnark(atof(line));

	botFile.getline(line, sizeof(line));
	botFile.getline(line, sizeof(line));//preferences - tripmine
	SetPrefTripMine(atof(line));

	botFile.close();
	return TRUE;
	}
	*/
}


