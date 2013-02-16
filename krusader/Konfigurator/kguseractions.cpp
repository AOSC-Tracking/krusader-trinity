/***************************************************************************
                        kguseractions.cpp  -  description
                             -------------------
    copyright            : (C) 2004 by Jonas B�r
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

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kguseractions.h"
#include "../defaults.h"
#include "../krusader.h"
#include "../ActionMan/actionman.h"

#include <tdelocale.h>
#include <kpushbutton.h>
#include <kdebug.h>


KgUserActions::KgUserActions( bool first, TQWidget* parent,  const char* name ) :
  KonfiguratorPage( first, parent, name )
{
   TQGridLayout *kgUserActionLayout = new TQGridLayout( parent, 2, 1,
   		0 /* margin */, 6 /* spacing */, "kgUserActionLayout" );

   // ============= Info Group =============
   TQGroupBox *InfoGroup = createFrame( i18n( "Information" ), parent, "kgUserActionInfoGroup" );
   TQGridLayout *InfoGrid = createGridLayout( InfoGroup->layout() );

   // terminal for the UserActions
   TQLabel *labelInfo = new TQLabel( i18n(
   		"Here you can configure settings about useractions.\n"
   		"To set up, configure and manage your useractions please use ActionMan."
   		), InfoGroup, "InformationLabel" );
   InfoGrid->addWidget( labelInfo, 0, 0 );
   KPushButton *actionmanButton = new KPushButton( i18n("Start ActionMan"), InfoGroup, "actionmanButton");
   connect( actionmanButton, TQT_SIGNAL( clicked() ), TQT_SLOT( startActionMan() ) );
   InfoGrid->addWidget( actionmanButton, 1, 0 );

   kgUserActionLayout->addWidget( InfoGroup, 0 ,0 );

   // ============= Terminal Group =============
   TQGroupBox *terminalGroup = createFrame( i18n( "Terminal execution" ), parent, "kgUserActionTerminalGroup" );
   TQGridLayout *terminalGrid = createGridLayout( terminalGroup->layout() );

   // terminal for the UserActions
   TQLabel *labelTerminal = new TQLabel( i18n( "Terminal for UserActions:" ),
   		terminalGroup, "TerminalLabel" );
   terminalGrid->addWidget( labelTerminal, 0, 0 );
   KonfiguratorURLRequester *urlReqUserActions = createURLRequester( "UserActions",
   		"Terminal", _UserActions_Terminal, terminalGroup, false );
   terminalGrid->addWidget( urlReqUserActions, 0, 1 );

   kgUserActionLayout->addWidget( terminalGroup, 1 ,0 );

   // ============= Outputcollection Group =============
   TQGroupBox *outputGroup = createFrame( i18n( "Output collection" ), parent, "kgUserActionOutputGroup" );
   TQGridLayout *outputGrid = createGridLayout( outputGroup->layout() );

   TQHBox *hbox;
   hbox = new TQHBox( outputGroup, "HBoxNormalFont" );
   new TQLabel( i18n( "Normal font:" ), hbox, "NormalFontLabel" );
   createFontChooser( "UserActions", "Normal Font", _UserActions_NormalFont, hbox );
   createSpacer ( hbox );
   outputGrid->addWidget( hbox, 2, 0 );

   hbox = new TQHBox( outputGroup, "HBoxFixedFont" );
   new TQLabel( i18n( "Font with fixed width:" ), hbox, "FixedFontLabel" );
   createFontChooser( "UserActions", "Fixed Font", _UserActions_FixedFont, hbox );
   createSpacer ( hbox );
   outputGrid->addWidget( hbox, 3, 0 );

   KonfiguratorCheckBox *useFixed = createCheckBox( "UserActions", "Use Fixed Font", _UserActions_UseFixedFont,
   		i18n("Use fixed width font as default"), outputGroup );
   outputGrid->addWidget( useFixed, 4, 0 );

   kgUserActionLayout->addWidget( outputGroup, 2 ,0 );
}

void KgUserActions::startActionMan() {
   ActionMan actionMan( TQT_TQWIDGET(parent()) );
}


#include "kguseractions.moc"
