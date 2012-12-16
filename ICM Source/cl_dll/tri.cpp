//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// Triangle rendering, if any

#include "hud.h"
#include "cl_util.h"

// Triangle rendering apis are in gEngfuncs.pTriAPI

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"

#define DLLEXPORT __declspec( dllexport )

extern "C"
{
	void DLLEXPORT HUD_DrawNormalTriangles( void );
	void DLLEXPORT HUD_DrawTransparentTriangles( void );
};

//#define TEST_IT
#if defined( TEST_IT )

/*
=================
Draw_Triangles

Example routine.  Draws a sprite offset from the player origin.
=================
*/
void Draw_Triangles( void )
{
	cl_entity_t *player;
	vec3_t org;

	// Load it up with some bogus data
	player = gEngfuncs.GetLocalPlayer();
	if ( !player )
		return;

	org = player->origin;

	org.x += 50;
	org.y += 50;

	if (gHUD.m_hsprCursor == 0)
	{
		char sz[256];
		sprintf( sz, "sprites/cursor.spr" );
		gHUD.m_hsprCursor = SPR_Load( sz );
	}

	if ( !gEngfuncs.pTriAPI->SpriteTexture( (struct model_s *)gEngfuncs.GetSpritePointer( gHUD.m_hsprCursor ), 0 ))
	{
		return;
	}
	
	// Create a triangle, sigh
	gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
	gEngfuncs.pTriAPI->CullFace( TRI_NONE );
	gEngfuncs.pTriAPI->Begin( TRI_QUADS );
	// Overload p->color with index into tracer palette, p->packedColor with brightness
	gEngfuncs.pTriAPI->Color4f( 1.0, 1.0, 1.0, 1.0 );
	// UNDONE: This gouraud shading causes tracers to disappear on some cards (permedia2)
	gEngfuncs.pTriAPI->Brightness( 1 );
	gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
	gEngfuncs.pTriAPI->Vertex3f( org.x, org.y, org.z );

	gEngfuncs.pTriAPI->Brightness( 1 );
	gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
	gEngfuncs.pTriAPI->Vertex3f( org.x, org.y + 50, org.z );

	gEngfuncs.pTriAPI->Brightness( 1 );
	gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
	gEngfuncs.pTriAPI->Vertex3f( org.x + 50, org.y + 50, org.z );

	gEngfuncs.pTriAPI->Brightness( 1 );
	gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
	gEngfuncs.pTriAPI->Vertex3f( org.x + 50, org.y, org.z );

	gEngfuncs.pTriAPI->End();
	gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
}

#endif

/*
=================
HUD_DrawNormalTriangles

Non-transparent triangles-- add them here
=================
*/
void DLLEXPORT HUD_DrawNormalTriangles( void )
{

	gHUD.m_Spectator.DrawOverview();
	
#if defined( TEST_IT )
//	Draw_Triangles();
#endif
}

/*
=================
HUD_DrawTransparentTriangles

Render any triangles with transparent rendermode needs here
=================
*/
void DLLEXPORT HUD_DrawTransparentTriangles( void )
{
	// render team overlays
	cl_entity_t *thisplayer = gEngfuncs.GetLocalPlayer();
	float size = 25;
	vec3_t angles, forward, right, up;
	gEngfuncs.GetViewAngles( (float *)angles );
	AngleVectors( angles, forward, right, up);

	if (gHUD.m_Teamplay)
	{
		for( int i=0; i < MAX_PLAYERS+1; i++)
		{
			cl_entity_t *player = gEngfuncs.GetEntityByIndex(i);
			int team = g_PlayerExtraInfo[player->index].teamnumber;
			// they aren't a player, continue the looping
			if( g_PlayerInfoList[i].name == NULL )
				continue;
			// if they are this player, then continue
			if( player == thisplayer )
				continue;
			if( g_PlayerExtraInfo[thisplayer->index].teamnumber == team )
			{
				// render the sprite above their heads
				if(player)
				{
					vec3_t org;
					vec3_t point;
					org = player->origin;
	
					// starting position
					org.z += 52;
					org = org + right * -size/2;
	
					if (gHUD.m_hsprCursor == 0)
					{
						char sz[256];
						sprintf( sz, "sprites/flare3.spr" );
						gHUD.m_hsprCursor = SPR_Load( sz );
					}

					if ( !gEngfuncs.pTriAPI->SpriteTexture( (struct model_s *)gEngfuncs.GetSpritePointer( gHUD.m_hsprCursor ), 0 ))
						return;
	
	
					// Create a triangle, sigh
					gEngfuncs.pTriAPI->RenderMode( kRenderTransAdd );
					gEngfuncs.pTriAPI->CullFace( TRI_NONE );
					gEngfuncs.pTriAPI->Begin( TRI_QUADS );
	
					// now draw that tiny quad
					// the triapi calls are pretty similar to opengl
					gEngfuncs.pTriAPI->Color4f( 0.0, 1.0, 0.0, 1.0 );
	
					gEngfuncs.pTriAPI->Brightness( 1.0 );
					gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
					point = org;
					gEngfuncs.pTriAPI->Vertex3fv( point );
	
					gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
					point = org + up * size;
					gEngfuncs.pTriAPI->Vertex3fv( point );
	
					gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
					point = org + right * size + up * size;
					gEngfuncs.pTriAPI->Vertex3fv( point );
	
					gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
					point = org + right * size;
					gEngfuncs.pTriAPI->Vertex3fv( point );
	
					gEngfuncs.pTriAPI->End();
					gEngfuncs.pTriAPI->RenderMode( kRenderTransAdd );
				}
			}	
		}
	}
#if defined( TEST_IT )
//	Draw_Triangles();
#endif
}