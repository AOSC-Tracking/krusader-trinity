/***************************************************************************
                         virt_vfs.cpp  -  description
                            -------------------
   begin                : Fri Dec 5 2003
   copyright            : (C) 2003 by Shie Erlich & Rafi Yanai
   email                : 
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <unistd.h>
#include <time.h>
 
#include <tdefileitem.h>
#include <tdeglobalsettings.h>
#include <kurl.h>
#include <tdemessagebox.h>
#include <tdelocale.h>
#include <kdirsize.h>
#include <kstandarddirs.h>

#include "krpermhandler.h"
#include "../krusader.h"
#include "../defaults.h"
#include "virt_vfs.h"

#define VIRT_VFS_DB "virt_vfs.db"

TQDict<KURL::List> virt_vfs::virtVfsDict;
TDEConfig* virt_vfs::virt_vfs_db=0;

virt_vfs::virt_vfs( TQObject* panel, bool quiet ) : vfs( panel, quiet ) {
	// set the writable attribute
	isWritable = true;

	virtVfsDict.setAutoDelete( true );
	if ( virtVfsDict.isEmpty() ) {
		restore();
	}

	vfs_type = VIRT;
}

virt_vfs::~virt_vfs() {}

bool virt_vfs::populateVfsList( const KURL& origin, bool /*showHidden*/ ) {
	vfs_origin = origin;
	vfs_origin.adjustPath(-1);
	path = origin.path( -1 ).mid( 1 );
	if ( path.isEmpty() ) path = "/";

	KURL::List* urlList = virtVfsDict[ path ];
	if ( !urlList ) {
		urlList = new KURL::List();
		virtVfsDict.insert( path, urlList );
		virtVfsDict[ "/" ] ->append( KURL::fromPathOrURL( "virt:/" + path ) );
	}
	
	if ( urlList->isEmpty() ) return true;
	KURL::List::iterator it;
	for ( it = urlList->begin() ; it != urlList->end() ; /*++it*/ ) {
		KURL url = *it;
		// translate url->vfile and remove urls that no longer exist from the list
		vfile* vf = stat(url);
		if ( !vf ) {
			it = urlList->remove( it );
			// the iterator is advanced automaticly
			continue;
		}
		foundVfile( vf );
		++it;
	}
	save();
	return true;
}

void virt_vfs::vfs_addFiles( KURL::List *fileUrls, TDEIO::CopyJob::CopyMode /*mode*/, TQObject* /*toNotify*/, TQString /*dir*/, PreserveMode /*pmode*/ ) {
	if ( path == "/" ) {
		if ( !quietMode )
			KMessageBox::error( krApp, i18n( "You can't copy files directly to the 'virt:/' directory.\nYou can create a sub directory and copy your files into it." ), i18n( "Error" ) );
		return ;
	}
	
	KURL::List* urlList = virtVfsDict[ path ];
	for( unsigned i=0; i != fileUrls->count(); i++ ) {
		if( !urlList->contains( (*fileUrls)[ i ] ) )
			urlList->push_back( (*fileUrls)[ i ] );
	}

	vfs_refresh();
}

void virt_vfs::vfs_delFiles( TQStringList *fileNames ) {
	if ( path == "/" ) {
		for ( uint i = 0 ; i < fileNames->count(); ++i ) {
			TQString filename = ( *fileNames ) [ i ];
			virtVfsDict[ "/" ] ->remove( TQString("virt:/")+filename );
			virtVfsDict.remove( filename );
		}
		vfs_refresh();
		return ;
	}

	KURL::List filesUrls;
	KURL url;

	// names -> urls
	for ( uint i = 0 ; i < fileNames->count(); ++i ) {
		TQString filename = ( *fileNames ) [ i ];
		filesUrls.append( vfs_getFile( filename ) );
	}
	TDEIO::Job *job;

	// delete of move to trash ?
	krConfig->setGroup( "General" );
	if ( krConfig->readBoolEntry( "Move To Trash", _MoveToTrash ) ) {
#if KDE_IS_VERSION(3,4,0)
		job = TDEIO::trash( filesUrls, true );
#else
		job = new TDEIO::CopyJob( filesUrls, TDEGlobalSettings::trashPath(), TDEIO::CopyJob::Move, false, true );
#endif
		connect( job, TQ_SIGNAL( result( TDEIO::Job* ) ), krApp, TQ_SLOT( changeTrashIcon() ) );
	} else
		job = new TDEIO::DeleteJob( filesUrls, false, true );

	// refresh will remove the deleted files...
	connect( job, TQ_SIGNAL( result( TDEIO::Job* ) ), this, TQ_SLOT( vfs_refresh( TDEIO::Job* ) ) );
}

void virt_vfs::vfs_removeFiles( TQStringList *fileNames ) {
	if ( path == "/" )
		return; 
	
	// removing the URLs from the collection
	for ( uint i = 0 ; i < fileNames->count(); ++i ) {
		KURL::List* urlList = virtVfsDict[ path ];
		if( urlList )
			urlList->remove( vfs_getFile( ( *fileNames ) [ i ] ) );
	}
	
	vfs_refresh();
}

KURL::List* virt_vfs::vfs_getFiles( TQStringList* names ) {
	KURL url;
	KURL::List* urls = new KURL::List();
	for ( TQStringList::Iterator name = names->begin(); name != names->end(); ++name ) {
		url = vfs_getFile( *name );
		urls->append( url );
	}
	return urls;
}

KURL virt_vfs::vfs_getFile( const TQString& name ) {
	vfile * vf = vfs_search( name );
	if ( !vf ) return KURL(); // empty

	KURL url = vf->vfile_getUrl();
	if ( vf->vfile_isDir() ) url.adjustPath( + 1 );
	return url;
}

void virt_vfs::vfs_mkdir( const TQString& name ) {
	if ( path != "/" ) {
		if ( !quietMode )
			KMessageBox::error( krApp, i18n( "Creating new directories is allowed only in the 'virt:/' directory." ), i18n( "Error" ) );
		return ;
	}
	KURL::List* temp = new KURL::List();
	virtVfsDict.insert( name, temp );
	virtVfsDict[ "/" ] ->append( TQString( "virt:/" )+name );

	vfs_refresh();
}

void virt_vfs::vfs_rename( const TQString& fileName, const TQString& newName ) {
	KURL::List fileUrls;
	KURL url , dest;

	vfile* vf = vfs_search( fileName );
	if ( !vf ) return ; // not found

	if ( path == "/" ) {
		virtVfsDict[ "/" ] ->append( TQString( "virt:/" ) + newName  );
		virtVfsDict[ "/" ] ->remove( TQString( "virt:/" ) + fileName );
		virtVfsDict.insert( newName, virtVfsDict.take( fileName ) );
		vfs_refresh();
		return ;
	}

	url = vf->vfile_getUrl();
	fileUrls.append( url );

	dest = fromPathOrURL( newName );
	// add the new url to the list
	// the the list is refreshed only existing files remain -
	// so we don't have to worry if the job was successful
	virtVfsDict[ path ] ->append( dest );

	TDEIO::Job *job = new TDEIO::CopyJob( fileUrls, dest, TDEIO::CopyJob::Move, true, false );
	connect( job, TQ_SIGNAL( result( TDEIO::Job* ) ), this, TQ_SLOT( vfs_refresh( TDEIO::Job* ) ) );
}

void virt_vfs::slotStatResult( TDEIO::Job* job ) {
	if( !job || job->error() ) entry = TDEIO::UDSEntry();
	else entry = static_cast<TDEIO::StatJob*>(job)->statResult();
	busy = false;
}

vfile* virt_vfs::stat( const KURL& url ) {
	if( url.protocol() == "virt" ){
		TQString path = url.path().mid(1);
		if( path.isEmpty() ) path = "/";
		vfile * temp = new vfile( path, 0, "drwxr-xr-x", time( 0 ), false, getuid(), getgid(), "inode/directory", "", 0 );
		temp->vfile_setUrl( url );
		return temp;
	}
	KFileItem* kfi;
	if ( url.isLocalFile() ) {
		kfi = new KFileItem( KFileItem::Unknown, KFileItem::Unknown, url, true );
	}
	else {
		busy = true;
		TDEIO::StatJob* statJob = TDEIO::stat( url, false );
		connect( statJob, TQ_SIGNAL( result( TDEIO::Job* ) ), this, TQ_SLOT( slotStatResult( TDEIO::Job* ) ) );
		while ( busy && vfs_processEvents() );
		if( entry.isEmpty()  ) return 0; // statJob failed
		
		kfi = new KFileItem(entry, url, true );
	}
	
	if ( !kfi->time( TDEIO::UDS_MODIFICATION_TIME ) ){
		 delete kfi;
		 return 0; // file not found		
	}
	
	vfile *temp;

	// get file statistics
	TQString name;
	if( url.isLocalFile() )
		name = url.path();
	else
		name = url.prettyURL();

	TDEIO::filesize_t size = kfi->size();
	time_t mtime = kfi->time( TDEIO::UDS_MODIFICATION_TIME );
	bool symLink = kfi->isLink();
	mode_t mode = kfi->mode() | kfi->permissions();
	TQString perm = KRpermHandler::mode2TQString( mode );
// set the mimetype
	TQString mime = TQString();
	TQString symDest = "";
	if ( symLink ) {
		symDest = kfi->linkDest();
		if ( kfi->isDir() ) perm[ 0 ] = 'd';
	}

	// create a new virtual file object
	if ( kfi->user().isEmpty() )
		temp = new vfile( name, size, perm, mtime, symLink, getuid(), getgid(), mime, symDest, mode );
	else {
		TQString currentUser = url.user();
		if ( currentUser.contains( "@" ) )  /* remove the FTP proxy tags from the username */
			currentUser.truncate( currentUser.find( '@' ) );
		if ( currentUser.isEmpty() )
			currentUser = KRpermHandler::uid2user( getuid() );
		temp = new vfile( name, size, perm, mtime, symLink, kfi->user(), kfi->group(), currentUser, mime, symDest, mode );
	}

	temp->vfile_setUrl( kfi->url() );
	delete kfi;
	return temp;
}

TDEConfig*  virt_vfs::getVirtDB(){
	if( !virt_vfs_db ){
		virt_vfs_db = new TDEConfig(VIRT_VFS_DB,false,"data");
	}
	return virt_vfs_db; 
}

bool virt_vfs::save(){
	TDEConfig* db = getVirtDB();
	
	db->setGroup("virt_db");
	TQDictIterator<KURL::List> it( virtVfsDict ); // See TQDictIterator
	for( ; it.current(); ++it ){
		KURL::List::iterator url;
		TQStringList entry;
		for ( url = it.current()->begin() ; url != it.current()->end() ; ++url ) {
			entry.append( (*url).prettyURL() );
		}
		db->writeEntry(it.currentKey(),entry);
	}
	
	db->sync();
	
	return true;
}

bool virt_vfs::restore(){
	TDEConfig* db = getVirtDB();
	db->setGroup("virt_db");
	
	TQMap<TQString, TQString> map = db->entryMap("virt_db");
	TQMap<TQString, TQString>::Iterator it;
	KURL::List* urlList;
	for ( it = map.begin(); it != map.end(); ++it ) {
		urlList = new KURL::List( db->readListEntry(it.key()) );
		virtVfsDict.insert( it.key(),urlList );
	}

	if( !virtVfsDict["/" ]){
		urlList = new KURL::List();
		virtVfsDict.insert( "/", urlList );	
	}
		
	return true;
}

void virt_vfs::vfs_calcSpace( TQString name , TDEIO::filesize_t* totalSize, unsigned long* totalFiles, unsigned long* totalDirs, bool* stop ) {
	if ( stop && *stop ) return ;
	if( path == "/" ) {
		KURL::List* urlList = virtVfsDict[ name ];
		if ( urlList )
			for( unsigned i=0; (i != urlList->size()) && !(*stop); i++ )
				calculateURLSize( (*urlList)[ i ], totalSize, totalFiles, totalDirs, stop );
		return;        
	}                
	return vfs::vfs_calcSpace( name, totalSize, totalFiles, totalDirs, stop );
}

#include "virt_vfs.moc"
