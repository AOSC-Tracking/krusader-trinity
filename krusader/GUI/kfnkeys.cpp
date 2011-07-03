/***************************************************************************
                                kfnkeys.cpp
                             -------------------
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

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <tqtooltip.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <tqfontmetrics.h>
#include <tqwhatsthis.h>
#include "kfnkeys.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../krslots.h"

KFnKeys::KFnKeys(TQWidget *tqparent, char *name): TQWidget(tqparent,name) {
		krConfig->setGroup("Look&Feel");
		////////////////////////////////
#define SETUP(TARGET) {\
	TARGET->setMinimumWidth(45);\
}
				
		setFont( KGlobalSettings::generalFont() );
		tqlayout=new TQGridLayout(this,1,9);	// 9 keys
    F2=new TQPushButton( i18n("F2 Term  ") ,this);
      TQToolTip::add( F2, "<qt>" + i18n( "<p>Open terminal in current directory.</p>"
                     "<p>The terminal can be defined in Konfigurator, "
                     "default is <b>konsole</b>.</p>" ) + "</qt>" );
      connect(F2,TQT_SIGNAL(clicked()), SLOTS, TQT_SLOT(terminal()));
      SETUP(F2);
			
		F3=new TQPushButton( i18n("F3 View  ") ,this);	
			TQToolTip::add( F3, i18n( "Open file in viewer." ) );
			connect(F3,TQT_SIGNAL(clicked()), SLOTS, TQT_SLOT(view()));
			SETUP(F3);
		
		F4=new TQPushButton( i18n("F4 Edit  ") ,this);
			TQToolTip::add( F4, "<qt>" + i18n( "<p>Edit file.</p>"
									   "<p>The editor can be defined in Konfigurator, "
									   "default is <b>internal editor</b>.</p>" ) + "</qt>" );
			connect(F4,TQT_SIGNAL(clicked()), SLOTS, TQT_SLOT(edit()));
			SETUP(F4);
			
		F5=new TQPushButton( i18n("F5 Copy  ") ,this);	
			TQToolTip::add( F5, i18n( "Copy file from one panel to the other." ) );
			connect(F5,TQT_SIGNAL(clicked()), SLOTS, TQT_SLOT(copyFiles()));
			SETUP(F5);
			
		F6=new TQPushButton( i18n("F6 Move") ,this);
			TQToolTip::add( F6, i18n( "Move file from one panel to the other." ) );
			connect(F6,TQT_SIGNAL(clicked()), SLOTS, TQT_SLOT(moveFiles()));
			SETUP(F6);
			
		F7=new TQPushButton( i18n("F7 Mkdir ") ,this);	
			TQToolTip::add( F7, i18n( "Create directory in current panel." ) );
			connect(F7,TQT_SIGNAL(clicked()), SLOTS, TQT_SLOT(mkdir()));
			SETUP(F7);
			
		F8=new TQPushButton( i18n("F8 Delete") ,this);
			TQToolTip::add( F8, i18n( "Delete file, directory, etc." ) );
			connect(F8,TQT_SIGNAL(clicked()), SLOTS, TQT_SLOT(deleteFiles()));
			SETUP(F8);
			
		F9=new TQPushButton( i18n("F9 Rename") ,this);	
			TQToolTip::add( F9, i18n( "Rename file, directory, etc." ) );
			connect(F9,TQT_SIGNAL(clicked()), SLOTS, TQT_SLOT(rename()));
			SETUP(F9);
			
		F10=new TQPushButton( i18n("F10 Quit ") ,this);
			TQToolTip::add( F10, i18n( "Quit Krusader." ) );
			connect(F10,TQT_SIGNAL(clicked()),krApp, TQT_SLOT(slotClose()));
			SETUP(F10);

    // set a tighter box around the keys
    int h = TQFontMetrics(F2->font()).height()+2;
    F2->setMaximumHeight(h); F3->setMaximumHeight(h);
    F4->setMaximumHeight(h); F5->setMaximumHeight(h);
    F6->setMaximumHeight(h); F7->setMaximumHeight(h);
    F8->setMaximumHeight(h); F9->setMaximumHeight(h);
    F10->setMaximumHeight(h);
    
		tqlayout->addWidget(F2,0,0);
		tqlayout->addWidget(F3,0,1);
		tqlayout->addWidget(F4,0,2);
		tqlayout->addWidget(F5,0,3);
		tqlayout->addWidget(F6,0,4);
		tqlayout->addWidget(F7,0,5);
		tqlayout->addWidget(F8,0,6);
		tqlayout->addWidget(F9,0,7);
		tqlayout->addWidget(F10,0,8);
		
		tqlayout->activate();
}

void KFnKeys::updateShortcuts() {
	F2->setText(krF2->shortcut().toString() + i18n(" Term"));
	F3->setText(krF3->shortcut().toString() + i18n(" View"));
	F4->setText(krF4->shortcut().toString() + i18n(" Edit"));
	F5->setText(krF5->shortcut().toString() + i18n(" Copy"));
	F6->setText(krF6->shortcut().toString() + i18n(" Move"));
	F7->setText(krF7->shortcut().toString() + i18n(" Mkdir"));
	F8->setText(krF8->shortcut().toString() + i18n(" Delete"));
	F9->setText(krF9->shortcut().toString() + i18n(" Rename"));
	F10->setText(krF10->shortcut().toString() + i18n(" Quit"));
}

#include "kfnkeys.moc"

