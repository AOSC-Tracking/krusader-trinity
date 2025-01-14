/***************************************************************************
                       virtualcopyjob.h  -  description
                             -------------------
    copyright            : (C) 2006 + by Csaba Karai
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

#ifndef __VIRTUAL_COPY_JOB_H__
#define __VIRTUAL_COPY_JOB_H__

#include <tdeio/job.h>
#include "preservingcopyjob.h"
#include <tqvaluelist.h>
#include <tqtimer.h>
#include <tqdict.h>
#include <tqmap.h>

class vfs;

typedef enum {
  ST_STARTING               = 0,
  ST_CALCULATING_TOTAL_SIZE = 1,
  ST_CREATING_DIRECTORY     = 2,
  ST_COPYING                = 3
} State;

class VirtualCopyJob : public TDEIO::Job
{
  TQ_OBJECT
  

public:
  VirtualCopyJob( const TQStringList *names, vfs * vfs, const KURL& dest, const KURL& baseURL,
                  PreserveMode pmode, TDEIO::CopyJob::CopyMode mode, bool asMethod, bool showProgressInfo );

protected:
  void statNextDir();
  void createNextDir();
  void copyCurrentDir();
  void directoryFinished( const TQString & );
  
protected slots:
  void slotStart();
  void slotReport();
  
  void slotKdsResult( TDEIO::Job * );
  void slotStatResult( TDEIO::Job * );
  void slotMkdirResult( TDEIO::Job * );
  void slotCopyResult( TDEIO::Job * );

  void slotCopying(TDEIO::Job *, const KURL &, const KURL &);
  void slotMoving(TDEIO::Job *, const KURL &, const KURL &);
  void slotCreatingDir(TDEIO::Job *, const KURL &);
  
  void slotProcessedFiles (TDEIO::Job *, unsigned long);
  void slotProcessedDirs (TDEIO::Job *, unsigned long);
  void slotProcessedSize (TDEIO::Job *, TDEIO::filesize_t);

signals:
  void totalFiles( TDEIO::Job *job, unsigned long files );
  void totalDirs( TDEIO::Job *job, unsigned long dirs );
  void processedFiles( TDEIO::Job *job, unsigned long files );
  void processedDirs( TDEIO::Job *job, unsigned long dirs );
  
private:
  TDEIO::filesize_t          m_totalSize;
  unsigned long            m_totalFiles;
  unsigned long            m_totalSubdirs;

  TDEIO::filesize_t          m_processedSize;
  unsigned long            m_processedFiles;
  unsigned long            m_processedSubdirs;  
    
  TDEIO::filesize_t          m_tempSize;
  unsigned long            m_tempFiles;
  unsigned long            m_tempSubdirs;  
    
  TQValueList<KURL>         m_dirsToGetSize;
  
  TQDict<KURL::List>        m_filesToCopy;
  
  TQMap<TQString,int>        m_size;
  TQMap<TQString,int>        m_filenum;
  TQMap<TQString,int>        m_subdirs;
  
  KURL                     m_baseURL;
  KURL                     m_dest;
  PreserveMode             m_pmode;
  TDEIO::CopyJob::CopyMode   m_mode;
  bool                     m_asMethod;
  bool                     m_showProgressInfo;
  
  State                    m_state;
  
  TQTimer                   m_reportTimer;
  
  KURL                     m_current;
  TQString                  m_currentDir;
  
  TQStringList              m_dirStack;
};

#endif /* __VIRTUAL_COPY_JOB_H__ */
