/***************************************************************************
                          kcmmodebutton.cpp  -  description
                             -------------------

    this file contains a class KCMDModeButton, which represents a button with
    popup menu to switch the mode of the krusader built-in command-line

    begin                : Oct 2006
    inspired by          : other Krusader source files
    author of this file  : Vaclav Juza
    email                : vaclavjuza at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kcmdmodebutton.h"

#include "../krusader.h"
#include "../krusaderview.h"

#include <tdeactionclasses.h>
#include <tdelocale.h>
#include <kiconloader.h>
#include <tdepopupmenu.h>

#include <kdebug.h>

KCMDModeButton::KCMDModeButton( TQWidget *parent, const char *name ) : TQToolButton( parent, name ) {
  setFixedSize( 22, 20 );
/* // from the old terminal-button:
  setTextLabel( i18n( "If pressed, Krusader executes command line in a terminal." ) );
  TQWhatsThis::add( terminal, i18n(
                        "The 'run in terminal' button allows Krusader "
                        "to run console (or otherwise non-graphical) "
                        "programs in a terminal of your choice. If it's "
                        "pressed, terminal mode is active." ) );
*/
  setIconSet( SmallIcon( "konsole" ) );
  adjustSize();
  action = new TDEActionMenu( i18n("Execution mode") );
  TQ_CHECK_PTR( action );
  for( int i=0; Krusader::execTypeArray[i] != 0; i++ )
  {
    action->insert( *Krusader::execTypeArray[i] );
  }
  TQPopupMenu *pP = action->popupMenu();
  TQ_CHECK_PTR( pP );
  setPopup( pP );
  setPopupDelay( 10 );
  setAcceptDrops( false );
}

KCMDModeButton::~KCMDModeButton() {
  delete action;
}

/** called when clicked to the button */
void KCMDModeButton::openPopup() {
  TQPopupMenu * pP = popup();
  if ( pP ) {
    popup() ->exec( mapToGlobal( TQPoint( 0, 0 ) ) );
  }
}



#include "kcmdmodebutton.moc"
