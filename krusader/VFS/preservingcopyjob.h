/***************************************************************************
                     preservingcopyjob.h  -  description
                             -------------------
    copyright            : (C) 2005 + by Csaba Karai
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

#ifndef __PRESERVING_COPY_JOB_H__
#define __PRESERVING_COPY_JOB_H__

#include "config.h"
#include <time.h>
#include <tdeio/jobclasses.h>
#include <tqmap.h>
#include <tqvaluelist.h>

typedef enum {
  PM_NONE          = 0,
  PM_PRESERVE_ATTR = 1,
  PM_DEFAULT       = 2
} PreserveMode;


class Attributes {
public:
	Attributes();
	Attributes( time_t tIn, uid_t uIn, gid_t gIn, mode_t modeIn, const TQString & aclIn );
	Attributes( time_t tIn, TQString user, TQString group, mode_t modeIn, const TQString & aclIn );

	time_t   time;
	uid_t    uid;
	gid_t    gid;
	mode_t   mode;
	TQString  acl;
};

class PreservingCopyJob : public TDEIO::CopyJob
{
  TQ_OBJECT
  

public:

  PreservingCopyJob( const KURL::List& src, const KURL& dest, CopyMode mode, bool asMethod, bool showProgressInfo );

  static TDEIO::CopyJob *createCopyJob( PreserveMode pmode, const KURL::List& src, const KURL& dest, CopyMode mode, bool asMethod, bool showProgressInfo );

public slots:
  void slotAboutToCreate (TDEIO::Job *, const TQValueList< TDEIO::CopyInfo > &);
  void slotCopyingDone( TDEIO::Job *, const KURL &, const KURL &, bool, bool);
  void slotFinished();
  virtual void slotResult( Job *job );
  void slotListEntries(TDEIO::Job *job, const TDEIO::UDSEntryList &list);
  
private:
  TQMap<KURL, Attributes> fileAttributes;
  TQMap<TDEIO::Job *, KURL> pendingJobs;
  TQValueList<KURL>       directoriesToStamp;
  TQValueList<KURL>       originalDirectories;
};

#endif /* __PRESERVING_COPY_JOB_H__ */
