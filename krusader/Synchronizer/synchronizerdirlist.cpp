/***************************************************************************
                   synchronizerdirlist.cpp  -  description
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

#include "synchronizerdirlist.h"
#include "../VFS/vfs.h"
#include "../VFS/krpermhandler.h"
#include <dirent.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfileitem.h>
#include <klargefile.h>
#include <tqapplication.h>
#include <tqdir.h>
#include <kdeversion.h>


#if KDE_IS_VERSION(3,5,0) && defined( HAVE_POSIX_ACL )
#include <sys/acl.h>
#ifdef HAVE_NON_POSIX_ACL_EXTENSIONS
#include <acl/libacl.h>
#endif
#endif

SynchronizerDirList::SynchronizerDirList( TQWidget *w, bool hidden ) : TQObject(), TQDict<vfile>(), fileIterator( 0 ),
                                   parentWidget( w ), busy( false ), result( false ), ignoreHidden( hidden ), currentUrl() {
  setAutoDelete( true );
}

SynchronizerDirList::~SynchronizerDirList() {
  if( fileIterator )
    delete fileIterator;
}

vfile * SynchronizerDirList::search( const TQString &name, bool ignoreCase ) {
  if( !ignoreCase )
    return (*this)[ name ];

  vfile *item = first();
  TQString file = name.lower();

  while( item )
  {
    if( file == item->vfile_getName().lower() )
      return item;
    item = next();
  }
  return 0;
}

vfile * SynchronizerDirList::first() {
  return fileIterator->toFirst();
}

vfile * SynchronizerDirList::next() {
  return ++(*fileIterator);
}

bool SynchronizerDirList::load( const TQString &urlIn, bool wait ) {
  if( busy )
    return false;

  currentUrl = urlIn;
  KURL url = vfs::fromPathOrURL( urlIn );

  if( fileIterator == 0 )
    fileIterator = new TQDictIterator<vfile> ( *this );

  clear();

  if( url.isLocalFile() ) {
    TQString path = url.path( -1 );
    DIR* dir = opendir(path.local8Bit());
    if(!dir)  {
      KMessageBox::error(parentWidget, i18n("Can't open the %1 directory!").tqarg( path ), i18n("Error"));
      emit finished( result = false );
      return false;
    }

    struct dirent* dirEnt;
    TQString name;

    while( (dirEnt=readdir(dir)) != NULL ){
      name = TQString::fromLocal8Bit(dirEnt->d_name);

      if (name=="." || name == "..") continue;
      if (ignoreHidden && name.startsWith( "." ) ) continue;

      TQString fullName = path + "/" + name;

      KDE_struct_stat stat_p;
      KDE_lstat(fullName.local8Bit(),&stat_p);

      TQString perm = KRpermHandler::mode2TQString(stat_p.st_mode);

      bool symLink= S_ISLNK(stat_p.st_mode);
      TQString symlinkDest;

      if( symLink ){  // who the link is pointing to ?
        char symDest[256];
        bzero(symDest,256); 
        int endOfName=0;
        endOfName=readlink(fullName.local8Bit(),symDest,256);
        if ( endOfName != -1 ) {
          TQString absSymDest = symlinkDest = TQString::fromLocal8Bit( symDest );

          if( !absSymDest.startsWith( "/" ) )
            absSymDest = TQDir::cleanDirPath( path + "/" + absSymDest );

          if ( TQDir( absSymDest ).exists() )
            perm[0] = 'd';
        }
      }

      TQString mime = TQString();

      KURL fileURL = KURL::fromPathOrURL( fullName );

      vfile* item=new vfile(name,stat_p.st_size,perm,stat_p.st_mtime,symLink,stat_p.st_uid,
                        stat_p.st_gid,mime,symlinkDest,stat_p.st_mode);
      item->vfile_setUrl( fileURL );

      insert( name, item );
    }

    closedir( dir );
    emit finished( result = true );
    return true;
  } else {
    KIO::Job *job = KIO::listDir( url, false, true );
    connect( job, TQT_SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ),
             this, TQT_SLOT( slotEntries( KIO::Job*, const KIO::UDSEntryList& ) ) );
    connect( job, TQT_SIGNAL( result( KIO::Job* ) ),
             this, TQT_SLOT( slotListResult( KIO::Job* ) ) );
    busy = true;

    if( !wait )
      return true;

    while( busy )
      tqApp->processEvents();
    return result;
  }
}

void SynchronizerDirList::slotEntries( KIO::Job * job, const KIO::UDSEntryList& entries ) 
{
  KIO::UDSEntryListConstIterator it = entries.begin();
  KIO::UDSEntryListConstIterator end = entries.end();

  int rwx = -1;
  TQString prot = (( KIO::ListJob *)job )->url().protocol();

  if( prot == "krarc" || prot == "tar" || prot == "zip" )
    rwx = PERM_ALL;

  while( it != end )
  {
    KFileItem kfi( *it, (( KIO::ListJob *)job )->url(), true, true );
    TQString key = kfi.text();
    if( key != "." && key != ".." && (!ignoreHidden || !key.startsWith(".") ) ) {
      mode_t mode = kfi.mode() | kfi.permissions();
      TQString perm = KRpermHandler::mode2TQString( mode );
      if ( kfi.isDir() ) 
        perm[ 0 ] = 'd';

      vfile *item = new vfile( kfi.text(), kfi.size(), perm, kfi.time( KIO::UDS_MODIFICATION_TIME ),
          kfi.isLink(), kfi.user(), kfi.group(), kfi.user(), 
          kfi.mimetype(), kfi.linkDest(), mode, rwx
#if KDE_IS_VERSION(3,5,0) && defined( HAVE_POSIX_ACL )
                                              , kfi.ACL().asString()
#endif
                                                                     );
      insert( key, item );
    }
    ++it;
  }
}

void SynchronizerDirList::slotListResult( KIO::Job *job ) {
  busy = false;
  if ( job && job->error() ) {
    job->showErrorDialog( parentWidget );
    emit finished( result = false );
    return;
  }
  emit finished( result = true );
}

#include "synchronizerdirlist.moc"
