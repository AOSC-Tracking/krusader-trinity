/***************************************************************************
                               kmountmangui.h
                            -------------------
   begin                : Thu May 4 2000
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

                                                    H e a d e r    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#ifndef KMOUNTMANGUI_H
#define KMOUNTMANGUI_H

#include <sys/param.h>
#include <kdialogbase.h>
#include <tqlistview.h>
#include <kjanuswidget.h>
#include <tqtimer.h>
#include <kurl.h>
#include <tqvaluelist.h>
#include <kmountpoint.h>
#include <tqdatetime.h>
#include "kmountman.h"

#define  WATCHER_DELAY    3000

class KRFSDisplay;
class KRdirWatch;


// forward definitions
class fsData;

class KMountManGUI : public KDialogBase {
   TQ_OBJECT
  

   enum Pages {
      Filesystems = 0
   };

public:
   KMountManGUI();
   ~KMountManGUI();
   void createLayout();   // calls the various tab layout-creators
   void createMainPage(); // creator of the main page - filesystems

protected slots:
   void doubleClicked( TQListViewItem * );
   void clicked( TQListViewItem *, const TQPoint& pos, int col );
   void changeActive( TQListViewItem * );
   void checkMountChange(); // check whether the mount table was changed

	void updateList();     // fill-up the filesystems list
	void getSpaceData();
	void gettingSpaceData();
	void gettingSpaceData(const TQString &mountPoint, unsigned long kBSize, 
								unsigned long kBUsed, unsigned long kBAvail);

protected:
	void addItemToMountList( TQListView *lst, fsData &fs );
		
signals:
   void refreshPanel( const KURL & );
	void finishedGettingSpaceData();

private:
   KRFSDisplay *info;
   TQFrame *mainPage;
   KJanusWidget *widget;
   TQListView *mountList;
   TQTimer *watcher;
   TQDateTime lastMtab;
	// used for the getSpace - gotSpace functions
	KMountPoint::List possible, mounted;
	TQValueList<fsData> fileSystems;
	TQValueList<fsData> fileSystemsTemp;  // first collect to a temporary place
	int numOfMountPoints;
};


// Data container for a single-filesystem data
// maximum size supported is 2GB of 1kb blocks == 2048GB, enough.
// not really needed, but kept for backward compatiblity
/////////////////////////////////////////////////////////////////
class fsData {
public:
   fsData() : Name( 0 ), Type( 0 ), MntPoint( 0 ), TotalBlks( 0 ),
   FreeBlks( 0 ), Mounted( false ) {}

   // get information
   inline TQString name() {
      return Name;
   }
   inline TQString shortName() {
      return Name.right( Name.length() - Name.find( "/", 1 ) - 1 );
   }
   inline TQString type() {
      return Type;
   }
   inline TQString mntPoint() {
      return MntPoint;
   }
   inline long totalBlks() {
      return TotalBlks;
   }
   inline long freeBlks() {
      return FreeBlks;
   }
   inline TDEIO::filesize_t totalBytes() {
      return TotalBlks * 1024;
   }
   inline TDEIO::filesize_t freeBytes() {
      return FreeBlks * 1024;
   }
   //////////////////// insert a good round function here /////////////////
   int usedPerct() {
      if ( TotalBlks == 0 )
         return 0;
      float res = ( ( float ) ( TotalBlks - FreeBlks ) ) / ( ( float ) TotalBlks ) * 100;
      if ( ( res - ( int ) res ) > 0.5 )
         return ( int ) res + 1;
      else
         return ( int ) res;
   }
   inline bool mounted() {
      return Mounted;
   }

   // set information
   inline void setName( TQString n_ ) {
      Name = n_;
   }
   inline void setType( TQString t_ ) {
      Type = t_;
   }
   inline void setMntPoint( TQString m_ ) {
      MntPoint = m_;
   }
   inline void setTotalBlks( long t_ ) {
      TotalBlks = t_;
   }
   inline void setFreeBlks( long f_ ) {
      FreeBlks = f_;
   }
   inline void setMounted( bool m_ ) {
      Mounted = m_;
   }

private:
   TQString Name;       // i.e: /dev/cdrom
   TQString Type;       // i.e: iso9600
   TQString MntPoint;   // i.e: /mnt/cdrom
   long TotalBlks;  // measured in 1024bytes per block
   long FreeBlks;
   bool Mounted;    // true if filesystem is mounted

   // additional attributes of a filesystem, parsed from fstab
public:
   TQString options;    // additional fstab options
};

class KrMountDetector {
#if defined(BSD)
   TQString checksum;
#else
   TQDateTime lastMtab;
#endif
public:
  KrMountDetector();
  static KrMountDetector * getInstance();
  bool hasMountsChanged();
};



#endif
