#include <string.h>

#include <yalecad/buster.h>
#include <yalecad/hash.h>
#include <yalecad/file.h>
#include <yalecad/message.h>
#include <yalecad/relpos.h>
#include <yalecad/string.h>
#include <yalecad/debug.h>

#ifndef NOGRAPHICS
#include <yalecad/draw.h>
#include <yalecad/colors.h>
#endif

/* below is what we expect to be a typical standard cell input */
/* user may change parameters if they wish. Subject to change */
#define EXPECTEDNUMNETS 10000
#define EXPECTEDNUMCELLS 10000
#define EXPECTEDSWAPGROUPS  10
#define EXPECTEDNUMPADS     10
#define E                    0

/* below is what we expect to be a large floorplanning input */
/* user may change parameters if they wish. Subject to change */
#define EXPECTEDNUMUNCON  100
#define EXPECTEDNUMPADS   10
#define EXPECTEDCORNERS   8
#define EXPECTEDINSTANCES 1
#define EXPECTEDPINGRP    1
