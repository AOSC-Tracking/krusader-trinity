/***************************************************************************
                            panelfunc.cpp
                         -------------------
copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
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
#include <unistd.h>
// TQt Includes
#include <tqdir.h>
#include <tqtextstream.h>
#include <tqeventloop.h>
#include <tqclipboard.h>
// TDE Includes
#include <tdelocale.h>
#include <kprocess.h>
#include <kpropertiesdialog.h>
#include <kopenwith.h>
#include <tdemessagebox.h>
#include <kcursor.h>
#include <kstandarddirs.h>
#include <tdetempfile.h>
#include <kurl.h>
#include <krun.h>
#include <kinputdialog.h>
#include <kdebug.h>
#include <tdeio/netaccess.h>
#include <kstandarddirs.h>
#include <ktempdir.h>
#include <kurlrequester.h>
#include <kprocio.h>
#include <kdesktopfile.h>
// Krusader Includes
#include "panelfunc.h"
#include "krcalcspacedialog.h"
#include "krdetailedview.h"
#include "../krusader.h"
#include "../krslots.h"
#include "../defaults.h"
#include "../VFS/vfile.h"
#include "../VFS/vfs.h"
#include "../VFS/virt_vfs.h"
#include "../VFS/krarchandler.h"
#include "../VFS/krpermhandler.h"
#include "../VFS/krvfshandler.h"
#include "../VFS/preservingcopyjob.h"
#include "../VFS/virtualcopyjob.h"
#include "../Dialogs/packgui.h"
#include "../Dialogs/krdialogs.h"
#include "../Dialogs/krpleasewait.h"
#include "../Dialogs/krspwidgets.h"
#include "../Dialogs/checksumdlg.h"
#include "../KViewer/krviewer.h"
#include "../resources.h"
#include "../krservices.h"
#include "../GUI/syncbrowsebutton.h"
#include "../Queue/queue_mgr.h"
#include "krdrag.h"
#include <kurldrag.h>

//////////////////////////////////////////////////////////
//////          ----------      List Panel -------------                ////////
//////////////////////////////////////////////////////////

ListPanelFunc::ListPanelFunc( ListPanel *parent ) :
panel( parent ), inRefresh( false ), vfsP( 0 ) {
	urlStack.push( "file:/" );
	connect( &delayTimer, TQ_SIGNAL( timeout() ), this, TQ_SLOT( doOpenUrl() ) );
}

void ListPanelFunc::openUrl( const TQString& url, const TQString& nameToMakeCurrent ) {
	openUrl( vfs::fromPathOrURL(
		// KURLRequester is buggy: it should return a string containing "/home/shie/downloads"
		// but it returns "~/downloads" which is parsed incorrectly by vfs::fromPathOrURL.
		// replacedPath should replace ONLY $HOME and environment variables
		panel->origin->completionObject()->replacedPath(url) )
		, nameToMakeCurrent );
}

void ListPanelFunc::immediateOpenUrl( const KURL& urlIn ) {
	KURL url = urlIn;
	url.cleanPath();

	// check for special cases first - don't refresh here !
	// you may call openUrl or vfs_refresh()
	if ( !url.isValid() ) {
		if ( url.url() == "~" ) {
			return openUrl( TQDir::homeDirPath() );
		} else if ( !url.url().startsWith( "/" ) ) {
			// possible relative URL - translate to full URL
			url = files() ->vfs_getOrigin();
			url.addPath( urlIn.url() );
			//kdDebug()<< urlIn.url() << "," << url.url() <<endl;
		} else {
			panel->slotStartUpdate();  // refresh the panel
			return ;
		}
	}

	// if we are not refreshing to current URL
	bool is_equal_url = files() ->vfs_getOrigin().equals( url, true );

	if ( !is_equal_url ) {
		// change the cursor to busy
		panel->setCursor( KCursor::waitCursor() );
	}

	if ( !nameToMakeCurrent.isEmpty() ) {
		panel->view->setNameToMakeCurrent( nameToMakeCurrent );
		// if the url we're refreshing into is the current one, then the
		// partial url will not generate the needed signals to actually allow the
		// view to use nameToMakeCurrent. do it here instead (patch by Thomas Jarosch)
		if ( is_equal_url ) {
		    panel->view->setCurrentItem( nameToMakeCurrent );
		    panel->view->makeItemVisible( panel->view->getCurrentKrViewItem() );
		}
	}

	vfs* v = 0;
	if ( !urlStack.top().equals( url ) )
		urlStack.push( url );
	// count home many urls is in the stack, so later on, we'll know if the refresh was a success
	uint stackSize = urlStack.size();
	bool refreshFailed = true; // assume the worst
	while ( true ) {
		KURL u = urlStack.pop();
		//u.adjustPath(-1); // remove trailing "/"
		u.cleanPath(); // Resolves "." and ".." components in path.
		v = KrVfsHandler::getVfs( u, panel, files() );
		if ( !v )
			continue; //this should not happen !
		if ( v != vfsP ) {
			if( vfsP->vfs_canDelete() )
				delete vfsP;
			else {
				connect( vfsP, TQ_SIGNAL( deleteAllowed() ), vfsP, TQ_SLOT( deleteLater() ) );
				vfsP->vfs_requestDelete();
			}
			vfsP = v; // v != 0 so this is safe
		} else {
			if( vfsP->vfs_isBusy() )
			{
				delayURL = url;               /* this function is useful for FTP url-s and bookmarks */
				delayTimer.start( 100, true );  /* if vfs is busy try refreshing later */
				return;
			}
		}
		connect( files(), TQ_SIGNAL(startJob(TDEIO::Job* )),
				panel, TQ_SLOT(slotJobStarted(TDEIO::Job* )));
		if ( vfsP->vfs_refresh( u ) ) {
			break; // we have a valid refreshed URL now
		}
		if ( vfsP == 0 )  // the object was deleted during vfs_refresh? Hoping the best...
			return;
		// prevent repeated error messages
		if ( vfsP->vfs_isDeleting() )
			break;
		vfsP->vfs_setQuiet( true );
	}
	vfsP->vfs_setQuiet( false );

	// if we popped exactly 1 url from the stack, it means the url we were
	// given was refreshed successfully.
	if (stackSize == urlStack.size() + 1)
		refreshFailed = false;

	// update the urls stack
	if ( !files() ->vfs_getOrigin().equals( urlStack.top() ) ) {
		urlStack.push( files() ->vfs_getOrigin() );
	}
	// disconnect older signals
	disconnect( files(), TQ_SIGNAL( addedVfile( vfile* ) ), 0, 0 );
	disconnect( files(), TQ_SIGNAL( updatedVfile( vfile* ) ), 0, 0 );
	disconnect( files(), TQ_SIGNAL( deletedVfile( const TQString& ) ), 0, 0 );
	disconnect( files(), TQ_SIGNAL( cleared() ), 0, 0 );
	// connect to the vfs's dirwatch signals
	connect( files(), TQ_SIGNAL( addedVfile( vfile* ) ),
	         panel, TQ_SLOT( slotItemAdded( vfile* ) ) );
	connect( files(), TQ_SIGNAL( updatedVfile( vfile* ) ),
	         panel, TQ_SLOT( slotItemUpdated( vfile* ) ) );
	connect( files(), TQ_SIGNAL( deletedVfile( const TQString& ) ),
	         panel, TQ_SLOT( slotItemDeleted( const TQString& ) ) );
	connect( files(), TQ_SIGNAL( cleared() ),
	         panel, TQ_SLOT( slotCleared() ) );

	// on local file system change the working directory
	if ( files() ->vfs_getType() == vfs::NORMAL )
		chdir( files() ->vfs_getOrigin().path().local8Bit() );

	// see if the open url operation failed, and if so,
	// put the attempted url in the origin bar and let the user change it
	if (refreshFailed) {
		panel->origin->setURL(urlIn.prettyURL());
		panel->origin->setFocus();
	}
}

void ListPanelFunc::openUrl( const KURL& url, const TQString& nameToMakeCurrent ) {
	panel->inlineRefreshCancel();
	// first the other dir, then the active! Else the focus changes and the other becomes active
	if ( panel->syncBrowseButton->state() == SYNCBROWSE_CD ) {
		// prevents that the sync-browsing circles itself to death
		static bool inSync = false;
		if( ! inSync ){
			inSync = true;
			//do sync-browse stuff....
			KURL otherDir = OTHER_PANEL->virtualPath();
			OTHER_FUNC->files() ->vfs_setQuiet( true );
			// the trailing slash is nessesary because krusader provides Dir's without it
			// we can't use openUrl because the delay don't allow a check if the panel has realy changed!
			OTHER_FUNC->immediateOpenUrl( KURL::relativeURL( panel->virtualPath().url() + "/", url.url() ) );
			OTHER_FUNC->files() ->vfs_setQuiet( false );
			// now we need to test ACTIVE_PANEL because the openURL has changed the active panel!!
			if ( ACTIVE_PANEL->virtualPath().equals( otherDir ) ) {
				// deactivating the sync-browse if syncbrowse not possible
				panel->syncBrowseButton->setOn( false );
			}
			inSync = false;
		}
	}
	this->nameToMakeCurrent = nameToMakeCurrent;
	delayURL = url;               /* this function is useful for FTP url-s and bookmarks */
	delayTimer.start( 0, true );  /* to avoid tqApp->processEvents() deadlock situaltion */
}

void ListPanelFunc::refresh() {
	openUrl(panel->virtualPath()); // re-read the files
}

void ListPanelFunc::doOpenUrl() {
	immediateOpenUrl( delayURL );
}

void ListPanelFunc::goBack() {
	if ( urlStack.isEmpty() )
		return ;

	if ( urlStack.top().equals( files() ->vfs_getOrigin() ) )
		urlStack.pop();
	openUrl( urlStack.top(), files() ->vfs_getOrigin().fileName() );

	if ( urlStack.isEmpty() )
		krBack->setEnabled( false );
}

void ListPanelFunc::redirectLink() {
	if ( files() ->vfs_getType() != vfs::NORMAL ) {
		KMessageBox::sorry( krApp, i18n( "You can edit links only on local file systems" ) );
		return ;
	}

	vfile *vf = files() ->vfs_search( panel->getCurrentName() );
	if ( !vf )
		return ;

	TQString file = files() ->vfs_getFile( vf->vfile_getName() ).path( -1 );
	TQString currentLink = vf->vfile_getSymDest();
	if ( currentLink.isEmpty() ) {
		KMessageBox::sorry( krApp, i18n( "The current file is not a link, so I can't redirect it." ) );
		return ;
	}

	// ask the user for a new destination
	bool ok = false;
	TQString newLink =
	    KInputDialog::getText( i18n( "Link Redirection" ),
	                           i18n( "Please enter the new link destination:" ), currentLink, &ok, krApp );

	// if the user canceled - quit
	if ( !ok || newLink == currentLink )
		return ;
	// delete the current link
	if ( unlink( file.local8Bit() ) == -1 ) {
		KMessageBox::sorry( krApp, i18n( "Can't remove old link: " ) + file );
		return ;
	}
	// try to create a new symlink
	if ( symlink( newLink.local8Bit(), file.local8Bit() ) == -1 ) {
		KMessageBox:: /* --=={ Patch by Heiner <h.eichmann@gmx.de> }==-- */sorry( krApp, i18n( "Failed to create a new link: " ) + file );
		return ;
	}
}

void ListPanelFunc::krlink( bool sym ) {
	if ( files() ->vfs_getType() != vfs::NORMAL ) {
		KMessageBox::sorry( krApp, i18n( "You can create links only on local file systems" ) );
		return ;
	}

	TQString name = panel->getCurrentName();

	// ask the new link name..
	bool ok = false;
	TQString linkName =
	    KInputDialog::getText( i18n( "New link" ), i18n( "Create a new link to: " ) + name, name, &ok, krApp );

	// if the user canceled - quit
	if ( !ok || linkName == name )
		return ;

	// if the name is already taken - quit
	if ( files() ->vfs_search( linkName ) != 0 ) {
		KMessageBox::sorry( krApp, i18n( "A directory or a file with this name already exists." ) );
		return ;
	}

	if ( linkName.left( 1 ) != "/" )
		linkName = files() ->vfs_workingDir() + "/" + linkName;

	if ( linkName.contains( "/" ) )
		name = files() ->vfs_getFile( name ).path( -1 );

	if ( sym ) {
		if ( symlink( name.local8Bit(), linkName.local8Bit() ) == -1 )
			KMessageBox::sorry( krApp, i18n( "Failed to create a new symlink: " ) + linkName +
			                    i18n( " To: " ) + name );
	} else {
		if ( link( name.local8Bit(), linkName.local8Bit() ) == -1 )
			KMessageBox::sorry( krApp, i18n( "Failed to create a new link: " ) + linkName +
			                    i18n( " To: " ) + name );
	}
}

void ListPanelFunc::view() {
	TQString fileName = panel->getCurrentName();
	if ( fileName.isNull() )
		return ;

	// if we're trying to view a directory, just exit
	vfile * vf = files() ->vfs_search( fileName );
	if ( !vf || vf->vfile_isDir() )
		return ;
	if ( !vf->vfile_isReadable() ) {
		KMessageBox::sorry( 0, i18n( "No permissions to view this file." ) );
		return ;
	}
	// call KViewer.
	KrViewer::view( files() ->vfs_getFile( fileName ) );
	// nothing more to it!
}

void ListPanelFunc::terminal() {
	TQString save = getcwd( 0, 0 );
	chdir( panel->realPath().local8Bit() );

	TDEProcess proc;
	krConfig->setGroup( "General" );
	TQString term = krConfig->readEntry( "Terminal", _Terminal );
	proc << KrServices::separateArgs( term );

	if ( term.contains( "konsole" ) )    /* KDE 3.2 bug (konsole is killed by pressing Ctrl+C) */
	{                                  /* Please remove the patch if the bug is corrected */
		proc << "&";
		proc.setUseShell( true );
	}

	if ( !proc.start( TDEProcess::DontCare ) )
		KMessageBox::sorry( krApp, i18n( "<qt>Can't open <b>%1</b></qt>" ).arg(term) );

	chdir( save.local8Bit() );
}

void ListPanelFunc::editFile() {
	TQString name = panel->getCurrentName();
	if ( name.isNull() )
		return ;

	if ( files() ->vfs_search( name ) ->vfile_isDir() ) {
		KMessageBox::sorry( krApp, i18n( "You can't edit a directory" ) );
		return ;
	}

	if ( !files() ->vfs_search( name ) ->vfile_isReadable() ) {
		KMessageBox::sorry( 0, i18n( "No permissions to edit this file." ) );
		return ;
	}

	KrViewer::edit( files() ->vfs_getFile( name ) );
}

void ListPanelFunc::moveFiles() {
	PreserveMode pmode = PM_DEFAULT;

	TQStringList fileNames;
	panel->getSelectedNames( &fileNames );
	if ( fileNames.isEmpty() )
		return ;  // safety

	KURL dest = panel->otherPanel->virtualPath();
	KURL virtualBaseURL;

	TQString destProtocol = dest.protocol();
	if ( destProtocol == "krarc" || destProtocol == "tar" || destProtocol == "zip" ) {
		KMessageBox::sorry( krApp, i18n( "Moving into archive is disabled" ) );
		return ;
	}

	krConfig->setGroup( "Advanced" );
	if ( krConfig->readBoolEntry( "Confirm Move", _ConfirmMove ) ) {
		bool preserveAttrs = krConfig->readBoolEntry( "PreserveAttributes", _PreserveAttributes );
		TQString s;

  if( fileNames.count() == 1 )
    s = i18n("Move %1 to:").arg(fileNames.first());
  else
    s = i18n("Move %n file to:", "Move %n files to:", fileNames.count());

		// ask the user for the copy dest
		virtualBaseURL = getVirtualBaseURL();
		dest = KChooseDir::getDir(s, dest, panel->virtualPath(), preserveAttrs, virtualBaseURL);
		if ( dest.isEmpty() ) return ; // the user canceled
		if( preserveAttrs )
			pmode = PM_PRESERVE_ATTR;
		else
			pmode = PM_NONE;
	}

	if ( fileNames.isEmpty() )
		return ; // nothing to copy

	KURL::List* fileUrls = files() ->vfs_getFiles( &fileNames );

	// after the delete return the cursor to the first unmarked
	// file above the current item;
	panel->prepareToDelete();

	if( !virtualBaseURL.isEmpty() ) {
		// keep the directory structure for virtual paths
		VirtualCopyJob *vjob = new VirtualCopyJob( &fileNames, files(), dest, virtualBaseURL, pmode, TDEIO::CopyJob::Move, false, true );
		connect( vjob, TQ_SIGNAL( result( TDEIO::Job* ) ), this, TQ_SLOT( refresh() ) );
		if ( dest.equals( panel->otherPanel->virtualPath(), true ) )
			connect( vjob, TQ_SIGNAL( result( TDEIO::Job* ) ), panel->otherPanel->func, TQ_SLOT( refresh() ) );
	}
	// if we are not moving to the other panel :
	else if ( !dest.equals( panel->otherPanel->virtualPath(), true ) ) {
		// you can rename only *one* file not a batch,
		// so a batch dest must alwayes be a directory
		if ( fileNames.count() > 1 ) dest.adjustPath(1);
		TDEIO::Job* job = PreservingCopyJob::createCopyJob( pmode, *fileUrls, dest, TDEIO::CopyJob::Move, false, true );
		job->setAutoErrorHandlingEnabled( true );
		// refresh our panel when done
		connect( job, TQ_SIGNAL( result( TDEIO::Job* ) ), this, TQ_SLOT( refresh() ) );
		// and if needed the other panel as well
		if ( dest.equals( panel->otherPanel->virtualPath(), true ) )
			connect( job, TQ_SIGNAL( result( TDEIO::Job* ) ), panel->otherPanel->func, TQ_SLOT( refresh() ) );

	} else { // let the other panel do the dirty job
		//check if copy is supported
		if ( !otherFunc() ->files() ->vfs_isWritable() ) {
			KMessageBox::sorry( krApp, i18n( "You can't move files to this file system" ) );
			return ;
		}
		// finally..
		otherFunc() ->files() ->vfs_addFiles( fileUrls, TDEIO::CopyJob::Move, files(), "", pmode );
	}
}

// called from SLOTS to begin the renaming process
void ListPanelFunc::rename() {
	panel->view->renameCurrentItem();
}

// called by signal itemRenamed() from the view to complete the renaming process
void ListPanelFunc::rename( const TQString &oldname, const TQString &newname ) {
	if ( oldname == newname )
		return ; // do nothing
	panel->view->setNameToMakeCurrentIfAdded( newname );
	// as always - the vfs do the job
	files() ->vfs_rename( oldname, newname );
}

void ListPanelFunc::mkdir() {
	// ask the new dir name..
	bool ok = false;
	TQString dirName =
	    KInputDialog::getText( i18n( "New directory" ), i18n( "Directory's name:" ), "", &ok, krApp );

	// if the user canceled - quit
	if ( !ok || dirName.isEmpty() )
		return ;

	TQStringList dirTree = TQStringList::split( "/", dirName );

	for ( TQStringList::Iterator it = dirTree.begin(); it != dirTree.end(); ++it ) {
		// check if the name is already taken
		if ( files() ->vfs_search( *it ) ) {
			// if it is the last dir to be created - quit
			if ( *it == dirTree.last() ) {
				KMessageBox::sorry( krApp, i18n( "A directory or a file with this name already exists." ) );
				return ;
			}
			// else go into this dir
			else {
				immediateOpenUrl( *it );
				continue;
			}
		}

		panel->view->setNameToMakeCurrent( *it );
		// as always - the vfs do the job
		files() ->vfs_mkdir( *it );
		if ( dirTree.count() > 1 )
			immediateOpenUrl( *it );
	} // for
}

KURL ListPanelFunc::getVirtualBaseURL() {
	if( files()->vfs_getType() != vfs::VIRT || otherFunc()->files()->vfs_getType() == vfs::VIRT )
		return KURL();

	TQStringList fileNames;
	panel->getSelectedNames( &fileNames );

	KURL::List* fileUrls = files() ->vfs_getFiles( &fileNames );
	if( fileUrls->count() == 0 )
		return KURL();

	KURL base = (*fileUrls)[ 0 ].upURL();

	if( base.protocol() == "virt" ) // is it a virtual subfolder?
		return KURL();          // --> cannot keep the directory structure

	for( unsigned i=1; i < fileUrls->count(); i++ ) {
		if( base.isParentOf( (*fileUrls)[ i ] ) )
			continue;
		if( base.protocol() != (*fileUrls)[ i ].protocol() )
			return KURL();

		do {
			KURL oldBase = base;
			base = base.upURL();
			if( oldBase.equals( base, true ) )
				return KURL();
			if( base.isParentOf( (*fileUrls)[ i ] ) )
				break;
		}while( true );
	}
	return base;
}

void ListPanelFunc::copyFiles() {
	PreserveMode pmode = PM_DEFAULT;

	TQStringList fileNames;
	panel->getSelectedNames( &fileNames );
	if ( fileNames.isEmpty() )
		return ;  // safety

	KURL dest = panel->otherPanel->virtualPath();
	KURL virtualBaseURL;

	// confirm copy
	krConfig->setGroup( "Advanced" );
	if ( krConfig->readBoolEntry( "Confirm Copy", _ConfirmCopy ) ) {
		bool preserveAttrs = krConfig->readBoolEntry( "PreserveAttributes", _PreserveAttributes );
		TQString s;

  if( fileNames.count() == 1 )
    s = i18n("Copy %1 to:").arg(fileNames.first());
  else
    s = i18n("Copy %n file to:", "Copy %n files to:", fileNames.count());

		// ask the user for the copy dest
		virtualBaseURL = getVirtualBaseURL();
		dest = KChooseDir::getDir(s, dest, panel->virtualPath(), preserveAttrs, virtualBaseURL );
		if ( dest.isEmpty() ) return ; // the user canceled
		if( preserveAttrs )
			pmode = PM_PRESERVE_ATTR;
		else
			pmode = PM_NONE;
	}

	KURL::List* fileUrls = files() ->vfs_getFiles( &fileNames );

	if( !virtualBaseURL.isEmpty() ) {
		// keep the directory structure for virtual paths
		VirtualCopyJob *vjob = new VirtualCopyJob( &fileNames, files(), dest, virtualBaseURL, pmode, TDEIO::CopyJob::Copy, false, true );
		connect( vjob, TQ_SIGNAL( result( TDEIO::Job* ) ), this, TQ_SLOT( refresh() ) );
		if ( dest.equals( panel->otherPanel->virtualPath(), true ) )
			connect( vjob, TQ_SIGNAL( result( TDEIO::Job* ) ), panel->otherPanel->func, TQ_SLOT( refresh() ) );
	}
	// if we are  not copying to the other panel :
	else if ( !dest.equals( panel->otherPanel->virtualPath(), true ) ) {
		// you can rename only *one* file not a batch,
		// so a batch dest must alwayes be a directory
		if ( fileNames.count() > 1 ) dest.adjustPath(1);
		TDEIO::Job* job = PreservingCopyJob::createCopyJob( pmode, *fileUrls, dest, TDEIO::CopyJob::Copy, false, true );
		job->setAutoErrorHandlingEnabled( true );
		if ( dest.equals( panel->virtualPath(), true ) ||
			dest.upURL().equals( panel->virtualPath(), true ) )
			// refresh our panel when done
			connect( job, TQ_SIGNAL( result( TDEIO::Job* ) ), this, TQ_SLOT( refresh() ) );
	// let the other panel do the dirty job
	} else {
		//check if copy is supported
		if ( !otherFunc() ->files() ->vfs_isWritable() ) {
			KMessageBox::sorry( krApp, i18n( "You can't copy files to this file system" ) );
			return ;
		}
		// finally..
		otherFunc() ->files() ->vfs_addFiles( fileUrls, TDEIO::CopyJob::Copy, 0, "", pmode );
	}
}

void ListPanelFunc::deleteFiles(bool reallyDelete) {
	// check that the you have write perm
	if ( !files() ->vfs_isWritable() ) {
		KMessageBox::sorry( krApp, i18n( "You do not have write permission to this directory" ) );
		return ;
	}

	// first get the selected file names list
	TQStringList fileNames;
	panel->getSelectedNames( &fileNames );
	if ( fileNames.isEmpty() )
		return ;

	krConfig->setGroup( "General" );
	bool trash = krConfig->readBoolEntry( "Move To Trash", _MoveToTrash );
	// now ask the user if he want to delete:
	krConfig->setGroup( "Advanced" );
	if ( krConfig->readBoolEntry( "Confirm Delete", _ConfirmDelete ) ) {
		TQString s, b;

		if ( !reallyDelete && trash && files() ->vfs_getType() == vfs::NORMAL ) {
			s = i18n( "Do you really want to move this item to the trash?", "Do you really want to move these %n items to the trash?", fileNames.count() );
			b = i18n( "&Trash" );
		} else if( files() ->vfs_getType() == vfs::VIRT && files()->vfs_getOrigin().equals( KURL("virt:/"), true ) ) {
			s = i18n( "Do you really want to delete this virtual item (physical files stay untouched)?", "Do you really want to delete these virtual items (physical files stay untouched)?", fileNames.count() );
			b = i18n( "&Delete" );
		} else if( files() ->vfs_getType() == vfs::VIRT ) {
			s = i18n( "<qt>Do you really want to delete this item <b>physically</b> (not just removing it from the virtual items)?</qt>", "<qt>Do you really want to delete these %n items <b>physically</b> (not just removing them from the virtual items)?</qt>", fileNames.count() );
			b = i18n( "&Delete" );
		} else {
			s = i18n( "Do you really want to delete this item?", "Do you really want to delete these %n items?", fileNames.count() );
			b = i18n( "&Delete" );
		}

		// show message
		// note: i'm using continue and not yes/no because the yes/no has cancel as default button
		if ( KMessageBox::warningContinueCancelList( krApp, s, fileNames,
                                                     i18n( "Warning" ), b ) != KMessageBox::Continue )
			return ;
	}
	//we want to warn the user about non empty dir
	// and files he don't have permission to delete
	krConfig->setGroup( "Advanced" );
	bool emptyDirVerify = krConfig->readBoolEntry( "Confirm Unempty Dir", _ConfirmUnemptyDir );
	emptyDirVerify = ( ( emptyDirVerify ) && ( files() ->vfs_getType() == vfs::NORMAL ) );

	TQDir dir;
	for ( TQStringList::Iterator name = fileNames.begin(); name != fileNames.end(); ) {
		vfile * vf = files() ->vfs_search( *name );

		// verify non-empty dirs delete... (only for normal vfs)
		if ( emptyDirVerify && vf->vfile_isDir() && !vf->vfile_isSymLink() ) {
			dir.setPath( panel->virtualPath().path() + "/" + ( *name ) );
			if ( dir.entryList(TQDir::All | TQDir::System | TQDir::Hidden ).count() > 2 ) {
				switch ( KMessageBox::warningYesNoCancel( krApp,
																		i18n( "<qt><p>Directory <b>%1</b> is not empty!</p><p>Skip this one or Delete All?</p></qt>" ).arg(*name),
																		TQString(), i18n( "&Skip" ), i18n( "&Delete All" ) ) ) {
						case KMessageBox::Cancel :
						return ;
						case KMessageBox::No :
						emptyDirVerify = false;
						break;
						case KMessageBox::Yes :
						name = fileNames.remove( name );
						continue;
				}
			}
		}
		++name;
	}

	if ( fileNames.count() == 0 )
		return ;  // nothing to delete

	// after the delete return the cursor to the first unmarked
	// file above the current item;
	panel->prepareToDelete();

	// let the vfs do the job...
	if (reallyDelete) {
		// if reallyDelete, then make sure nothing gets moved to trash
		krConfig->setGroup("General");
		krConfig->writeEntry( "Move To Trash", false );
	}
	files() ->vfs_delFiles( &fileNames );
	if (reallyDelete) {
		krConfig->setGroup("General");
		krConfig->writeEntry( "Move To Trash", trash);
	}
}

// this is done when you double click on a file
void ListPanelFunc::execute( TQString& name ) {
	if ( name == ".." ) {
		dirUp();
		return ;
	}
	vfile *vf = files() ->vfs_search( name );
	if ( vf == 0 )
		return ;

	KURL origin = files() ->vfs_getOrigin();

	TQString protocol = origin.isLocalFile() ? KrServices::registerdProtocol( vf->vfile_getMime() ) : "";
	if ( protocol == "tar" || protocol == "krarc" ) {
		bool encrypted;
		TQString type = KRarcHandler::getType( encrypted, vf->vfile_getUrl().path(), vf->vfile_getMime(), false );
		if ( !KRarcHandler::arcHandled( type ) )   // if the specified archive is disabled delete the protocol
			protocol = "";
	}

	if ( vf->vfile_isDir() ) {
		origin = files() ->vfs_getFile( name );
		panel->view->setNameToMakeCurrent( TQString() );
		openUrl( origin );
	} else if ( !protocol.isEmpty() ) {
		KURL path = files() ->vfs_getFile( vf->vfile_getName() );
		path.setProtocol( protocol );
		openUrl( path );
	} else {
		KURL url = files() ->vfs_getFile( name );
		KFileItem kfi( vf->vfile_getEntry(), url,true );
		kfi.run();
	}
}

void ListPanelFunc::dirUp() {
	openUrl( files() ->vfs_getOrigin().upURL(), files() ->vfs_getOrigin().fileName() );
}

void ListPanelFunc::pack() {
	TQStringList fileNames;
	panel->getSelectedNames( &fileNames );
	if ( fileNames.isEmpty() )
		return ;  // safety

	if ( fileNames.count() == 0 )
		return ; // nothing to pack

	// choose the default name
	TQString defaultName = panel->virtualPath().fileName();
	if ( defaultName == "" )
		defaultName = "pack";
	if ( fileNames.count() == 1 )
		defaultName = fileNames.first();
	// ask the user for archive name and packer
	new PackGUI( defaultName, vfs::pathOrURL( panel->otherPanel->virtualPath(), -1 ), fileNames.count(), fileNames.first() );
	if ( PackGUI::type == TQString() )
		return ; // the user canceled

	// check for partial URLs
	if( !PackGUI::destination.contains(":/") && !PackGUI::destination.startsWith("/") ){
		PackGUI::destination = panel->virtualPath().prettyURL()+"/"+PackGUI::destination;
	}

	TQString destDir = PackGUI::destination;
	if( !destDir.endsWith( "/" ) )
		destDir += "/";

	bool packToOtherPanel = ( destDir == panel->otherPanel->virtualPath().prettyURL(1) );

	// on remote URL-s first pack into a temp file then copy to its right place
	KURL destURL = vfs::fromPathOrURL( destDir + PackGUI::filename + "." + PackGUI::type );
	KTempFile *tempDestFile = 0;
	TQString arcFile;
	if ( destURL.isLocalFile() )
		arcFile = destURL.path();
	else if( destURL.protocol() == "virt" ) {
		KMessageBox::error( krApp, i18n( "Cannot pack files onto a virtual destination!" ) );
		return;
	}
	else {
		tempDestFile = new KTempFile( TQString(), "." + PackGUI::type );
		tempDestFile->setAutoDelete( true );
		arcFile = tempDestFile->name();
		TQFile::remove
			( arcFile );
	}

	if (  TQFileInfo( arcFile ).exists() ) {
		TQString msg = i18n( "<qt><p>The archive <b>%1.%2</b> already exists. Do you want to overwrite it?</p><p>All data in the previous archive will be lost!</p></qt>").arg(PackGUI::filename).arg(PackGUI::type);
		if( PackGUI::type == "zip" ) {
			msg = i18n( "<qt><p>The archive <b>%1.%2</b> already exists. Do you want to overwrite it?</p><p>Zip will replace identically named entries in the zip archive or add entries for new names.</p></qt>").arg(PackGUI::filename).arg(PackGUI::type);
		}
		if ( KMessageBox::warningContinueCancel( krApp,msg,TQString(),i18n( "&Overwrite" ))
		        == KMessageBox::Cancel )
			return ; // stop operation
	}
	// tell the user to wait
	krApp->startWaiting( i18n( "Counting files to pack" ), 0, true );

	// get the files to be packed:
	files() ->vfs_getFiles( &fileNames );

	TDEIO::filesize_t totalSize = 0;
	unsigned long totalDirs = 0, totalFiles = 0;
	if( !calcSpace( fileNames, totalSize, totalFiles, totalDirs ) )
		return;

	// download remote URL-s if necessary
	TQString arcDir;
	KTempDir *tempDir = 0;

	if ( files() ->vfs_getOrigin().isLocalFile() )
		arcDir = files() ->vfs_workingDir();
	else {
		tempDir = new KTempDir();
		tempDir->setAutoDelete( true );
		arcDir = tempDir->name();
		KURL::List *urlList = files() ->vfs_getFiles( &fileNames );
		TDEIO::NetAccess::dircopy( *urlList, vfs::fromPathOrURL( arcDir ), 0 );
		delete urlList;
	}

	// pack the files
	// we must chdir() first because we supply *names* not URL's
	TQString save = getcwd( 0, 0 );
	chdir( arcDir.local8Bit() );
	KRarcHandler::pack( fileNames, PackGUI::type, arcFile, totalFiles + totalDirs, PackGUI::extraProps );
	chdir( save.local8Bit() );

	// delete the temporary directory if created
	if ( tempDir )
		delete tempDir;

	// copy from the temp file to it's right place
	if ( tempDestFile ) {
		TDEIO::NetAccess::file_move( vfs::fromPathOrURL( arcFile ), destURL );
		delete tempDestFile;
	}

	if ( packToOtherPanel )
		panel->otherPanel->func->refresh();
}

void ListPanelFunc::testArchive() {
	TQString arcName = panel->getCurrentName();
	if ( arcName.isNull() )
		return ;
	if ( arcName == ".." )
		return ; // safety

	KURL arcURL = files() ->vfs_getFile( arcName );
	TQString url = TQString();

	// download the file if it's on a remote filesystem
	if ( !arcURL.isLocalFile() ) {
		url = locateLocal( "tmp", TQString( arcName ) );
		if ( !TDEIO::NetAccess::download( arcURL, url, 0 ) ) {
			KMessageBox::sorry( krApp, i18n( "Krusader is unable to download: " ) + arcURL.fileName() );
			return ;
		}
	} else
		url = arcURL.path( -1 );

	TQString mime = files() ->vfs_search( arcName ) ->vfile_getMime();
	bool encrypted = false;
	TQString type = KRarcHandler::getType( encrypted, url, mime );

	// check we that archive is supported
	if ( !KRarcHandler::arcSupported( type ) ) {
		KMessageBox::sorry( krApp, i18n( "%1, unknown archive type." ).arg( arcName ) );
		return ;
	}

	TQString password = encrypted ? KRarcHandler::getPassword( url ) : TQString();

	// test the archive
	if ( KRarcHandler::test( url, type, password ) )
		KMessageBox::information( krApp, i18n( "%1, test passed." ).arg( arcName ) );
	else
		KMessageBox::error( krApp, i18n( "%1, test failed!" ).arg( arcName ) );

	// remove the downloaded file if necessary
	if ( url != arcURL.path( -1 ) )
		TQFile( url ).remove();
}

void ListPanelFunc::unpack() {
	TQStringList fileNames;
	panel->getSelectedNames( &fileNames );
	if ( fileNames.isEmpty() )
		return ;  // safety

	TQString s;
  if(fileNames.count() == 1)
    s = i18n("Unpack %1 to:").arg(fileNames[0]);
  else
    s = i18n("Unpack %n file to:", "Unpack %n files to:", fileNames.count());

	// ask the user for the copy dest
	KURL dest = KChooseDir::getDir(s, panel->otherPanel->virtualPath(), panel->virtualPath());
	if ( dest.isEmpty() ) {
		return ; // the user canceled
	}

	bool packToOtherPanel = ( dest.equals( panel->otherPanel->virtualPath(), true ) );

	for ( unsigned int i = 0; i < fileNames.count(); ++i ) {
		TQString arcName = fileNames[ i ];
		if ( arcName.isNull() )
			return ;
		if ( arcName == ".." )
			return ; // safety

		// download the file if it's on a remote filesystem
		KURL arcURL = files() ->vfs_getFile( arcName );
		TQString url = TQString();
		if ( !arcURL.isLocalFile() ) {
			url = locateLocal( "tmp", TQString( arcName ) );
			if ( !TDEIO::NetAccess::download( arcURL, url, 0 ) ) {
				KMessageBox::sorry( krApp, i18n( "Krusader is unable to download: " ) + arcURL.fileName() );
				continue;
			}
		} else
			url = arcURL.path( -1 );

    // for local destionation, check whether it exists or not
		if ( dest.isLocalFile() ) {
			TQDir destdir = TQDir(dest.path(1));
			if (!destdir.exists()) {
				// Destination folder does not exists
				int res = KMessageBox::warningContinueCancel( NULL,
						i18n("The destination folder does not exist.\nDo you want to create it?"),
						i18n("Create folder"));
				if ( res != KMessageBox::Continue ) {
					return;
				}
				// Create destination folder. If failed, return
				if (!destdir.mkdir(destdir.absPath())) {
					KMessageBox::error(NULL, i18n("Unable to create the destionation folder. Aborting operation."), i18n("Error!"));
				}
			}
    }
    
		// if the destination is in remote directory use temporary one instead
		dest.adjustPath(1);
		KURL originalDestURL;
		KTempDir *tempDir = 0;

		if ( !dest.isLocalFile() ) {
			originalDestURL = dest;
			tempDir = new KTempDir();
			tempDir->setAutoDelete( true );
			dest = tempDir->name();
		}

		// determining the type
		TQString mime = files() ->vfs_search( arcName ) ->vfile_getMime();
		bool encrypted = false;
		TQString type = KRarcHandler::getType( encrypted, url, mime );

		// check we that archive is supported
		if ( !KRarcHandler::arcSupported( type ) ) {
			KMessageBox::sorry( krApp, i18n( "%1, unknown archive type" ).arg( arcName ) );
			continue;
		}

		TQString password = encrypted ? KRarcHandler::getPassword( url ) : TQString();

		// unpack the files
		KRarcHandler::unpack( url, type, password, dest.path( -1 ) );

		// remove the downloaded file if necessary
		if ( url != arcURL.path( -1 ) )
			TQFile( url ).remove();

		// copy files to the destination directory at remote files
		if ( tempDir ) {
			TQStringList nameList = TQDir( dest.path( -1 ) ).entryList();
			KURL::List urlList;
			for ( unsigned int i = 0; i != nameList.count(); i++ )
				if ( nameList[ i ] != "." && nameList[ i ] != ".." )
					urlList.append( vfs::fromPathOrURL( dest.path( 1 ) + nameList[ i ] ) );
			if ( urlList.count() > 0 )
				TDEIO::NetAccess::dircopy( urlList, originalDestURL, 0 );
			delete tempDir;
		}
	}
	if ( packToOtherPanel )
		panel->otherPanel->func->refresh();
}

// a small ugly function, used to prevent duplication of EVERY line of
// code (maybe except 3) from createChecksum and matchChecksum
static void checksum_wrapper(ListPanel *panel, TQStringList& args, bool &folders) {
	KrViewItemList items;
	panel->view->getSelectedKrViewItems( &items );
	if ( items.isEmpty() ) return ; // nothing to do
	// determine if we need recursive mode (md5deep)
	folders=false;
	for ( KrViewItemList::Iterator it = items.begin(); it != items.end(); ++it ) {
		if (panel->func->getVFile(*it)->vfile_isDir()) {
			folders = true;
			args << (*it)->name();
		} else args << (*it)->name();
	}
}

void ListPanelFunc::createChecksum() {
	TQStringList args;
	bool folders;
	checksum_wrapper(panel, args, folders);
	CreateChecksumDlg dlg(args, folders, panel->realPath());
}

void ListPanelFunc::matchChecksum() {
	TQStringList args;
	bool folders;
	checksum_wrapper(panel, args, folders);
	TQValueList<vfile*> checksumFiles = files()->vfs_search(
		KRQuery(MatchChecksumDlg::checksumTypesFilter)
	);
	MatchChecksumDlg dlg(args, folders, panel->realPath(),
		(checksumFiles.size()==1 ? checksumFiles[0]->vfile_getUrl().prettyURL() : TQString()));
}

void ListPanelFunc::calcSpace() {
	TQStringList items;
	panel->view->getSelectedItems( &items );
	if ( items.isEmpty() ) {
		panel->view->selectAllIncludingDirs();
		panel->view->getSelectedItems( &items );
		if ( items.isEmpty() )
			return ; // nothing to do
	}

	KrCalcSpaceDialog calc( krApp, panel, items, false );
	calc.exec();
	for ( TQStringList::ConstIterator it = items.begin(); it != items.end(); ++it ) {
		KrViewItem *viewItem = panel->view->findItemByName( *it );
		if ( viewItem )
			panel->view->updateItem(viewItem);
	}
	panel->slotUpdateTotals();
}

bool ListPanelFunc::calcSpace( const TQStringList & items, TDEIO::filesize_t & totalSize, unsigned long & totalFiles, unsigned long & totalDirs ) {
	KrCalcSpaceDialog calc( krApp, panel, items, true );
	calc.exec();
	totalSize = calc.getTotalSize();
	totalFiles = calc.getTotalFiles();
	totalDirs = calc.getTotalDirs();
	return !calc.wasCanceled();
}

void ListPanelFunc::FTPDisconnect() {
	// you can disconnect only if connected !
	if ( files() ->vfs_getType() == vfs::FTP ) {
		krFTPDiss->setEnabled( false );
		panel->view->setNameToMakeCurrent( TQString() );
		openUrl( panel->realPath() ); // open the last local URL
	}
}

void ListPanelFunc::newFTPconnection() {
	KURL url = KRSpWidgets::newFTP();
	// if the user canceled - quit
	if ( url.isEmpty() )
		return ;

	krFTPDiss->setEnabled( true );
	openUrl( url );
}

void ListPanelFunc::properties() {
	TQStringList names;
	panel->getSelectedNames( &names );
	if ( names.isEmpty() )
		return ;  // no names...
	KFileItemList fi;
	fi.setAutoDelete( true );

	for ( unsigned int i = 0 ; i < names.count() ; ++i ) {
		vfile* vf = files() ->vfs_search( names[ i ] );
		if ( !vf )
			continue;
		KURL url = files()->vfs_getFile( names[i] );
		fi.append( new KFileItem( vf->vfile_getEntry(), url ) );
	}

	if ( fi.isEmpty() )
		return ;

	// Show the properties dialog
	KPropertiesDialog *dlg = new KPropertiesDialog( fi );
	connect( dlg, TQ_SIGNAL( applied() ), SLOTS, TQ_SLOT( refresh() ) );
}

void ListPanelFunc::refreshActions() {
	vfs::VFS_TYPE vfsType = files() ->vfs_getType();

	//  set up actions
	krMultiRename->setEnabled( vfsType == vfs::NORMAL );  // batch rename
	//krProperties ->setEnabled( vfsType == vfs::NORMAL || vfsType == vfs::FTP ); // file properties
	krFTPDiss ->setEnabled( vfsType == vfs::FTP );     // disconnect an FTP session
	krCreateCS->setEnabled( vfsType == vfs::NORMAL );
	/*
	  krUnpack->setEnabled(true);                            // unpack archive
	  krTest->setEnabled(true);                              // test archive
	  krSelect->setEnabled(true);                            // select a group by filter
	  krSelectAll->setEnabled(true);                         // select all files
	  krUnselect->setEnabled(true);                          // unselect by filter
	  krUnselectAll->setEnabled( true);                      // remove all selections
	  krInvert->setEnabled(true);                            // invert the selection
	  krFTPConnect->setEnabled(true);                        // connect to an ftp
	  krFTPNew->setEnabled(true);                            // create a new connection
	  krAllFiles->setEnabled(true);                          // show all files in list
	  krCustomFiles->setEnabled(true);                       // show a custom set of files
	  krBack->setEnabled(func->canGoBack());                 // go back
	  krRoot->setEnabled(true);                              // go all the way up
	      krExecFiles->setEnabled(true);                         // show only executables
	*/
}

ListPanelFunc::~ListPanelFunc() {
	if( !vfsP ) {
		if( vfsP->vfs_canDelete() )
			delete vfsP;
		else {
			connect( vfsP, TQ_SIGNAL( deleteAllowed() ), vfsP, TQ_SLOT( deleteLater() ) );
			vfsP->vfs_requestDelete();
		}
	}
	vfsP = 0;
}

vfs* ListPanelFunc::files() {
	if ( !vfsP )
		vfsP = KrVfsHandler::getVfs( "/", panel, 0 );
	return vfsP;
}


void ListPanelFunc::copyToClipboard( bool move ) {
	if( files()->vfs_getOrigin().equals( KURL("virt:/"), true ) ) {
		if( move )
			KMessageBox::error( krApp, i18n( "Cannot cut a virtual URL collection to the clipboard!" ) );
		else
			KMessageBox::error( krApp, i18n( "Cannot copy a virtual URL collection onto the clipboard!" ) );
		return;
	}

	TQStringList fileNames;

	panel->getSelectedNames( &fileNames );
	if ( fileNames.isEmpty() )
		return ;  // safety

	KURL::List* fileUrls = files() ->vfs_getFiles( &fileNames );
	if ( fileUrls ) {
		KRDrag * urlData = KRDrag::newDrag( *fileUrls, move, krApp->mainView, "krusader" );
		TQApplication::clipboard() ->setData( urlData );

		if( move && files()->vfs_getType() == vfs::VIRT )
			( static_cast<virt_vfs*>( files() ) )->vfs_removeFiles( &fileNames );

		delete fileUrls;
	}
}

void ListPanelFunc::pasteFromClipboard() {
	TQClipboard * cb = TQApplication::clipboard();
	TQMimeSource * data = cb->data();
	KURL::List urls;
	if ( KURLDrag::canDecode( data ) ) {
		KURLDrag::decode( data, urls );
		bool cutSelection = KRDrag::decodeIsCutSelection( data );

		KURL destUrl = panel->virtualPath();

		files()->vfs_addFiles( &urls, cutSelection ? TDEIO::CopyJob::Move : TDEIO::CopyJob::Copy, otherFunc()->files(),
			"", PM_DEFAULT );
	}
}

#include "panelfunc.moc"
