/***************************************************************************
                     synchronizertask.cpp  -  description
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

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "synchronizer.h"
#include "synchronizertask.h"
#include "synchronizerfileitem.h"
#include "synchronizerdirlist.h"
#include <tqtimer.h>
#include <tqfile.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include "../VFS/vfs.h"

CompareTask::CompareTask( SynchronizerFileItem *parentIn, const TQString &leftURL,
                          const TQString &rightURL, const TQString &leftDir,
                          const TQString &rightDir, bool hidden ) : SynchronizerTask (),  m_parent( parentIn ),
                          m_url( leftURL ), m_dir( leftDir ), m_otherUrl( rightURL ),
                          m_otherDir( rightDir ), m_duplicate( true ),
                          m_dirList( 0 ), m_otherDirList( 0 ) {
  ignoreHidden = hidden;
}

CompareTask::CompareTask( SynchronizerFileItem *parentIn, const TQString &urlIn,
                          const TQString &dirIn, bool isLeftIn, bool hidden ) : SynchronizerTask (),
                          m_parent( parentIn ), m_url( urlIn ), m_dir( dirIn ),
                          m_isLeft( isLeftIn ), m_duplicate( false ),
                          m_dirList( 0 ), m_otherDirList( 0 ) {
  ignoreHidden = hidden;
}

CompareTask::~CompareTask() {
  if( m_dirList ) {
    delete m_dirList;
    m_dirList = 0;
  }
  if( m_otherDirList ) {
    delete m_otherDirList;
    m_otherDirList = 0;
  }
}

void CompareTask::start() {
  if( m_state == ST_STATE_NEW ) {
    m_state = ST_STATE_PENDING;
    m_loadFinished = m_otherLoadFinished = false;

    m_dirList = new SynchronizerDirList( parentWidget, ignoreHidden );
    connect( m_dirList, TQ_SIGNAL( finished( bool ) ), this, TQ_SLOT( slotFinished( bool ) ));
    m_dirList->load( m_url, false );

    if( m_duplicate ) {
      m_otherDirList = new SynchronizerDirList( parentWidget, ignoreHidden );
      connect( m_otherDirList, TQ_SIGNAL( finished( bool ) ), this, TQ_SLOT( slotOtherFinished( bool ) ));
      m_otherDirList->load( m_otherUrl, false );
    }
  }
}

void CompareTask::slotFinished( bool result ) {
  if( !result ) {
    m_state = ST_STATE_ERROR;
    return;
  }
  m_loadFinished = true;

  if( m_otherLoadFinished || !m_duplicate ) 
    m_state = ST_STATE_READY;
}


void CompareTask::slotOtherFinished( bool result ) {
  if( !result ) {
    m_state = ST_STATE_ERROR;
    return;
  }
  m_otherLoadFinished = true;

  if( m_loadFinished )
    m_state = ST_STATE_READY;
}

CompareContentTask::CompareContentTask( Synchronizer *syn, SynchronizerFileItem *itemIn, const KURL &leftURLIn,
                                        const KURL &rightURLIn, TDEIO::filesize_t sizeIn ) : SynchronizerTask(), 
                                        leftURL( leftURLIn ), rightURL( rightURLIn ),
                                        size( sizeIn ), errorPrinted(false), leftReadJob( 0 ),
                                        rightReadJob( 0 ), compareArray(), item( itemIn ), timer( 0 ),
                                        leftFile( 0 ), rightFile( 0 ), received( 0 ), sync( syn ) {
}

CompareContentTask::~CompareContentTask() {
  abortContentComparing();

  if( timer )
    delete timer;
  if( leftFile )
    delete leftFile;
  if( rightFile )
    delete rightFile;
}

void CompareContentTask::start() {
  m_state = ST_STATE_PENDING;

  if( leftURL.isLocalFile() && rightURL.isLocalFile() ) {
    leftFile = new TQFile( leftURL.path() );
    if( !leftFile->open( IO_ReadOnly ) ) {
      KMessageBox::error(parentWidget, i18n("Error at opening %1!").arg( leftURL.path() ));
      m_state = ST_STATE_ERROR;
      return;
    }

    rightFile = new TQFile( rightURL.path() );
    if( !rightFile->open( IO_ReadOnly ) ) {
      KMessageBox::error(parentWidget, i18n("Error at opening %1!").arg( rightURL.path() ));
      m_state = ST_STATE_ERROR;
      return;
    }

    timer = new TQTimer( this );
    connect( timer, TQ_SIGNAL(timeout()), this, TQ_SLOT(sendStatusMessage()) );
    timer->start( 1000, true );

    localFileCompareCycle();
  } else {
    leftReadJob = TDEIO::get( leftURL, false, false );
    rightReadJob = TDEIO::get( rightURL, false, false );

    connect(leftReadJob, TQ_SIGNAL(data(TDEIO::Job *, const TQByteArray &)),
            this, TQ_SLOT(slotDataReceived(TDEIO::Job *, const TQByteArray &)));
    connect(rightReadJob, TQ_SIGNAL(data(TDEIO::Job *, const TQByteArray &)),
            this, TQ_SLOT(slotDataReceived(TDEIO::Job *, const TQByteArray &)));
    connect(leftReadJob, TQ_SIGNAL(result(TDEIO::Job*)),
            this, TQ_SLOT(slotFinished(TDEIO::Job *)));
    connect(rightReadJob, TQ_SIGNAL(result(TDEIO::Job*)),
            this, TQ_SLOT(slotFinished(TDEIO::Job *)));

    rightReadJob->suspend();

    timer = new TQTimer( this );
    connect( timer, TQ_SIGNAL(timeout()), this, TQ_SLOT(sendStatusMessage()) );
    timer->start( 1000, true );
  }
}

void CompareContentTask::localFileCompareCycle() {

  bool different = false;

  char leftBuffer[ 1440 ]; 
  char rightBuffer[ 1440 ];

  TQTime timer;
  timer.start();

  int cnt = 0;

  while ( !leftFile->atEnd() && !rightFile->atEnd() )
  {
    int leftBytes = leftFile->readBlock( leftBuffer, sizeof( leftBuffer ) );
    int rightBytes = rightFile->readBlock( rightBuffer, sizeof( rightBuffer ) );
    
    if( leftBytes != rightBytes ) {
      different = true;
      break;
    }

    if( leftBytes <= 0 )
      break;

    received += leftBytes;

    if( memcmp( leftBuffer, rightBuffer, leftBytes ) ) {
      different = true;
      break;
    }

    if( (++cnt % 16) == 0 && timer.elapsed() >= 250 )
      break;
  }

  if( different ) {
    sync->compareContentResult( item, false );
    m_state = ST_STATE_READY;
    return;
  }

  if( leftFile->atEnd() && rightFile->atEnd() ) {
    sync->compareContentResult( item, true );
    m_state = ST_STATE_READY;
    return;
  }

  TQTimer::singleShot( 0, this, TQ_SLOT( localFileCompareCycle() ) );
}


void CompareContentTask::slotDataReceived(TDEIO::Job *job, const TQByteArray &data)
{
  int bufferLen = compareArray.size();
  int dataLen   = data.size();

  if( job == leftReadJob )
    received += dataLen;

  do
  {
    if( bufferLen == 0 )
    {
      compareArray.duplicate( data.data(), dataLen );
      break;
    }

    int minSize   = ( dataLen < bufferLen ) ? dataLen : bufferLen;

    for( int i = 0; i != minSize; i++ )
      if( data[i] != compareArray[i] )
      {
        abortContentComparing();
        return;
      }

    if( minSize == bufferLen )
    {
      compareArray.duplicate( data.data() + bufferLen, dataLen - bufferLen );
      if( dataLen == bufferLen )
        return;
      break;
    }
    else
    {
      compareArray.duplicate( compareArray.data() + dataLen, bufferLen - dataLen );
      return;
    }

  }while ( false );

  TDEIO::TransferJob *otherJob = ( job == leftReadJob ) ? rightReadJob : leftReadJob;

  if( otherJob == 0 )
  {
    if( compareArray.size() )
      abortContentComparing();
  }
  else
  {
    if( !((TDEIO::TransferJob *)job)->isSuspended() )
    {
      ((TDEIO::TransferJob *)job)->suspend();
      otherJob->resume();
    }
  }
}

void CompareContentTask::slotFinished(TDEIO::Job *job)
{
  TDEIO::TransferJob *otherJob = ( job == leftReadJob ) ? rightReadJob : leftReadJob;

  if( job == leftReadJob )
    leftReadJob = 0;
  else
    rightReadJob = 0;

  if( otherJob )
    otherJob->resume();

  if( job->error() )
  {
    timer->stop();
    abortContentComparing();
  }

  if( job->error() && job->error() != TDEIO::ERR_USER_CANCELED && !errorPrinted )
  {
    errorPrinted = true;
    KMessageBox::error(parentWidget, i18n("IO error at comparing file %1 with %2!")
                       .arg( vfs::pathOrURL( leftURL ) )
                       .arg( vfs::pathOrURL( rightURL ) ) );
  }

  if( leftReadJob == 0 && rightReadJob == 0 )
  {
    if( !compareArray.size() )
      sync->compareContentResult( item, true );
    else
      sync->compareContentResult( item, false );

    m_state = ST_STATE_READY;
  }
}

void CompareContentTask::abortContentComparing()
{
  if( timer )
    timer->stop();

  if( leftReadJob )
    leftReadJob->kill( false );
  if( rightReadJob )
    rightReadJob->kill( false );

  if( item->task() >= TT_UNKNOWN )
    sync->compareContentResult( item, false );

  m_state = ST_STATE_READY;
}

void CompareContentTask::sendStatusMessage()
{
  double perc = (size == 0) ? 1. : (double)received / (double)size;
  int percent = (int)(perc * 10000. + 0.5);
  TQString statstr = TQString( "%1.%2%3" ).arg( percent / 100 ).arg( ( percent / 10 )%10 ).arg( percent % 10 ) + "%";
  setStatusMessage( i18n( "Comparing file %1 (%2)..." ).arg( leftURL.fileName() ).arg( statstr ) );
  timer->start( 500, true );
}

#include "synchronizertask.moc"
