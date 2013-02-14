/***************************************************************************
                                krusaderview.h
                             -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
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

#ifndef KRUSADERVIEW_H
#define KRUSADERVIEW_H

// KDE includes
#include <tdelistview.h>
#include <klocale.h>
#include <tdeaccel.h>
#include <tdeapplication.h>
#include <tdeparts/part.h>

// QT includes
#include <tqlayout.h>
#include <tqsplitter.h>
#include <tqwidget.h>
#include <tqpushbutton.h>
#include <tqhbox.h>
#include <tqvaluelist.h>

class PanelManager;
class ListPanel;

// forward declaration
class KFnKeys;
class KCMDLine;

class KrusaderView : public TQWidget  {
   Q_OBJECT
  

public:
	KrusaderView(TQWidget *parent=0);
	virtual ~KrusaderView(){}
  void start(TQStringList leftTabs, TQStringList leftTypes, TQValueList<int> leftProps, int leftActiveTab, 
             TQStringList rightTabs, TQStringList rightTypes, TQValueList<int> rightProps, int rightActiveTab,
             bool leftSideActive);
  void cmdLineFocus();  // command line receive's keyboard focus
  void cmdLineUnFocus();// return focus from command line to active panel
  inline PanelManager *activeManager() const { return (activePanel==left ? leftMng : rightMng); }
  inline PanelManager *inactiveManager() const { return (activePanel==left ? rightMng : leftMng); }
  TQValueList<int> getTerminalEmulatorSplitterSizes();
  virtual bool eventFilter ( TQObject * watched, TQEvent * e );
  /** if the KonsolePart for the Terminal Emulator is not yet loaded, load it */
  void createTE();
  
public slots:
  void slotCurrentChanged(TQString p);
	void slotSetActivePanel(ListPanel *p);
  void slotTerminalEmulator(bool);
	// manage the function keys to the CURRENT vfs
	//////////////////////////////////////////////
  // Tab - switch focus
  void panelSwitch();
  void toggleVerticalMode();
  
  void focusTerminalEmulator();
  void switchFullScreenTE();
  
  void profiles( TQString profileName = TQString() );  
  void loadPanelProfiles( TQString group );
  void savePanelProfiles( TQString group );

protected slots:
  void killTerminalEmulator();

public:
  ListPanel  *activePanel;
  ListPanel  *left,*right;								// the actual panels
  PanelManager *leftMng, *rightMng;       // saving them for panel swaps
  KFnKeys			*fnKeys;										// function keys
  KCMDLine    *cmdLine;                   // command line widget
  TQHBox       *terminal_dock;             // docking widget for terminal emulator
  KParts::ReadOnlyPart *konsole_part;     // the actual part pointer
  TQSplitter		*horiz_splitter, *vert_splitter;
  TQValueList<int> verticalSplitterSizes;

private:
  TQGridLayout *mainLayout, *terminal_layout;
};

#endif
