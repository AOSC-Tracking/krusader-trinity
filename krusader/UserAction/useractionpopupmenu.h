//
// C++ Interface: UserActionPopupMenu
//
// Description: 
//
//
// Author: Jonas Bähr, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef USERACTIONPOPUPMENU_H
#define USERACTIONPOPUPMENU_H

#include <kpopupmenu.h>

class KURL;

class UserActionPopupMenu : public KPopupMenu {
public:
   UserActionPopupMenu( KURL currentURL, TQWidget *tqparent = 0 );
};

#endif // ifndef USERACTIONPOPUPMENU_H
