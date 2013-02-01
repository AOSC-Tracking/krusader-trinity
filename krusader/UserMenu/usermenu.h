/***************************************************************************
                     usermenu.h  -  description
                        -------------------
begin                : Sat Dec 6 2003
copyright            : (C) 2003 by Shie Erlich & Rafi Yanai
email                :
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef USERMENU_H
#define USERMENU_H

#include <kpopupmenu.h>

class TQWidget;
class UserMenu;

class UserMenuGui: public TDEPopupMenu {
   public:
      UserMenuGui( UserMenu* menu, TQWidget *parent = 0 );
      void run();
      void createMenu();

   protected:
      void readEntries();
};

class UserMenu : public TQWidget {
   public:
      UserMenu( TQWidget *parent = 0, const char *name = 0 );
      void exec();
      void update();
   
   private:
      UserMenuGui* _popup;
};

#endif
