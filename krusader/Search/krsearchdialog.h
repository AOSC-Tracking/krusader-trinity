/***************************************************************************
                                 krsearchdialog.h
                             -------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
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



#ifndef KRSEARCHDIALOG_H
#define KRSEARCHDIALOG_H

#include "../Filter/filtertabs.h"
#include "../Filter/generalfilter.h"
#include "../VFS/krquery.h"
#include "../VFS/krpermhandler.h"
#include "krsearchmod.h"
#include "../GUI/profilemanager.h"

#include <tqwidget.h>
#include <ksqueezedtextlabel.h>
#include <tqstringlist.h>
#include <sys/types.h>
#include <time.h>
#include <tqstring.h>
#include <tqtabwidget.h>
#include <tqlistview.h>
#include <tqstringlist.h>
#include <kglobal.h>
#include <klocale.h>

class KrSearchDialog : public TQDialog  {
   Q_OBJECT
  
public: 
  KrSearchDialog(TQString profile = 0, TQWidget* parent = 0, const char* name = 0, bool modal = false, WFlags fl = 0 );

  void prepareGUI();
    
  static KrSearchDialog *SearchDialog;
  
public slots:
  void startSearch();
  void stopSearch();
  void feedToListBox();
  void copyToClipBoard();
  void found(TQString what, TQString where, KIO::filesize_t size, time_t mtime, TQString perm, TQString foundText);
  void closeDialog( bool isAccept = true );
  void resultDoubleClicked(TQListViewItem*);
  void resultClicked(TQListViewItem*);

  virtual void keyPressEvent(TQKeyEvent *e);
  virtual void closeEvent(TQCloseEvent *e);
  virtual void rightClickMenu(TQListViewItem*, const TQPoint&, int);
  virtual void resizeEvent( TQResizeEvent *e );

protected slots:
  void reject();

private:
  bool gui2query();
  void editCurrent();
  void viewCurrent();

private:
  ProfileManager *profileManager;

  FilterTabs * filterTabs;
  GeneralFilter * generalFilter;
      
  TQPushButton* mainHelpBtn;
  TQPushButton* mainSearchBtn;
  TQPushButton* mainStopBtn;
  TQPushButton* mainCloseBtn;
  TQPushButton* mainFeedToListBoxBtn;
  
  TQTabWidget* searcherTabs;  
  TQWidget* resultTab;
  TQGridLayout* resultLayout;
  TQLabel* foundLabel;
  KrSqueezedTextLabel *foundTextLabel;
  KSqueezedTextLabel *searchingLabel;
  
  TQListView* resultsList;

  KRQuery *query;
  KRSearchMod *searcher;
  TQStringList savedSearches;
  bool isSearching;
  bool closed;
  
  static TQString lastSearchText;
  static int     lastSearchType;
  static bool    lastSearchForCase;
  static bool    lastRemoteContentSearch;
  static bool    lastContainsWholeWord;
  static bool    lastContainsWithCase;
  static bool    lastSearchInSubDirs;
  static bool    lastSearchInArchives;
  static bool    lastFollowSymLinks;
  
  int            sizeX;
  int            sizeY;
};

class ResultListViewItem : public TQListViewItem
{
public:
  ResultListViewItem( TQListView *resultsList, TQString name, TQString where, KIO::filesize_t size, 
                      TQDateTime date, TQString perm ) : TQListViewItem( resultsList, name, where, 
                      KRpermHandler::parseSize(size), 
                      TDEGlobal::locale()->formatDateTime( date ), perm )
  {
    fileSize = size;
    fileDate = date;
    setDragEnabled( true );
  }  

  void setFoundText(TQString text) { _foundText=text; }
  const TQString& foundText() const { return _foundText; }
  
  virtual int compare(TQListViewItem *i,int col,bool ascending ) const
  {
    if( col == 2 ) {
      ResultListViewItem *other = (ResultListViewItem *)i;
      KIO::filesize_t otherSize = other->getSize();
      
      if( fileSize == otherSize )
        return 0;
      if( fileSize > otherSize )
        return 1;
      return -1;
    }
    if( col == 3 ) {
      ResultListViewItem *other = (ResultListViewItem *)i;
      TQDateTime otherDate = other->getDate();
      
      if( fileDate == otherDate )
        return 0;
      if( fileDate > otherDate )
        return 1;
      return -1;
    }
    return TQListViewItem::compare( i, col, ascending );
  }

  KIO::filesize_t getSize() {
    return fileSize;
  }

  TQDateTime getDate() {
    return fileDate;
  }
  
private:
  KIO::filesize_t fileSize;
  TQDateTime       fileDate;
  TQString _foundText;
};

#endif
