/***************************************************************************
                          combiner.cpp  -  description
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

#include "combiner.h"
#include "../VFS/vfs.h"
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <tdefileitem.h>
#include <tdeio/job.h>
#include <tqfileinfo.h>

Combiner::Combiner( TQWidget* parent,  KURL baseURLIn, KURL destinationURLIn, bool unixNamingIn ) :
  TQProgressDialog( parent, "Krusader::Combiner", true, 0 ), baseURL( baseURLIn ), destinationURL( destinationURLIn ), 
  hasValidSplitFile( false ), fileCounter ( 0 ), permissions( -1 ), receivedSize( 0 ),
  combineReadJob( 0 ), combineWriteJob( 0 ), unixNaming( unixNamingIn )
{
  crcContext = new CRC32();

  splitFile = "";
  
  setTotalSteps( 100 );
  setAutoClose( false );  /* don't close or reset the dialog automatically */
  setAutoReset( false );
}

Combiner::~Combiner()
{
  combineAbortJobs();
  delete crcContext;
}

void Combiner::combine()
{
  setCaption( i18n("Krusader::Combining...") );
  setLabelText( i18n("Combining the file %1...").arg( vfs::pathOrURL( baseURL ) ));

    /* check whether the .crc file exists */
  splURL = baseURL;
  splURL.setFileName( baseURL.fileName() + ".crc" );
  KFileItem file(KFileItem::Unknown, KFileItem::Unknown, splURL );
  file.refresh();

  if( !file.isReadable() )
  {
    int ret = KMessageBox::questionYesNo(0, i18n("The CRC information file (%1) is missing!\n"
        "Validity checking is impossible without it. Continue combining?")
        .arg( vfs::pathOrURL( splURL ) ) );

    if( ret == KMessageBox::No )
    {
       emit reject();
       return;
    }

    openNextFile();
  }
  else
  {
    permissions = file.permissions() | TQFileInfo::WriteUser;
    
    combineReadJob = TDEIO::get( splURL, false, false );

    connect(combineReadJob, TQ_SIGNAL(data(TDEIO::Job *, const TQByteArray &)),
            this, TQ_SLOT(combineSplitFileDataReceived(TDEIO::Job *, const TQByteArray &)));
    connect(combineReadJob, TQ_SIGNAL(result(TDEIO::Job*)),
            this, TQ_SLOT(combineSplitFileFinished(TDEIO::Job *)));
  }

  exec();
}

void Combiner::combineSplitFileDataReceived(TDEIO::Job *, const TQByteArray &byteArray)
{
  splitFile += TQString( byteArray );
}

void Combiner::combineSplitFileFinished(TDEIO::Job *job)
{
  combineReadJob = 0;
  TQString error;
  
  if( job->error() )
    error = i18n("Error at reading the CRC file (%1)!").arg( vfs::pathOrURL( splURL ) );
  else
  {
    splitFile.remove( '\r' ); // Windows compatibility
    TQStringList splitFileContent = TQStringList::split( '\n', splitFile );
    
    bool hasFileName = false, hasSize = false, hasCrc = false;
    
    for(unsigned int i = 0; i != splitFileContent.count(); i++ )
    {
      int ndx = splitFileContent[i].find( '=' );    
      if( ndx == -1 )
        continue;      
      TQString token = splitFileContent[i].left( ndx ).stripWhiteSpace();
      TQString value = splitFileContent[i].mid( ndx + 1 );      
    
      if( token == "filename" )
      {
        expectedFileName = value;
        hasFileName = true;
      }
      else if( token == "size" ) 
      {
        sscanf( value.stripWhiteSpace().ascii(), "%llu", &expectedSize );
        hasSize = true;
      }
      if( token == "crc32" )
      {
        expectedCrcSum   = value.stripWhiteSpace().rightJustify( 8, '0' );
        hasCrc = true;
      }
    }
    
    if( !hasFileName || !hasSize || !hasCrc )
      error = i18n("Not a valid CRC file!");
    else
      hasValidSplitFile = true;
  }
      
  if( !error.isEmpty() )
  {
    int ret = KMessageBox::questionYesNo( 0, error+ "\n" +
      i18n("Validity checking is impossible without a good CRC file. Continue combining?")  );
    if( ret == KMessageBox::No )
    {
       emit reject();
       return;
    }
  }
      
  openNextFile();
}

void Combiner::openNextFile()
{  
  if( unixNaming )
  {
    if( readURL.isEmpty() )
      readURL = baseURL;
    else
    {
      TQString name = readURL.fileName();
      int pos = name.length()-1;
      TQChar ch;
      
      do
      {
        ch = name.at( pos ).latin1() + 1;
        if( ch == TQChar( 'Z' + 1 ) )
          ch = 'A';
        if( ch == TQChar( 'z' + 1 ) )
          ch = 'a';
        name[ pos ] = ch;
        pos--;
      } while( pos >=0 && ch.upper() == TQChar( 'A' ) );
      
      readURL.setFileName( name );
    }    
  }
  else
  {
    TQString index( "%1" );      /* determining the filename */
    index = index.arg(++fileCounter).rightJustify( 3, '0' );
    readURL = baseURL;
    readURL.setFileName( baseURL.fileName() + "." + index );
  }

      /* creating a write job */
  combineReadJob = TDEIO::get( readURL, false, false );

  connect(combineReadJob, TQ_SIGNAL(data(TDEIO::Job *, const TQByteArray &)),
          this, TQ_SLOT(combineDataReceived(TDEIO::Job *, const TQByteArray &)));
  connect(combineReadJob, TQ_SIGNAL(result(TDEIO::Job*)),
          this, TQ_SLOT(combineReceiveFinished(TDEIO::Job *)));
  if( hasValidSplitFile )
    connect(combineReadJob, TQ_SIGNAL(percent (TDEIO::Job *, unsigned long)),
                            this, TQ_SLOT(combineWritePercent(TDEIO::Job *, unsigned long)));
  
}

void Combiner::combineDataReceived(TDEIO::Job *, const TQByteArray &byteArray)
{
  if( byteArray.size() == 0 )
    return;

  crcContext->update( (unsigned char *)byteArray.data(), byteArray.size() );
  transferArray = byteArray.copy();

  receivedSize += byteArray.size();

  if( combineWriteJob == 0 )
  {
    writeURL = destinationURL;
    writeURL.addPath( baseURL.fileName() );
    if( hasValidSplitFile )
      writeURL.setFileName( expectedFileName );
    else if( unixNaming )
      writeURL.setFileName( baseURL.fileName() + ".out" );
    
    combineWriteJob = TDEIO::put( writeURL, permissions, true, false, false );    

    connect(combineWriteJob, TQ_SIGNAL(dataReq(TDEIO::Job *, TQByteArray &)),
                             this, TQ_SLOT(combineDataSend(TDEIO::Job *, TQByteArray &)));
    connect(combineWriteJob, TQ_SIGNAL(result(TDEIO::Job*)),
                             this, TQ_SLOT(combineSendFinished(TDEIO::Job *)));
  }  

  if( combineWriteJob )
  {
    if( combineReadJob ) combineReadJob->suspend();    /* start writing */
    combineWriteJob->resume();
  }
}

void Combiner::combineReceiveFinished(TDEIO::Job *job)
{
  combineReadJob = 0;   /* TDEIO automatically deletes the object after Finished signal */
    
  if( job->error() )
  {
    if( combineWriteJob )         /* write out the remaining part of the file */
      combineWriteJob->resume();
    
    if( fileCounter == 1 )
    {
      combineAbortJobs();
      KMessageBox::questionYesNo(0, i18n("Can't open the first split file of %1!")
                                 .arg( vfs::pathOrURL( baseURL ) ) );
      emit reject();
      return;
    }    

    if( hasValidSplitFile )
    {
      TQString crcResult = TQString( "%1" ).arg( crcContext->result(), 0, 16 ).upper().stripWhiteSpace()
                                         .rightJustify(8, '0');
      
      if( receivedSize != expectedSize )
        error = i18n("Incorrect filesize! The file might have been corrupted!");
      else if ( crcResult != expectedCrcSum.upper().stripWhiteSpace() )
        error = i18n("Incorrect CRC checksum! The file might have been corrupted!");
    }
    return;
  }  
  openNextFile();
}

void Combiner::combineDataSend(TDEIO::Job *, TQByteArray &byteArray)
{
  byteArray = transferArray;
  transferArray = TQByteArray();

  if( combineReadJob )
  {
    combineReadJob->resume();    /* start reading */
    combineWriteJob->suspend();
  }
}

void Combiner::combineSendFinished(TDEIO::Job *job)
{
  combineWriteJob = 0;  /* TDEIO automatically deletes the object after Finished signal */

  if( job->error() )    /* any error occurred? */
  {
    combineAbortJobs();
    KMessageBox::error(0, i18n("Error writing file %1!").arg( vfs::pathOrURL( writeURL ) ) );
    emit reject();
    return;
  }

  if( !error.isEmpty() )   /* was any error message at reading ? */
  {
    combineAbortJobs();             /* we cannot write out it in combineReceiveFinished */
    KMessageBox::error(0, error );  /* because emit accept closes it in this function */
    emit reject();
    return;
  }

  emit accept();
}

void Combiner::combineAbortJobs()
{
  if( combineReadJob )
    combineReadJob->kill();
  if( combineWriteJob )
    combineWriteJob->kill();

  combineReadJob = combineWriteJob = 0;
}

void Combiner::combineWritePercent(TDEIO::Job *, unsigned long)
{
  int percent = (int)((((double)receivedSize / expectedSize ) * 100. ) + 0.5 );
  setProgress( percent );
}

#include "combiner.moc"
