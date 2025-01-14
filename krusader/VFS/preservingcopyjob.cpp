/***************************************************************************
                    preservingcopyjob.cpp  -  description
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

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "preservingcopyjob.h"
#include "../defaults.h"
#include "../krusader.h"
#include <utime.h>
#include <klargefile.h>
#include <tdeio/job.h>
#include <tdeio/jobclasses.h>
#include <tdefileitem.h>
#include <tqfile.h>
#include <pwd.h>
#include <grp.h>


#if KDE_IS_VERSION(3,5,0) && defined( HAVE_POSIX_ACL )
#include <sys/acl.h>
#ifdef HAVE_NON_POSIX_ACL_EXTENSIONS
#include <acl/libacl.h>
#endif
#endif

Attributes::Attributes() 
{
  time = (time_t)-1;
  uid = (uid_t)-1;
  gid = (gid_t)-1;
  mode = (mode_t)-1;
  acl = TQString();
}

Attributes::Attributes( time_t tIn, uid_t uIn, gid_t gIn, mode_t modeIn, const TQString & aclIn ) 
{
  time = tIn, uid = uIn, gid = gIn, mode = modeIn, acl = aclIn;
}

Attributes::Attributes( time_t tIn, TQString user, TQString group, mode_t modeIn, const TQString & aclIn ) 
{
  time = tIn;
  uid = (uid_t)-1;
  struct passwd* pw = getpwnam(TQFile::encodeName( user ));
  if ( pw != 0L )
    uid = pw->pw_uid;
  gid = (gid_t)-1;
  struct group* g = getgrnam(TQFile::encodeName( group ));
  if ( g != 0L )
    gid = g->gr_gid;
  mode = modeIn;
  acl = aclIn;
}

PreservingCopyJob::PreservingCopyJob( const KURL::List& src, const KURL& dest, CopyMode mode,
  bool asMethod, bool showProgressInfo ) : TDEIO::CopyJob( src, dest, mode, asMethod, showProgressInfo )
{
  if( dest.isLocalFile() )
  {
    connect( this, TQ_SIGNAL( aboutToCreate (TDEIO::Job *, const TQValueList< TDEIO::CopyInfo > &) ),
             this, TQ_SLOT( slotAboutToCreate (TDEIO::Job *, const TQValueList< TDEIO::CopyInfo > &) ) );
    connect( this, TQ_SIGNAL( copyingDone( TDEIO::Job *, const KURL &, const KURL &, bool, bool) ),
             this, TQ_SLOT( slotCopyingDone( TDEIO::Job *, const KURL &, const KURL &, bool, bool) ) );
    connect( this, TQ_SIGNAL( result( TDEIO::Job * ) ),
             this, TQ_SLOT( slotFinished() ) );
  }
}

void PreservingCopyJob::slotAboutToCreate( TDEIO::Job */*job*/, const TQValueList< TDEIO::CopyInfo > &files )
{
  for ( TQValueList< TDEIO::CopyInfo >::ConstIterator it = files.begin(); it != files.end(); ++it ) {
  
    if( (*it).uSource.isLocalFile() ) {
      KDE_struct_stat stat_p;
      KDE_lstat( (*it).uSource.path(-1).local8Bit(),&stat_p);    /* getting the date information */
      
      TQString aclStr;
#if KDE_IS_VERSION(3,5,0) && defined( HAVE_POSIX_ACL )
      acl_t acl = acl_get_file( (*it).uSource.path(-1).local8Bit(), ACL_TYPE_ACCESS );

      bool aclExtended = false;
      if( acl )
      {
#if HAVE_NON_POSIX_ACL_EXTENSIONS
        aclExtended = acl_equiv_mode( acl, 0 );
#else
        acl_entry_t entry;
        int ret = acl_get_entry( acl, ACL_FIRST_ENTRY, &entry );
        while ( ret == 1 ) {
          acl_tag_t currentTag;
          acl_get_tag_type( entry, &currentTag );
          if ( currentTag != ACL_USER_OBJ &&
            currentTag != ACL_GROUP_OBJ &&
            currentTag != ACL_OTHER )
          {
            aclExtended = true;
            break;
          }
          ret = acl_get_entry( acl, ACL_NEXT_ENTRY, &entry );
        }
#endif
      }


      if ( acl && !aclExtended ) {
        acl_free( acl );
        acl = NULL;
      }
      if( acl )
      {
        char *aclString = acl_to_text( acl, 0 );
        aclStr = TQString::fromLatin1( aclString );
        acl_free( (void*)aclString );
        acl_free( acl );
      }
#endif
      fileAttributes[ (*it).uSource ] = Attributes( stat_p.st_mtime, stat_p.st_uid, stat_p.st_gid, stat_p.st_mode & 07777, aclStr );
    }
    else {
      time_t mtime = (*it).mtime;
  
      if( mtime != 0 && mtime != ((time_t) -1 ) )       /* is it correct? */
        fileAttributes[ (*it).uSource ].time = mtime;

      int permissions = (*it).permissions;
      fileAttributes[ (*it).uSource ].mode = permissions;
    }
  }
}

void PreservingCopyJob::slotResult( Job *job ) {
  if( !job->error() ) {
    if( job->inherits( "TDEIO::StatJob" ) ) {       /* Unfortunately TDEIO forgets to set times when the file is in the */
      KURL url = ((TDEIO::SimpleJob *)job)->url();  /* base directory. That's why we capture every StatJob and set the */
                                                /* time manually. */
      TDEIO::UDSEntry entry = static_cast<TDEIO::StatJob*>(job)->statResult();      
      KFileItem kfi(entry, url );
    
#if KDE_IS_VERSION(3,5,0) && defined( HAVE_POSIX_ACL )
      fileAttributes[ url ] = Attributes( kfi.time( TDEIO::UDS_MODIFICATION_TIME ), kfi.user(), kfi.group(), kfi.mode(), kfi.ACL().asString() );
#else
      fileAttributes[ url ] = Attributes( kfi.time( TDEIO::UDS_MODIFICATION_TIME ), kfi.user(), kfi.group(), kfi.mode(), TQString() );
#endif
    }
  }

  CopyJob::slotResult( job );
  
  for( unsigned j=0; j != subjobs.count(); j++ ) {
    if( subjobs.at( j )->inherits( "TDEIO::ListJob" ) ) {
      disconnect( subjobs.at( j ), TQ_SIGNAL( entries (TDEIO::Job *, const TDEIO::UDSEntryList &) ),
                  this, TQ_SLOT( slotListEntries (TDEIO::Job *, const TDEIO::UDSEntryList &) ) );
      connect( subjobs.at( j ), TQ_SIGNAL( entries (TDEIO::Job *, const TDEIO::UDSEntryList &) ),
                  this, TQ_SLOT( slotListEntries (TDEIO::Job *, const TDEIO::UDSEntryList &) ) );
    }
  }
}

void PreservingCopyJob::slotListEntries(TDEIO::Job *job, const TDEIO::UDSEntryList &list) {
  TDEIO::UDSEntryListConstIterator it = list.begin();
  TDEIO::UDSEntryListConstIterator end = list.end();
  for (; it != end; ++it) {
    KURL url = ((TDEIO::SimpleJob *)job)->url();
    TQString relName, user, group;
    time_t mtime = (time_t)-1;
    mode_t mode = 0755;
    TQString acl;
    
    TDEIO::UDSEntry::ConstIterator it2 = (*it).begin();
    for( ; it2 != (*it).end(); it2++ ) {
      switch ((*it2).m_uds) {
      case TDEIO::UDS_NAME:
        if( relName.isEmpty() )
          relName = (*it2).m_str;
        break;
      case TDEIO::UDS_URL:
        relName = KURL((*it2).m_str).fileName();
        break;
      case TDEIO::UDS_MODIFICATION_TIME:
        mtime = (time_t)((*it2).m_long);
        break;
      case TDEIO::UDS_USER:
        user = (*it2).m_str;
        break;
      case TDEIO::UDS_GROUP:
        group = (*it2).m_str;
        break;
      case TDEIO::UDS_ACCESS:
        mode = (*it2).m_long;
        break;
#if KDE_IS_VERSION(3,5,0) && defined( HAVE_POSIX_ACL )
      case TDEIO::UDS_ACL_STRING:
        acl = (*it2).m_str;
        break;
#endif
      }
    }
    url.addPath( relName );

    fileAttributes[ url ] = Attributes( mtime, user, group, mode, acl );
  }
}

void PreservingCopyJob::slotCopyingDone( TDEIO::Job *, const KURL &from, const KURL &to, bool postpone, bool)
{
  if( postpone ) { // the directories are stamped at the last step, so if it's a directory, we postpone
    unsigned i=0;
    TQString path = to.path( -1 );

    for( ; i != directoriesToStamp.count(); i++ ) // sort the URL-s to avoid parent time stamp modification
      if( path >= directoriesToStamp[ i ].path( -1 ) )
        break;

    directoriesToStamp.insert( directoriesToStamp.at( i ), to );
    originalDirectories.insert( originalDirectories.at( i ), from );
  }
  else if( fileAttributes.count( from ) ) {
    Attributes attrs = fileAttributes[ from ];
    fileAttributes.remove( from );
    
    time_t mtime = attrs.time;
   
    if( to.isLocalFile() )
    {
      if( mtime != 0 && mtime != ((time_t) -1 ) )
      {
        struct utimbuf timestamp;

        timestamp.actime  = time( 0 );
        timestamp.modtime = mtime;

        utime( (const char *)( to.path( -1 ).local8Bit() ), &timestamp );
      }

      if( attrs.uid != (uid_t)-1 )
        chown( (const char *)( to.path( -1 ).local8Bit() ), attrs.uid, (gid_t)-1 );
      if( attrs.gid != (gid_t)-1 )
        chown( (const char *)( to.path( -1 ).local8Bit() ), (uid_t)-1, attrs.gid );

      if( attrs.mode != (mode_t) -1 )
        chmod( (const char *)( to.path( -1 ).local8Bit() ), attrs.mode );

#if KDE_IS_VERSION(3,5,0) && defined( HAVE_POSIX_ACL )
      if( !attrs.acl.isNull() )
      {
        acl_t acl = acl_from_text( attrs.acl.latin1() );
        if( acl && !acl_valid( acl ) )
          acl_set_file( to.path( -1 ).local8Bit(), ACL_TYPE_ACCESS, acl );
        if( acl )
          acl_free( acl );
      }
#endif
    }
  }
}

void PreservingCopyJob::slotFinished() {
  for( unsigned i=0; i != directoriesToStamp.count(); i++ ) {
    KURL from = originalDirectories[ i ];
    KURL to = directoriesToStamp[ i ];

    slotCopyingDone( 0, from, to, false, false );
  }
}

TDEIO::CopyJob * PreservingCopyJob::createCopyJob( PreserveMode pmode, const KURL::List& src, const KURL& dest, CopyMode mode, bool asMethod, bool showProgressInfo )
{
  if( ! dest.isLocalFile() )
    pmode = PM_NONE;
  if( mode == TDEIO::CopyJob::Link )
    pmode = PM_NONE;

  switch( pmode )
  {
  case PM_PRESERVE_ATTR:
    return new PreservingCopyJob( src, dest, mode, asMethod, showProgressInfo );
  case PM_DEFAULT:
    {
      TQString group = krConfig->group();
      krConfig->setGroup( "Advanced" );
      bool preserve = krConfig->readBoolEntry( "PreserveAttributes", _PreserveAttributes );
      krConfig->setGroup( group );
      if( preserve )
        return new PreservingCopyJob( src, dest, mode, asMethod, showProgressInfo );
      else
        return new TDEIO::CopyJob( src, dest, mode, asMethod, showProgressInfo );
    }
  case PM_NONE:
  default:
    return new TDEIO::CopyJob( src, dest, mode, asMethod, showProgressInfo );
  }
}

#include "preservingcopyjob.moc"
