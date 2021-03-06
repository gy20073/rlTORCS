/***************************************************************************

    file                 : grscene.cpp
    created              : Mon Aug 21 20:13:56 CEST 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: grscene.cpp,v 1.47.2.7 2012/06/10 09:32:24 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#ifdef WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#include <GL/glut.h>
#include <plib/ssg.h>
#include <plib/ssgAux.h>
#include <tgfclient.h>
#include <track.h>
#include <car.h>
#include <graphic.h>
#include <robottools.h>
#include <portability.h>

#include "grmain.h"
#include "grshadow.h"
#include "grskidmarks.h"
#include "grsmoke.h"
#include "grcar.h"
#include "grcam.h"
#include "grscene.h"
#include "grutil.h"
#include "grssgext.h"
#include "grtexture.h"

/////////////////////////////////// by yurong
#include <collect_segmentation.h>
/////////////////////////////////// end by yurong


int grWrldX;
int grWrldY;
int grWrldZ;
int grWrldMaxSize;
tTrack 	 *grTrack;

ssgStateSelector	*grEnvSelector;
grMultiTexState	*grEnvState=NULL;
grMultiTexState	*grEnvShadowState=NULL;
grMultiTexState	*grEnvShadowStateOnCars=NULL;
#define NB_BG_FACES	36
#define BG_DIST		1.0

ssgRoot *TheScene = 0;
static ssgRoot *TheBackground = 0;

/* TheScene kid order */
ssgBranch *SunAnchor = 0;
ssgBranch *LandAnchor = 0;
ssgBranch *CarsAnchor = 0;
ssgBranch *ShadowAnchor = 0;
ssgBranch *PitsAnchor = 0;
ssgBranch *SmokeAnchor = 0;
ssgBranch *SkidAnchor = 0;
ssgBranch *CarlightAnchor = 0;

ssgBranch *ThePits = 0;
ssgTransform *sun_grscene = NULL ;

static void initBackground(void);

extern ssgEntity *grssgLoadAC3D ( const char *fname, const ssgLoaderOptions* options );

int preScene(ssgEntity *e)
{
  return TRUE;
}


int
grInitScene(void)
{
	void *hndl = grTrackHandle;
	ssgLight *light = ssgGetLight(0);

	GLfloat mat_specular[]   = {0.3, 0.3, 0.3, 1.0};
	GLfloat mat_shininess[]  = {50.0};
	GLfloat light_position[] = {0, 0, 200, 0.0};
	GLfloat lmodel_ambient[] = {0.2, 0.2, 0.2, 1.0};
	GLfloat lmodel_diffuse[] = {0.8, 0.8, 0.8, 1.0};
	GLfloat fog_clr[]        = {1.0, 1.0, 1.0, 0.5};

	mat_specular[0] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_SPEC_R, NULL, mat_specular[0]);
	mat_specular[1] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_SPEC_G, NULL, mat_specular[1]);
	mat_specular[2] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_SPEC_B, NULL, mat_specular[2]);

	lmodel_ambient[0] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_AMBIENT_R, NULL, lmodel_ambient[0]);
	lmodel_ambient[1] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_AMBIENT_G, NULL, lmodel_ambient[1]);
	lmodel_ambient[2] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_AMBIENT_B, NULL, lmodel_ambient[2]);

	lmodel_diffuse[0] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_DIFFUSE_R, NULL, lmodel_diffuse[0]);
	lmodel_diffuse[1] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_DIFFUSE_G, NULL, lmodel_diffuse[1]);
	lmodel_diffuse[2] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_DIFFUSE_B, NULL, lmodel_diffuse[2]);

	mat_shininess[0] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_SHIN, NULL, mat_shininess[0]);

	light_position[0] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_LIPOS_X, NULL, light_position[0]);
	light_position[1] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_LIPOS_Y, NULL, light_position[1]);
	light_position[2] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_LIPOS_Z, NULL, light_position[2]);

	glShadeModel(GL_SMOOTH);

	light->setPosition(light_position[0],light_position[1],light_position[2]);
	light->setColour(GL_AMBIENT,lmodel_ambient);
	light->setColour(GL_DIFFUSE,lmodel_diffuse);
	light->setColour(GL_SPECULAR,mat_specular);
	light->setSpotAttenuation(0.0, 0.0, 0.0);

	sgCopyVec3 (fog_clr,  grTrack->graphic.bgColor);
	sgScaleVec3 (fog_clr, 0.8);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogfv(GL_FOG_COLOR, fog_clr);
	glFogf(GL_FOG_DENSITY, 0.05);
	glHint(GL_FOG_HINT, GL_DONT_CARE);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);

	if (!sun_grscene) {
		ssgaLensFlare *sun_obj = NULL ;
		sun_obj = new ssgaLensFlare () ;
		sun_grscene = new ssgTransform ;
		sun_grscene->setTransform( light_position );
		sun_grscene-> addKid(sun_obj);
		SunAnchor-> addKid(sun_grscene);
    }

    /* GUIONS GL_TRUE */
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);

#ifdef GL_SEPARATE_SPECULAR_COLOR
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
#else
#ifdef GL_SEPARATE_SPECULAR_COLOR_EXT
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL_EXT,GL_SEPARATE_SPECULAR_COLOR_EXT);
#endif
#endif

    return 0;
}

static ssgLoaderOptionsEx	options;
ssgEntity *desc;
// yurong here
#ifdef COLLECTSEG
ssgEntity *replace_middle;
ssgEntity *replace_side;
extern int drawIndicator;
ssgBranch* temp = new ssgBranch();
ssgBranch* temp_scene = new ssgBranch();
#endif
// yurong here
int
grLoadScene(tTrack *track)
{
	void *hndl = grTrackHandle;
	const char *acname;
	// ssgEntity *desc;
	const int BUFSIZE = 256;
	char buf[BUFSIZE];

	if (maxTextureUnits==0) {
		InitMultiTex();
	}

	ssgSetCurrentOptions(&options);
	ssgAddTextureFormat(".png", grLoadPngTexture);
	grRegisterCustomSGILoader();

	grTrack = track;
	TheScene = new ssgRoot;

	/* Landscape */
	LandAnchor = new ssgBranch;
	TheScene->addKid(LandAnchor);

	/* Pit stops walls */
	PitsAnchor = new ssgBranch;
	TheScene->addKid(PitsAnchor);

	/* Skid Marks */
	SkidAnchor = new ssgBranch;
	TheScene->addKid(SkidAnchor);

	/* Car shadows */
	ShadowAnchor = new ssgBranch;
	TheScene->addKid(ShadowAnchor);

	/* Car lights */
	CarlightAnchor = new ssgBranch;
	TheScene->addKid(CarlightAnchor);

	/* Cars */
	CarsAnchor = new ssgBranch;
	TheScene->addKid(CarsAnchor);

	/* Smoke */
	SmokeAnchor = new ssgBranch;
	TheScene->addKid(SmokeAnchor);

	/* Lens Flares */
	SunAnchor = new ssgBranch;
	TheScene->addKid(SunAnchor);


	initBackground();

	grWrldX = (int)(track->max.x - track->min.x + 1);
	grWrldY = (int)(track->max.y - track->min.y + 1);
	grWrldZ = (int)(track->max.z - track->min.z + 1);
	grWrldMaxSize = (int)(MAX(MAX(grWrldX, grWrldY), grWrldZ));

	acname = GfParmGetStr(hndl, TRK_SECT_GRAPH, TRK_ATT_3DDESC, "track.ac");
	if (strlen(acname) == 0) {
		return -1;
	}

	snprintf(buf, BUFSIZE, "tracks/%s/%s;data/textures;data/img;.", grTrack->category, grTrack->internalname);
	ssgTexturePath(buf);
	snprintf(buf, BUFSIZE, "tracks/%s/%s", grTrack->category, grTrack->internalname);
	ssgModelPath(buf);

	desc = grssgLoadAC3D(acname, NULL);
	LandAnchor->addKid(desc);

#ifdef COLLECTSEG
	replace_middle = grssgLoadAC3D("generato-city_city_replace_middle.ac", NULL);
	replace_side = grssgLoadAC3D("generato-city_city_replace_side.ac", NULL);
	temp->addKid(replace_middle);
	temp->addKid(replace_side);
	temp->addKid(desc);
#endif

	// yurong
	return 0;
}

// yurong



void grDrawScene(void)
{
    TRACE_GL("refresh: ssgCullAndDraw start");
#ifdef COLLECTSEG
    if (drawIndicator == 1) {
    	LandAnchor->replaceKid(desc, replace_side);
    }
    else if (drawIndicator == 2){
    	LandAnchor->replaceKid(desc, replace_middle);
    }
    else if (drawIndicator == 3){
    	temp_scene->addKid(ShadowAnchor);
    	TheScene->removeKid(ShadowAnchor);
		temp_scene->addKid(CarlightAnchor);
    	TheScene->removeKid(CarlightAnchor);
		temp_scene->addKid(CarsAnchor);
    	TheScene->removeKid(CarsAnchor);
		temp_scene->addKid(SmokeAnchor);
    	TheScene->removeKid(SmokeAnchor);
    }
#endif COLLECTSEG
    ssgCullAndDraw(TheScene);
#ifdef COLLECTSEG
    if (drawIndicator == 1) {
    	LandAnchor->replaceKid(replace_side, desc);
    }
    else if (drawIndicator == 2){
    	LandAnchor->replaceKid(replace_middle, desc);
    }
    else if (drawIndicator == 3){
		TheScene->addKid(SmokeAnchor);
		temp_scene->removeKid(SmokeAnchor);
		TheScene->addKid(CarsAnchor);
		temp_scene->removeKid(CarsAnchor);
		TheScene->addKid(CarlightAnchor);
		temp_scene->removeKid(CarlightAnchor);
		TheScene->addKid(ShadowAnchor);
		temp_scene->removeKid(ShadowAnchor);
    }
#endif
    TRACE_GL("refresh: ssgCullAndDraw");
}


void grShutdownScene(void)
{
	if (TheScene) {
		delete TheScene;
		TheScene = 0;
	}

	if (TheBackground) {
		delete TheBackground;
		TheBackground = 0;
	}

	if (grEnvState != NULL) {
		ssgDeRefDelete(grEnvState);
		grEnvState = NULL;
	}
	if (grEnvShadowState != NULL) {
		ssgDeRefDelete(grEnvShadowState);
		grEnvShadowState = NULL;
	}
	if (grEnvShadowStateOnCars != NULL) {
		ssgDeRefDelete(grEnvShadowStateOnCars);
		grEnvShadowStateOnCars = NULL;
	}
	if(grEnvSelector != NULL) {
		delete grEnvSelector;
		grEnvSelector = NULL;
	}

	options.endLoad();
}


static void
initBackground(void)
{
    int			i;
    float		x, y, z1, z2;
    double		alpha;
    float		texLen;
    tTrackGraphicInfo	*graphic;
    ssgSimpleState	*envst;
    sgVec3		vtx;
    sgVec4		clr;
    sgVec3		nrm;
    sgVec2		tex;
    const int BUFSIZE = 1024;
	char buf[BUFSIZE];
    ssgVtxTable 	*bg;
    ssgVertexArray	*bg_vtx;
    ssgTexCoordArray	*bg_tex;
    ssgColourArray	*bg_clr;
    ssgNormalArray	*bg_nrm;
    ssgSimpleState	*bg_st;

    snprintf(buf, BUFSIZE, "tracks/%s/%s;data/img;data/textures;.", grTrack->category, grTrack->internalname);
    grFilePath = buf;
    grGammaValue = 1.8;
    grMipMap = 0;

    graphic = &grTrack->graphic;
    glClearColor(graphic->bgColor[0], graphic->bgColor[1], graphic->bgColor[2], 1.0);

    TheBackground = new ssgRoot();
    clr[0] = clr[1] = clr[2] = 1.0;
    clr[3] = 1.0;
    nrm[0] = nrm[2] = 0.0;
    nrm[1] = 1.0;

    z1 = -0.5;
    z2 = 1.0;
    switch (graphic->bgtype) {
    case 0:
	bg_vtx = new ssgVertexArray(NB_BG_FACES + 1);
	bg_tex = new ssgTexCoordArray(NB_BG_FACES + 1);
	bg_clr = new ssgColourArray(1);
	bg_nrm = new ssgNormalArray(1);

	bg_clr->add(clr);
	bg_nrm->add(nrm);

	for (i = 0; i < NB_BG_FACES + 1; i++) {
	    alpha = (float)i * 2 * PI / (float)NB_BG_FACES;
	    texLen = (float)i / (float)NB_BG_FACES;

	    x = BG_DIST * cos(alpha);
	    y = BG_DIST * sin(alpha);

	    vtx[0] = x;
	    vtx[1] = y;
	    vtx[2] = z1;
	    bg_vtx->add(vtx);
	    tex[0] = texLen*4.0;
	    tex[1] = 0;
	    bg_tex->add(tex);

	    vtx[0] = x;
	    vtx[1] = y;
	    vtx[2] = z2;
	    bg_vtx->add(vtx);
	    tex[0] = texLen*4.0;
	    tex[1] = 1.0;
	    bg_tex->add(tex);
	}
	bg = new ssgVtxTable(GL_TRIANGLE_STRIP, bg_vtx, bg_nrm, bg_tex, bg_clr);
	bg_st = (ssgSimpleState*)grSsgLoadTexState(graphic->background);
	bg_st->disable(GL_LIGHTING);
	bg->setState(bg_st);
	bg->setCullFace(0);
	TheBackground->addKid(bg);
	break;

    case 2:
	bg_vtx = new ssgVertexArray(NB_BG_FACES + 1);
	bg_tex = new ssgTexCoordArray(NB_BG_FACES + 1);
	bg_clr = new ssgColourArray(1);
	bg_nrm = new ssgNormalArray(1);

	bg_clr->add(clr);
	bg_nrm->add(nrm);

	for (i = 0; i < NB_BG_FACES / 4 + 1; i++) {
	    alpha = (float)i * 2 * PI / (float)NB_BG_FACES;
	    texLen = (float)i / (float)NB_BG_FACES;

	    x = BG_DIST * cos(alpha);
	    y = BG_DIST * sin(alpha);

	    vtx[0] = x;
	    vtx[1] = y;
	    vtx[2] = z1;
	    bg_vtx->add(vtx);
	    tex[0] = texLen*4.0;
	    tex[1] = 0;
	    bg_tex->add(tex);

	    vtx[0] = x;
	    vtx[1] = y;
	    vtx[2] = z2;
	    bg_vtx->add(vtx);
	    tex[0] = texLen*4.0;
	    tex[1] = 0.5;
	    bg_tex->add(tex);
	}
	bg = new ssgVtxTable(GL_TRIANGLE_STRIP, bg_vtx, bg_nrm, bg_tex, bg_clr);
	bg_st = (ssgSimpleState*)grSsgLoadTexState(graphic->background);
	bg_st->disable(GL_LIGHTING);
	bg->setState(bg_st);
	bg->setCullFace(0);
	TheBackground->addKid(bg);

	bg_vtx = new ssgVertexArray(NB_BG_FACES + 1);
	bg_tex = new ssgTexCoordArray(NB_BG_FACES + 1);
	bg_clr = new ssgColourArray(1);
	bg_nrm = new ssgNormalArray(1);

	bg_clr->add(clr);
	bg_nrm->add(nrm);

	for (i = NB_BG_FACES/4; i < NB_BG_FACES / 2 + 1; i++) {
	    alpha = (float)i * 2 * PI / (float)NB_BG_FACES;
	    texLen = (float)i / (float)NB_BG_FACES;

	    x = BG_DIST * cos(alpha);
	    y = BG_DIST * sin(alpha);

	    vtx[0] = x;
	    vtx[1] = y;
	    vtx[2] = z1;
	    bg_vtx->add(vtx);
	    tex[0] = texLen*4.0;
	    tex[1] = 0.5;
	    bg_tex->add(tex);

	    vtx[0] = x;
	    vtx[1] = y;
	    vtx[2] = z2;
	    bg_vtx->add(vtx);
	    tex[0] = texLen*4.0;
	    tex[1] = 1.0;
	    bg_tex->add(tex);
	}
	bg = new ssgVtxTable(GL_TRIANGLE_STRIP, bg_vtx, bg_nrm, bg_tex, bg_clr);
	bg_st = (ssgSimpleState*)grSsgLoadTexState(graphic->background);
	bg_st->disable(GL_LIGHTING);
	bg->setState(bg_st);
	bg->setCullFace(0);
	TheBackground->addKid(bg);

	bg_vtx = new ssgVertexArray(NB_BG_FACES + 1);
	bg_tex = new ssgTexCoordArray(NB_BG_FACES + 1);
	bg_clr = new ssgColourArray(1);
	bg_nrm = new ssgNormalArray(1);

	bg_clr->add(clr);
	bg_nrm->add(nrm);

	for (i = NB_BG_FACES / 2; i < 3 * NB_BG_FACES / 4 + 1; i++) {
	    alpha = (float)i * 2 * PI / (float)NB_BG_FACES;
	    texLen = (float)i / (float)NB_BG_FACES;

	    x = BG_DIST * cos(alpha);
	    y = BG_DIST * sin(alpha);

	    vtx[0] = x;
	    vtx[1] = y;
	    vtx[2] = z1;
	    bg_vtx->add(vtx);
	    tex[0] = texLen*4.0;
	    tex[1] = 0.0;
	    bg_tex->add(tex);

	    vtx[0] = x;
	    vtx[1] = y;
	    vtx[2] = z2;
	    bg_vtx->add(vtx);
	    tex[0] = texLen*4.0;
	    tex[1] = 0.5;
	    bg_tex->add(tex);
	}
	bg = new ssgVtxTable(GL_TRIANGLE_STRIP, bg_vtx, bg_nrm, bg_tex, bg_clr);
	bg_st = (ssgSimpleState*)grSsgLoadTexState(graphic->background);
	bg_st->disable(GL_LIGHTING);
	bg->setState(bg_st);
	bg->setCullFace(0);
	TheBackground->addKid(bg);

	bg_vtx = new ssgVertexArray(NB_BG_FACES + 1);
	bg_tex = new ssgTexCoordArray(NB_BG_FACES + 1);
	bg_clr = new ssgColourArray(1);
	bg_nrm = new ssgNormalArray(1);

	bg_clr->add(clr);
	bg_nrm->add(nrm);

	for (i = 3 * NB_BG_FACES / 4; i < NB_BG_FACES + 1; i++) {
	    alpha = (float)i * 2 * PI / (float)NB_BG_FACES;
	    texLen = (float)i / (float)NB_BG_FACES;

	    x = BG_DIST * cos(alpha);
	    y = BG_DIST * sin(alpha);

	    vtx[0] = x;
	    vtx[1] = y;
	    vtx[2] = z1;
	    bg_vtx->add(vtx);
	    tex[0] = texLen*4.0;
	    tex[1] = 0.5;
	    bg_tex->add(tex);

	    vtx[0] = x;
	    vtx[1] = y;
	    vtx[2] = z2;
	    bg_vtx->add(vtx);
	    tex[0] = texLen*4.0;
	    tex[1] = 1.0;
	    bg_tex->add(tex);
	}
	bg = new ssgVtxTable(GL_TRIANGLE_STRIP, bg_vtx, bg_nrm, bg_tex, bg_clr);
	bg_st = (ssgSimpleState*)grSsgLoadTexState(graphic->background);
	bg_st->disable(GL_LIGHTING);
	bg->setState(bg_st);
	bg->setCullFace(0);
	TheBackground->addKid(bg);

	break;


    case 4:
	z1 = -1.0;
	z2 = 1.0;

	bg_vtx = new ssgVertexArray(NB_BG_FACES + 1);
	bg_tex = new ssgTexCoordArray(NB_BG_FACES + 1);
	bg_clr = new ssgColourArray(1);
	bg_nrm = new ssgNormalArray(1);

	bg_clr->add(clr);
	bg_nrm->add(nrm);

	for (i = 0; i < NB_BG_FACES + 1; i++) {
	    alpha = (double)i * 2 * PI / (double)NB_BG_FACES;
	    texLen = 1.0 - (float)i / (float)NB_BG_FACES;

	    x = BG_DIST * cos(alpha);
	    y = BG_DIST * sin(alpha);

	    vtx[0] = x;
	    vtx[1] = y;
	    vtx[2] = z1;
	    bg_vtx->add(vtx);
	    tex[0] = texLen;
	    tex[1] = 0;
	    bg_tex->add(tex);

	    vtx[0] = x;
	    vtx[1] = y;
	    vtx[2] = z2;
	    bg_vtx->add(vtx);
	    tex[0] = texLen;
	    tex[1] = 1.0;
	    bg_tex->add(tex);
	}
	bg = new ssgVtxTable(GL_TRIANGLE_STRIP, bg_vtx, bg_nrm, bg_tex, bg_clr);
	bg_st = (ssgSimpleState*)grSsgLoadTexState(graphic->background);
	bg_st->disable(GL_LIGHTING);
	bg->setState(bg_st);
	bg->setCullFace(0);
	TheBackground->addKid(bg);
	break;

    default:
	break;
    }

    /* Environment Mapping Settings */
    grEnvSelector = new ssgStateSelector(graphic->envnb);
    for (i = 0; i < graphic->envnb; i++) {
      GfOut("Loading Environment Mapping Image %s\n", graphic->env[i]);
      envst = (ssgSimpleState*)grSsgLoadTexState(graphic->env[i]);
      envst->enable(GL_BLEND);
      grEnvSelector->setStep(i, envst);
    }
    grEnvSelector->selectStep(0); /* mandatory !!! */
    grEnvState=(grMultiTexState*)grSsgEnvTexState(graphic->env[0]);
    grEnvShadowState=(grMultiTexState*)grSsgEnvTexState("envshadow.png");
    grEnvShadowStateOnCars=(grMultiTexState*)grSsgEnvTexState("shadow2.rgb");

	if (grEnvState != NULL) {
		grEnvState->ref();
	}

	if (grEnvShadowState == NULL) {
		ulSetError ( UL_WARNING, "grscene:initBackground Failed to open envshadow.png for reading") ;
		ulSetError ( UL_WARNING, "        mandatory for top env mapping ") ;
		ulSetError ( UL_WARNING, "        should be in the .xml !! ") ;
		ulSetError ( UL_WARNING, "        copy the envshadow.png from g-track-2 to the track you selected ") ;
		ulSetError ( UL_WARNING, "        c'est pas classe comme sortie, mais ca evite un crash ") ;
		GfScrShutdown();
		exit(-1);
	} else {
		grEnvShadowState->ref();
	}

	if (grEnvShadowStateOnCars == NULL) {
		// ulSetError ( UL_WARNING, "grscene:initBackground Failed to open shadow2.rgb for reading") ;
		// ulSetError ( UL_WARNING, "        no shadow mapping on cars for this track ") ;
	} else {
		grEnvShadowStateOnCars->ref();
	}
}


void grDrawBackground(class cGrCamera *cam, class cGrBackgroundCam *bgCam)
{
    TRACE_GL("grDrawBackground: ssgCullAndDraw start");

    bgCam->update(cam);
    bgCam->action();

    ssgCullAndDraw(TheBackground);

    TRACE_GL("grDrawBackground: ssgCullAndDraw");
}

void
grCustomizePits(void)
{
	tTrackPitInfo *pits;
	int i;
	tdble x, y;
	tdble x2, y2, z2;

	ThePits = new ssgBranch();
	PitsAnchor->addKid(ThePits);

	pits = &(grTrack->pits);
	/* draw the pit identification */

	switch (pits->type) {
	case TR_PIT_ON_TRACK_SIDE:
		for (i = 0; i < pits->nMaxPits; i++) {
			const int BUFSIZE=256;
			char buf[BUFSIZE];
			t3Dd normalvector;
			sgVec3 vtx;
			sgVec4 clr = {0,0,0,1};
			sgVec3 nrm;
			sgVec2 tex;
			ssgState *st;
			ssgVertexArray *pit_vtx = new ssgVertexArray(4);
			ssgTexCoordArray *pit_tex = new ssgTexCoordArray(4);
			ssgColourArray *pit_clr = new ssgColourArray(1);
			ssgNormalArray *pit_nrm = new ssgNormalArray(1);

			pit_clr->add(clr);

			if (pits->driversPits[i].car[0]) {
				// If we have more than one car in the pit use the team pit logo of driver 0.
				if (pits->driversPits[i].freeCarIndex == 1) {
					// One car assigned to the pit.
					snprintf(buf, BUFSIZE, "drivers/%s/%d;drivers/%s;data/textures;data/img;.",
						pits->driversPits[i].car[0]->_modName, pits->driversPits[i].car[0]->_driverIndex,
						pits->driversPits[i].car[0]->_modName);
				} else {
					// Multiple cars assigned to the pit.
					snprintf(buf, BUFSIZE, "drivers/%s;data/textures;data/img;.", pits->driversPits[i].car[0]->_modName);
				}
			} else {
				snprintf(buf, BUFSIZE, "data/textures;data/img;.");
			}


			st = grSsgLoadTexStateEx("logo.rgb", buf, FALSE, FALSE);
			((ssgSimpleState*)st)->setShininess(50);

			RtTrackLocal2Global(&(pits->driversPits[i].pos), &x, &y, pits->driversPits[i].pos.type);
			RtTrackSideNormalG(pits->driversPits[i].pos.seg, x, y, pits->side, &normalvector);
			x2 = x - pits->width/2.0 * normalvector.x + pits->len/2.0 * normalvector.y;
			y2 = y - pits->width/2.0 * normalvector.y - pits->len/2.0 * normalvector.x;
			z2 = RtTrackHeightG(pits->driversPits[i].pos.seg, x2, y2);

			nrm[0] = normalvector.x;
			nrm[1] = normalvector.y;
			nrm[2] = 0;
			pit_nrm->add(nrm);

			tex[0] = -0.7;
			tex[1] = 0.33;
			vtx[0] = x2;
			vtx[1] = y2;
			vtx[2] = z2;
			pit_tex->add(tex);
			pit_vtx->add(vtx);

			tex[0] = -0.7;
			tex[1] = 1.1;
			vtx[0] = x2;
			vtx[1] = y2;
			vtx[2] = z2 + 4.8;
			pit_tex->add(tex);
			pit_vtx->add(vtx);

			x2 = x - pits->width/2.0 * normalvector.x - pits->len/2.0 * normalvector.y;
			y2 = y - pits->width/2.0 * normalvector.y + pits->len/2.0 * normalvector.x;
			z2 = RtTrackHeightG(pits->driversPits[i].pos.seg, x2, y2);

			tex[0] = 1.3;
			tex[1] = 0.33;
			vtx[0] = x2;
			vtx[1] = y2;
			vtx[2] = z2;
			pit_tex->add(tex);
			pit_vtx->add(vtx);

			tex[0] = 1.3;
			tex[1] = 1.1;
			vtx[0] = x2;
			vtx[1] = y2;
			vtx[2] = z2 + 4.8;
			pit_tex->add(tex);
			pit_vtx->add(vtx);

			ssgVtxTable *pit = new ssgVtxTable(GL_TRIANGLE_STRIP, pit_vtx, pit_nrm, pit_tex, pit_clr);
			pit->setState(st);
			pit->setCullFace(0);
			ThePits->addKid(pit);
		}
	break;
	case TR_PIT_ON_SEPARATE_PATH:
	break;
	case TR_PIT_NONE:
	break;
	}
}
