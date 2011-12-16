/***************************************************************************
                          diskusage.h  -  description
                             -------------------
    copyright            : (C) 2004 by Csaba Karai
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

#ifndef __DISK_USAGE_H__
#define __DISK_USAGE_H__

#include "../VFS/vfs.h"
#include "filelightParts/fileTree.h"

#include <tqdialog.h>
#include <tqlabel.h>
#include <tqdict.h>
#include <tqptrlist.h>
#include <tqptrdict.h>
#include <tqvaluestack.h>
#include <tqptrstack.h>
#include <kurl.h>
#include <ksqueezedtextlabel.h>
#include <tqwidgetstack.h>
#include <tqscrollview.h>
#include <tqtimer.h>

#define VIEW_LINES      0
#define VIEW_DETAILED   1
#define VIEW_FILELIGHT  2
#define VIEW_LOADER     3

typedef TQDict<void> Properties;

class DUListView;
class DULines;
class DUFilelight;
class KPopupMenu;
class LoaderWidget;

class DiskUsage : public TQWidgetStack
{
  Q_OBJECT
  TQ_OBJECT
  
public:
  DiskUsage( TQString confGroup, TQWidget *parent = 0, char *name = 0);
  ~DiskUsage();
  
  void       load( KURL dirName );
  void       close();
  void       stopLoad();
  bool       isLoading()     { return loading; }
  
  void       setView( int view );
  int        getActiveView() { return activeView; }
  
  Directory* getDirectory( TQString path );
  File *     getFile( TQString path );
  
  TQString    getConfigGroup() { return configGroup; }
  
  void *     getProperty( File *, TQString );
  void       addProperty( File *, TQString, void * );
  void       removeProperty( File *, TQString );
  
  int        exclude( File *file, bool calcPercents = true, int depth = 0 );
  void       includeAll();
  
  int        del( File *file, bool calcPercents = true, int depth = 0 );
  
  TQString    getToolTip( File * );
  
  void       rightClickMenu( File *, KPopupMenu * = 0, TQString = TQString() );
  
  void       changeDirectory( Directory *dir );
  
  Directory* getCurrentDir();
  File*      getCurrentFile();
  
  TQPixmap    getIcon( TQString mime );
  
  KURL       getBaseURL() { return baseURL; }
  
public slots:  
  void       dirUp();
  void       clear();
  
signals:
  void       enteringDirectory( Directory * );
  void       clearing();
  void       changed( File * );
  void       changeFinished();
  void       deleted( File * );
  void       deleteFinished();
  void       status( TQString );
  void       viewChanged( int );
  void       loadFinished( bool );
  void       newSearch();

protected slots:
  void       slotLoadDirectory();

protected:
  TQDict< Directory > contentMap;
  TQPtrDict<Properties> propertyMap;
    
  Directory* currentDirectory;
  KIO::filesize_t currentSize;
 
  virtual void keyPressEvent( TQKeyEvent * );
  virtual bool event( TQEvent * );
  
  int        calculateSizes( Directory *dir = 0, bool emitSig = false, int depth = 0 );
  int        calculatePercents( bool emitSig = false, Directory *dir = 0 , int depth = 0 );
  int        include( Directory *dir, int depth = 0 );
  void       createtqStatus();
  void       executeAction( int, File * = 0 );
  
  KURL       baseURL;             //< the base URL of loading

  DUListView                *listView;
  DULines                   *lineView;
  DUFilelight               *filelightView;
  LoaderWidget              *loaderView;
  
  Directory *root;
  
  int        activeView;
  
  TQString    configGroup;
  
  bool       first;
  bool       loading;
  bool       abortLoading;
  bool       clearAfterAbort;
  bool       deleting;

  TQValueStack<TQString> directoryStack;
  TQPtrStack<Directory> parentStack;

  vfs       * searchVfs;
  vfile     * currentVfile;
  Directory * currentParent;
  TQString     dirToCheck;
  
  int   fileNum; 
  int   dirNum;
  int   viewBeforeLoad;

  TQTimer loadingTimer;
};


class LoaderWidget : public TQScrollView
{
  Q_OBJECT
  TQ_OBJECT
  
public:
  LoaderWidget( TQWidget *parent = 0, const char *name = 0 );
  
  void init();
  void setCurrentURL( KURL url );
  void setValues( int fileNum, int dirNum, KIO::filesize_t total );  
  bool wasCancelled()  { return cancelled; }
  
public slots:
  void slotCancelled();
  
protected:
  virtual void resizeEvent ( TQResizeEvent *e );
  
  TQLabel *totalSize;
  TQLabel *files;
  TQLabel *directories;
  
  KSqueezedTextLabel *searchedDirectory;
  TQWidget *widget;
    
  bool   cancelled;
};

#endif /* __DISK_USAGE_GUI_H__ */
