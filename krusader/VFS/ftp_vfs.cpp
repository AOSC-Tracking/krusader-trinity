/***************************************************************************
                      ftp_vfs.cpp
                  -------------------
   copyright            : (C) 2000 by Rafi Yanai
   e-mail               : krusader@users.sourceforge.net
   web site             : http://krusader.sourceforge.net
---------------------------------------------------------------------------

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

// Sys includes
#include <time.h>
#include <sys/param.h>
#include <unistd.h>
#ifdef BSD
#include <sys/types.h>
#endif 
// QT includes
#include <tqdir.h>
#include <tqregexp.h>
#include <tqtimer.h>
#include <tqeventloop.h>
// KDE includes
#include <kio/jobclasses.h>
#include <klocale.h>
#include <kio/job.h>
#include <kmessagebox.h>
#include <kprotocolinfo.h>
#include <kdebug.h> 
// Krusader includes
#include "ftp_vfs.h"
#include "krpermhandler.h"
#include "../Dialogs/krdialogs.h"
#include "../Dialogs/krprogress.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../resources.h"

ftp_vfs::ftp_vfs( TQObject* panel ) : vfs( panel ), busy( false ) {
	// set the writable attribute
	isWritable = true;
	vfs_type = FTP;
}

ftp_vfs::~ftp_vfs() {
	busy = false;
}

void ftp_vfs::slotAddFiles( TDEIO::Job *, const TDEIO::UDSEntryList& entries ) {
	int rwx = -1;
	
	TQString prot = vfs_origin.protocol();
	if( prot == "krarc" || prot == "tar" || prot == "zip" )
		rwx = PERM_ALL;
	
	TDEIO::UDSEntryListConstIterator it = entries.begin();
	TDEIO::UDSEntryListConstIterator end = entries.end();

	// as long as u can find files - add them to the vfs
	for ( ; it != end; ++it ) {
		KFileItem kfi( *it, vfs_origin, true, true );
		vfile *temp;

		// get file statistics
		TQString name = kfi.text();
		// ignore un-needed entries
		if ( name.isEmpty() || name == "." || name == ".." ) continue;

		TDEIO::filesize_t size = kfi.size();
		time_t mtime = kfi.time( TDEIO::UDS_MODIFICATION_TIME );
		bool symLink = kfi.isLink();
		mode_t mode = kfi.mode() | kfi.permissions();
		TQString perm = KRpermHandler::mode2TQString( mode );
		// set the mimetype
		TQString mime = kfi.mimetype();
		TQString symDest = "";
		if ( symLink ) {
			symDest = kfi.linkDest();
			if ( kfi.isDir() ) perm[ 0 ] = 'd';
		}

		// create a new virtual file object
		if ( kfi.user().isEmpty() )
			temp = new vfile( name, size, perm, mtime, symLink, getuid(), getgid(), mime, symDest, mode, rwx );
		else {
			TQString currentUser = vfs_origin.user();
			if ( currentUser.contains( "@" ) )  /* remove the FTP proxy tags from the username */
				currentUser.truncate( currentUser.find( '@' ) );
			if ( currentUser.isEmpty() ) {
				if( vfs_origin.host().isEmpty() )
					currentUser = KRpermHandler::uid2user( getuid() );
				else {
					currentUser = ""; // empty, but not TQString()
				}
			}
#if KDE_IS_VERSION(3,5,0)
			temp = new vfile( name, size, perm, mtime, symLink,
			                  kfi.user(), kfi.group(), currentUser, 
			                  mime, symDest, mode, rwx, kfi.ACL().asString(),
			                  kfi.defaultACL().asString() );
#else
			temp = new vfile( name, size, perm, mtime, symLink, kfi.user(), kfi.group(), currentUser, mime, symDest, mode, rwx );
#endif
		}

#if KDE_IS_VERSION(3,4,0)
		if( !kfi.localPath().isEmpty() ){
			temp->vfile_setUrl( kfi.localPath() );
		} else {
			temp->vfile_setUrl( kfi.url() );
		}
#else
		temp->vfile_setUrl( kfi.url() );
#endif
		temp->vfile_setIcon( kfi.iconName() );
		foundVfile( temp );
	}
}

void ftp_vfs::slotPermanentRedirection( TDEIO::Job*, const KURL&, const KURL& newUrl ) {
	vfs_origin = newUrl;
	vfs_origin.adjustPath(-1);
}

void ftp_vfs::slotRedirection( TDEIO::Job *, const KURL &url ) {
	// update the origin
	vfs_origin = url;
	vfs_origin.adjustPath(-1);
}

void ftp_vfs::slotListResult( TDEIO::Job *job ) {
	if ( job && job->error() ) {
		// we failed to refresh
		listError = true;
		// display error message
		if ( !quietMode ) job->showErrorDialog( krApp );
	}
	busy = false;
}

bool ftp_vfs::populateVfsList( const KURL& origin, bool showHidden ) {
	TQString errorMsg = TQString();
	if ( !origin.isValid() )
		errorMsg = i18n( "Malformed URL:\n%1" ).arg( origin.url() );
	if ( !KProtocolInfo::supportsListing( origin ) ) {
		if( origin.protocol() == "ftp" && KProtocolInfo::supportsReading( origin ) ) 
			errorMsg = i18n( "Krusader doesn't support FTP access via HTTP.\nIf it is not the case, please check and change the Proxy settings in kcontrol." );
		else
			errorMsg = i18n( "Protocol not supported by Krusader:\n%1" ).arg( origin.url() );
	}

	if ( !errorMsg.isEmpty() ) {
		if ( !quietMode ) KMessageBox::sorry( krApp, errorMsg );
		return false;
	}

	busy = true;

	vfs_origin = origin;
	vfs_origin.adjustPath(-1);

	//TQTimer::singleShot( 0,this,TQT_SLOT(startLister()) );
	listError = false;
	// Open the directory	marked by origin
	krConfig->setGroup( "Look&Feel" );
	//vfs_origin.adjustPath(+1);
	TDEIO::Job *job = TDEIO::listDir( vfs_origin, false, showHidden );
	connect( job, TQT_SIGNAL( entries( TDEIO::Job*, const TDEIO::UDSEntryList& ) ),
	         this, TQT_SLOT( slotAddFiles( TDEIO::Job*, const TDEIO::UDSEntryList& ) ) );
	connect( job, TQT_SIGNAL( redirection( TDEIO::Job*, const KURL& ) ),
	         this, TQT_SLOT( slotRedirection( TDEIO::Job*, const KURL& ) ) );
	connect( job, TQT_SIGNAL( permanentRedirection( TDEIO::Job*, const KURL&, const KURL& ) ),
	         this, TQT_SLOT( slotPermanentRedirection( TDEIO::Job*, const KURL&, const KURL& ) ) );

	connect( job, TQT_SIGNAL( result( TDEIO::Job* ) ),
	         this, TQT_SLOT( slotListResult( TDEIO::Job* ) ) );

	job->setWindow( krApp );

	if ( !quietMode ) {
		emit startJob( job );
		//new KrProgress(job); ==> disabled because of in-panel refresh
	}

	while ( busy && vfs_processEvents());

	if ( listError ) return false;

	return true;
}


// copy a file to the vfs (physical)
void ftp_vfs::vfs_addFiles( KURL::List *fileUrls, TDEIO::CopyJob::CopyMode mode, TQObject* toNotify, TQString dir,  PreserveMode /*pmode*/ ) {
	KURL destUrl = vfs_origin;

	if ( dir != "" ) {
		destUrl.addPath( dir );
		destUrl.cleanPath();  // removes the '..', '.' and extra slashes from the URL.

		if ( destUrl.protocol() == "tar" || destUrl.protocol() == "zip" || destUrl.protocol() == "krarc" ) {
			if ( TQDir( destUrl.path( -1 ) ).exists() )
				destUrl.setProtocol( "file" );  // if we get out from the archive change the protocol
		}
	}

	TDEIO::Job* job = new TDEIO::CopyJob( *fileUrls, destUrl, mode, false, true );
	connect( job, TQT_SIGNAL( result( TDEIO::Job* ) ), this, TQT_SLOT( vfs_refresh( TDEIO::Job* ) ) );
	if ( mode == TDEIO::CopyJob::Move )  // notify the other panel
		connect( job, TQT_SIGNAL( result( TDEIO::Job* ) ), toNotify, TQT_SLOT( vfs_refresh( TDEIO::Job* ) ) );
}

// remove a file from the vfs (physical)
void ftp_vfs::vfs_delFiles( TQStringList *fileNames ) {
	KURL::List filesUrls;
	KURL url;

	// names -> urls
	for ( uint i = 0 ; i < fileNames->count(); ++i ) {
		TQString filename = ( *fileNames ) [ i ];
		url = vfs_origin;
		url.addPath( filename );
		filesUrls.append( url );
	}
	TDEIO::Job *job = new TDEIO::DeleteJob( filesUrls, false, true );
	connect( job, TQT_SIGNAL( result( TDEIO::Job* ) ), this, TQT_SLOT( vfs_refresh( TDEIO::Job* ) ) );
}


KURL::List* ftp_vfs::vfs_getFiles( TQStringList* names ) {
	KURL url;
	KURL::List* urls = new KURL::List();
	for ( TQStringList::Iterator name = names->begin(); name != names->end(); ++name ) {
		url = vfs_getFile( *name );
		urls->append( url );
	}
	return urls;
}


// return a path to the file
KURL ftp_vfs::vfs_getFile( const TQString& name ) {
	vfile * vf = vfs_search( name );
	if ( !vf ) return KURL(); // empty

	KURL url = vf->vfile_getUrl();
	if ( vf->vfile_isDir() ) url.adjustPath( + 1 );
	return url;
}

void ftp_vfs::vfs_mkdir( const TQString& name ) {
	KURL url = vfs_origin;
	url.addPath( name );

	TDEIO::SimpleJob* job = TDEIO::mkdir( url );
	connect( job, TQT_SIGNAL( result( TDEIO::Job* ) ), this, TQT_SLOT( vfs_refresh( TDEIO::Job* ) ) );
}

void ftp_vfs::vfs_rename( const TQString& fileName, const TQString& newName ) {
	KURL::List fileUrls;
	KURL oldUrl = vfs_origin;
	oldUrl.addPath( fileName ) ;

	fileUrls.append( oldUrl );

	KURL newUrl = vfs_origin;
	newUrl.addPath( newName );

	TDEIO::Job *job = new TDEIO::CopyJob( fileUrls, newUrl, TDEIO::CopyJob::Move, true, true );
	connect( job, TQT_SIGNAL( result( TDEIO::Job* ) ), this, TQT_SLOT( vfs_refresh( TDEIO::Job* ) ) );
}

TQString ftp_vfs::vfs_workingDir() {
	return vfs_origin.url( -1 );
}

#include "ftp_vfs.moc"
