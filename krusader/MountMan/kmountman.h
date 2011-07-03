/***************************************************************************
                             kmountman.h
                          -------------------
 begin                : Thu May 4 2000
 copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
 e-mail               : krusader@users.sourceforge.net
 web site             : http://krusader.sourceforge.net
---------------------------------------------------------------------------
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
#ifndef KMOUNTMAN_H
#define KMOUNTMAN_H

// QT includes
#include <tqobject.h>
#include <tqstring.h>

// KDE includes
#include <kdeversion.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kio/global.h>
#include <kmountpoint.h>

// krusader includes
#include <stdlib.h>
#include <math.h>

class KMountManGUI;

class KMountMan : public TQObject {
   Q_OBJECT
  TQ_OBJECT
   friend class KMountManGUI;

public:
   enum mnttqStatus {DOESNT_EXIST, NOT_MOUNTED, MOUNTED};

   inline bool operational() {
      return Operational;
   } // check this 1st
   
	void mainWindow();                        // opens up the GUI
   void mount( TQString mntPoint, bool blocking=true ); // this is probably what you need for mount
   void unmount( TQString mntPoint, bool blocking=true ); // this is probably what you need for unmount
   mnttqStatus gettqStatus( TQString mntPoint );  // return the status of a mntPoint (if any)
   void autoMount( TQString path );           // just call it before refreshing into a dir
   static void eject( TQString mntPoint );
   bool ejectable( TQString path );
   TQString convertSize( KIO::filesize_t size );
	bool invalidFilesystem(TQString type);
	bool nonmountFilesystem(TQString type, TQString mntPoint);

   KMountMan();
   ~KMountMan();

public slots:
   void delayedPerformAction( int idx );
   void performAction( int idx );
   void quickList();

protected slots:
	void jobResult(KIO::Job *job);
	
protected:
	// used internally
	static KMountPoint *findInListByMntPoint(KMountPoint::List &lst, TQString value); 
   void toggleMount( TQString mntPoint ); 
		
private:
   TQString *_actions;

private:
   bool Operational;   // if false, something went terribly wrong on startup
	bool waiting; // used to block krusader while waiting for (un)mount operation
   KMountManGUI *mountManGui;
	// the following is the FS type
	TQStringList invalid_fs;
	TQStringList nonmount_fs;
	// the following is the FS name
		TQStringList nonmount_fs_mntpoint;
};

#endif
