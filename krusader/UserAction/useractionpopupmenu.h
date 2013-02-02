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

#include <tdepopupmenu.h>

class KURL;

class UserActionPopupMenu : public TDEPopupMenu {
public:
   UserActionPopupMenu( KURL currentURL, TQWidget *parent = 0 );
};

#endif // ifndef USERACTIONPOPUPMENU_H
