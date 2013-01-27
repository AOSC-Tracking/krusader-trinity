/***************************************************************************
                                 krpermhandler.h
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


#ifndef KRPERMHANDLER_H
#define KRPERMHANDLER_H

#include <tqstring.h>
#include <tqfileinfo.h>
#include <sys/types.h>
#include <tqdict.h>
#include <tqintdict.h>
#include <tdeio/global.h>

#define NO_PERM      0
#define UNKNOWN_PERM 1
#define ALLOWED_PERM 2

class KRpermHandler {
public: 
	 KRpermHandler(){}
	~KRpermHandler(){}
	
	static void init();
	
	static gid_t group2gid(TQString group);
	static uid_t user2uid (TQString user);

	static TQString gid2group(gid_t groupId);
	static TQString uid2user (uid_t userId);

	static char writeable (TQString perm, gid_t gid, uid_t uid, int rwx=-1);
	static char readable  (TQString perm, gid_t gid, uid_t uid, int rwx=-1);
	static char executable(TQString perm, gid_t gid, uid_t uid, int rwx=-1);
	
  static bool fileWriteable (TQString localFile);
	static bool fileReadable  (TQString localFile);
	static bool fileExecutable(TQString localFile);
	
	static char ftpWriteable ( TQString fileOwner, TQString userName, TQString perm );
	static char ftpReadable ( TQString fileOwner, TQString userName, TQString perm );
	static char ftpExecutable( TQString fileOwner, TQString userName, TQString perm );

	static bool dirExist (TQString path);
	static bool fileExist(TQString fullPath);
  static bool fileExist(TQString Path, TQString name);

	static TQString mode2TQString(mode_t m);
  static TQString parseSize(TDEIO::filesize_t val);
	static TQString date2qstring(TQString date);
	static time_t  TQString2time(TQString date);

private:
	// cache for passwd and group entries
	static TQDict<uid_t> 	    *passwdCache;
  static TQDict<gid_t> 	    *groupCache;
	static TQIntDict<char>     *currentGroups;
  static TQIntDict<TQString>  *uidCache;
  static TQIntDict<TQString>  *gidCache;
};

#endif
