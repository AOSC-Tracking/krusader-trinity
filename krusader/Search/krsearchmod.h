/***************************************************************************
                                 krsearchmod.h
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



#ifndef KRSEARCHMOD_H
#define KRSEARCHMOD_H

#include <tqobject.h>
#include <tqvaluestack.h>
#include <tqstringlist.h>
#include <tqdatetime.h>
#include <time.h>
#include <tdeio/global.h>
#include <kurl.h>
#include "../VFS/ftp_vfs.h"
#include "../VFS/virt_vfs.h"


class KRQuery;
class ftp_vfs;

class KRSearchMod : public TQObject  {
  TQ_OBJECT
  
public: 
  KRSearchMod(const KRQuery *q);
  ~KRSearchMod();

  void scanURL( KURL url );
  void start();
  void stop();
  
private:
  void scanLocalDir( KURL url );
  void scanRemoteDir( KURL url );

signals:
  void finished();
  void searching(const TQString&);
  void found(TQString what, TQString where, TDEIO::filesize_t size, time_t mtime, TQString perm, TQString textFound);

private slots:
  void slotProcessEvents( bool & stopped );

private:
  bool stopSearch;
  TQValueStack<KURL> scannedUrls;
  TQValueStack<KURL> unScannedUrls;
  KRQuery *query;
  TQStringList results;
  
  ftp_vfs *remote_vfs;
  virt_vfs *virtual_vfs;
  
  TQTime timer;
};

#endif
