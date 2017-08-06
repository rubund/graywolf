#define _DEFAULT_SOURCE	1
#include <features.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/resource.h>

#ifdef SYS5
#include <sys/times.h>
#else /* SYS5 */
#include <sys/timeb.h>
#endif /* SYS5 */

#ifndef NOGRAPHICS
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>           /* Definition of AT_* constants */
#include <unistd.h>
#include <dirent.h>

#include <yalecad/assign.h>
#include <yalecad/base.h>
#include <yalecad/bitset.h>
#include <yalecad/buster.h>
#include <yalecad/cleanup.h>
#include <yalecad/dbinary.h>
#include <yalecad/deck.h>
#include <yalecad/dialog.h>
#include <yalecad/dset.h>
#include <yalecad/edcolors.h>
#include <yalecad/file.h>
#include <yalecad/graph.h>
#include <yalecad/grid.h>
#include <yalecad/hash.h>
#include <yalecad/heap.h>
#include <yalecad/linalg.h>
#include <yalecad/list.h>
#include <yalecad/log.h>
#include <yalecad/mac.h>
#include <yalecad/message.h>
#include <yalecad/mst.h>
#include <yalecad/mytime.h>
#include <yalecad/okmalloc.h>
#include <yalecad/program.h>
#include <yalecad/project.h>
#include <yalecad/plot.h>
#include <yalecad/queue.h>
#include <yalecad/quicksort.h>
#include <yalecad/radixsort.h>
#include <yalecad/rbtree.h>
#include <yalecad/relpos.h>
#include <yalecad/set.h>
#include <yalecad/stack.h>
#include <yalecad/stat.h>
#include <yalecad/string.h>
#include <yalecad/system.h>
#include <yalecad/tech.h>
#include <yalecad/tree.h>
#include <yalecad/time.h>
#include <yalecad/wgraphics.h>
#include <yalecad/yreadpar.h>
#include <yalecad/menus.h>
#include <yalecad/rand.h>
#include <yalecad/debug.h>

#ifndef NOGRAPHICS
#include <yalecad/draw.h>
#include <yalecad/colors.h>
#endif

#define	EXPECTEDNUMNETS		1000
#define	EXPECTEDNUMCELLS		3500

/* below is what we expect to be a large floorplanning input */
/* user may change parameters if they wish. Subject to change */
#define EXPECTEDNUMUNCON  100
#define EXPECTEDNUMPADS   10
#define EXPECTEDCORNERS   8
#define EXPECTEDINSTANCES 1
#define EXPECTEDPINGRP    1

#ifndef EXTERN
#define EXTERN extern
#endif
