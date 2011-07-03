/***************************************************************************
                          paneltabbar.cpp  -  description
                             -------------------
    begin                : Sun Jun 2 2002
    copyright            : (C) 2002 by Shie Erlich & Rafi Yanai
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

#include "paneltabbar.h"
#include "Panel/listpanel.h"
#include "krusaderview.h"
#include "krslots.h"
#include "defaults.h"
#include <kaction.h>
#include <klocale.h>
#include <kshortcut.h>
#include <tqevent.h>
#include <tqwidgetstack.h>
#include <tqfontmetrics.h>
#include <tqtooltip.h>
#include <kdebug.h>

#define DISPLAY(X)	(X.isLocalFile() ? X.path() : X.prettyURL())

PanelTabBar::PanelTabBar(TQWidget *tqparent): TQTabBar(tqparent), _maxTabLength(0) {
  _panelActionMenu = new KActionMenu( i18n("Panel"), TQT_TQOBJECT(this) );

  setAcceptDrops(true);  
  insertAction(krNewTab);
  insertAction(krDupTab);
  insertAction(krPreviousTab);
  insertAction(krNextTab);
  insertAction(krCloseTab);
  krCloseTab->setEnabled(false); //can't close a single tab

  setShape(TQTabBar::TriangularBelow);
}

void PanelTabBar::mousePressEvent( TQMouseEvent* e ) {
  TQTab* clickedTab = selectTab( e->pos() );
  if( !clickedTab ) { // clicked on nothing ...
    TQTabBar::mousePressEvent(e);
    return;
  }
  // else implied
  setCurrentTab( clickedTab );
  emit changePanel(dynamic_cast<PanelTab*>(clickedTab)->panel);

  if ( e->button() == Qt::RightButton ) {
    // show the popup menu
    _panelActionMenu->popup( e->globalPos() );
  } else
  if ( e->button() == Qt::LeftButton ) { // we need to change tabs
    // first, find the correct panel to load
    int id = currentTab();
    ListPanel *listpanel = dynamic_cast<PanelTab*>(tab(id))->panel;
    emit changePanel(listpanel);
  } else
  if (e->button() == Qt::MidButton) { // close the current tab
    emit closeCurrentTab();
  }
  TQTabBar::mousePressEvent(e);
}

void PanelTabBar::insertAction( KAction* action ) {
  _panelActionMenu->insert( action );
}

int PanelTabBar::addPanel(ListPanel *panel, bool setCurrent ) {
  int newId = addTab(new PanelTab(squeeze(DISPLAY(panel->virtualPath())), panel));

  // make sure all tabs lengths are correct
  for (int i=0; i<count(); i++)
    tabAt(i)->setText(squeeze(DISPLAY(dynamic_cast<PanelTab*>(tabAt(i))->panel->virtualPath()), i));
  tqlayoutTabs();
  
  if( setCurrent )
    setCurrentTab(newId);

  // enable close-tab action
  if (count()>1) {
    krCloseTab->setEnabled(true);
  }

  connect(dynamic_cast<PanelTab*>(tab(newId))->panel, TQT_SIGNAL(pathChanged(ListPanel*)),
          this, TQT_SLOT(updateTab(ListPanel*)));

  return newId;
}

ListPanel* PanelTabBar::removeCurrentPanel(ListPanel* &panelToDelete) {
  int id = currentTab();
  ListPanel *oldp = dynamic_cast<PanelTab*>(tab(id))->panel; // old panel to kill later
  disconnect(dynamic_cast<PanelTab*>(tab(id))->panel);
  removeTab(tab(id));

  for (int i=0; i<count(); i++)
    tabAt(i)->setText(squeeze(DISPLAY(dynamic_cast<PanelTab*>(tabAt(i))->panel->virtualPath()), i));
  tqlayoutTabs();

  // setup current one
  id = currentTab();
  ListPanel *p = dynamic_cast<PanelTab*>(tab(id))->panel;
  // disable close action?
  if (count()==1) {
    krCloseTab->setEnabled(false);
  }

  panelToDelete = oldp;
  return p;
}

void PanelTabBar::updateTab(ListPanel *panel) {
  // find which is the correct tab
  for (int i=0; i<count(); i++) {
    if (dynamic_cast<PanelTab*>(tabAt(i))->panel == panel) {
      tabAt(i)->setText(squeeze(DISPLAY(panel->virtualPath()),i));
      break;
    }
  }
}

void PanelTabBar::duplicateTab() {
  int id = currentTab();
  emit newTab(dynamic_cast<PanelTab*>(tab(id))->panel->virtualPath());
}

void PanelTabBar::closeTab() {
  emit closeCurrentTab();
}

TQString PanelTabBar::squeeze(TQString text, int index) {
  TQString originalText = text;
  
  TQString lastGroup = krConfig->group();  
  krConfig->setGroup( "Look&Feel" );
  bool longNames = krConfig->readBoolEntry( "Fullpath Tab Names", _FullPathTabNames );
  krConfig->setGroup( lastGroup );
  
  if( !longNames )
  {
    while( text.endsWith( "/" ) )
      text.truncate( text.length() -1 );      
    if( text.isEmpty() || text.endsWith(":") )
      text += "/";
    else
    {
      TQString shortName;
                    
      if( text.tqcontains( ":/" ) )
        shortName = text.left( text.tqfind( ":/" ) ) + ":";
    
      shortName += text.mid( text.tqfindRev( "/" ) + 1 );      
      text = shortName;
    }
    
    if( index >= 0 )
      setToolTip( index, originalText );
    
    index = -1;
  }
  
  TQFontMetrics fm(fontMetrics());

  // set the real max length
  _maxTabLength = (TQT_TQWIDGET(tqparent())->width()-(6*fm.width("W")))/fm.width("W");
  // each tab gets a fair share of the max tab length
  int _effectiveTabLength = _maxTabLength / (count() == 0 ? 1 : count());

  int labelWidth = fm.width("W")*_effectiveTabLength;
  int textWidth = fm.width(text);
  if (textWidth > labelWidth) {
    // start with the dots only
    TQString squeezedText = "...";
    int squeezedWidth = fm.width(squeezedText);

    // estimate how many letters we can add to the dots on both sides
    int letters = text.length() * (labelWidth - squeezedWidth) / textWidth / 2;
    if (labelWidth < squeezedWidth) letters=1;
    squeezedText = text.left(letters) + "..." + text.right(letters);
    squeezedWidth = fm.width(squeezedText);

    if (squeezedWidth < labelWidth) {
        // we estimated too short
        // add letters while text < label
        do {
                letters++;
                squeezedText = text.left(letters) + "..." + text.right(letters);
                squeezedWidth = fm.width(squeezedText);
        } while (squeezedWidth < labelWidth);
        letters--;
        squeezedText = text.left(letters) + "..." + text.right(letters);
    } else if (squeezedWidth > labelWidth) {
        // we estimated too long
        // remove letters while text > label
        do {
            letters--;
            squeezedText = text.left(letters) + "..." + text.right(letters);
            squeezedWidth = fm.width(squeezedText);
        } while (letters && squeezedWidth > labelWidth);
    }

    if( index >= 0 )
      setToolTip( index, originalText );

    if (letters < 5) {
    	// too few letters added -> we give up squeezing
      //return text;
    	return squeezedText;
    } else {
	    return squeezedText;
    }
  } else {
    if( index >= 0 )
      removeToolTip( index );
      
    return text;
  };
}

void PanelTabBar::resizeEvent ( TQResizeEvent *e ) {
    TQTabBar::resizeEvent( e );
     
    for (int i=0; i<count(); i++)
      tabAt(i)->setText(squeeze(DISPLAY(dynamic_cast<PanelTab*>(tabAt(i))->panel->virtualPath()), i));
    tqlayoutTabs();
}


void PanelTabBar::dragEnterEvent(TQDragEnterEvent *e) {
	TQTab *t = selectTab(e->pos());
	if (!t) return;
	if (tab(currentTab()) != t) {
		setCurrentTab(t);
		emit changePanel(dynamic_cast<PanelTab*>(t)->panel);
	}
}

void PanelTabBar::dragMoveEvent(TQDragMoveEvent *e) {
	TQTab *t = selectTab(e->pos());
	if (!t) return;
	if (tab(currentTab()) != t) {
		setCurrentTab(t);
		emit changePanel(dynamic_cast<PanelTab*>(t)->panel);
	}
}

// -----------------------------> PanelTab <----------------------------

PanelTab::PanelTab(const TQString & text): TQTab(text) {}
PanelTab::PanelTab(const TQString & text, ListPanel *p): TQTab(text), panel(p) {}

#include "paneltabbar.moc"
