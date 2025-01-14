/***************************************************************************
                         dirhistorybutton.cpp  -  description
                            -------------------
   begin                : Sun Jan 4 2004
   copyright            : (C) 2004 by Shie Erlich & Rafi Yanai
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

#include "dirhistorybutton.h"
#include "dirhistoryqueue.h"

#include "../VFS/vfs.h"
#include <tqpopupmenu.h>
#include <tqdir.h>
#include <tdelocale.h>
#include <kiconloader.h>

#include <kdebug.h>

DirHistoryButton::DirHistoryButton( DirHistoryQueue* hQ, TQWidget *parent, const char *name ) : TQToolButton( parent, name ) {
	TDEIconLoader * iconLoader = new TDEIconLoader();
	TQPixmap icon = iconLoader->loadIcon( "history", TDEIcon::Toolbar, 16 );

	setFixedSize( icon.width() + 4, icon.height() + 4 );
	setPixmap( icon );
	setTextLabel( i18n( "Open the directory history list" ), true );
	setPopupDelay( 10 ); // 0.01 seconds press
	setAcceptDrops( false );

	popupMenu = new TQPopupMenu( this );
	TQ_CHECK_PTR( popupMenu );

	setPopup( popupMenu );
	popupMenu->setCheckable( true );

	historyQueue = hQ;

	connect( popupMenu, TQ_SIGNAL( aboutToShow() ), this, TQ_SLOT( slotAboutToShow() ) );
	connect( popupMenu, TQ_SIGNAL( activated( int ) ), this, TQ_SLOT( slotPopupActivated( int ) ) );
}

DirHistoryButton::~DirHistoryButton() {}

void DirHistoryButton::openPopup() {
	TQPopupMenu * pP = popup();
	if ( pP ) {
		popup() ->exec( mapToGlobal( TQPoint( 0, height() ) ) );
	}
}
/** No descriptions */
void DirHistoryButton::slotPopup() {
	//  kdDebug() << "History slot" << endl;
}
/** No descriptions */
void DirHistoryButton::slotAboutToShow() {
	//  kdDebug() << "about to show" << endl;
	popupMenu->clear();
	KURL::List::iterator it;

	int id = 0;
	for ( it = historyQueue->urlQueue.begin(); it != historyQueue->urlQueue.end(); ++it ) {
		popupMenu->insertItem( (*it).prettyURL(), id++ );
	}
	if ( id > 0 ) {
		popupMenu->setItemChecked( 0, true );
	}
}
/** No descriptions */
void DirHistoryButton::slotPopupActivated( int id ) {
	emit openUrl( historyQueue->urlQueue[ id ] );
}

#include "dirhistorybutton.moc"
