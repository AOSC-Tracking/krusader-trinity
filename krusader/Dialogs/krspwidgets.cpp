/***************************************************************************
                                krspwidgets.cpp
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

#include "krspwidgets.h"
#include "../krusader.h"
#include "../krusaderview.h"
#include "../Panel/listpanel.h"
#include "../kicons.h"
#include "../Filter/filtertabs.h"
#include <tqcombobox.h>
#include <tqlabel.h>
#include <tqlineedit.h>
#include <tqcheckbox.h>
#include <tqlistbox.h>
#include <tqspinbox.h>
#include <klocale.h>
#include <kcombobox.h>
#include <kiconloader.h>
#include <kcursor.h>
#include <tqbitmap.h>
#include "../resources.h"

///////////////////// initiation of the static members ////////////////////////
TQStrList KRSpWidgets::tqmaskList;

///////////////////////////////////////////////////////////////////////////////

KRSpWidgets::KRSpWidgets(){
}

KRQuery KRSpWidgets::getMask(TQString caption, bool nameOnly ) {
  if( !nameOnly ) {
    return FilterTabs::getQuery();
  }
  else {
    KRMaskChoiceSub *p=new KRMaskChoiceSub();
    p->setCaption(caption);
    p->exec();
    if (p->selection->currentText()=="") return KRQuery();
    else return KRQuery( p->selection->currentText() );
  }
}

/////////////////////////// newFTP ////////////////////////////////////////
KURL KRSpWidgets::newFTP() {
	newFTPSub *p=new newFTPSub();
	p->exec();
	if (p->url->currentText()=="") return KURL(); // empty url
	KURL url;
	
	TQString protocol = p->prefix->currentText();
	protocol.truncate(protocol.length() - 3); // remove the trailing ://
	TQString username = p->username->text().simplifyWhiteSpace();
	TQString password = p->password->text().simplifyWhiteSpace();
	TQString uri = p->url->currentText();

	int uriStart = uri.tqfindRev( '@' ); /* lets the user enter user and password in the URI field */
	if( uriStart != -1 ) {
		TQString uriUser = uri.left( uriStart );
		TQString uriPsw = TQString();
		uri = uri.mid( uriStart + 1 );

		int pswStart = uriUser.tqfind( ':' ); /* getting the password name from the URL */
		if( pswStart != -1 ) {
			uriPsw = uriUser.mid( pswStart + 1 );
			uriUser = uriUser.left( pswStart );
		}

		if( !uriUser.isEmpty() )           /* handling the ftp proxy username and password also */
			username = username.isEmpty() ? uriUser : username + "@" + uriUser;

		if( !uriPsw.isEmpty() )           /* handling the ftp proxy username and password also */
			password = password.isEmpty() ? uriPsw : password + "@" + uriPsw;
	}

	TQString host = uri;               /* separating the hostname and path from the uri */
	TQString path = TQString();
	int pathStart = uri.tqfind( "/" );
	if( pathStart != -1 ) {
		path = host.mid( pathStart );
		host = host.left( pathStart );
	}

	/* setting the parameters of the URL */
	url.setProtocol(protocol);
	url.setHost( host );
	url.setPath( path );
	if ( protocol == "ftp" || protocol == "fish" || protocol == "sftp" )
		url.setPort(p->port->cleanText().toInt());
	if (!username.isEmpty())
		url.setUser( username );
	if (!password.isEmpty())
		url.setPass( password );
  
  return url;
}

newFTPSub::newFTPSub() : newFTPGUI(0,0,true) {
  url->setFocus();
  setGeometry(krApp->x()+krApp->width()/2-width()/2,krApp->y()+krApp->height()/2-height()/2,width(),height());
}

void newFTPSub::accept() {
  url->addToHistory( url->currentText() );
  // save the history and completion list when the history combo is
  // destroyed
  krConfig->setGroup("Private");
  TQStringList list = url->completionObject()->items();
  krConfig->writeEntry( "newFTP Completion list", list );
  list = url->historyItems();
  krConfig->writeEntry( "newFTP History list", list );

  newFTPGUI::accept();
}

void newFTPSub::reject() {
  url->setCurrentText("");
  newFTPGUI::reject();
}

/////////////////////////// KRMaskChoiceSub ///////////////////////////////
KRMaskChoiceSub::KRMaskChoiceSub() : KRMaskChoice(0,0,true) {
  PixmapLabel1->setPixmap(krLoader->loadIcon("kr_select", KIcon::Desktop, 32));
  label->setText(i18n("Enter a selection:"));
  // the predefined selections list
  krConfig->setGroup("Private");
  TQStrList lst;
  int i=krConfig->readListEntry("Predefined Selections",lst);
  if (i>0) preSelections->insertStrList(lst);
  // the combo-box tweaks
  selection->setDuplicatesEnabled(false);
  selection->insertStrList(KRSpWidgets::tqmaskList);
  selection->lineEdit()->setText("*");
  selection->lineEdit()->selectAll();
  selection->setFocus();
}

void KRMaskChoiceSub::reject() {
  selection->clear();
  KRMaskChoice::reject();
}

void KRMaskChoiceSub::accept() {
  bool add = true;
  char *tmp;
  // make sure we don't have that already
  for ( tmp = KRSpWidgets::tqmaskList.first(); tmp ; tmp = KRSpWidgets::tqmaskList.next() )
    if (TQString(tmp).simplifyWhiteSpace() == selection->currentText().simplifyWhiteSpace()) {
      // break if we found one such as this
      add = false;
      break;
    }

  if (add)
    KRSpWidgets::tqmaskList.insert(0,selection->currentText().local8Bit());
  // write down the predefined selections list
  TQStrList list;
  TQListBoxItem *i=preSelections->firstItem();
  while (i!=0) {
    if (i->text().tqfind(i18n("compare mode"))==-1)
      list.append(i->text().local8Bit());
    i=i->next();
  }
  krConfig->setGroup("Private");
  krConfig->writeEntry("Predefined Selections",list);
  KRMaskChoice::accept();
}

void KRMaskChoiceSub::addSelection() {
  TQString temp=selection->currentText();
  bool itemExists=false;
  TQListBoxItem *i=preSelections->firstItem();
  // check if the selection already exists
  while (i!=0)
    if (i->text()==temp) {
      itemExists=true;
      break;
    } else i=i->next();
  if (temp!="" && !itemExists) {
    preSelections->insertItem(selection->currentText());
    preSelections->update();
  }
}

void KRMaskChoiceSub::deleteSelection() {
  if (preSelections->currentItem()!=-1 &&
      preSelections->currentText().tqfind(i18n("compare mode"))==-1) {
    preSelections->removeItem(preSelections->currentItem());
    preSelections->update();
  }
}

void KRMaskChoiceSub::clearSelections() {
  preSelections->clear();
  preSelections->update();
}

void KRMaskChoiceSub::acceptFromList(TQListBoxItem *i) {
  selection->insertItem(i->text(),0);
  accept();
}

////////////////////////// QuickNavLineEdit ////////////////////

QuickNavLineEdit::QuickNavLineEdit(const TQString &string, TQWidget *tqparent, const char *name):
	KLineEdit(string, tqparent, name) { init(); }
	
QuickNavLineEdit::QuickNavLineEdit(TQWidget *tqparent, const char *name): 
	KLineEdit(tqparent, name) { init(); }

int QuickNavLineEdit::findCharFromPos(const TQString & str, const TQFontMetrics & metrics, int pos)
{
	if (pos < 0)
		return -1;
	for (int i = 1; i <= (int)str.length(); ++i)
		if (metrics.width(str, i) > pos)
			return i;
	return str.length();
}

void QuickNavLineEdit::init() {
	_numOfSelectedChars=0;
	_dummyDisplayed=false;
	_pop=0;
	//setCompletionMode( KGlobalSettings::CompletionPopupAuto );  ==> removed by public demand
}

void QuickNavLineEdit::leaveEvent(TQEvent *) {
	clearAll();
}

void QuickNavLineEdit::mousePressEvent( TQMouseEvent *m ) {
	if (m->state()!=ControlButton) clearAll();
	else
	{
		if (!_numOfSelectedChars)
		{
			_numOfSelectedChars = charCount(m);
			if (_numOfSelectedChars < 0)
				_numOfSelectedChars = 0;
		}
		if (_numOfSelectedChars)
			emit returnPressed(text().left(_numOfSelectedChars));
	}
	KLineEdit::mousePressEvent(m);
}

int QuickNavLineEdit::charCount(const TQMouseEvent * const m,TQString * const str) {
	// find how much of the string we've selected (approx) 
	// and select from from the start to the closet slash (on the right)
	const TQString tx = text().simplifyWhiteSpace();
	if (tx.isEmpty()) {
		clearAll();
    return -1;
	}
	
	int numOfChars = findCharFromPos(tx, fontMetrics(), m->x() - 5);
	if(str) *str=tx;
	return tx.tqfind('/', numOfChars);
}

void QuickNavLineEdit::mouseMoveEvent( TQMouseEvent *m) {
	if (m->state()!=ControlButton) { // works only with ctrl pressed
		clearAll();
		KLineEdit::mouseMoveEvent(m);
		return;
	}
  TQString tx;
  int idx=charCount(m,&tx);
		if (idx == -1 && !_dummyDisplayed) { // pointing on or after the current directory
			if (_pop) delete _pop;
			_pop = KPassivePopup::message( i18n("Quick Navigation"),
				"<qt>" + i18n("Already at <i>%1</i>").tqarg(tx.left(idx)) + "</qt>",
				*(KCursor::handCursor().bitmap()), this);

			_dummyDisplayed=true;
			_numOfSelectedChars=0;
		} else if (idx>0 && idx!=_numOfSelectedChars) {
			_numOfSelectedChars=idx;
			if (_pop) delete _pop;
			_dummyDisplayed=false;

			_pop = KPassivePopup::message( i18n("Quick Navigation"),
				"<qt>" + i18n("Click to go to <i>%1</i>").tqarg(tx.left(idx)) + "</qt>",
				*(KCursor::handCursor().bitmap()), this );
		}
	KLineEdit::mouseMoveEvent(m);
}

