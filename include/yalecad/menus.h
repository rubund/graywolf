#ifndef INC_MENUS_H
#define INC_MENUS_H

#include <yalecad/base.h>

void TWdrawMenus();

void TWfreeMenuWindows();

void TWinforMenus();

void TWmouse_tracking_start();

BOOL TWmouse_tracking_end();

BOOL TWmouse_tracking_pt( INT *x, INT *y );

#endif /*INC_MENUS_H*/

