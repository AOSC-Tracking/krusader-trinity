/***************************************************************************
                         kglookfeel.h  -  description
                             -------------------
    copyright            : (C) 2003 by Csaba Karai
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

#ifndef __KGLOOKFEEL_H__
#define __KGLOOKFEEL_H__

#include "konfiguratorpage.h"

class TQListView;

class KgLookFeel : public KonfiguratorPage
{
  TQ_OBJECT
  

public:
  KgLookFeel( bool first, TQWidget* parent=0,  const char* name=0 );

  virtual int activeSubPage();

protected:
  KonfiguratorCheckBoxGroup *cbs;
  KonfiguratorCheckBoxGroup* quicksearchCheckboxes;
  KonfiguratorCheckBoxGroup *pnlcbs;
  KonfiguratorCheckBoxGroup *panelToolbarActive;
  KonfiguratorRadioButtons  *mouseRadio;
  KonfiguratorCheckBoxGroup *mouseCheckboxes;
  TQListView* mousePreview;

protected slots:
  void slotDisable();
  void slotEnablePanelToolbar();
  void slotSelectionModeChanged();
  
private:
  void setupOperationTab();
  void setupPanelTab();
  void setupPanelToolbarTab();
  void setupMouseModeTab();
  TQTabWidget *tabWidget;
};

#endif /* __KGLOOKFEEL_H__ */
