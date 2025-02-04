/***************************************************************************
                          virt_vfs.h  -  description
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

#ifndef VIRT_VFS_H
#define VIRT_VFS_H

#include <tdeconfig.h>

#include "vfs.h"

/**
  *@author Shie Erlich & Rafi Yanai
  */

class virt_vfs : public vfs  {
TQ_OBJECT
  
public: 
	virt_vfs(TQObject* panel, bool quiet=false);
	~virt_vfs();
	
	/// Copy a file to the vfs (physical).
	void vfs_addFiles(KURL::List *fileUrls,TDEIO::CopyJob::CopyMode mode,TQObject* toNotify,TQString dir = "",  PreserveMode pmode = PM_DEFAULT );	
	/// Remove a file from the vfs (physical)
	void vfs_delFiles(TQStringList *fileNames);	
	/// Remove a file from the collection (only its link, not the file)
	void vfs_removeFiles(TQStringList *fileNames);	
	/// Return a list of URLs for multiple files	
	KURL::List* vfs_getFiles(TQStringList* names);
	/// Return a URL to a single file	
	KURL vfs_getFile(const TQString& name);
	/// Create a new directory
	void vfs_mkdir(const TQString& name);
	/// Rename file
	void vfs_rename(const TQString& fileName,const TQString& newName);
	/// Calculate the amount of space occupied by a file or directory (recursive).
	virtual void vfs_calcSpace(TQString name ,TDEIO::filesize_t *totalSize,unsigned long *totalFiles,unsigned long *totalDirs, bool * stop);
	
	/// Return the VFS working dir
	TQString vfs_workingDir(){ return TQString(); }
	
protected slots:
	void slotStatResult(TDEIO::Job *job);

protected:
	/// Save the dictionary to file
	bool save();
	/// Restore the dictionary from file
	bool restore();	
	/// return the URLs DB
	TDEConfig*  getVirtDB();

	bool populateVfsList(const KURL& origin, bool showHidden);
	vfile* stat(const KURL& url);
	
	static TQDict<KURL::List> virtVfsDict;
	static TDEConfig* virt_vfs_db;
	bool busy;
	TQString path;
	TDEIO::UDSEntry entry;
};

#endif
