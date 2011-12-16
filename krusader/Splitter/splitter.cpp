/***************************************************************************
                          splitter.cpp  -  description
                             -------------------
    copyright            : (C) 2003 by Csaba Karai
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

#include "splitter.h"
#include "../VFS/vfs.h"
#include <tqlayout.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kio/job.h>
#include <kfileitem.h>
#include <tqfileinfo.h>
    
Splitter::Splitter( TQWidget* parent,  KURL fileNameIn, KURL destinationDirIn ) :
  TQProgressDialog( parent, "Krusader::Splitter", true, 0 ), splitSize( 0 )
{
  fileName = fileNameIn;

  destinationDir = destinationDirIn;

  crcContext = new CRC32();
  
  setTotalSteps( 100 );
  setAutoClose( false );  /* don't close or reset the dialog automatically */
  setAutoReset( false );
}

Splitter::~Splitter()
{
  splitAbortJobs();
  delete crcContext;
}

void Splitter::split( KIO::filesize_t splitSizeIn )
{
  KFileItem file(KFileItem::Unknown, KFileItem::Unknown, fileName );
  file.refresh();

  permissions = file.permissions() | TQFileInfo::WriteUser;
  
  splitSize = splitSizeIn;

  setCaption( i18n("Krusader::Splitting...") );
  setLabelText( i18n("Splitting the file %1...").tqarg( vfs::pathOrURL( fileName ) ) );

  if( file.isDir() )
  {
    KMessageBox::error(0, i18n("Can't split a directory!"));
    return;
  }
  
  fileSize = 0;
  fileNumber = 0;

  splitReadJob = KIO::get( fileName, false, false );
    
  connect(splitReadJob, TQT_SIGNAL(data(KIO::Job *, const TQByteArray &)),
                        this, TQT_SLOT(splitDataReceived(KIO::Job *, const TQByteArray &)));
  connect(splitReadJob, TQT_SIGNAL(result(KIO::Job*)),
                        this, TQT_SLOT(splitReceiveFinished(KIO::Job *)));
  connect(splitReadJob, TQT_SIGNAL(percent (KIO::Job *, unsigned long)),
                        this, TQT_SLOT(splitReceivePercent(KIO::Job *, unsigned long)));

  splitWriteJob = 0;
  noValidWriteJob = true;

  exec();
}

void Splitter::splitDataReceived(KIO::Job *, const TQByteArray &byteArray)
{
  if( byteArray.size() == 0 )
    return;

  crcContext->update( (unsigned char *)byteArray.data(), byteArray.size() );  
  fileSize += byteArray.size();

  if( noValidWriteJob )
    splitCreateWriteJob();

  transferArray = byteArray.copy();
  if(splitWriteJob)
  {
    splitReadJob->suspend();    /* start writing */
    splitWriteJob->resume();
  }
}

void Splitter::splitReceiveFinished(KIO::Job *job)
{
  splitReadJob = 0;   /* KIO automatically deletes the object after Finished signal */
  
  if( splitWriteJob )        /* write out the end of the file */
    splitWriteJob->resume();

  if( job->error() )    /* any error occurred? */
  {
    splitAbortJobs();
    KMessageBox::error(0, i18n("Error reading file %1!").tqarg( vfs::pathOrURL( fileName ) ) );
    emit reject();
    return;
  }

  TQString crcResult = TQString( "%1" ).tqarg( crcContext->result(), 0, 16 ).upper().stripWhiteSpace()
                                     .rightJustify(8, '0');

  splitFile = TQString( "filename=%1\n" ).tqarg( fileName.fileName()     )+
              TQString( "size=%1\n" )    .tqarg( KIO::number( fileSize ) )+
              TQString( "crc32=%1\n" )   .tqarg( crcResult );
}

void Splitter::splitReceivePercent (KIO::Job *, unsigned long percent)
{
  setProgress( percent );
}

void Splitter::splitCreateWriteJob()
{
  TQString index( "%1" );                   /* making the splitted filename */
  index = index.tqarg(++fileNumber).rightJustify( 3, '0' );
  TQString outFileName = fileName.fileName() + "." + index;
  
  writeURL = destinationDir;
  writeURL.addPath( outFileName );

      /* creating a write job */
  splitWriteJob = KIO::put( writeURL, permissions, true, false, false );
  outputFileSize = 0;
  connect(splitWriteJob, TQT_SIGNAL(dataReq(KIO::Job *, TQByteArray &)),
                         this, TQT_SLOT(splitDataSend(KIO::Job *, TQByteArray &)));
  connect(splitWriteJob, TQT_SIGNAL(result(KIO::Job*)),
                         this, TQT_SLOT(splitSendFinished(KIO::Job *)));
  noValidWriteJob = false;
}

void Splitter::splitDataSend(KIO::Job *, TQByteArray &byteArray)
{
  int bufferLen = transferArray.size();

  if( noValidWriteJob )      /* splitted file should be closed ? */
  {
    byteArray = TQByteArray();  /* giving empty buffer which indicates closing */
  }
  else if( outputFileSize + bufferLen > splitSize ) /* maximum length reached? */
  {
    int shortLen = splitSize - outputFileSize;
    
    byteArray.duplicate( transferArray.data(), shortLen );
    transferArray.duplicate( transferArray.data() + shortLen, bufferLen - shortLen );
    
    noValidWriteJob = true;   /* close the current segment */
  }
  else
  {
    outputFileSize += bufferLen;  /* write the whole buffer out to the split file */

    byteArray = transferArray;
    transferArray = TQByteArray();
  
    if(splitReadJob)
    {
      splitReadJob->resume();    /* start reading */
      splitWriteJob->suspend();
    }
  }
}

void Splitter::splitSendFinished(KIO::Job *job)
{
  splitWriteJob = 0;  /* KIO automatically deletes the object after Finished signal */

  if( job->error() )    /* any error occurred? */
  {
    splitAbortJobs();
    KMessageBox::error(0, i18n("Error writing file %1!").tqarg( vfs::pathOrURL( writeURL ) ) );
    emit reject();
    return;
  }
  
  if( transferArray.size() ) /* any data remained in the transfer buffer? */
    splitCreateWriteJob();      /* create a new write job */
  else
  {
      /* writing the split information file out */
    writeURL      = destinationDir;
    writeURL.addPath( fileName.fileName() + ".crc" );
    splitWriteJob = KIO::put( writeURL, permissions, true, false, false );
    connect(splitWriteJob, TQT_SIGNAL(dataReq(KIO::Job *, TQByteArray &)),
                           this, TQT_SLOT(splitFileSend(KIO::Job *, TQByteArray &)));
    connect(splitWriteJob, TQT_SIGNAL(result(KIO::Job*)),
                           this, TQT_SLOT(splitFileFinished(KIO::Job *)));
  }
}

void Splitter::splitAbortJobs()
{
  if( splitReadJob )
    splitReadJob->kill();
  if( splitWriteJob )
    splitWriteJob->kill();

  splitReadJob = splitWriteJob = 0;
}

void Splitter::splitFileSend(KIO::Job *, TQByteArray &byteArray)
{
  const char *content = splitFile.ascii();
  byteArray.duplicate( content, strlen ( content ) );
  splitFile = "";
}

void Splitter::splitFileFinished(KIO::Job *job)
{
  splitWriteJob = 0;  /* KIO automatically deletes the object after Finished signal */

  if( job->error() )    /* any error occurred? */
  {
    KMessageBox::error(0, i18n("Error at writing file %1!").tqarg( vfs::pathOrURL( writeURL ) ) );
    emit reject();
    return;
  }

  emit accept();
}

#include "splitter.moc"
