/***************************************************************************
                                 temp_vfs.cpp
                             -------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
    web site		 : http://krusader.sourceforge.net
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
// TQt includes
#include <tqdir.h>
// TDE includes
#include <tdemessagebox.h>
#include <kprocess.h>
// Krusader includes
#include "temp_vfs.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../VFS/krarchandler.h"
#include "../resources.h"
#include "../krservices.h"

temp_vfs::temp_vfs( TQString origin, TQString type, TQWidget* panel, bool ):
          normal_vfs(panel){
  vfs_type=TEMP;
   // first we need to create a temp diretory
  tmpDir = krApp->getTempDir();
  // then we must get the files from the origin to the tmp dir
  if( type == "-arj" || type == "-ace" ) handleAceArj(origin,type);
  else if( type == "-rpm" ) handleRpm(origin);
  else if( type == "-iso" ) handleIso(origin);
  else{
  if (!quietMode) KMessageBox::error(krApp,"Unknown temp_vfs type.");
    return;
  }
}

temp_vfs::~temp_vfs(){
	if( tmpvfs_type == ISO ){
		// unmount the ISO image
    KShellProcess umount;
		umount << "umount -f" << tmpDir;
    umount.start(TDEProcess::Block);
	}
 	// delete the temp dir
 	KShellProcess proc;
 	proc << "rm -rf" << tmpDir;
 	proc.start(TDEProcess::DontCare);
}

// return the working dir
TQString temp_vfs::vfs_workingDir(){
  // get the path inside the archive
  TQString path = vfs_origin.path(-1);
  path = path.mid(path.findRev('\\')+1);
  if(path.left(1) != "/") path = "/"+path;
  TQDir().mkdir(tmpDir+path);
  return tmpDir+path;
}

bool temp_vfs::vfs_refresh(const KURL& origin){
  KURL backup = vfs_origin;
  vfs_origin = origin;
  vfs_origin.adjustPath(-1);
  // get the directory...
  TQString path = origin.path(-1).mid(origin.path(-1).findRev('\\')+1);
  if(path.left(1) =="/") path.remove(0,1);
  if ( !normal_vfs::vfs_refresh(tmpDir+"/"+path) ){
    vfs_origin = backup;
    vfs_origin.adjustPath(-1);
    return false;
  }
  return true;
}

void temp_vfs::handleAceArj(TQString origin, TQString type){
    if (type == "-ace") {
	tmpvfs_type = ACE;
    }
    else if (type == "-arj") {
	tmpvfs_type = ARJ;
    }

	// for ace and arj we just unpack to the tmpDir
	if( !KRarcHandler::arcHandled(type) ){
  	if (!quietMode) KMessageBox::error(krApp,"This archive type is NOT supported");
    return;
  }
  else if( !KRarcHandler::unpack(origin,type, TQString(), tmpDir) ){
    return;
  }
}

void temp_vfs::handleRpm(TQString origin){
	// then extract the cpio archive from the rpm
	KShellProcess rpm;
  rpm << "rpm2cpio"<<"\""+origin+"\""+" > "+tmpDir+"/contents.cpio";
  rpm.start(TDEProcess::Block);
	// and write a nice header
	rpm.clearArguments();
	rpm << "rpm -qip"<<"\""+origin+"\""+" > "+tmpDir+"/header.txt";
	rpm.start(TDEProcess::Block);
	// and a file list
	rpm.clearArguments();
	rpm << "rpm -lpq"<<"\""+origin+"\""+" > "+tmpDir+"/filelist.txt";
	rpm.start(TDEProcess::Block);
	tmpvfs_type = RPM;
}

void temp_vfs::handleIso(TQString origin){
	// mount the ISO image
	KShellProcess mount;
	mount << KrServices::fullPathName( "mount" ) << "-o loop" << origin << tmpDir;
	mount.start(TDEProcess::Block);
	tmpvfs_type = ISO;
}
