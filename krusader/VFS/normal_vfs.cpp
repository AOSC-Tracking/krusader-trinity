/***************************************************************************
                       normal_vfs.cpp                             
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
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
// TQt includes
#include <tqdir.h>
#include <tqtimer.h>
// TDE includes
#include <tdemessagebox.h>
#include <kmimetype.h>
#include <tdeio/jobclasses.h>
#include <tdelocale.h>
#include <tdeglobalsettings.h>
#include <kdebug.h>
#include <klargefile.h>
#include <tdefileitem.h>
// Krusader includes
#include "normal_vfs.h"
#include "../Dialogs/krdialogs.h"
#include "../MountMan/kmountman.h"
#include "krpermhandler.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../resources.h"
#include "../krslots.h"

// header files for ACL
#if KDE_IS_VERSION(3,5,0)
#ifdef HAVE_POSIX_ACL
#include <sys/acl.h>
#ifdef HAVE_NON_POSIX_ACL_EXTENSIONS
#include <acl/libacl.h>
#endif
#endif
#endif

normal_vfs::normal_vfs(TQObject* panel):vfs(panel), watcher(0) {
  vfs_type=NORMAL;
}

bool normal_vfs::populateVfsList(const KURL& origin, bool showHidden){
	TQString path = origin.path(-1);
	
	// set the writable attribute to true, if that's not the case - the TDEIO job
	// will give the warnings and errors
	isWritable = true;
	
	if( watcher ) delete watcher; //stop watching the old dir
	watcher = 0;

	// set the origin...
	vfs_origin = origin;
	vfs_origin.adjustPath(-1);
	vfs_origin.setProtocol("file"); // do not remove !
	vfs_origin.cleanPath(-1);
	
	// check that the new origin exists
	if ( !TQDir(path).exists() )
	{
		if( !quietMode ) KMessageBox::error(krApp, i18n("Directory %1 does not exist!").arg( path ), i18n("Error"));
		return false;
	}
    
	krConfig->setGroup("Advanced");
	if (krConfig->readBoolEntry("AutoMount",_AutoMount)) krMtMan.autoMount(path);
	
	DIR* dir = opendir(path.local8Bit());
	if(!dir) 
	{
		if( !quietMode ) KMessageBox::error(krApp, i18n("Can't open the %1 directory!").arg( path ), i18n("Error"));
		return false;
	}

  // change directory to the new directory
        TQString save = getcwd( 0, 0 );
	if (chdir(path.local8Bit()) != 0) {
		if( !quietMode ) KMessageBox::error(krApp, i18n("Access denied to")+path, i18n("Error"));
		closedir(dir);
		return false;
	}

	struct dirent* dirEnt;
  TQString name;

	while( (dirEnt=readdir(dir)) != NULL ){
    name = TQString::fromLocal8Bit(dirEnt->d_name);

		// show hidden files ?
		if ( !showHidden && name.left(1) == "." ) continue ;
		// we dont need the ".",".." enteries
		if (name=="." || name == "..") continue;
	  
		vfile* temp = vfileFromName(name);
    foundVfile(temp);
  }
	// clean up
	closedir(dir);
        chdir( save.local8Bit() );
	
	if( panelConnected )
	{
		watcher = new KDirWatch();
		// connect the watcher
		connect(watcher,TQ_SIGNAL(dirty(const TQString&)),this,TQ_SLOT(vfs_slotDirty(const TQString&)));
		connect(watcher,TQ_SIGNAL(created(const TQString&)),this, TQ_SLOT(vfs_slotCreated(const TQString&)));
		connect(watcher,TQ_SIGNAL(deleted(const TQString&)),this, TQ_SLOT(vfs_slotDeleted(const TQString&)));	
		watcher->addDir(vfs_getOrigin().path(-1),true); //start watching the new dir
		watcher->startScan(true);
	}

  return true;
}

// copy a file to the vfs (physical)	
void normal_vfs::vfs_addFiles(KURL::List *fileUrls,TDEIO::CopyJob::CopyMode mode,TQObject* toNotify,TQString dir, PreserveMode pmode ){
  //if( watcher ) watcher->stopScan(); // we will refresh manually this time...	
  if( watcher ) {
    delete watcher;   // stopScan is buggy, leaves reference on the directory, that's why we delete the watcher
    watcher = 0;
  }

  KURL dest;
  dest.setPath(vfs_workingDir()+"/"+dir);

  TDEIO::Job* job = PreservingCopyJob::createCopyJob( pmode, *fileUrls,dest,mode,false,true );
  connect(job,TQ_SIGNAL(result(TDEIO::Job*)),this,TQ_SLOT(vfs_refresh()) );
  if(mode == TDEIO::CopyJob::Move) // notify the other panel
    connect(job,TQ_SIGNAL(result(TDEIO::Job*)),toNotify,TQ_SLOT(vfs_refresh(TDEIO::Job*)) );
  else
    job->setAutoErrorHandlingEnabled( true );
}

// remove a file from the vfs (physical)
void normal_vfs::vfs_delFiles(TQStringList *fileNames){
	KURL::List filesUrls;
	KURL url;
	TQDir local( vfs_workingDir() );
	vfile* vf;

//  if( watcher ) watcher->stopScan(); // we will refresh manually this time...	
	if( watcher ) {
		delete watcher;   // stopScan is buggy, leaves reference on the directory, that's why we delete the watcher
		watcher = 0;
	}

	// names -> urls
	for(uint i=0 ; i<fileNames->count(); ++i){
		TQString filename = (*fileNames)[i];
		vf = vfs_search(filename);
		url.setPath( vfs_workingDir()+"/"+filename);
		filesUrls.append(url);
	}
	TDEIO::Job *job;
	
	// delete of move to trash ?
	krConfig->setGroup("General");
	if( krConfig->readBoolEntry("Move To Trash",_MoveToTrash) ){
#if KDE_IS_VERSION(3,4,0)
	  job = TDEIO::trash(filesUrls, true );
#else
	  job = new TDEIO::CopyJob(filesUrls,TDEGlobalSettings::trashPath(),TDEIO::CopyJob::Move,false,true );
#endif
	  connect(job,TQ_SIGNAL(result(TDEIO::Job*)),SLOTS,TQ_SLOT(changeTrashIcon()));
	}
	else
	  job = new TDEIO::DeleteJob(filesUrls, false, true);
	
	connect(job,TQ_SIGNAL(result(TDEIO::Job*)),this,TQ_SLOT(vfs_refresh(TDEIO::Job*)));
}

// return a path to the file
KURL normal_vfs::vfs_getFile(const TQString& name){	
  TQString url;
	if ( vfs_workingDir() == "/" ) url = "/"+name;
	else url = vfs_workingDir()+"/"+name;

	return vfs::fromPathOrURL(url);
}

KURL::List* normal_vfs::vfs_getFiles(TQStringList* names){
  KURL::List* urls = new KURL::List();
  for(TQStringList::Iterator name = names->begin(); name != names->end(); ++name){
    urls->append( vfs_getFile(*name) );
  }
  return urls;
}

void normal_vfs::vfs_mkdir(const TQString& name){
	if (!TQDir(vfs_workingDir()).mkdir(name))
	  if (!quietMode) KMessageBox::sorry(krApp,i18n("Can't create a directory. Check your permissions."));
  vfs::vfs_refresh();
}

void normal_vfs::vfs_rename(const TQString& fileName,const TQString& newName){
  KURL::List fileUrls;
  KURL url , dest;

  //if( watcher ) watcher->stopScan(); // we will refresh manually this time...	
  if( watcher ) {
    delete watcher;   // stopScan is buggy, leaves reference on the directory, that's why we delete the watcher
    watcher = 0;
  }

  url.setPath( vfs_workingDir()+"/"+fileName );
  fileUrls.append(url);
  dest.setPath(vfs_workingDir()+"/"+newName);

  TDEIO::Job *job = new TDEIO::CopyJob(fileUrls,dest,TDEIO::CopyJob::Move,true,false );
  connect(job,TQ_SIGNAL(result(TDEIO::Job*)),this,TQ_SLOT(vfs_refresh(TDEIO::Job*)));
}

vfile* normal_vfs::vfileFromName(const TQString& name){
	TQString path = vfs_workingDir()+"/"+name;
	TQCString fileName = path.local8Bit();
	
	KDE_struct_stat stat_p;
	KDE_lstat(fileName.data(),&stat_p);
	TDEIO::filesize_t size = stat_p.st_size;
	TQString perm = KRpermHandler::mode2TQString(stat_p.st_mode);
	bool symLink= S_ISLNK(stat_p.st_mode);
	if( S_ISDIR(stat_p.st_mode) ) perm[0] = 'd';
	
	KURL mimeUrl = fromPathOrURL(path);
	TQString mime=TQString();

	char symDest[256];
	bzero(symDest,256); 
	if( S_ISLNK(stat_p.st_mode) ){  // who the link is pointing to ?
		int endOfName=0;
		endOfName=readlink(fileName.data(),symDest,256);
		if ( endOfName != -1 ){
			if ( TQDir(TQString::fromLocal8Bit( symDest ) ).exists() ) perm[0] = 'd';
			if ( !TQDir(vfs_workingDir()).exists( TQString::fromLocal8Bit ( symDest ) ) ) mime = "Broken Link !";
		}
		else krOut << "Failed to read link: "<< path<<endl;
	}
	
	int rwx = 0;
	if( ::access( fileName.data(), R_OK ) == 0 )
		rwx |= R_OK;
	if( ::access( fileName.data(), W_OK ) == 0 )
		rwx |= W_OK;
	if( ::access( fileName.data(), X_OK ) == 0 )
		rwx |= X_OK;
	
	// create a new virtual file object
	vfile* temp=new vfile(name,size,perm,stat_p.st_mtime,symLink,stat_p.st_uid,
                          stat_p.st_gid,mime,TQString::fromLocal8Bit( symDest ),stat_p.st_mode, rwx);
	temp->vfile_setUrl( mimeUrl );
	return temp;
}

void normal_vfs::getACL( vfile *file, TQString &acl, TQString &defAcl )
{
	acl = defAcl = TQString();
#if KDE_IS_VERSION(3,5,0) && defined( HAVE_POSIX_ACL )
	TQCString fileName = file->vfile_getUrl().path( -1 ).local8Bit();
#if HAVE_NON_POSIX_ACL_EXTENSIONS
	if ( acl_extended_file( fileName.data() ) )
	{
#endif
		acl = getACL( fileName.data(), ACL_TYPE_ACCESS );
		if( file->vfile_isDir() )
			defAcl = getACL( fileName.data(), ACL_TYPE_DEFAULT );
#if HAVE_NON_POSIX_ACL_EXTENSIONS
	}
#endif
#endif
}

TQString normal_vfs::getACL( const TQString & path, int type )
{
#if KDE_IS_VERSION(3,5,0) && defined( HAVE_POSIX_ACL )
	acl_t acl = 0;
	// do we have an acl for the file, and/or a default acl for the dir, if it is one?
	if ( ( acl = acl_get_file( path.ascii(), type ) ) != 0 )
	{
		bool aclExtended = false;
		
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
		
		if ( !aclExtended )
		{
			acl_free( acl );
			acl = 0;
		}
	}
	
	if( acl == 0 )
		return TQString();
	
	char *aclString = acl_to_text( acl, 0 );
	TQString ret = TQString::fromLatin1( aclString );
	acl_free( (void*)aclString );
	acl_free( acl );
	
	return ret;
#else
	return TQString();
#endif
}

void normal_vfs::vfs_slotRefresh()
{
	krConfig->setGroup("Advanced");
	int maxRefreshFrequency = krConfig->readNumEntry("Max Refresh Frequency", 1000);
	vfs_refresh();
	disconnect( &refreshTimer, TQ_SIGNAL( timeout() ), this, TQ_SLOT( vfs_slotRefresh() ) );
	refreshTimer.start( maxRefreshFrequency, true );
}

bool normal_vfs::burstRefresh(const TQString& path ){
	if( path == vfs_getOrigin().path(-1) ) {
		if( !refreshTimer.isActive() ) {
			// the directory itself is dirty - full refresh is needed
			TQTimer::singleShot(0, this, TQ_SLOT( vfs_slotRefresh() ) ); // safety: dirty signal comes from KDirWatch!
			return true;
		}
		disconnect( &refreshTimer, TQ_SIGNAL( timeout() ), this, TQ_SLOT( vfs_slotRefresh() ) );
		connect( &refreshTimer, TQ_SIGNAL( timeout() ), this, TQ_SLOT( vfs_slotRefresh() ) );
		postponedRefreshURL = fromPathOrURL(path);
		return true;
	}
	return false;
}

void normal_vfs::vfs_slotDirty(const TQString& path){ 
	if( disableRefresh ){
		postponedRefreshURL = fromPathOrURL(path);
		return;
	}
	
	if( burstRefresh( path ) )
		return;        
	
	KURL url = fromPathOrURL(path);
	TQString name = url.fileName();
	
	// do we have it already ?
	if( !vfs_search(name ) ) return vfs_slotCreated(path);
	
	// we have an updated file..
	removeFromList(name);
	vfile* vf = vfileFromName(name);
	addToList(vf);
	emit updatedVfile(vf);
}

void normal_vfs::vfs_slotCreated(const TQString& path){  
	if( disableRefresh ){
		postponedRefreshURL = fromPathOrURL(path);
		return;
	}	
	
	if( burstRefresh( path ) )
		return;        
	
	
	KURL url = fromPathOrURL(path);
	TQString name = url.fileName();	
	// if it's in the CVS - it's an update not new file
	if( vfs_search(name) )
		return vfs_slotDirty(path);
	
	vfile* vf = vfileFromName(name);
	addToList(vf);
	emit addedVfile(vf);	
}

void normal_vfs::vfs_slotDeleted(const TQString& path){ 
	if( disableRefresh ){
		postponedRefreshURL = fromPathOrURL(path);
		return;
	}
	
	if( burstRefresh( path ) )
		return;        
	
	
	KURL url = fromPathOrURL(path);
	TQString name = url.fileName();
	
	// if it's not in the CVS - do nothing
	if( vfs_search(name) ){
		emit deletedVfile(name);
		removeFromList(name);	
	}	
}

#include "normal_vfs.moc"
