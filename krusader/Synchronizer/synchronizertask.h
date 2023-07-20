/***************************************************************************
                      synchronizertask.h  -  description
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

#ifndef __SYNCHRONIZER_TASK_H__
#define __SYNCHRONIZER_TASK_H__

#include <tqobject.h>
#include <tdeio/job.h>

class Synchronizer;
class SynchronizerDirList;
class SynchronizerFileItem;
class TQProgressDialog;
class TQTimer;
class TQFile;

#define ST_STATE_NEW      0
#define ST_STATE_PENDING  1
#define ST_STATE_STATUS   2
#define ST_STATE_READY    3
#define ST_STATE_ERROR    4

class SynchronizerTask : public TQObject {
  TQ_OBJECT
  

public:
  SynchronizerTask() : TQObject(), m_state( ST_STATE_NEW ), m_statusMessage( TQString() ) {}
  virtual ~SynchronizerTask() {};

  inline int start( TQWidget *parentWidget ) { this->parentWidget = parentWidget; start(); return state(); }

  inline int  state() { return m_state; }

  void setStatusMessage( const TQString & statMsg ) { 
    if( m_state == ST_STATE_PENDING || m_state == ST_STATE_STATUS )
      m_state = ST_STATE_STATUS;
      m_statusMessage = statMsg;
  }

  TQString status() {
    if( m_state == ST_STATE_STATUS ) {
      m_state = ST_STATE_PENDING;
      return m_statusMessage; 
    }
    return TQString();
  }

protected:
  virtual void start() {};
  int m_state;
  TQString m_statusMessage;
  TQWidget *parentWidget;
};


class CompareTask : public SynchronizerTask {
  TQ_OBJECT
  

public:
  CompareTask( SynchronizerFileItem *parentIn, const TQString &leftURL,
                const TQString &rightURL, const TQString &leftDir,
                const TQString &rightDir, bool ignoreHidden );
  CompareTask( SynchronizerFileItem *parentIn, const TQString &urlIn,
                const TQString &dirIn, bool isLeftIn, bool ignoreHidden );
  virtual ~CompareTask();

  inline bool isDuplicate()                      { return m_duplicate; }
  inline bool isLeft()                           { return !m_duplicate && m_isLeft; }
  inline const TQString & leftURL()               { return m_url; }
  inline const TQString & rightURL()              { return m_otherUrl; }
  inline const TQString & leftDir()               { return m_dir; }
  inline const TQString & rightDir()              { return m_otherDir; }
  inline const TQString & url()                   { return m_url; }
  inline const TQString & dir()                   { return m_dir; }
  inline SynchronizerFileItem * parent()         { return m_parent; }
  inline SynchronizerDirList * leftDirList()     { return m_dirList; }
  inline SynchronizerDirList * rightDirList()    { return m_otherDirList; }
  inline SynchronizerDirList * dirList()         { return m_dirList; }

protected slots:
  virtual void start();
  void slotFinished( bool result );
  void slotOtherFinished( bool result );

private:
  SynchronizerFileItem * m_parent;
  TQString m_url;
  TQString m_dir;
  TQString m_otherUrl;
  TQString m_otherDir;
  bool m_isLeft;
  bool m_duplicate;
  SynchronizerDirList * m_dirList;
  SynchronizerDirList * m_otherDirList;
  bool m_loadFinished;
  bool m_otherLoadFinished;
  bool ignoreHidden;
};


class CompareContentTask : public SynchronizerTask {
  TQ_OBJECT
  

public:
  CompareContentTask( Synchronizer *, SynchronizerFileItem *, const KURL &, const KURL &, TDEIO::filesize_t );
  virtual ~CompareContentTask();

public slots:
  void    slotDataReceived(TDEIO::Job *job, const TQByteArray &data);
  void    slotFinished(TDEIO::Job *job);
  void    sendStatusMessage();

protected:
  virtual void start();

protected slots:
  void    localFileCompareCycle();

private:
  void    abortContentComparing();

  KURL                   leftURL;        // the currently processed URL (left)
  KURL                   rightURL;       // the currently processed URL (right)
  TDEIO::filesize_t        size;           // the size of the compared files

  bool                   errorPrinted;   // flag indicates error
  TDEIO::TransferJob      *leftReadJob;    // compare left read job
  TDEIO::TransferJob      *rightReadJob;   // compare right read job
  TQByteArray             compareArray;   // the array for comparing
  SynchronizerFileItem  *item;           // the item for content compare
  TQTimer                *timer;          // timer to show the process dialog at compare by content

  TQFile                 *leftFile;       // the left side local file
  TQFile                 *rightFile;      // the right side local file

  TDEIO::filesize_t        received;       // the received size
  Synchronizer          *sync;
};

#endif /* __SYNCHRONIZER_TASK_H__ */
