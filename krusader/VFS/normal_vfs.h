/***************************************************************************
                          normal_vfs.h
                      -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net

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

#ifndef NORMAL_VFS
#define NORMAL_VFS

// TQt includes
#include <tqstring.h>
// TDE includes
#include <tdefileitem.h>
#include <kdirwatch.h>
#include <kurl.h>
// Krusader includes
#include "vfs.h"

/**
 * The normal_vfs class is Kruasder implemention for local directories.
 * As this is the most common case, we try to make it as fast and efficent as possible.
 */
class normal_vfs : public vfs{
	TQ_OBJECT
  
public:
	// the constructor simply uses the inherited constructor
	normal_vfs(TQObject* panel);
 ~normal_vfs(){if( watcher ) delete watcher;}

	/// Copy a file to the vfs (physical).
	virtual void vfs_addFiles(KURL::List *fileUrls,TDEIO::CopyJob::CopyMode mode,TQObject* toNotify,TQString dir = "", PreserveMode pmode = PM_DEFAULT );
	/// Remove a file from the vfs (physical)
	virtual void vfs_delFiles(TQStringList *fileNames);
	/// Return a list of URLs for multiple files
	virtual KURL::List* vfs_getFiles(TQStringList* names);
	/// Return a URL to a single file
	virtual KURL vfs_getFile(const TQString& name);
	/// Create a new directory
	virtual void vfs_mkdir(const TQString& name);
	/// Rename file
	virtual void vfs_rename(const TQString& fileName,const TQString& newName);

	/// return the VFS working dir
	virtual TQString vfs_workingDir() { return vfs_origin.path(-1); }

	/// Get ACL permissions
	static void getACL( vfile *file, TQString &acl, TQString &defAcl );

public slots:
	void vfs_slotRefresh();
	void vfs_slotDirty(const TQString& path);
	void vfs_slotCreated(const TQString& path);
	void vfs_slotDeleted(const TQString& path);

protected:
	/// Re-reads files and stats and fills the vfile list
	virtual bool populateVfsList(const KURL& origin, bool showHidden);

	TQTimer refreshTimer;         //< Timer to exclude sudden refreshes
	KDirWatch *watcher;          //< The internal dir watcher - use to detect changes in directories
	vfile*   vfileFromName(const TQString& name);

private:
	bool burstRefresh( const TQString &path );
	static TQString getACL( const TQString & path, int type );
};

#endif
