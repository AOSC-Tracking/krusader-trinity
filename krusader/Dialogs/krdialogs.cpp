/***************************************************************************
                                krdialogs.cpp
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


// Krusader includes
#include "krdialogs.h"
// QT includes
#include <tqmessagebox.h>
#include <tqwidget.h>
#include <tqapplication.h>
#include <tqfontmetrics.h>
#include <tqtooltip.h>
// KDE includes
#include <tdelocale.h>
#include <kurlcompletion.h>
#include <tdeapplication.h>
#include <kstandarddirs.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kstandarddirs.h>
#include <tdeversion.h>
#include <tqcheckbox.h>
#include <tderecentdocument.h>
#include <tqhbox.h>
// Krusader includes
#include "../krusader.h"
#include "../resources.h"
#include "../VFS/vfs.h"
#include "../defaults.h"
#include <tqdir.h>

KURL KChooseDir::getDir(TQString text,const KURL& url, const KURL& cwd) {
	KURLRequesterDlg *dlg = new KURLRequesterDlg( vfs::pathOrURL( url, 1 ),text,krApp,"");
	dlg->urlRequester()->completionObject()->setDir(cwd.url());
	dlg->urlRequester()->setMode(KFile::LocalOnly | KFile::Directory);
	KURL u;
	if (dlg->exec() == TQDialog::Accepted) {
		u = vfs::fromPathOrURL(dlg->urlRequester()->completionObject()->replacedPath(
			dlg->urlRequester()->lineEdit()->text()));
		if (u.isRelativeURL(u.url())) {
			KURL temp = u;
			u = cwd;
			u.addPath(temp.url());
			u.cleanPath();
			if( u.protocol() == "zip" || u.protocol() == "krarc" || u.protocol() == "tar" || u.protocol() == "iso" ) {
				if( TQDir( u.path() ).exists() )
					u.setProtocol( "file" );
			}
		}
	}
	delete dlg;
	return u;
}

KURL KChooseDir::getDir(TQString text,const KURL& url, const KURL& cwd, bool &preserveAttrs ) {
	KURLRequesterDlgForCopy *dlg = new KURLRequesterDlgForCopy( vfs::pathOrURL( url, 1 ),text, preserveAttrs, krApp,"" );
	dlg->urlRequester()->completionObject()->setDir(cwd.url());
	KURL u;
	if (dlg->exec() == TQDialog::Accepted) {
		u = vfs::fromPathOrURL(dlg->urlRequester()->completionObject()->replacedPath(
			dlg->urlRequester()->lineEdit()->text()));
		if (u.isRelativeURL(u.url())) {
			KURL temp = u;
			u = cwd;
			u.addPath(temp.url());
			u.cleanPath();
			if( u.protocol() == "zip" || u.protocol() == "krarc" || u.protocol() == "tar" || u.protocol() == "iso" ) {
				if( TQDir( u.path() ).exists() )
					u.setProtocol( "file" );
			}
		}
	}
	preserveAttrs = dlg->preserveAttrs();
	delete dlg;
	return u;
}

KURL KChooseDir::getDir(TQString text,const KURL& url, const KURL& cwd, bool &preserveAttrs, KURL &baseURL ) {
	KURLRequesterDlgForCopy *dlg = new KURLRequesterDlgForCopy( vfs::pathOrURL( url, 1 ),text, preserveAttrs, krApp,"", true, baseURL );
	dlg->urlRequester()->completionObject()->setDir(cwd.url());
	KURL u;
	if (dlg->exec() == TQDialog::Accepted) {
		u = vfs::fromPathOrURL(dlg->urlRequester()->completionObject()->replacedPath(
			dlg->urlRequester()->lineEdit()->text()));
		if (u.isRelativeURL(u.url())) {
			KURL temp = u;
			u = cwd;
			u.addPath(temp.url());
			u.cleanPath();
			if( u.protocol() == "zip" || u.protocol() == "krarc" || u.protocol() == "tar" || u.protocol() == "iso" ) {
				if( TQDir( u.path() ).exists() )
					u.setProtocol( "file" );
			}
		}
		
		if( dlg->copyDirStructure() ) {
			baseURL = dlg->baseURL();
		} else {
			baseURL = KURL();
		}
	}
	preserveAttrs = dlg->preserveAttrs();
	delete dlg;
	return u;
}

KURLRequesterDlgForCopy::KURLRequesterDlgForCopy( const TQString& urlName, const TQString& _text, bool presAttrs, TQWidget *parent,
                                                  const char *name, bool modal, KURL baseURL )
			:   KDialogBase( Plain, TQString(), Ok|Cancel|User1, Ok, parent, name, modal, true, KStdGuiItem::clear() ),
			baseUrlCombo( 0 ), copyDirStructureCB( 0 ) {
	
	TQVBoxLayout * topLayout = new TQVBoxLayout( plainPage(), 0, spacingHint() );

	TQLabel * label = new TQLabel( _text, plainPage() );
	topLayout->addWidget( label );

	urlRequester_ = new KURLRequester( urlName, plainPage(), "urlRequester" );
	urlRequester_->setMinimumWidth( urlRequester_->sizeHint().width() * 3 );
	topLayout->addWidget( urlRequester_ );
	preserveAttrsCB = new TQCheckBox(i18n("Preserve attributes (only for local targets)"), plainPage());
	preserveAttrsCB->setChecked( presAttrs );
	topLayout->addWidget( preserveAttrsCB );
	if( !baseURL.isEmpty() ) {
		TQFrame *line = new TQFrame( plainPage(), "sepLine" );
		line->setFrameStyle( TQFrame::HLine | TQFrame::Sunken );
		topLayout->addWidget( line );
		copyDirStructureCB = new TQCheckBox(i18n("Keep virtual directory structure"), plainPage());
		connect( copyDirStructureCB, TQT_SIGNAL( toggled( bool ) ), this, TQT_SLOT( slotDirStructCBChanged() ) );
		copyDirStructureCB->setChecked( false );
		topLayout->addWidget( copyDirStructureCB );
		TQHBox * hbox = new TQHBox( plainPage(), "copyDirStructure" );
		new TQLabel( i18n("Base URL:"),  hbox, "baseURLLabel" );
		
		baseUrlCombo = new TQComboBox( hbox, "baseUrlRequester" );
		baseUrlCombo->setMinimumWidth( baseUrlCombo->sizeHint().width() * 3 );
		baseUrlCombo->setEnabled( copyDirStructureCB->isChecked() );
		KURL temp = baseURL, tempOld;
		do {
			TQString baseURLText = vfs::pathOrURL( temp );
			baseUrlCombo->insertItem( baseURLText );
			tempOld = temp;
			temp = temp.upURL();
		}while( !tempOld.equals( temp, true ) );
		baseUrlCombo->setCurrentItem( 0 );
		
		topLayout->addWidget( hbox );
	}
	urlRequester_->setFocus();
	connect( urlRequester_->lineEdit(), TQT_SIGNAL(textChanged(const TQString&)),
		TQT_SLOT(slotTextChanged(const TQString&)) );
	bool state = !urlName.isEmpty();
	enableButtonOK( state );
	enableButton( KDialogBase::User1, state );
	connect( this, TQT_SIGNAL( user1Clicked() ), TQT_SLOT( slotClear() ) );
}

KURLRequesterDlgForCopy::KURLRequesterDlgForCopy() {
}

bool KURLRequesterDlgForCopy::preserveAttrs() {
	return preserveAttrsCB->isChecked();
}

bool KURLRequesterDlgForCopy::copyDirStructure() {
	if( copyDirStructureCB == 0 )
		return false;
	return copyDirStructureCB->isChecked();
}

void KURLRequesterDlgForCopy::slotTextChanged(const TQString & text) {
	bool state = !text.stripWhiteSpace().isEmpty();
	enableButtonOK( state );
	enableButton( KDialogBase::User1, state );
}

void KURLRequesterDlgForCopy::slotClear() {
	urlRequester_->clear();
}

void KURLRequesterDlgForCopy::slotDirStructCBChanged() {
	baseUrlCombo->setEnabled( copyDirStructureCB->isChecked() );
}

KURL KURLRequesterDlgForCopy::selectedURL() const {
	if ( result() == TQDialog::Accepted ) {
		KURL url = KURL::fromPathOrURL( urlRequester_->url() );
		if( url.isValid() )
			TDERecentDocument::add(url);                                
		return url;
	}        
	else
		return KURL();
}

KURLRequester * KURLRequesterDlgForCopy::urlRequester() {
	return urlRequester_;
}

KURL KURLRequesterDlgForCopy::baseURL() const {
	if( baseUrlCombo == 0 )
		return KURL();
	return vfs::fromPathOrURL( baseUrlCombo->currentText() );
}

KRGetDate::KRGetDate(TQDate date, TQWidget *parent, const char *name) : KDialog(parent, name,true,WStyle_DialogBorder) {
  dateWidget = new KDatePicker(this, date);
  dateWidget->resize(dateWidget->sizeHint());
  setMinimumSize(dateWidget->sizeHint());
  setMaximumSize(dateWidget->sizeHint());
  resize(minimumSize());
  connect(dateWidget, TQT_SIGNAL(dateSelected(TQDate)), this, TQT_SLOT(setDate(TQDate)));
  connect(dateWidget, TQT_SIGNAL(dateEntered(TQDate)), this, TQT_SLOT(setDate(TQDate)));

  // keep the original date - incase ESC is pressed
  originalDate  = date;
}

TQDate KRGetDate::getDate() {
  if (exec() == TQDialog::Rejected) chosenDate.setYMD(0,0,0);
  hide();
  return chosenDate;
}

void KRGetDate::setDate(TQDate date) {
  chosenDate = date;
  accept();
}

#include "krdialogs.moc"
