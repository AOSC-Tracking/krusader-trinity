/***************************************************************************
                                krsearchmod.cpp
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

                                                    S o u r c e    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "krsearchmod.h"
#include "../VFS/krquery.h"
#include "../krusader.h"
#include "../resources.h"
#include "../VFS/vfile.h"
#include "../VFS/krpermhandler.h"
#include "../VFS/krarchandler.h"

#include <klocale.h>
#include <tqdir.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <tqtextstream.h>
#include <tqregexp.h>
#include <klargefile.h>
#include <kurlrequesterdlg.h>

#include <kmimetype.h>

#define  EVENT_PROCESS_DELAY     250

KRSearchMod::KRSearchMod( const KRQuery* q )
{
  stopSearch = false; /// ===> added
  query = new KRQuery( *q );
  connect( query, TQT_SIGNAL( status( const TQString & ) ), 
           this,  TQT_SIGNAL( searching(const TQString&) ) );
  connect( query, TQT_SIGNAL( processEvents( bool & ) ), 
           this,  TQT_SLOT  ( slotProcessEvents( bool & ) ) );

  remote_vfs = 0;
  virtual_vfs = 0;
}

KRSearchMod::~KRSearchMod()
{
  delete query;
  if( remote_vfs )
    delete remote_vfs;
  if( virtual_vfs )
    delete virtual_vfs;
}

void KRSearchMod::start()
{
  unScannedUrls.clear();
  scannedUrls.clear();
  timer.start();
  
  KURL::List whereToSearch = query->searchInDirs();

  // search every dir that needs to be searched
  for ( unsigned int i = 0; i < whereToSearch.count(); ++i )
      scanURL( whereToSearch [ i ] );

  emit finished();
}

void KRSearchMod::stop()
{
  stopSearch = true;
}

void KRSearchMod::scanURL( KURL url )
{
  if( stopSearch ) return;

  unScannedUrls.push( url );
  while ( !unScannedUrls.isEmpty() )
  {
    KURL urlToCheck = unScannedUrls.pop();

    if( stopSearch ) return;

    if( query->isExcluded( urlToCheck ) ) {
      if( !query->searchInDirs().tqcontains( urlToCheck ) )
        continue;
    }

    if( scannedUrls.tqcontains( urlToCheck ) )
      continue;
    scannedUrls.push( urlToCheck );

    emit searching( vfs::pathOrURL( urlToCheck ) );

    if ( urlToCheck.isLocalFile() )
      scanLocalDir( urlToCheck );
    else
      scanRemoteDir( urlToCheck );
  }
}

void KRSearchMod::scanLocalDir( KURL urlToScan )
{
  TQString dir = urlToScan.path( 1 );

  DIR* d = opendir( dir.local8Bit() );
  if ( !d ) return ;

  struct dirent* dirEnt;

  while ( ( dirEnt = readdir( d ) ) != NULL )
  {
    TQString name = TQString::fromLocal8Bit( dirEnt->d_name );

    // we dont scan the ".",".." enteries
    if ( name == "." || name == ".." ) continue;

    KDE_struct_stat stat_p;
    KDE_lstat( ( dir + name ).local8Bit(), &stat_p );

    KURL url = vfs::fromPathOrURL( dir + name );

    TQString mime = TQString();
    if ( query->searchInArchives() || !query->hasMimeType() )
      mime = KMimeType::findByURL( url, stat_p.st_mode, true, false ) ->name();

    // creating a vfile object for matching with krquery
    vfile * vf = new vfile(name, (KIO::filesize_t)stat_p.st_size, KRpermHandler::mode2TQString(stat_p.st_mode),
                           stat_p.st_mtime, S_ISLNK(stat_p.st_mode), stat_p.st_uid, stat_p.st_gid,
                           mime, "", stat_p.st_mode);
    vf->vfile_setUrl( url );

    if ( query->isRecursive() )
    {
      if ( S_ISLNK( stat_p.st_mode ) && query->followLinks() )
        unScannedUrls.push( vfs::fromPathOrURL( TQDir( dir + name ).canonicalPath() ) );
      else if ( S_ISDIR( stat_p.st_mode ) )
        unScannedUrls.push( url );
    }
    if ( query->searchInArchives() )
    {
      TQString type = mime.right( 4 );
      if ( mime.tqcontains( "-rar" ) ) type = "-rar";

      if ( KRarcHandler::arcSupported( type ) )
      {
        KURL archiveURL = url;
        bool encrypted;
        TQString realType = KRarcHandler::getType( encrypted, url.path(), mime );

        if( !encrypted ) {
          if ( realType == "-tbz" || realType == "-tgz" || realType == "tarz" || realType == "-tar" )
            archiveURL.setProtocol( "tar" );
          else
            archiveURL.setProtocol( "krarc" );

          unScannedUrls.push( archiveURL );
        }
      }
    }

    if( query->match( vf ) )
    {
      // if we got here - we got a winner
      results.append( dir + name );
      emit found( name, dir, ( KIO::filesize_t ) stat_p.st_size, stat_p.st_mtime, KRpermHandler::mode2TQString( stat_p.st_mode ), query->foundText() );
    }
    delete vf;

    if( timer.elapsed() >= EVENT_PROCESS_DELAY ) {
      tqApp->processEvents();
      timer.start();
      if( stopSearch ) return;    
    }
  }
  // clean up
  closedir( d );
}

void KRSearchMod::scanRemoteDir( KURL url )
{
  vfs * vfs_;
  

  if( url.protocol() == "virt" ) 
  {
    if( virtual_vfs == 0 )
      virtual_vfs = new virt_vfs( 0 );
    vfs_ = virtual_vfs;
  }
  else
  { 
    if( remote_vfs == 0 )
      remote_vfs = new ftp_vfs( 0 );
    vfs_ = remote_vfs;
  }

  if ( !vfs_->vfs_refresh( url ) ) return ;

  for ( vfile * vf = vfs_->vfs_getFirstFile(); vf != 0 ; vf = vfs_->vfs_getNextFile() )
  {
    TQString name = vf->vfile_getName();
    KURL fileURL = vfs_->vfs_getFile( name );

    if ( query->isRecursive() && (( vf->vfile_isSymLink() && query->followLinks() ) || vf->vfile_isDir() ) )
        unScannedUrls.push( fileURL );

    if( query->match( vf ) )
    {
      // if we got here - we got a winner
      results.append( vfs::pathOrURL( fileURL, -1 ) );
      
      emit found( fileURL.fileName(), vfs::pathOrURL( fileURL.upURL(), -1 ), vf->vfile_getSize(), vf->vfile_getTime_t(), vf->vfile_getPerm(), query->foundText() );
    }

    if( timer.elapsed() >= EVENT_PROCESS_DELAY ) {
      tqApp->processEvents();
      timer.start();
      if( stopSearch ) return;    
    }
  }
}

void KRSearchMod::slotProcessEvents( bool & stopped ) {
  tqApp->processEvents();
  stopped = stopSearch;
}

#include "krsearchmod.moc"
