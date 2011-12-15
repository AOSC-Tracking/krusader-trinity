/***************************************************************************
                                 krdialogs.h
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
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

#ifndef KCHOSEDIR_H
#define KCHOSEDIR_H

// KDE includes
#include <kdialog.h>
#include <kanimwidget.h>
#include <kurlrequesterdlg.h>
#include <kdatepicker.h>
#include <kdialogbase.h>
// QT includes
#include <tqlineedit.h>
#include <layout.h>
#include <tqlabel.h>
#include <tqpushbutton.h>
#include <tqdatetime.h>
#include <tqpixmap.h>
#include <tqprogressdialog.h>
#include <tqsemimodal.h>
#include <tqcheckbox.h>
#include <tqcombobox.h>

/** \class KChooseDir
 * Used for asking the user for a folder.
 * example:
 * \code
 * KURL u = KChooseDir::getDir("target folder", "/suggested/path", ACTIVE_PANEL->virtualPath());
 * if (u.isEmpty()) { 
 *   // user canceled (either by pressing cancel, or esc
 * } else {
 *   // do you thing here: you've got a safe url to use
 * }
 * \endcode
 */
class KChooseDir {
public:
	/**
	 * \param text - description of the info requested from the user
	 * \param url - a suggested url to appear in the box as a default choice
	 * \param cwd - a path which is the current working directory (usually ACTIVE_PANEL->virtualPath()).
	 *              this is used for completion of partial urls
	 */
	static KURL getDir(TQString text,const KURL& url, const KURL& cwd);
	static KURL getDir(TQString text,const KURL& url, const KURL& cwd, bool & preserveAttrs );
	static KURL getDir(TQString text,const KURL& url, const KURL& cwd, bool & preserveAttrs, KURL &baseURL );
};

class KURLRequesterDlgForCopy : public KDialogBase {
  Q_OBJECT
  TQ_OBJECT
public:
	KURLRequesterDlgForCopy( const TQString& url, const TQString& text, bool presAttrs,
				TQWidget *parent, const char *name, bool modal=true, KURL baseURL = KURL() );
	KURLRequesterDlgForCopy();

	KURL selectedURL() const;
	KURL baseURL() const;
	bool preserveAttrs();
	bool copyDirStructure();
        
	KURLRequester *urlRequester();
private slots:
	void slotClear();
	void slotTextChanged(const TQString &);
	void slotDirStructCBChanged();
private:
	KURLRequester *urlRequester_;
	TQComboBox *baseUrlCombo;
	TQCheckBox *preserveAttrsCB;
	TQCheckBox *copyDirStructureCB;
};

class KRGetDate : public KDialog {
  Q_OBJECT
  TQ_OBJECT
public:
  KRGetDate(TQDate date=TQDate::currentDate(), TQWidget *parent = 0, const char *name = 0);
  TQDate getDate();

private slots:
  void setDate(TQDate);

private:
  KDatePicker *dateWidget;
  TQDate chosenDate, originalDate;
};

#endif
