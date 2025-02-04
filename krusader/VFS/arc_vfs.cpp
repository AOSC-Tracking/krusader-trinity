/***************************************************************************
                                arc_vfs.cpp
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
#include <sys/param.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
// TQt includes
#include <tqregexp.h>
#include <tqdir.h>
#include <tqdatetime.h>
#include <tqfileinfo.h>
// TDE includes
#include <tdemessagebox.h>
#include <tdelocale.h>
#include <kprocess.h>
#include <tdeio/jobclasses.h>
#include <tqprogressdialog.h>
#include <tdeglobalsettings.h>
#include <kmimetype.h>
#include <kcursor.h>
#include <klargefile.h>
// krusader includes
#include "arc_vfs.h"
#include "krpermhandler.h"
#include "krarchandler.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../resources.h"
#include "../Dialogs/krdialogs.h"

#define MAX_FILES 500

//constructor
arc_vfs::arc_vfs(TQString origin,TQString type,TQObject* panel,bool write):
  vfs(panel),arcFile(origin),changed(false),prefix(""),ignoreLines(0){

	if ( type == "tarz" ) type = "-tgz";
	
	// set the cursor to busy mode
  if (!quietMode) krApp->setCursor(KCursor::waitCursor());

  // set the writable attribute
	isWritable = KRpermHandler::fileWriteable(origin);
  isWritable = ( write && isWritable ); 	

  vfs_type = vfs::ERROR;

	// create the temp dir..
  tmpDir = krApp->getTempDir();
 	if( tmpDir.isEmpty() ){
  	error = true;
		return;
	}

	TQString password = TQString();
  krConfig->setGroup("Archives");
  // fill the command options
  if( type == "gzip" ){
    cmd = KrServices::fullPathName ( "gzip" );
    listCmd = "-l";
    delCmd  = "";
    addCmd  = KrServices::fullPathName ( "gzip" ) + " -c";
    getCmd  = "-dc";
		ignoreLines = -1;
		isWritable = false;
  }
  if(type == "zip2"){
    cmd = KrServices::fullPathName( "bzip2" );
    listCmd = "";
    delCmd  = "";
    addCmd  = KrServices::fullPathName( "bzip2" )+ " -c";
    getCmd  = "-dc";
		ignoreLines = -1;
    isWritable = false;
  }
  if(type == "-tar"){
    cmd = KrServices::fullPathName( "tar" );
    listCmd = " -tvf";
    delCmd  = cmd+" --delete -vf";
    addCmd  = cmd+" -uvf";
    getCmd  = " -xvf";
  }
	if(type == "-tgz"){
    cmd = KrServices::fullPathName( "tar" );
    listCmd = " -tzvf";
    delCmd  = "";
    addCmd  = cmd+" -uvzf";
    getCmd  = " -xzvf";
    isWritable = false;
  }
  if(type == "-tbz"){
    cmd = KrServices::fullPathName( "tar" );
    listCmd = " -tjvf";
    delCmd  = "";
    addCmd  = cmd+" -uvjf";
    getCmd  = " -xjvf";
    isWritable = false;
  }
	if(type == "-zip"){
    password = KRarcHandler::getPassword(arcFile,type);
		cmd = KrServices::fullPathName( "unzip" );
    listCmd = "-ZTs ";
    TQString zipcmd = KrServices::fullPathName( "zip" );
    delCmd  = zipcmd+" -d";
    addCmd  = zipcmd+" -ry";
    getCmd  = " -o";
		if( !password.isEmpty() ){
    	//listCmd = listCmd + " -P "+password;
			delCmd = delCmd + " -P "+password;
			addCmd = addCmd + " -P "+password;
			getCmd = getCmd + " -P "+password;
		}
    ignoreLines = 1;
  }
  // "-rpm" is used only to list the rpm - to extract files use "+rpm"
  if(type == "-rpm"){
    //rpm can't handle files with " " in them so replace " " with "\ "
    arcFile.replace(TQRegExp(" "),"\\ ");

    cmd = KrServices::fullPathName( "rpm" );
    listCmd = " --dump -lpq ";
    delCmd  = "";
    addCmd  = "";
    getCmd  = "";
    isWritable    = false;
  }
  if( type == "+rpm" ){
		// extract the cpio archive from the rpm
		KShellProcess rpm;
  	rpm << "rpm2cpio"<<"\""+arcFile+"\""+" > "+tmpDir+"/contents.cpio";
  	rpm.start(TDEProcess::Block);
		arcFile = tmpDir+"/contents.cpio";
	}
	if(type == "cpio" || type == "+rpm" ){
    cmd = KrServices::fullPathName( "cpio" );
    listCmd = "-tvF ";
    delCmd  = "";
    addCmd  = "";
    getCmd  = " --force-local --no-absolute-filenames -ivdF";
    isWritable    = false;
  }
  if(type == "-rar"){
    bool doRar = krConfig->readBoolEntry("Do Rar",_DoRar);
    cmd = KrServices::fullPathName( "unrar" );
    listCmd = " -c- v ";
    delCmd  = "";
    addCmd  = (doRar ? TQString(KrServices::fullPathName( "rar" ) + " -r a ") : TQString("")) ;
    getCmd  = " x -y ";
    ignoreLines = 8;
    isWritable    = (doRar && isWritable );
  }

	getDirs();
	// set the cursor to normal mode
  if (!quietMode) krApp->setCursor(KCursor::arrowCursor());
}

// return the working dir
TQString arc_vfs::vfs_workingDir(){
  // get the path inside the archive
  TQString path = vfs_origin.right((vfs_origin.length()-vfs_origin.findRev('\\'))-1);
  if(path.left(1) != "/") path = "/"+path;
  TQDir().mkdir(tmpDir+path);
  return tmpDir+path;
}

arc_vfs::~arc_vfs(){
  // set the cursor to busy mode
  if (!quietMode) krApp->setCursor(KCursor::waitCursor());
	// don't touch messed-up archives
	if(!error) repack();
  // delete the temp dir
  KShellProcess proc;
  proc<<"rm"<<"-rf"<<tmpDir;
  proc.start(TDEProcess::Block);

	// set the cursor to normal mode
  if (!quietMode) krApp->setCursor(KCursor::arrowCursor());
}	

bool arc_vfs::getDirs(){
  if( !listCmd.isEmpty() ){
    // write the temp file
    KShellProcess proc;
    proc << cmd << listCmd << "\""+arcFile+"\"" <<" > " << tmpDir+"/tempfilelist";
    proc.start(TDEProcess::Block);
    if( !proc.normalExit() || !proc.exitStatus() == 0 ){
      if (!quietMode) KMessageBox::error(krApp, i18n("<qt>Can't read <b>%1</b>. Archive "
                      "might be corrupted!</qt>").arg(arcFile.mid(arcFile.findRev('/')+1)));
     error = true;
		 return false;
    }
		
		// clear the dir list
  	dirList.clear();

  	// prepare the first dir entry - the "" entry
  	arc_dir *tempdir = new arc_dir("");
  	vfs_filesP = &(tempdir->entries);
  	dirList.append(tempdir);

    // parse the temp file
    TQFile temp(tmpDir+"/tempfilelist");
    temp.open(IO_ReadOnly);
    char buf[1000];
    TQString line;
    if(vfs_type == "gzip" || vfs_type == "-zip" )
      temp.readLine(line,10000);  // skip the first line - it's garbage
    if( vfs_type == "-rar" ){
      while(temp.readLine(line,10000) != -1)
        if ( line.contains("----------") ) break;
    }
    while(temp.readLine(buf,1000) != -1){
      line = TQString::fromLocal8Bit(buf);
      if ( line.contains("----------") ) break;
      parseLine(line.stripWhiteSpace(),&temp);

    }
    temp.close();
    TQDir().remove(tmpDir+"/tempfilelist");
  }
  else { // bzip2
		// clear the dir list
  	dirList.clear();

  	// prepare the first dir entry - the "" entry
  	arc_dir *tempdir = new arc_dir("");
  	vfs_filesP = &(tempdir->entries);
  	dirList.append(tempdir);

	 	parseLine("",0);
	}
	return true;
}


// copy a file to the vfs (physical)
void arc_vfs::vfs_addFiles(KURL::List *fileUrls,TDEIO::CopyJob::CopyMode mode,TQObject* toNotify,TQString dir, PreserveMode /*pmode*/ ){
  if ( addCmd.isEmpty() ) return;

  // get the path inside the archive
  TQString path = vfs_origin.right((vfs_origin.length()-vfs_origin.findRev('\\'))-1);
  path = path+"/";
  if(dir != "" ) dir = "/"+dir;
  if(path.left(1) != "/") path = "/"+path;

  // make sure the destination exist
  for( int i=0; i >= 0 ; i= TQString(tmpDir+path+dir).find('/',i+1) ){
    TQDir().mkdir(TQString(tmpDir+path+dir).left(i));
  }

  changed = true; //rescan the archive
	
	KURL dest;
	dest.setPath(tmpDir+path+dir);

  TDEIO::Job* job = new TDEIO::CopyJob(*fileUrls,dest,mode,false,true);
  connect(job,TQ_SIGNAL(result(TDEIO::Job*)),this,TQ_SLOT(vfs_refresh(TDEIO::Job*)) );
  if(mode == TDEIO::CopyJob::Move) // notify the other panel
   connect(job,TQ_SIGNAL(result(TDEIO::Job*)),toNotify,TQ_SLOT(vfs_refresh(TDEIO::Job*)) );
}
	

// remove a file from the vfs (physical)
void arc_vfs::vfs_delFiles(TQStringList *fileNames){
  if ( delCmd.isEmpty() ) return;
	// if we move to trash - just extract files and move them to trash -
  // the repack() will delete them for us
  krConfig->setGroup("General");
	if( krConfig->readBoolEntry("Move To Trash",_MoveToTrash) ) {
	  KURL::List* filesUrls = vfs_getFiles(fileNames); // extract
	  changed = true;
	
	  TDEIO::Job *job = new TDEIO::CopyJob(*filesUrls,TDEGlobalSettings::trashPath(),TDEIO::CopyJob::Move,false,true );
	  connect(job,TQ_SIGNAL(result(TDEIO::Job*)),this,TQ_SLOT(vfs_refresh(TDEIO::Job*)));
	}
	// else we have to delete the files from both the archive and the temp dir
	else {
	  // change dir to the temp dir
    TQString save = getcwd(0,0);
    chdir(tmpDir.local8Bit());

    TQStringList files;
    TDEIO::filesize_t totalSizeVal = 0;
    unsigned long totalFilesVal =  0;
    	
	  // names -> urls
    for(TQStringList::Iterator name = fileNames->begin(); name != fileNames->end(); ++name )
      processName(*name,&files,&totalSizeVal,&totalFilesVal);


		KShellProcess proc1 , proc2;
		krApp->startWaiting(i18n("Deleting Files..."),files.count()+ignoreLines);
	  connect(&proc1,TQ_SIGNAL(receivedStdout(TDEProcess*,char*,int)),
            krApp, TQ_SLOT(incProgress(TDEProcess*,char*,int)) );

    proc1 <<  delCmd << "\""+arcFile+"\"";
    proc2 << "rm -rf";
    for(unsigned int i =0; i < files.count(); ){
      proc1 << (prefix+*files.at(i));
      proc2 << tmpDir+"/"+(*files.at(i));
			extFiles.remove(*files.at(i++));
			if ( i%MAX_FILES==0 || i==files.count() ){
				proc1.start(TDEProcess::NotifyOnExit,TDEProcess::AllOutput);
    		proc2.start();
        while( proc1.isRunning() || proc2.isRunning() ) tqApp->processEvents(); // busy wait - need to find something better...
        proc1.clearArguments() ; proc2.clearArguments();
				proc1 <<  delCmd << "\""+arcFile+"\"";
    		proc2 << "rm -rf";
			}
    }
	  krApp->stopWait();

	  changed = true;
	  chdir (save.local8Bit());
	  vfs_refresh(vfs_origin);
  }
}	

// return a path to the file
TQString arc_vfs::vfs_getFile(TQString name){
  // get the current file path
  TQString path = vfs_origin.right((vfs_origin.length()-vfs_origin.findRev('\\'))-1);
  if(path.left(1)=="/") path.remove(0,1);
  if(path != "") path = path+"/";

  TQStringList temp(name);
  vfs_getFiles(&temp);

  return tmpDir+"/"+path+name;
}

KURL::List* arc_vfs::vfs_getFiles(TQStringList* names){
  KURL url;
  KURL::List* urls = new KURL::List();

  // get the current file path
  TQString path = vfs_origin.right((vfs_origin.length()-vfs_origin.findRev('\\'))-1);
  if(path.left(1)=="/") path.remove(0,1);
  if(path != "") path = path+"/";

  // change dir to the temp dir
  TQString save = getcwd(0,0);
	chdir(tmpDir.local8Bit());
	// names -> urls
  TQStringList files;
  TDEIO::filesize_t totalSize = 0;
  unsigned long totalFiles = 0;
  for(TQStringList::Iterator name = names->begin(); name != names->end(); ++name ){
    processName(*name,&files,&totalSize,&totalFiles);
    url.setPath(tmpDir+"/"+path+(*name));
    urls->append(url);
  }
  // check the urls for unpacked files and directories
	for(TQStringList::Iterator file = files.begin(); file != files.end(); ++file ){
  	if ( (*file).right(1)=="/" ){
			TQDir(tmpDir).mkdir(*file);
      if( vfs_type == "-rar" ) file = files.remove(file--);
		}
		// don't unpack the same file twice
		else if( extFiles.contains(*file) ){
      file = files.remove(file--);
		}
  }
  // unpack ( if needed )
	if ( files.count() > 0 ){
		krApp->startWaiting(i18n("Unpacking Files"),files.count()+ignoreLines);
    KShellProcess proc;
	  connect(&proc,TQ_SIGNAL(receivedStdout(TDEProcess*,char*,int)),
            krApp, TQ_SLOT(incProgress(TDEProcess*,char*,int)) );
		
		proc << cmd << getCmd << "\""+arcFile+"\"";
  	if( vfs_type == "gzip" || vfs_type == "zip2" ) proc << ">";
		for(unsigned int i=0 ; i < files.count() ; ){
  		proc << (prefix+*files.at(i++));
			if ( i%MAX_FILES==0 || i==files.count() ){
				proc.start(TDEProcess::NotifyOnExit,TDEProcess::AllOutput);
        while( proc.isRunning() ) tqApp->processEvents();
				proc.clearArguments();
				proc << cmd << getCmd << "\""+arcFile+"\"";
  		}
		}
    getExtFiles(); // this will update the extFiles list.
		krApp->stopWait();
	}
  // restore dir
	chdir(save.local8Bit());
	
  return urls;
}

// make dir
void arc_vfs::vfs_mkdir(TQString name){
  TQString path = vfs_origin.right((vfs_origin.length()-vfs_origin.findRev('\\'))-1);
  if(path.left(1)=="/") path.remove(0,1);
  if(path != "") path = path+"/";

  TQDir(tmpDir).mkdir(path+name);
  changed = true; //rescan the archive
  vfs_refresh(vfs_origin);
}
	
// rename file
void arc_vfs::vfs_rename(TQString fileName,TQString newName){
	KURL::List temp;
	temp.append(vfs_getFile(fileName));
  TQString path = vfs_origin.right((vfs_origin.length()-vfs_origin.findRev('\\'))-1);
  if(path.left(1)=="/") path.remove(0,1);
  if(path != "") path = path+"/";

  TQDir(tmpDir).mkdir(path);
  changed = true; //rescan the archive

	KURL dest;
	dest.setPath(tmpDir+path+"/"+newName);

  TDEIO::Job* job = new TDEIO::CopyJob(temp,dest,TDEIO::CopyJob::Move,false,false);
  connect(job,TQ_SIGNAL(result(TDEIO::Job*)),this,TQ_SLOT(vfs_refresh(TDEIO::Job*)) );
}

bool arc_vfs::vfs_refresh(TQString origin){
	if ( error ) return false;
	
  if ( changed || origin == vfs_origin ){
		repack(); // repack dirs only if needed
		if ( !getDirs() ){
			if (!quietMode) emit startUpdate();
    	return true;
		}
		changed = false;
	}

  vfs_origin = origin;
  // get the directory...
  TQString path = origin.right((origin.length()-origin.findRev('\\'))-1);
  if(path.left(1) =="/") path.remove(0,1);

  vfs_filesP = findDir(path);

  if (!quietMode) emit startUpdate();
  return true;
}

// service functions
TQString arc_vfs::nextWord(TQString &s,char d) {
  s=s.stripWhiteSpace();
  int j=s.find(d,0);
  TQString temp=s.left(j); // find the leftmost word.
  s.remove(0,j);
  return temp;
}

void arc_vfs::getFilesToPack(TQStringList* filesToPack,TQString dir_name){
  bool newDir = false;
  vfileDict *vfs_filesP_backup = vfs_filesP; // save vfs_filesP

  // init all the diffrent lists (and list pointers);
  vfs_filesP=findDir(dir_name);
  if ( vfs_filesP == 0) newDir = true;
  if(dir_name != "") dir_name = dir_name+"/";

 	DIR* dir = opendir(tmpDir.local8Bit()+"/"+dir_name.local8Bit());
  if(!dir) return ;

	struct dirent* dirEnt;
  TQString name;
	KDE_struct_stat stat_p;
	while( (dirEnt=readdir(dir)) != NULL ){
    name = dirEnt->d_name;
		if ( name == ".." || name == "." ) continue;
	  if( KDE_lstat(tmpDir.local8Bit()+"/"+dir_name.local8Bit()+name.local8Bit(),&stat_p) ) continue;
	  extFile temp(dir_name+name,stat_p.st_mtime,stat_p.st_size);
		// add to the list file that are diffrent than the ones packed
    if( S_ISDIR(stat_p.st_mode) ){ // recurse on all sub dirs
      if( !findDir(dir_name+name) ){
      	// add to the list only new && empty dirs
				if( newDir && TQDir(dir_name+name).entryList(TQDir::All | TQDir::AccessMask).count() <= 2 )
					filesToPack->append( dir_name+name);
      }
			getFilesToPack(filesToPack,dir_name+name);
			continue;
		}

		// if the file don't exist add it to the archive and to the extFiles
    if( newDir || !extFiles.contains( dir_name+name ) ){
    	filesToPack->append( dir_name+name );
			extFiles.append( temp );
    } // else if the file exist but was modified - repack it;
    else if( !extFiles.contains( temp ) ){
			filesToPack->append( dir_name+name );
			extFiles.remove( dir_name+name );
			extFiles.append( temp );
    }
  }
  vfs_filesP = vfs_filesP_backup; // restore  vfs_filesP
}

void arc_vfs::getFilesToDelete(TQStringList* filesToDelete,TQString){
	// sync the extFiles - and find out which files were deleted
	TQString file;
	for(unsigned int i=0 ; i<extFiles.count(); ){
		file = tmpDir+"/"+(*extFiles.at(i)).url;
		if( !KRpermHandler::fileExist(file) ){
  		filesToDelete->append( (*extFiles.at(i)).url );
    	extFiles.remove(extFiles.at(i));
   	}
		else ++i;
  }
}

void arc_vfs::getExtFiles(TQString dir_name){
	DIR* dir = opendir(tmpDir.local8Bit()+"/"+dir_name.local8Bit());
  if(!dir){
    kdWarning() << "faild to opendir(): " << tmpDir.local8Bit()+"/"+dir_name.local8Bit() << endl;
		return ;
	}

	if( dir_name != "") dir_name = dir_name+"/";
	
	struct dirent* dirEnt;
  TQString name;
	KDE_struct_stat stat_p;
	while( (dirEnt=readdir(dir)) != NULL ){
    name = dirEnt->d_name;
		if ( name == ".." || name == "." ) continue;
	  if( KDE_lstat(tmpDir.local8Bit()+"/"+dir_name.local8Bit()+name.local8Bit(),&stat_p) ) continue;
	  extFile temp(dir_name+name,stat_p.st_mtime,stat_p.st_size);
		// recurse on all sub dirs
    if( S_ISDIR(stat_p.st_mode) ){
      getExtFiles(dir_name+name);
		}
    // if the file is not in extFiles - it is newly extracted.
    // note: getFilesToPack() updates time + size !
		else if( !extFiles.contains( dir_name+name ) ){
			extFiles.append( temp );
    }
  }	
}

void arc_vfs::repack(){
  TQString save = getcwd(0,0);
  chdir(tmpDir.local8Bit());
	
  // delete from the archive files that were unpacked and deleted
	if( vfs_isWritable() ){
		TQStringList filesToDelete;
		getFilesToDelete(&filesToDelete);
		if( !filesToDelete.isEmpty() ){
			KShellProcess delProc;
  		krApp->startWaiting(i18n("Deleting Files..."),filesToDelete.count()+ignoreLines);
	 		connect(&delProc,TQ_SIGNAL(receivedStdout(TDEProcess*,char*,int)),
               krApp, TQ_SLOT(incProgress(TDEProcess*,char*,int)) );

			delProc << delCmd << "\""+arcFile+"\"";
			for( unsigned int i=0 ; i < filesToDelete.count() ;){
				delProc << (*filesToDelete.at(i++));
				if( i%MAX_FILES==0 || i==filesToDelete.count() ){
  	  		delProc.start(TDEProcess::NotifyOnExit,TDEProcess::AllOutput);
    			while( delProc.isRunning() )  tqApp->processEvents();
					delProc.clearArguments();
		    	delProc << delCmd << "\""+arcFile+"\"";
  			}
			}
  		krApp->stopWait();
		}
	}

  // finaly repack tmpDir
  if( vfs_isWritable() || vfs_type=="gzip" || vfs_type=="zip2" ){
    TQStringList filesToPack;
		getFilesToPack(&filesToPack);
		if( !filesToPack.isEmpty() ){
			KShellProcess addProc;
			krApp->startWaiting(i18n("Repacking..."),filesToPack.count()+ignoreLines);
    	connect(&addProc,TQ_SIGNAL(receivedStdout(TDEProcess*,char*,int)),
            krApp, TQ_SLOT(incProgress(TDEProcess*,char*,int)) );

			if( vfs_type=="gzip" || vfs_type=="zip2" ){
      	addProc << addCmd << *filesToPack.at(0)<< ">" << "\""+arcFile+"\"";
				addProc.start(TDEProcess::NotifyOnExit);
      	while( addProc.isRunning() ) tqApp->processEvents();
     	}
    	else {
				addProc << addCmd << "\""+arcFile+"\"";
    		for( unsigned int i=0 ; i<filesToPack.count(); ){
      		addProc << "\""+prefix+(*filesToPack.at(i++))+"\"";
					if( i%MAX_FILES==0 || i==filesToPack.count() ){
      			addProc.start(TDEProcess::NotifyOnExit,TDEProcess::AllOutput);
      			while( addProc.isRunning() ) tqApp->processEvents(); // busy wait - need to find something better...
        		addProc.clearArguments();
		    		addProc << addCmd << "\""+arcFile+"\"";
					}
    		}
			}
			krApp->stopWait();
		}
 	}
  chdir(save.local8Bit());
}

vfileDict* arc_vfs::findDir(TQString name){
  for(arc_dir* temp = dirList.first();temp != 0 ; temp = dirList.next()){
    if(temp->name == name) return &(temp->entries);
  }
  return 0;
}

arc_vfs::arc_dir* arc_vfs::findArcDir(TQString name){
for(arc_dir* temp = dirList.first();temp != 0 ; temp = dirList.next()){
    if(temp->name == name) return temp;
  }
  return 0;
}

TQString arc_vfs::changeDir(TQString name){
  if(name.left(2) == "./") {
    prefix = "./";
    name.remove(0,2);
  }

  if(!name.contains('/')){
    vfs_filesP = findDir("");
    return name;
  }
  // seperate the path from the name
  TQString path = name.left(name.findRev('/'));
  name = name.mid(name.findRev('/')+1);
  // see if the path exists
  if ((vfs_filesP=findDir(path)) == 0){
   //create a new dir entry
   TQString Pname = path.mid(path.findRev('/')+1);
   if(Pname.isEmpty()) return name;
   TQString tempName = arcFile;
   TQFileInfo qfi(tempName.replace(TQRegExp("\\"),""));
   vfile* vf=new vfile(Pname,0,"drwxr-xr-x",qfi.lastModified().toTime_t(),false,
                 qfi.owner(),qfi.group(),"inode/directory","",0 );
   // add  dirs if needed
   changeDir(path);

   vfile* vf2 = vfs_search(Pname);
   if(vf2 != 0) vfs_removeFromList(vf2);
   vfs_addToList(vf);

   // add a new arc_dir
   dirList.append(new arc_dir(path));
   vfs_filesP = findDir(path);
  }
  return name;
}

// calculate space
void arc_vfs::vfs_calcSpace(TQString name ,TDEIO::filesize_t *totalSize,unsigned long *totalFiles,unsigned long *totalDirs,bool* stop){
  if ( *stop ) return;
  vfile* vf = vfs_search(name);

  // get the path inside the archive
  TQString path = vfs_origin.right((vfs_origin.length()-vfs_origin.findRev('\\'))-1);
  path = path+"/";
  if(path.left(1) == "/") path.remove(0,1);

  if( !vf->vfile_isDir() ){ // single files are simple :)
    ++(*totalFiles);
    (*totalSize) += vf->vfile_getSize();
  }
  else { // handle directories : (
    ++(*totalDirs);

    TQString origin_backup = vfs_origin; // backup the vfs origin
    vfs_origin = vfs_origin+"/"+name;
    vfileDict* vfs_filesP_backup = vfs_filesP; // backup the vfs_filesP
    vfs_filesP = findDir(path+name);

    // process all the files in the directory.
    for( vf = vfs_getFirstFile(); vf != 0; vf = vfs_getNextFile() ){
      if (*stop) return;
      vfs_calcSpace(vf->vfile_getName(),totalSize,totalFiles,totalDirs,stop);
    }

    vfs_origin = origin_backup;     // restore origin
    vfs_filesP = vfs_filesP_backup; // restore vfs_filesP
  }
}

void arc_vfs::processName(const TQString& name, TQStringList *urls,TDEIO::filesize_t *totalSize,unsigned long *totalFiles ){
  vfile* vf = vfs_search(name);
	if ( vf == 0 ) return;

  // get the path inside the archive
  TQString path = vfs_origin.right((vfs_origin.length()-vfs_origin.findRev('\\'))-1);
  path = path+"/";
  if(path.left(1) == "/") path.remove(0,1);

  if( !vf->vfile_isDir() || vf->vfile_isSymLink() ){ // single files are simple :)
    ++(*totalFiles);
    (*totalSize) += vf->vfile_getSize();
    urls->append(path+name);
  } else { // handle directories : (
    urls->append(path+name+"/");
    TQString origin_backup = vfs_origin; // backup the vfs origin
    vfs_origin = vfs_origin+"/"+name;
    vfileDict* vfs_filesP_backup = vfs_filesP; // backup the vfs_filesP
    vfs_filesP = findDir(path+name);

    // process all the files in the directory.
    for( vf = vfs_getFirstFile(); vf != 0; vf = vfs_getNextFile() )
      processName(vf->vfile_getName(),urls,totalSize,totalFiles);

    vfs_origin = origin_backup;     // restore origin
    vfs_filesP = vfs_filesP_backup; // restore vfs_filesP
  }
}

void arc_vfs::parseLine(TQString line, TQFile* temp){
  TQString name;
  TDEIO::filesize_t size = 0;
  TQString perm;
  TQFileInfo qfi(arcFile);
  time_t mtime = qfi.lastModified().toTime_t();
  bool link = false;
  uid_t owner = getuid();
  gid_t group = getgid();
  TQString dest = "";
  mode_t mode = 0;


  // parse gziped files
  if(vfs_type == "gzip"){
    KDE_struct_stat stat_p;
    KDE_stat(arcFile.local8Bit(),&stat_p);

    nextWord(line);
    size = nextWord(line).toLong();
    nextWord(line);
    name = nextWord(line,'\n');
    if(name.contains('/')) name = name.mid(name.findRev('/')+1,name.length());
    perm  = KRpermHandler::mode2TQString(stat_p.st_mode) ;
    owner = KRpermHandler::user2uid(qfi.owner());
    group = KRpermHandler::group2gid(qfi.group());
    mode  = stat_p.st_mode;
  }

  // parse bzip2ed files
  if( vfs_type == "zip2" ){
    KDE_struct_stat stat_p;
    KDE_stat(arcFile.local8Bit(),&stat_p);

    name = qfi.fileName();
    name = name.left(name.findRev('.'));
    //long size = qfi.size();
    perm  = KRpermHandler::mode2TQString(stat_p.st_mode) ;
    owner = KRpermHandler::user2uid(qfi.owner());
    group = KRpermHandler::group2gid(qfi.group());
    mode  = stat_p.st_mode;
  }

  // parse tar files
  if(vfs_type == "-tar" || vfs_type == "-tbz" || vfs_type == "-tgz" ){
    perm = nextWord(line);
    TQString temp = nextWord(line);
    owner = temp.left(temp.findRev('/')).toInt();
    group = temp.mid(temp.find('/')+1,temp.length()).toInt();
    size = nextWord(line).toLong();
    temp = nextWord(line);
    name = nextWord(line,'\n');
    if (name.startsWith("/"))  // fix full-paths problem in tar (thanks to Heiner!)
      name.remove(0, 1);
    if( name.contains(" -> ") ){
      link = true;
			dest = name.mid(name.find(" -> ")+4);
			name = name.left(name.find(" -> "));
    }
  }

  // parse zipped files
  if(vfs_type == "-zip"){
    perm = nextWord(line);
    if(perm.length() != 10)
      perm = (perm.at(0)=='d')? "drwxr-xr-x" : "-rw-r--r--" ;
    if (nextWord(line).contains("file")) return;
    nextWord(line);
    size = nextWord(line).toLong();
    nextWord(line);nextWord(line);
    TQString temp = nextWord(line);
    name = nextWord(line,'\n');
  }

  // parse cpio packages
  if(vfs_type == "cpio" || vfs_type == "+rpm"){
		perm = nextWord(line);
    nextWord(line);nextWord(line);nextWord(line);
		size = nextWord(line).toLong();
		nextWord(line);nextWord(line);nextWord(line);
		TQString tempName = arcFile;
    TQFileInfo qfi(tempName.replace(TQRegExp("\\"),""));
		name = nextWord(line,'\n');
    if ( name.left(1) == "/" ) name.remove(0,1);
    if( name.contains(" -> ") ){
      link = true;
      dest = name.mid(name.find(" -> ")+4);
			name = name.left(name.find(" -> "));
    }
  }
  // parse rared files
  if(vfs_type == "-rar"){
    name = nextWord(line,'\n');
    temp->readLine(line,10000);
    size = nextWord(line).toLong();
    nextWord(line);
    nextWord(line);
    perm = nextWord(line);
    if(perm.length() != 10)
      perm = (perm.at(1)=='D')? "drwxr-xr-x" : "-rw-r--r--" ;
  }
  // parse rpm packages
  if(vfs_type == "-rpm"){
		name = nextWord(line);
    if ( name.left(1) == "/" ) name.remove(0,1);
    size = nextWord(line).toLong();
    mtime = nextWord(line).toLong();
    nextWord(line);
    perm = KRpermHandler::mode2TQString(nextWord(line).toLong());
    perm = (perm.at(0)=='d')? "drwxr-xr-x" : "-rw-r--r--" ;
  }

  if ( perm[0]=='d'  && name.right(1) != "/" )  name = name+"/";
  name = changeDir(name);
  if(name.length() < 1) return;


  TQString mime = KMimeType::findByURL( "/"+name,0,true,true)->name();
  vfile* vf=new vfile(name,size,perm,mtime,link,owner,group,mime,dest,mode);
  vfile* vf2 = vfs_search(name);
  if(vf2 != 0) vfs_removeFromList(vf2);
  vfs_addToList(vf);
}

#include "arc_vfs.moc"
