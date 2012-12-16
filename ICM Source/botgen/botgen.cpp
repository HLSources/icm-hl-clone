/*
lol, my first console program.
i think its full of errors and contains stupid stuff *g*
fuck it

--arsh0r
*/

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* stolen from main.cc */
void center_print (FILE *file, char *line, int maxwidth)
{
  int imax;
  int i;
  imax = (maxwidth - strlen(line)) >> 1;
  for (i=0; i<imax; i++) fputc (' ', file);
  fputs (line, file);
}

/* remove leading spaces, newline junk at end.  returns pointer to 
 cleaned string, which is between s0 and the null */
char *clean_string (char *s0)
{
    char *s = s0;
    char *ptr;
    /* find first nonblank */
    while (isspace (*s))
        s++;
    /* truncate string at first non-alphanumeric */
    ptr = s;
    while (isprint (*ptr))
        ptr++;
    *ptr = 0;
    return s;
}

char *get_string (char *description, char *defaultvalue)
{
	char inBuffer[1024];
    printf ("%s [%s]: ", description, defaultvalue);
    fgets (inBuffer, sizeof(inBuffer), stdin);
    if ( strcmp(inBuffer, "\n" ) )
    {
        return (clean_string(inBuffer));
    }
    else
    {
        return(defaultvalue);
    }
}

int get_int (char *description, int defaultvalue, int min, int max)
{
	char inBuffer[1024];
    int illegal;
    printf ("%s [%i]: ", description, defaultvalue);
    fgets (inBuffer, sizeof(inBuffer), stdin);
    if ( strcmp(inBuffer, "\n" ) )
    {
        illegal = (1 != sscanf (inBuffer, "%d", &defaultvalue));
        if (illegal || defaultvalue<min || defaultvalue>max)
        {
                return (23);
        }
        else
        {
                return (defaultvalue);
        }
    }
    else
    {
        return(defaultvalue);
    }
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    FILE *botFile;
    bool isUpdate = false;
    char botFileName[64] = "icmbot";
    
    char bot_skins[10][32] = {
        	"barney",
        	"gina",
        	"gman",
        	"gordon",
        	"helmet",
        	"hgrunt",
        	"recon",
        	"robo",
        	"scientist",
        	"zombie"
   	};
   	
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
   
    center_print (stdout, "bot generat0r for ingram chillin' mod v0.1\n", 80);
    center_print (stdout, "discord23 productions\n\n", 80);
    strcpy (botFileName ,get_string("*.bot filename", botFileName));

    strcpy(botData.botName, botFileName);
    strcpy(botData.botModel, bot_skins[rand() % 10]);
    botData.botTopColor = rand() % 256;
    botData.botBottomColor = rand() % 256;
    botData.botJumpPropensity = rand() % 101;
    botData.botAccuracy = rand() % 101;
    botData.botAggression = rand() % 101;
    botData.botPerception = rand() % 101;
    botData.botReflexes = rand() % 101;
    botData.botSnipermac = rand() % 101;
    botData.botLightsaber = rand() % 101;
    botData.botGrumbern = rand() % 101;
    botData.botMac10 = rand() % 101;
    botData.botRocketmac = rand() % 101;

    strcat (botFileName, ".bot");

    if (botFile = fopen( botFileName, "rb" ))
    {
   		fread( &botData, sizeof(botData), 1, botFile);
        fclose( botFile );        
        isUpdate = true;
    }
    
    if (botFile = fopen( botFileName, "wb" ))
	{
	    strcpy (botData.botName, get_string("name", botData.botName));
        strcpy (botData.botModel, get_string("model", botData.botModel));
        botData.botTopColor = get_int("top color (0-255)", botData.botTopColor, 0, 255);
        botData.botBottomColor = get_int("bottom color (0-255)", botData.botBottomColor, 0, 255);
        botData.botJumpPropensity = get_int("jump propensity (0=elephant 100=kangaroo)", botData.botJumpPropensity, 0, 100);
        botData.botAccuracy = get_int("trait: accuracy (1-100)", botData.botAccuracy, 0, 100);
        botData.botAggression = get_int("trait: aggression (1-100)", botData.botAggression, 0, 100);
        botData.botPerception = get_int("trait: perception (1-100)", botData.botPerception, 0, 100);
        botData.botReflexes = get_int("trait: reflexes (1-100)", botData.botReflexes, 0, 100);
        botData.botSnipermac = get_int("weapon preference: snipermac (1-100)", botData.botSnipermac, 0, 100);
        botData.botLightsaber = get_int("weapon preference: lightsaber (1-100)", botData.botLightsaber, 0, 100);
        botData.botGrumbern = get_int("weapon preference: grumbern (1-100)", botData.botGrumbern, 0, 100);
        botData.botMac10 = get_int("weapon preference: mac10 (1-100)", botData.botMac10, 0, 100);
        botData.botRocketmac = get_int("weapon preference: rocketmac (1-100)", botData.botRocketmac, 0, 100);

        fwrite( &botData, sizeof(botData), 1, botFile );
        fclose( botFile );
    }
    if (isUpdate) 
    {
        printf ("botfile successfully updated!\n");
    }
    else
    {
        printf ("botfile successfully created!\n");
    }
    getchar();
    return 0;
}
