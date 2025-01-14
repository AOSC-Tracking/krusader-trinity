/***************************************************************************
                         KgProtocols.h  -  description
                             -------------------
    copyright            : (C) 2004 by Csaba Karai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __KgProtocols_H__
#define __KgProtocols_H__

#include "konfiguratorpage.h"
#include <tqlistbox.h>
#include <tqpushbutton.h>
#include <tqlistview.h>
 
class KgProtocols : public KonfiguratorPage
{
  TQ_OBJECT
  

public:
  KgProtocols( bool first, TQWidget* parent=0,  const char* name=0 );

  virtual void loadInitialValues();
  virtual void setDefaults();
  virtual bool apply();
  virtual bool isChanged();
  
  static  void init();
  
public slots:
  void         slotDisableButtons();  
  void         slotAddProtocol();
  void         slotRemoveProtocol();
  void         slotAddMime();
  void         slotRemoveMime();
    
protected:
  void         loadListCapableProtocols();
  void         loadMimes();
  TQWidget*     addSpacer( TQWidget *parent, const char *widgetName = 0);
  
  void         addProtocol( TQString name, bool changeCurrent = false );
  void         removeProtocol( TQString name );
  void         addMime( TQString name, TQString protocol );
  void         removeMime( TQString name );
  
  TQListView   *linkList;
  
  TQListBox    *protocolList;
  TQListBox    *mimeList;
  
  TQPushButton *btnAddProtocol;
  TQPushButton *btnRemoveProtocol;
  TQPushButton *btnAddMime;
  TQPushButton *btnRemoveMime;
  
  static TQString defaultProtocols;
  static TQString defaultIsoMimes;
  static TQString defaultKrarcMimes;
  static TQString defaultTarMimes;
};

#endif /* __KgProtocols_H__ */
