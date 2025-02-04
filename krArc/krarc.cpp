/***************************************************************************
                                 krarc.cpp
                             -------------------
    begin                : Sat Jun 14 14:42:49 IDT 2003
    copyright            : (C) 2003 by Rafi Yanai & Shie Erlich
    email                : krusader@users.sf.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>

#include <tqdir.h>
#include <tqfile.h>
#include <tqfileinfo.h>
#include <tqregexp.h>
#include <tqdir.h>

#include <tdefileitem.h>
#include <kdebug.h>
#include <tdemessagebox.h>
#include <kinstance.h>
#include <tdelocale.h>
#include <kurl.h>
#include <tdetempfile.h>
#include <klargefile.h>
#include <kstandarddirs.h>
#include <tdeio/job.h>
#include <ktar.h>

#include <iostream>
#include "krarc.h"

#define MAX_IPC_SIZE           (1024*32)
#define TRIES_WITH_PASSWORDS   3

#if 0
#define KRDEBUG(X...) do{   \
	TQFile f("/tmp/debug");    \
	f.open(IO_WriteOnly | IO_Append);     \
	TQTextStream stream( &f ); \
  stream << "Pid:" << (int)getpid() << " " <<__FUNCTION__<<"(" <<__LINE__<<"): "; \
  stream << X << endl;      \
	f.close();                \
} while(0);
#else
#define KRDEBUG(X...)
#endif

using namespace TDEIO;
extern "C" {

int kdemain( int argc, char **argv ){
	TDEInstance instance( "tdeio_krarc" );

	if (argc != 4) {
		kdWarning() << "Usage: tdeio_krarc  protocol domain-socket1 domain-socket2" << endl;
		exit(-1);
	}

	tdeio_krarcProtocol slave(argv[2], argv[3]);
	slave.dispatchLoop();

	return 0;
}

} // extern "C"

tdeio_krarcProtocol::tdeio_krarcProtocol(const TQCString &pool_socket, const TQCString &app_socket)
	: SlaveBase("tdeio_krarc", pool_socket, app_socket), archiveChanged(true), arcFile(0L),extArcReady(false),
		password(TQString()) {

	krConfig = new TDEConfig( "krusaderrc" );
	krConfig->setGroup( "Dependencies" );

	dirDict.setAutoDelete(true);

	arcTempDir = locateLocal("tmp",TQString());
	TQString dirName = "krArc"+TQDateTime::currentDateTime().toString(TQt::ISODate);
	dirName.replace(TQRegExp(":"),"_");
	TQDir(arcTempDir).mkdir(dirName);
	arcTempDir = arcTempDir+dirName+"/";
}

/* ---------------------------------------------------------------------------------- */
tdeio_krarcProtocol::~tdeio_krarcProtocol(){
	// delete the temp directory
	KrShellProcess proc;
	proc << "rm -rf "<< arcTempDir;
	proc.start(TDEProcess::Block);
}

/* ---------------------------------------------------------------------------------- */
void tdeio_krarcProtocol::receivedData(TDEProcess*,char* buf,int len){
	TQByteArray d(len);
	d.setRawData(buf,len);
	data(d);
	d.resetRawData(buf,len);
	processedSize(len);
	decompressedLen += len;
}

void tdeio_krarcProtocol::mkdir(const KURL& url,int permissions){
	KRDEBUG(url.path());

	if( !setArcFile( url ) ) {
		error(ERR_CANNOT_ENTER_DIRECTORY,url.path());
		return;
	}
	if( newArchiveURL && !initDirDict(url) ){
		error(ERR_CANNOT_ENTER_DIRECTORY,url.path());
		return;
	}

	if( putCmd.isEmpty() ){
		error(ERR_UNSUPPORTED_ACTION,
		i18n("Creating directories is not supported with %1 archives").arg(arcType) );
		return;
	}

	if( arcType == "arj" || arcType == "lha" ) {
		TQString arcDir = url.path().mid(arcFile->url().path().length());
		if( arcDir.right(1) != "/") arcDir = arcDir+"/";

		if( dirDict.find( arcDir ) == 0 )
			addNewDir( arcDir );
		finished();
		return;
	}

	//TQString tmpDir = arcTempDir+url.path();
	TQString arcDir  = findArcDirectory(url);
	TQString tmpDir = arcTempDir + arcDir.mid(1) + url.path().mid(url.path().findRev("/")+1);
	if( tmpDir.right(1) != "/" ) tmpDir = tmpDir+"/";

	if( permissions == -1 ) permissions = 0777; //set default permissions
	for( unsigned int i=arcTempDir.length();i<tmpDir.length(); i=tmpDir.find("/",i+1)){
		::mkdir(tmpDir.left(i).local8Bit(),permissions);
	}

	if( tmpDir.endsWith( "/" ) )
		tmpDir.truncate( tmpDir.length() - 1 );

	// pack the directory
	KrShellProcess proc;
	proc << putCmd << convertName( arcFile->url().path() ) + " " << convertFileName( tmpDir.mid(arcTempDir.length()) );
	infoMessage(i18n("Creating %1 ...").arg( url.fileName() ) );
	TQDir::setCurrent(arcTempDir);
	proc.start(TDEProcess::Block,TDEProcess::AllOutput);

	// delete the temp directory
	TQDir().rmdir(arcTempDir);

	if( !proc.normalExit() || !checkStatus( proc.exitStatus() ) )  {
		error(ERR_COULD_NOT_WRITE,url.path() + "\n\n" + proc.getErrorMsg() );
		return;
	}

	//  force a refresh of archive information
	initDirDict(url,true);
	finished();
}

void tdeio_krarcProtocol::put(const KURL& url,int permissions,bool overwrite,bool resume){
	KRDEBUG(url.path());
	if( !setArcFile( url ) ) {
		error(ERR_CANNOT_ENTER_DIRECTORY,url.path());
		return;
	}
	if( newArchiveURL && !initDirDict(url) ){
		error(ERR_CANNOT_ENTER_DIRECTORY,url.path());
		return;
	}

	if( putCmd.isEmpty() ){
		error(ERR_UNSUPPORTED_ACTION,
		i18n("Writing to %1 archives is not supported").arg(arcType) );
		return;
	}
	if( !overwrite && findFileEntry(url) ){
		error( ERR_FILE_ALREADY_EXIST,url.path() );
		return;
	}

	TQString arcDir  = findArcDirectory(url);
	TQString tmpFile = arcTempDir + arcDir.mid(1) + url.path().mid(url.path().findRev("/")+1);

	TQString tmpDir = arcTempDir+arcDir.mid(1)+"/";
	for( unsigned int i=arcTempDir.length();i<tmpDir.length(); i=tmpDir.find("/",i+1)){
		TQDir("/").mkdir(tmpDir.left(i));
	}
	int fd;
	if ( resume ) {
		fd = KDE_open( tmpFile.local8Bit(), O_RDWR );  // append if resuming
		KDE_lseek(fd, 0, SEEK_END); // Seek to end
	} else {
		// WABA: Make sure that we keep writing permissions ourselves,
		// otherwise we can be in for a surprise on NFS.
		mode_t initialMode;
		if ( permissions != -1)
			initialMode = permissions | S_IWUSR | S_IRUSR;
		else
			initialMode = 0666;

		fd = KDE_open(tmpFile.local8Bit(), O_CREAT | O_TRUNC | O_WRONLY, initialMode);
	}
	TQByteArray buffer;
	int readResult;
	do{
		dataReq();
		readResult = readData(buffer);
		write(fd,buffer.data(),buffer.size());
 	} while( readResult > 0 );
	close(fd);
	// pack the file
	KrShellProcess proc;
	proc << putCmd << convertName( arcFile->url().path() )+ " " <<convertFileName( tmpFile.mid(arcTempDir.length()) );
	infoMessage(i18n("Packing %1 ...").arg( url.fileName() ) );
	TQDir::setCurrent(arcTempDir);
	proc.start(TDEProcess::Block,TDEProcess::AllOutput);
	// remove the file
	TQFile::remove(tmpFile);

	if( !proc.normalExit() || !checkStatus( proc.exitStatus() ) )  {
		error(ERR_COULD_NOT_WRITE,url.path() + "\n\n" + proc.getErrorMsg() );
		return;
	}
	//  force a refresh of archive information
	initDirDict(url,true);
	finished();
}

void tdeio_krarcProtocol::get(const KURL& url ){
	get( url, TRIES_WITH_PASSWORDS );
}

void tdeio_krarcProtocol::get(const KURL& url, int tries ){
	bool decompressToFile = false;
	KRDEBUG(url.path());

	if( !setArcFile( url ) ) {
		error(ERR_CANNOT_ENTER_DIRECTORY,url.path());
		return;
	}
	if( newArchiveURL && !initDirDict(url) ){
		error(ERR_CANNOT_ENTER_DIRECTORY,url.path());
		return;
	}

	if( getCmd.isEmpty() ){
		error(ERR_UNSUPPORTED_ACTION,
		i18n("Retrieving data from %1 archives is not supported").arg(arcType) );
		return;
	}
	UDSEntry* entry = findFileEntry(url);
	if( !entry ){
		error(TDEIO::ERR_DOES_NOT_EXIST,url.path());
		return;
	}
	if(KFileItem(*entry,url).isDir()){
		error(TDEIO::ERR_IS_DIRECTORY,url.path());
		return;
	}
	TDEIO::filesize_t expectedSize = KFileItem(*entry,url).size();
	// for RPM files extract the cpio file first
	if( !extArcReady && arcType == "rpm"){
		KrShellProcess cpio;
		cpio << "rpm2cpio" << convertName( arcFile->url().path(-1) ) << " > " << arcTempDir+"contents.cpio";
		cpio.start(TDEProcess::Block,TDEProcess::AllOutput);
		if( !cpio.normalExit() || cpio.exitStatus() != 0 )  {
			error(ERR_COULD_NOT_READ,url.path() + "\n\n" + cpio.getErrorMsg() );
			return;
		}
		extArcReady = true;
	}
	// for DEB files extract the tar file first
	if ( !extArcReady && arcType == "deb" ) {
		KrShellProcess dpkg;
		dpkg << cmd + " --fsys-tarfile" << convertName( arcFile->url().path( -1 ) ) << " > " << arcTempDir + "contents.cpio";
		dpkg.start( TDEProcess::Block, TDEProcess::AllOutput );
		if( !dpkg.normalExit() || dpkg.exitStatus() != 0 )  {
			error(ERR_COULD_NOT_READ,url.path() + "\n\n" + dpkg.getErrorMsg() );
			return;
		}
		extArcReady = true;
	}

	// Use the external unpacker to unpack the file
	TQString file = url.path().mid(arcFile->url().path().length()+1);
	KrShellProcess proc;
	if( extArcReady ){
		proc << getCmd << arcTempDir+"contents.cpio " << convertName( "*"+file );
	} else if( arcType == "arj" || arcType == "ace" || arcType == "7z" ) {
		proc << getCmd << convertName( arcFile->url().path(-1) )+ " " << convertFileName( file );
		if( arcType == "ace" && TQFile( "/dev/ptmx" ).exists() ) // Don't remove, unace crashes if missing!!!
		proc << "<" << "/dev/ptmx";
		file = url.fileName();
		decompressToFile = true;
	} else {
		decompressedLen = 0;
		// Determine the mimetype of the file to be retrieved, and emit it.
		// This is mandatory in all slaves (for KRun/BrowserRun to work).
		KMimeType::Ptr mt = KMimeType::findByURL( arcTempDir+file, 0, false /* NOT local URL */ );
		emit mimeType( mt->name() );
		proc << getCmd << convertName( arcFile->url().path() )+" ";
		if( arcType != "gzip" && arcType != "bzip2" && arcType != "xz") proc << convertFileName( file );
		connect(&proc,TQ_SIGNAL(receivedStdout(TDEProcess*,char*,int)),
				this,TQ_SLOT(receivedData(TDEProcess*,char*,int)) );
	}
	infoMessage(i18n("Unpacking %1 ...").arg( url.fileName() ) );
	// change the working directory to our arcTempDir
	TQDir::setCurrent(arcTempDir);
	proc.start(TDEProcess::Block,TDEProcess::AllOutput);

	if( !extArcReady && !decompressToFile ) {
		if( !proc.normalExit() || !checkStatus( proc.exitStatus() ) ||
		   ( arcType != "bzip2" && arcType != "xz" && expectedSize != decompressedLen ) ) {
			if( encrypted && tries ) {
				invalidatePassword();
				get( url, tries - 1 );
				return;
			}
			error( TDEIO::ERR_ACCESS_DENIED, url.path() + "\n\n" + proc.getErrorMsg() );
			return;
		}
	}
	else{
		if( !proc.normalExit() || !checkStatus( proc.exitStatus() ) || !TQFileInfo( arcTempDir+file ).exists() ) {
			if( decompressToFile )
				TQFile(arcTempDir+file).remove();
			if( encrypted && tries ) {
				invalidatePassword();
				get( url, tries - 1 );
				return;
			}
			error( TDEIO::ERR_ACCESS_DENIED, url.path() );
			return;
		}
		// the follwing block is ripped from KDE file TDEIO::Slave
		// $Id: krarc.cpp,v 1.43 2007/01/13 13:39:51 ckarai Exp $
		TQCString _path( TQFile::encodeName(arcTempDir+file) );
		KDE_struct_stat buff;
		if( KDE_lstat( _path.data(), &buff ) == -1 ) {
			if ( errno == EACCES )
				error( TDEIO::ERR_ACCESS_DENIED, url.path() );
			else
				error( TDEIO::ERR_DOES_NOT_EXIST, url.path() );
			return;
		}
		if ( S_ISDIR( buff.st_mode ) ) {
			error( TDEIO::ERR_IS_DIRECTORY, url.path() );
			return;
		}
		if ( !S_ISREG(buff.st_mode) ) {
			error( TDEIO::ERR_CANNOT_OPEN_FOR_READING, url.path() );
			return;
		}
		int fd = KDE_open( _path.data(), O_RDONLY );
		if ( fd < 0 ) {
			error( TDEIO::ERR_CANNOT_OPEN_FOR_READING, url.path() );
			return;
		}
		// Determine the mimetype of the file to be retrieved, and emit it.
		// This is mandatory in all slaves (for KRun/BrowserRun to work).
		KMimeType::Ptr mt = KMimeType::findByURL( arcTempDir+file, buff.st_mode, true /* local URL */ );
		emit mimeType( mt->name() );

		TDEIO::filesize_t processed_size = 0;

		TQString resumeOffset = metaData("resume");
		if ( !resumeOffset.isEmpty() ){
			bool ok;
			TDEIO::fileoffset_t offset = resumeOffset.toLongLong(&ok);
			if (ok && (offset > 0) && (offset < buff.st_size)){
				if (KDE_lseek(fd, offset, SEEK_SET) == offset){
					canResume ();
					processed_size = offset;
				}
			}
		}

		totalSize( buff.st_size );

		char buffer[ MAX_IPC_SIZE ];
		TQByteArray array;
		while( 1 ){
			int n = ::read( fd, buffer, MAX_IPC_SIZE );
			if (n == -1){
				if (errno == EINTR)
					continue;
				error( TDEIO::ERR_COULD_NOT_READ, url.path());
				close(fd);
				return;
			}
			if (n == 0)
				break; // Finished

			array.setRawData(buffer, n);
			data( array );
			array.resetRawData(buffer, n);

			processed_size += n;
		}

		data( TQByteArray() );
		close( fd );
		processedSize( buff.st_size );
		finished();

		if( decompressToFile )
			TQFile(arcTempDir+file).remove();
		return;
	}
	// send empty buffer to mark EOF
	data(TQByteArray());
	finished();
}

void tdeio_krarcProtocol::del(KURL const & url, bool isFile){
	KRDEBUG(url.path());

	if( !setArcFile( url ) ) {
		error(ERR_CANNOT_ENTER_DIRECTORY,url.path());
		return;
	}
	if( newArchiveURL && !initDirDict(url) ){
		error(ERR_CANNOT_ENTER_DIRECTORY,url.path());
		return;
	}

	if( delCmd.isEmpty() ){
		error(ERR_UNSUPPORTED_ACTION,
		i18n("Deleting files from %1 archives is not supported").arg(arcType) );
		return;
	}
	if( !findFileEntry(url) ){
		if( ( arcType != "arj" && arcType != "lha" ) || isFile ) {
			error(TDEIO::ERR_DOES_NOT_EXIST,url.path());
			return;
		}
	}

	TQString file = url.path().mid(arcFile->url().path().length()+1);
	if( !isFile && file.right(1) != "/" ) {
		if(arcType == "zip") file = file + "/";
	}
	KrShellProcess proc;
	proc << delCmd << convertName( arcFile->url().path() )+" " << convertFileName( file );
	infoMessage(i18n("Deleting %1 ...").arg( url.fileName() ) );
	proc.start(TDEProcess::Block, TDEProcess::AllOutput);
	if( !proc.normalExit() || !checkStatus( proc.exitStatus() ) )  {
		error(ERR_COULD_NOT_WRITE,url.path() + "\n\n" + proc.getErrorMsg() );
		return;
	}
	//  force a refresh of archive information
	initDirDict(url,true);
	finished();
}

void tdeio_krarcProtocol::stat( const KURL & url ){
	KRDEBUG(url.path());
	if( !setArcFile( url ) ) {
		error(ERR_CANNOT_ENTER_DIRECTORY,url.path());
		return;
	}
	if( newArchiveURL && !initDirDict(url) ){
		error(ERR_CANNOT_ENTER_DIRECTORY,url.path());
		return;
	}

	if( listCmd.isEmpty() ){
		error(ERR_UNSUPPORTED_ACTION,
		i18n("Accessing files is not supported with the %1 archives").arg(arcType) );
		return;
	}
	TQString path = url.path(-1);
	KURL newUrl = url;

	// but treat the archive itself as the archive root
	if( path == arcFile->url().path(-1) ){
		newUrl.setPath(path+"/");
		path = newUrl.path();
	}
	// we might be stating a real file
	if( TQFileInfo(path).exists() ){
		KDE_struct_stat buff;
		KDE_stat( path.local8Bit(), &buff );
		TQString mime = KMimeType::findByPath(path,buff.st_mode)->name();
		statEntry(KFileItem(path,mime,buff.st_mode).entry());
		finished();
		return;
	}
	UDSEntry* entry = findFileEntry(newUrl);
	if( entry ){
		statEntry( *entry );
		finished();
	} else error( TDEIO::ERR_DOES_NOT_EXIST, path );
}

void tdeio_krarcProtocol::copy (const KURL &url, const KURL &dest, int, bool overwrite) {
	KRDEBUG(url.path());

	// KDE HACK: opening the password dlg in copy causes error for the COPY, and further problems
	// that's why encrypted files are not allowed to copy
	if( !encrypted && dest.isLocalFile() )
		do {
			if( url.fileName() != dest.fileName() )
				break;

			//the file exists and we don't want to overwrite
			if ((!overwrite) && ( TQFile( dest.path() ).exists() ) ) {
				error(ERR_FILE_ALREADY_EXIST, TQFile::encodeName(dest.path()) );
				return;
			};

			if( !setArcFile( url ) ) {
				error(ERR_CANNOT_ENTER_DIRECTORY,url.path());
				return;
			}
			if( newArchiveURL && !initDirDict(url) ){
				error(ERR_CANNOT_ENTER_DIRECTORY,url.path());
				return;
			}

			UDSEntry* entry = findFileEntry(url);
			if( copyCmd.isEmpty() || !entry )
				break;

			TQString file = url.path().mid(arcFile->url().path().length()+1);

			TQString destDir = dest.path( -1 );
			if( !TQDir( destDir ).exists() ) {
				int ndx = destDir.findRev( '/' );
				if( ndx != -1 )
					destDir.truncate( ndx+1 );
			}

			TQDir::setCurrent( destDir.local8Bit() );

			KrShellProcess proc;
			proc << copyCmd << convertName( arcFile->url().path(-1) )+" " << convertFileName( file );
			if( arcType == "ace" && TQFile( "/dev/ptmx" ).exists() ) // Don't remove, unace crashes if missing!!!
				proc << "<" << "/dev/ptmx";

			infoMessage(i18n("Unpacking %1 ...").arg( url.fileName() ) );
			proc.start(TDEProcess::Block, TDEProcess::AllOutput);
			if( !proc.normalExit() || !checkStatus( proc.exitStatus() ) )  {
				error(TDEIO::ERR_COULD_NOT_WRITE, dest.path(-1) + "\n\n" + proc.getErrorMsg() );
				return;
			}
			if( !TQFileInfo( dest.path(-1) ).exists() ) {
				error( TDEIO::ERR_COULD_NOT_WRITE, dest.path(-1) );
				return;
			}

			processedSize( KFileItem(*entry,url).size() );
			finished();
			TQDir::setCurrent( "/" ); /* for being able to umount devices after copying*/
			return;
		}while( 0 );

	error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_COPY));
}

void tdeio_krarcProtocol::listDir(const KURL& url){
	KRDEBUG(url.path());
	if( !setArcFile( url ) ) {
		error(ERR_CANNOT_ENTER_DIRECTORY,url.path());
		return;
	}
	if( listCmd.isEmpty() ){
		error(ERR_UNSUPPORTED_ACTION,
		i18n("Listing directories is not supported for %1 archives").arg(arcType) );
		return;
	}
	TQString path = url.path();
	if( path.right(1) != "/" ) path = path+"/";

	// it might be a real dir !
	if( TQFileInfo(path).exists() ){
		if( TQFileInfo(path).isDir() ){
			KURL redir;
			redir.setPath( url.path() );
			redirection(redir);
			finished();
		} else { // maybe it's an archive !
			error(ERR_IS_FILE,path);
		}
		return;
	}
	if( !initDirDict(url) ){
		error( ERR_CANNOT_ENTER_DIRECTORY, url.path());
		return;
	}
	TQString arcDir = path.mid(arcFile->url().path().length());
	arcDir.truncate(arcDir.findRev("/"));
	if(arcDir.right(1) != "/") arcDir = arcDir+"/";

	UDSEntryList* dirList = dirDict.find(arcDir);
	if( dirList == 0 ){
		error(ERR_CANNOT_ENTER_DIRECTORY,url.path());
		return;
	}
	totalSize(dirList->size());
	listEntries(*dirList);
	finished();
}

bool tdeio_krarcProtocol::setArcFile(const KURL& url){
	TQString path = url.path();
	time_t currTime = time( 0 );
	archiveChanged = true;
	newArchiveURL = true;
	// is the file already set ?
	if( arcFile && arcFile->url().path(-1) == path.left(arcFile->url().path(-1).length()) ){
		newArchiveURL = false;
		// Has it changed ?
		KFileItem* newArcFile = new KFileItem(arcFile->url(),TQString(),arcFile->mode());
		if( !newArcFile->cmp( *arcFile ) ){
			delete arcFile;
			password = TQString();
			extArcReady = false;
			arcFile = newArcFile;
		} else { // same old file
			delete newArcFile;
			archiveChanged = false;
			if( encrypted && password.isNull() )
				initArcParameters();
		}
	} else { // it's a new file...
		extArcReady = false;
		if( arcFile ){
			delete arcFile;
			password = TQString();
			arcFile = 0L;
		}
		TQString newPath = path;
		if(newPath.right(1) != "/") newPath = newPath+"/";
		for(int pos=0; pos >= 0; pos = newPath.find("/",pos+1)){
			TQFileInfo qfi(newPath.left(pos));
			if( qfi.exists() && !qfi.isDir() ){
				KDE_struct_stat stat_p;
				KDE_lstat(newPath.left(pos).local8Bit(),&stat_p);
				arcFile = new KFileItem(KURL::fromPathOrURL( newPath.left(pos) ),TQString(),stat_p.st_mode);
				break;
			}
		}
		if( !arcFile ){
			error( ERR_DOES_NOT_EXIST,path );
			return false; // file not found
		}
	}

	/* FIX: file change can only be detected if the timestamp between the two consequent
	   changes is more than 1s. If the archive is continuously changing (check: move files
	   inside the archive), krarc may erronously think, that the archive file is unchanged,
	   because the timestamp is the same as the previous one. This situation can only occur
	   if the modification time equals with the current time. While this condition is true,
	   we can say, that the archive is changing, so content reread is always necessary
	   during that period. */
	if( archiveChanging )
		archiveChanged = true;
	archiveChanging = ( currTime == arcFile->time( UDS_MODIFICATION_TIME ) );

	arcPath = arcFile->url().path(-1);
	arcType = detectArchive( encrypted, arcPath );

	if( arcType == "tbz" )
		arcType = "bzip2";
	else if( arcType == "tgz" )
		arcType = "gzip";
	else if( arcType == "txz" )
		arcType = "xz";

	if( arcType.isEmpty() ) {
		arcType = arcFile->mimetype();
		arcType = arcType.mid(arcType.findRev("-")+1);

		if( arcType == "jar" )
			arcType = "zip";
	}

	return initArcParameters();
}

bool tdeio_krarcProtocol::initDirDict(const KURL&url, bool forced){
	// set the archive location
	//if( !setArcFile(url.path()) ) return false;
	// no need to rescan the archive if it's not changed
	if( !archiveChanged && !forced ) return true;
	extArcReady = false;

	if( !setArcFile( url ) )
		return false;   /* if the archive was changed refresh the file information */

	// write the temp file
	KrShellProcess proc;
	KTempFile temp( TQString(), "tmp" );
	temp.setAutoDelete(true);
	if (arcType != "bzip2" && arcType != "xz") {
		if( arcType == "rpm" )
			proc << listCmd << convertName( arcPath ) <<" > " << temp.name();
		else
			proc << listCmd << convertName( arcFile->url().path(-1) ) <<" > " << temp.name();
		if( arcType == "ace" && TQFile( "/dev/ptmx" ).exists() ) // Don't remove, unace crashes if missing!!!
			proc << "<" << "/dev/ptmx";
		proc.start(TDEProcess::Block,TDEProcess::AllOutput);
		if( !proc.normalExit() || !checkStatus( proc.exitStatus() ) ) return false;
	}
	// clear the dir dictionary
	dirDict.clear();

	// add the "/" directory
	UDSEntryList* root = new UDSEntryList();
	dirDict.insert("/",root);
	// and the "/" UDSEntry
	UDSEntry entry;
	UDSAtom atom;
	atom.m_uds = UDS_NAME;
	atom.m_str = ".";
	entry.append(atom);
	mode_t mode = parsePermString("drwxr-xr-x");
	atom.m_uds = UDS_FILE_TYPE;
	atom.m_long = mode & S_IFMT; // keep file type only
	entry.append( atom );
	atom.m_uds = UDS_ACCESS;
	atom.m_long = mode & 07777; // keep permissions only
	entry.append( atom );

	root->append(entry);

	if (arcType == "bzip2" || arcType == "xz"){
		KRDEBUG("Got me here...");
		parseLine(0,"",temp.file());
		return true;
	}

	// parse the temp file
	temp.file()->open(IO_ReadOnly);
	char buf[1000];
	TQString line;

	int lineNo = 0;
	bool invalidLine = false;
	// the rar list is started with a ------ line.
	if(arcType == "rar" || arcType == "arj" || arcType == "lha" || arcType == "7z" ){
		while(temp.file()->readLine(buf,1000) != -1){
			line = TQString::fromLocal8Bit(buf);
			if( line.startsWith("----------") ) break;
		}
	}
	while(temp.file()->readLine(buf,1000) != -1) {
		line = TQString::fromLocal8Bit(buf);
		if( arcType == "rar" ) {
			// the rar list is ended with a ------ line.
			if( line.startsWith("----------") ) {
				invalidLine = !invalidLine;
				continue;
			}
			if( invalidLine )
				continue;
			else{
				temp.file()->readLine(buf,1000);
				line = line+TQString::fromLocal8Bit(buf);
				if( line[0]=='*' ) // encrypted archives starts with '*'
					line[0]=' ';
			}
		}
		if( arcType == "ace" ) {
			// the ace list begins with a number.
			if( !line[0].isDigit() ) continue;
		}
		if( arcType == "arj" ) {
			// the arj list is ended with a ------ line.
			if( line.startsWith("----------") ) {
				invalidLine = !invalidLine;
				continue;
			}
			if( invalidLine )
				continue;
			else {
				temp.file()->readLine(buf,1000);
				line = line+TQString::fromLocal8Bit(buf);
				temp.file()->readLine(buf,1000);
				line = line+TQString::fromLocal8Bit(buf);
				temp.file()->readLine(buf,1000);
				line = line+TQString::fromLocal8Bit(buf);
			}
		}
		if( arcType == "lha" || arcType == "7z" ) {
			// the arj list is ended with a ------ line.
			if( line.startsWith("----------") ) break;
		}
		parseLine(lineNo++,line.stripWhiteSpace(),temp.file());
	}
	// close and delete our file
	temp.file()->close();

	archiveChanged = false;
	return true;
}

TQString tdeio_krarcProtocol::findArcDirectory(const KURL& url){
	TQString path = url.path();
	if( path.right(1) == "/" ) path.truncate(path.length()-1);

	if( !initDirDict(url) ){
		return TQString();
	}
	TQString arcDir = path.mid(arcFile->url().path().length());
	arcDir.truncate(arcDir.findRev("/"));
	if(arcDir.right(1) != "/") arcDir = arcDir+"/";

	return arcDir;
}

UDSEntry* tdeio_krarcProtocol::findFileEntry(const KURL& url){
	TQString arcDir = findArcDirectory(url);
	if( arcDir.isEmpty() ) return 0;

	UDSEntryList* dirList = dirDict.find(arcDir);
	if( !dirList ){
		return 0;
	}
	TQString name = url.path();
	if( arcFile->url().path(-1) == url.path(-1) ) name = "."; // the "/" case
	else{
		if( name.right(1) == "/" ) name.truncate(name.length()-1);
		name = name.mid(name.findRev("/")+1);
	}

	UDSEntryList::iterator entry;
	UDSEntry::iterator atom;

	for ( entry = dirList->begin(); entry != dirList->end(); ++entry ){
		for( atom = (*entry).begin(); atom != (*entry).end(); ++atom ){
			if( (*atom).m_uds == UDS_NAME ){
				if((*atom).m_str == name){
					return &(*entry);
				} else break;
			}
		}
	}
	return 0;
}

TQString tdeio_krarcProtocol::nextWord(TQString &s,char d) {
	s=s.stripWhiteSpace();
	int j=s.find(d,0);
	TQString temp=s.left(j); // find the leftmost word.
	s.remove(0,j);
	return temp;
}

mode_t tdeio_krarcProtocol::parsePermString(TQString perm){
	mode_t mode=0;
	// file type
	if(perm[0] == 'd') mode |= S_IFDIR;
	if(perm[0] == 'l') mode |= S_IFLNK;
	if(perm[0] == '-') mode |= S_IFREG;
	// owner permissions
	if(perm[1] != '-') mode |= S_IRUSR;
	if(perm[2] != '-') mode |= S_IWUSR;
	if(perm[3] != '-') mode |= S_IXUSR;
	// group permissions
	if(perm[4] != '-') mode |= S_IRGRP;
	if(perm[5] != '-') mode |= S_IWGRP;
	if(perm[6] != '-') mode |= S_IXGRP;
	// other permissions
	if(perm[7] != '-') mode |= S_IROTH;
	if(perm[8] != '-') mode |= S_IWOTH;
	if(perm[9] != '-') mode |= S_IXOTH;

	return mode;
}

UDSEntryList* tdeio_krarcProtocol::addNewDir(TQString path){
	UDSEntryList* dir;

	// check if the current dir exists
	dir = dirDict.find(path);
	if(dir != 0) return dir; // dir exists- return it !

	// set dir to the parent dir
	dir = addNewDir(path.left(path.findRev("/",-2)+1));

	// add a new entry in the parent dir
	TQString name = path.mid(path.findRev("/",-2)+1);
	name = name.left(name.length()-1);

	UDSEntry entry;
	UDSAtom atom;
	atom.m_uds = UDS_NAME;
	atom.m_str = name;
	entry.append(atom);

	mode_t mode = parsePermString("drwxr-xr-x");

	atom.m_uds = UDS_FILE_TYPE;
	atom.m_long = mode & S_IFMT; // keep file type only
	entry.append( atom );

	atom.m_uds = UDS_ACCESS;
	atom.m_long = mode & 07777; // keep permissions only
	entry.append( atom );

	atom.m_uds = UDS_SIZE;
	atom.m_long = 0;
	entry.append( atom );

	atom.m_uds = UDS_MODIFICATION_TIME;
	atom.m_long = arcFile->time(UDS_MODIFICATION_TIME);
	entry.append( atom );

	dir->append(entry);

	// create a new directory entry and add it..
	dir = new UDSEntryList();
	dirDict.insert(path,dir);

	return dir;
}

void tdeio_krarcProtocol::parseLine(int lineNo, TQString line, TQFile*) {
	UDSEntryList* dir;
	UDSEntry entry;
	UDSAtom atom;

	TQString owner        = TQString();
	TQString group        = TQString();
	TQString symlinkDest  = TQString();
	TQString perm         = TQString();
	mode_t mode          = 0666;
	size_t size          = 0;
	time_t time          = ::time(0);
	TQString fullName     = TQString();

	if(arcType == "zip"){
		// permissions
		perm = nextWord(line);
		// ignore the next 2 fields
		nextWord(line); nextWord(line);
		// size
		size = nextWord(line).toLong();
		// ignore the next 2 fields
		nextWord(line);nextWord(line);
		// date & time
		TQString d = nextWord(line);
		TQDate qdate(d.mid(0,4).toInt(),d.mid(4,2).toInt(),d.mid(6,2).toInt());
		TQTime qtime(d.mid(9,2).toInt(),d.mid(11,2).toInt(),d.mid(13,2).toInt());
		time = TQDateTime(qdate,qtime).toTime_t();
		// full name
		fullName = nextWord(line,'\n');

		if(perm.length() != 10)
			perm = (perm.at(0)=='d' || fullName.endsWith( "/" )) ? "drwxr-xr-x" : "-rw-r--r--" ;
		mode = parsePermString(perm);
	}
	if(arcType == "rar") {
		// full name
		fullName = nextWord(line,'\n');
		// size
		size = nextWord(line).toLong();
		// ignore the next 2 fields
		nextWord(line); nextWord(line);
		// date & time
		TQString d = nextWord(line);
		int year = 1900 + d.mid(6,2).toInt();
		if( year < 1930 ) year+=100;
		TQDate qdate( year, d.mid(3,2).toInt(), d.mid(0,2).toInt() );
		TQString t = nextWord(line);
		TQTime qtime(t.mid(0,2).toInt(),t.mid(3,2).toInt(),0);
		time = TQDateTime(qdate,qtime).toTime_t();
		// permissions
		perm = nextWord(line);

		if( perm.length() == 7 ) // windows rar permission format
		{
			bool isDir  = ( perm.at(1).lower() == 'd' );
			bool isReadOnly = ( perm.at(2).lower() == 'r' );

			perm = isDir ? "drwxr-xr-x" : "-rw-r--r--";

			if( isReadOnly )
				perm.at( 2 ) = '-';
		}

		if(perm.length() != 10) perm = (perm.at(0)=='d')? "drwxr-xr-x" : "-rw-r--r--" ;
		mode = parsePermString(perm);
	}
	if(arcType == "arj"){
		nextWord(line);
		// full name
		fullName = nextWord(line,'\n');
		// ignore the next 2 fields
		nextWord(line); nextWord(line);
		// size
		size = nextWord(line).toLong();
		// ignore the next 2 fields
		nextWord(line); nextWord(line);
		// date & time
		TQString d = nextWord(line);
		int year = 1900 + d.mid(0,2).toInt();
		if( year < 1930 ) year+=100;
		TQDate qdate( year, d.mid(3,2).toInt(), d.mid(6,2).toInt() );
		TQString t = nextWord(line);
		TQTime qtime(t.mid(0,2).toInt(),t.mid(3,2).toInt(),0);
		time = TQDateTime(qdate,qtime).toTime_t();
		// permissions
		perm = nextWord(line);
		if(perm.length() != 10) perm = (perm.at(0)=='d')? "drwxr-xr-x" : "-rw-r--r--" ;
		mode = parsePermString(perm);
	}
	if(arcType == "rpm"){
		// full name
		fullName = nextWord(line);
		// size
		size = nextWord(line).toULong();
		// date & time
		time = nextWord(line).toULong();
		// next field is md5sum, ignore it
		nextWord(line);
		// permissions
		mode = nextWord(line).toULong(0,8);
		// Owner & Group
		owner = nextWord(line);
		group = nextWord(line);
		// symlink destination
		if( S_ISLNK(mode) ){
			// ignore the next 3 fields
			nextWord(line); nextWord(line); nextWord(line);
			symlinkDest = nextWord(line);
		}
	}
	if( arcType == "gzip" ){
		if( !lineNo ) return; //ignore the first line
		// first field is uncompressed size - ignore it
		nextWord(line);
		// size
		size = nextWord(line).toULong();
		// ignore the next field
		nextWord(line);
		// full name
		fullName = nextWord(line);
		fullName = fullName.mid(fullName.findRev("/")+1);
	}
	if( arcType == "bzip2" ){
		// There is no way to list bzip2 files, so we take our information from
		// the archive itself...
		fullName = arcFile->name();
		if( fullName.endsWith("bz2") ) fullName.truncate(fullName.length()-4);
		mode = arcFile->mode();
		size = arcFile->size();
	}
	if(arcType == "lha"){
		// permissions
		perm = nextWord(line);
		if(perm.length() != 10) perm = (perm.at(0)=='d')? "drwxr-xr-x" : "-rw-r--r--" ;
		mode = parsePermString(perm);
		// ignore the next field
		nextWord(line);
		// size
		size = nextWord(line).toLong();
		// ignore the next field
		nextWord(line);
		// date & time
		int month = (TQStringList::split(',', "Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec")).findIndex( nextWord(line) ) + 1;
		int day = nextWord(line).toInt();
		int year = TQDate::currentDate().year();
		TQString third = nextWord(line);
		TQTime qtime;

		if( third.contains(":" ) )
			qtime = TQTime::fromString( third );
		else
			year = third.toInt();

		TQDate qdate(year, month, day );

		time = TQDateTime(qdate, qtime).toTime_t();
		// full name
		fullName = nextWord(line,'\n');
	}
	if(arcType == "ace"){
		// date & time
		TQString d = nextWord(line);
		int year = 1900 + d.mid(6,2).toInt();
		if( year < 1930 ) year+=100;
		TQDate qdate( year, d.mid(3,2).toInt(), d.mid(0,2).toInt() );
		TQString t = nextWord(line);
		TQTime qtime(t.mid(0,2).toInt(),t.mid(3,2).toInt(),0);
		time = TQDateTime(qdate,qtime).toTime_t();
		// ignore the next field
		nextWord(line);
		// size
		size = nextWord(line).toLong();
		// ignore the next field
		nextWord(line);
		// full name
		fullName = nextWord(line,'\n');
		if( fullName[ 0 ] == '*' ) // encrypted archives starts with '*'
			fullName = fullName.mid( 1 );
	}
	if( arcType == "deb" ){
		// permissions
		perm = nextWord( line );
		mode = parsePermString( perm );
		// Owner & Group
		owner = nextWord( line,'/' );
		group = nextWord( line ).mid(1);
		// size
		size = nextWord( line ).toLong();
		// date & time
		TQString d = nextWord( line );
		TQDate qdate( d.mid( 0, 4 ).toInt(), d.mid( 5, 2 ).toInt(), d.mid( 8, 2 ).toInt() );
		TQString t = nextWord( line );
		TQTime qtime( t.mid( 0, 2 ).toInt(), t.mid( 3, 2 ).toInt(), 0 );
		time = TQDateTime( qdate, qtime ).toTime_t();
		// full name
		fullName = nextWord( line, '\n' ).mid( 1 );
		//if ( fullName.right( 1 ) == "/" ) return;
		if( fullName.contains("->") ){
			symlinkDest = fullName.mid(fullName.find("->")+2);
			fullName = fullName.left(fullName.find("->")-1);
		}
	}
	if(arcType == "7z"){
		// date & time
		TQString d = nextWord(line);
		TQDate qdate( d.mid(0,4).toInt(), d.mid(5,2).toInt(), d.mid(8,2).toInt() );
		TQString t = nextWord(line);
		TQTime qtime(t.mid(0,2).toInt(),t.mid(3,2).toInt(),t.mid(6,2).toInt() );
		time = TQDateTime(qdate,qtime).toTime_t();

		// permissions
		perm = nextWord(line);
		bool isDir  = ( perm.at(0).lower() == 'd' );
		bool isReadOnly = ( perm.at(1).lower() == 'r' );
		perm = isDir ? "drwxr-xr-x" : "-rw-r--r--";
		if( isReadOnly )
			perm.at( 2 ) = '-';

		mode = parsePermString(perm);

		// size
		size = nextWord(line).toLong();

		// ignore the next 15 characters
		line = line.mid( 15 );

		// full name
		fullName = nextWord(line,'\n');
	}
  if (arcType == "xz") {
    fullName = arcFile->name();
    if (fullName.endsWith("xz")) {
      fullName.truncate(fullName.length() - 3);
    }
    mode = arcFile->mode();
    size = arcFile->size();
  }

	if( fullName.right(1) == "/" ) fullName = fullName.left(fullName.length()-1);
	if( !fullName.startsWith("/") ) fullName = "/"+fullName;
	TQString path = fullName.left(fullName.findRev("/")+1);
	// set/create the directory UDSEntryList
	dir = dirDict.find(path);
	if(dir == 0) dir = addNewDir(path);
	TQString name = fullName.mid(fullName.findRev("/")+1);
	// file name
	atom.m_uds = UDS_NAME;
	atom.m_str = name;
	entry.append(atom);
	// file type
	atom.m_uds = UDS_FILE_TYPE;
	atom.m_long = mode & S_IFMT; // keep file type only
	entry.append( atom );
	// file permissions
	atom.m_uds = UDS_ACCESS;
	atom.m_long = mode & 07777; // keep permissions only
	entry.append( atom );
	// file size
	atom.m_uds = UDS_SIZE;
	atom.m_long = size;
	entry.append( atom );
	// modification time
	atom.m_uds = UDS_MODIFICATION_TIME;
	atom.m_long = time;
	entry.append( atom );
	// link destination
	if( !symlinkDest.isEmpty() ){
		atom.m_uds = UDS_LINK_DEST;
		atom.m_str = symlinkDest;
		entry.append( atom );
	}
	if( S_ISDIR(mode) ){
		fullName=fullName+"/";
		if(dirDict.find(fullName) == 0)
			dirDict.insert(fullName,new UDSEntryList());
		else {
			// try to overwrite an existing entry
			UDSEntryList::iterator entryIt;
			UDSEntry::iterator atomIt;

			for ( entryIt = dir->begin(); entryIt != dir->end(); ++entryIt )
				for( atomIt = (*entryIt).begin(); atomIt != (*entryIt).end(); ++atomIt )
					if( (*atomIt).m_uds == UDS_NAME )
						if((*atomIt).m_str == name) {
							for( atomIt = (*entryIt).begin(); atomIt != (*entryIt).end(); ++atomIt ) {
								switch( (*atomIt).m_uds ) {
								case UDS_MODIFICATION_TIME:
									(*atomIt).m_long = time;
									break;
								case UDS_ACCESS:
									(*atomIt).m_long = mode & 07777;
									break;
								}
							}
							return;
						}
			return; // there is alreay an entry for this directory
		}
	}

	// multi volume archives can add a file twice, use only one
	UDSEntryList::iterator dirEntryIt;
	UDSEntry::iterator dirAtomIt;
	for ( dirEntryIt = dir->begin(); dirEntryIt != dir->end(); ++dirEntryIt )
		for( dirAtomIt = (*dirEntryIt).begin(); dirAtomIt != (*dirEntryIt).end(); ++dirAtomIt )
			if( (*dirAtomIt).m_uds == UDS_NAME && (*dirAtomIt).m_str == name )
				return;

	dir->append(entry);
}

bool tdeio_krarcProtocol::initArcParameters() {
	KRDEBUG("arcType: "<<arcType);

	if(arcType == "zip"){
		cmd     = fullPathName( "unzip" );
		listCmd = fullPathName( "unzip" ) + " -ZTs-z-t-h ";
		getCmd  = fullPathName( "unzip" ) + " -p ";
		copyCmd = fullPathName( "unzip" ) + " -jo ";

		if( TDEStandardDirs::findExe( "zip" ).isEmpty() ) {
			delCmd  = TQString();
			putCmd  = TQString();
		} else {
			delCmd  = fullPathName( "zip" ) + " -d ";
			putCmd  = fullPathName( "zip" ) + " -ry ";
		}

		if( !getPassword().isEmpty() ) {
			getCmd += "-P '"+password+"' ";
			copyCmd += "-P '"+password+"' ";
			putCmd += "-P '"+password+"' ";
		}
	} else if (arcType == "rar") {
		if( TDEStandardDirs::findExe( "rar" ).isEmpty() ) {
			cmd     = fullPathName( "unrar" );
			listCmd = fullPathName( "unrar" ) + " -c- -v v ";
			getCmd  = fullPathName( "unrar" ) + " p -ierr -idp -c- -y ";
			copyCmd = fullPathName( "unrar" ) + " e -y ";
			delCmd  = TQString();
			putCmd  = TQString();
		} else {
			cmd     = fullPathName( "rar" );
			listCmd = fullPathName( "rar" ) + " -c- -v v ";
			getCmd  = fullPathName( "rar" ) + " p -ierr -idp -c- -y ";
			copyCmd = fullPathName( "rar" ) + " e -y ";
			delCmd  = fullPathName( "rar" ) + " d ";
			putCmd  = fullPathName( "rar" ) + " -r a ";
		}
		if( !getPassword().isEmpty() ) {
			getCmd += "-p'"+password+"' ";
			listCmd += "-p'"+password+"' ";
			copyCmd += "-p'"+password+"' ";
			if( !putCmd.isEmpty() ) {
				putCmd += "-p'"+password+"' ";
				delCmd += "-p'"+password+"' ";
			}
		}
	} else if(arcType == "rpm") {
		cmd     = fullPathName( "rpm" );
		listCmd = fullPathName( "rpm" ) + " --dump -lpq ";
		getCmd  = fullPathName( "cpio" ) + " --force-local --no-absolute-filenames -iuvdF";
		delCmd  = TQString();
		putCmd  = TQString();
		copyCmd = TQString();
	} else if(arcType == "gzip") {
		cmd     = fullPathName( "gzip" );
		listCmd = fullPathName( "gzip" ) + " -l";
		getCmd  = fullPathName( "gzip" ) + " -dc";
		copyCmd = TQString();
		delCmd  = TQString();
		putCmd  = TQString();
	} else if(arcType == "bzip2") {
		cmd     = fullPathName( "bzip2" );
		listCmd = fullPathName( "bzip2" );
		getCmd  = fullPathName( "bzip2" ) + " -dc";
		copyCmd = TQString();
		delCmd  = TQString();
		putCmd  = TQString();
	} else if(arcType == "arj"){
		cmd     = fullPathName( "arj" );
		listCmd = fullPathName( "arj" ) + " v -y -v ";
		getCmd  = fullPathName( "arj" ) + " -jyov -v e ";
		copyCmd = fullPathName( "arj" ) + " -jyov -v e ";
		delCmd  = fullPathName( "arj" ) + " d ";
		putCmd  = fullPathName( "arj" ) + " -r a ";
		if( !getPassword().isEmpty() ) {
			getCmd += "-g'"+password+"' ";
			copyCmd += "-g'"+password+"' ";
			putCmd += "-g'"+password+"' ";
		}
	} else if(arcType == "lha") {
		cmd     = fullPathName( "lha" );
		listCmd = fullPathName( "lha" ) + " l ";
		getCmd  = fullPathName( "lha" ) + " pq ";
		copyCmd = fullPathName( "lha" ) + " eif ";
		delCmd  = fullPathName( "lha" ) + " d ";
		putCmd  = fullPathName( "lha" ) + " a ";
	} else if(arcType == "ace") {
		cmd     = fullPathName( "unace" );
		listCmd = fullPathName( "unace" ) + " v";
		getCmd  = fullPathName( "unace" ) + " e -o ";
		copyCmd = fullPathName( "unace" ) + " e -o ";
		delCmd  = TQString();
		putCmd  = TQString();
		if( !getPassword().isEmpty() ) {
			getCmd += "-p'"+password+"' ";
			copyCmd += "-p'"+password+"' ";
		}
	} else if ( arcType == "deb" ) {
		cmd = fullPathName("dpkg");
		listCmd = fullPathName("dpkg")+" -c";
		getCmd = fullPathName("tar")+" xvf ";
		copyCmd = TQString();
		delCmd = TQString();
		putCmd = TQString();
	} else if (arcType == "7z") {
		cmd = fullPathName( "7z" );
		if( TDEStandardDirs::findExe(cmd).isEmpty() )
			cmd = fullPathName( "7za" );

		listCmd = cmd + " l -y ";
		getCmd  = cmd + " e -y ";
		copyCmd = cmd + " e -y ";
		delCmd  = cmd + " d -y ";
		putCmd  = cmd + " a -y ";
		if( !getPassword().isEmpty() ) {
			getCmd += "-p'"+password+"' ";
			listCmd += "-p'"+password+"' ";
			copyCmd += "-p'"+password+"' ";
			if( !putCmd.isEmpty() ) {
				putCmd += "-p'"+password+"' ";
				delCmd += "-p'"+password+"' ";
			}
		}
  } else if (arcType == "xz") {
    cmd     = fullPathName("xz");
    listCmd = fullPathName("xz");
    getCmd  = fullPathName("xz") + "-dc";
    copyCmd = TQString();
    delCmd  = TQString();
    putCmd  = TQString();
	} else {
		cmd     = TQString();
		listCmd = TQString();
		getCmd  = TQString();
		copyCmd = TQString();
		delCmd  = TQString();
		putCmd  = TQString();
	}

	if( TDEStandardDirs::findExe(cmd).isEmpty() ){
		error( TDEIO::ERR_CANNOT_LAUNCH_PROCESS,
		cmd+
		i18n("\nMake sure that the %1 binary are installed properly on your system.").arg(cmd));
		KRDEBUG("Failed to find cmd: " << cmd);
		return false;
	}
	return true;
}

bool tdeio_krarcProtocol::checkStatus( int exitCode ) {
	KRDEBUG( exitCode );

	if( arcType == "zip" || arcType == "rar" || arcType == "7z" )
		return exitCode == 0 || exitCode == 1;
	else if( arcType == "ace" || arcType == "bzip2" || arcType == "lha" || arcType == "rpm" || arcType == "arj" )
		return exitCode == 0;
	else if( arcType == "gzip"|| arcType == "xz" )
		return exitCode == 0 || exitCode == 2;
	else
		return exitCode == 0;
}

struct AutoDetectParams {
	TQString type;
	int location;
	TQString detectionString;
};

TQString tdeio_krarcProtocol::detectArchive( bool &encrypted, TQString fileName ) {
	static AutoDetectParams autoDetectParams[] = {{"zip",  0, "PK\x03\x04"},
	                                              {"rar",  0, "Rar!\x1a" },
	                                              {"arj",  0, "\x60\xea" },
	                                              {"rpm",  0, "\xed\xab\xee\xdb"},
	                                              {"ace",  7, "**ACE**" },
	                                              {"bzip2",0, "\x42\x5a\x68\x39\x31" },
	                                              {"gzip", 0, "\x1f\x8b"},
	                                              {"deb",  0, "!<arch>\ndebian-binary   " },
	                                              {"7z",   0, "7z\xbc\xaf\x27\x1c" },
	                                              {"xz",   0, "\xfd" "7zXZ\x00"} };
	static int autoDetectElems = sizeof( autoDetectParams ) / sizeof( AutoDetectParams );

	encrypted = false;

	TQFile arcFile( fileName );
	if ( arcFile.open( IO_ReadOnly ) ) {
		char buffer[ 1024 ];
		long sizeMax = arcFile.readBlock( buffer, sizeof( buffer ) );
		arcFile.close();

		for( int i=0; i < autoDetectElems; i++ ) {
			TQString detectionString = autoDetectParams[ i ].detectionString;
			int location = autoDetectParams[ i ].location;

			int endPtr = detectionString.length() + location;
			if( endPtr > sizeMax )
				continue;

			unsigned int j=0;
			for(; j != detectionString.length(); j++ ) {
				if( detectionString[ j ] == '?' )
					continue;
				if( buffer[ location + j ] != detectionString[ j ] )
					break;
			}

			if( j == detectionString.length() ) {
				TQString type = autoDetectParams[ i ].type;
				if( type == "bzip2" || type == "gzip" ) {
					KTar tapeArchive( fileName );
					if( tapeArchive.open( IO_ReadOnly ) ) {
						tapeArchive.close();
						if( type == "bzip2" )
							type = "tbz";
						else
							type = "tgz";
					}
				}
				else if( type == "zip" )
					encrypted = (buffer[6] & 1);
				else if( type == "arj" ) {
					if( sizeMax > 4 ) {
						long headerSize = ((unsigned char *)buffer)[ 2 ] + 256*((unsigned char *)buffer)[ 3 ];
						long fileHeader = headerSize + 10;
						if( fileHeader + 9 < sizeMax && buffer[ fileHeader ] == (char)0x60 && buffer[ fileHeader + 1 ] == (char)0xea )
							encrypted = (buffer[ fileHeader + 8 ] & 1 );
					}
				}
				else if( type == "rar" ) {
					if( sizeMax > 13 && buffer[ 9 ] == (char)0x73 ) {
						if( buffer[ 10 ] & 0x80 ) { // the header is encrypted?
							encrypted = true;
						} else {
							long offset = 7;
							long mainHeaderSize = ((unsigned char *)buffer)[ offset+5 ] + 256*((unsigned char *)buffer)[ offset+6 ];
							offset += mainHeaderSize;
							while( offset + 10 < sizeMax ) {
								long headerSize = ((unsigned char *)buffer)[ offset+5 ] + 256*((unsigned char *)buffer)[ offset+6 ];
								bool isDir = (buffer[ offset+7 ] == '\0' ) && (buffer[ offset+8 ] == '\0' ) &&
								             (buffer[ offset+9 ] == '\0' ) && (buffer[ offset+10 ] == '\0' );

								if( buffer[ offset + 2 ] != (char)0x74 )
									break;
								if( !isDir ) {
									encrypted = ( buffer[ offset + 3 ] & 4 ) != 0;
									break;
								}
								offset += headerSize;
							}
						}
					}
				}
				else if( type == "ace" ) {
						long offset = 0;
						long mainHeaderSize = ((unsigned char *)buffer)[ offset+2 ] + 256*((unsigned char *)buffer)[ offset+3 ] + 4;
						offset += mainHeaderSize;
						while( offset + 10 < sizeMax ) {
							long headerSize = ((unsigned char *)buffer)[ offset+2 ] + 256*((unsigned char *)buffer)[ offset+3 ] + 4;
							bool isDir = (buffer[ offset+11 ] == '\0' ) && (buffer[ offset+12 ] == '\0' ) &&
							             (buffer[ offset+13 ] == '\0' ) && (buffer[ offset+14 ] == '\0' );

							if( buffer[ offset + 4 ] != (char)0x01 )
								break;
							if( !isDir ) {
								encrypted = ( buffer[ offset + 6 ] & 64 ) != 0;
								break;
							}
							offset += headerSize;
						}
				}
				else if( type == "7z" ) {
					if( encryptedArchPath == fileName )
						encrypted = true;
					else {  // we try to find whether the 7z archive is encrypted
						// this is hard as the headers are also compresseds
						TQString tester = fullPathName( "7z" );
						if( TDEStandardDirs::findExe( tester ).isEmpty() ) {
							tester = fullPathName( "7za" );
							if( TDEStandardDirs::findExe( tester ).isEmpty() ) {
								return type;
							}
						}

						TQString testCmd = tester + " t -y ";
						lastData = encryptedArchPath = "";

						KrShellProcess proc;
						proc << testCmd << convertName( fileName );
						connect( &proc, TQ_SIGNAL( receivedStdout(TDEProcess*,char*,int) ),
						         this, TQ_SLOT( checkOutputForPassword( TDEProcess*,char*,int ) ) );
						proc.start(TDEProcess::Block,TDEProcess::AllOutput);
						encrypted = this->encrypted;

						if( encrypted )
							encryptedArchPath = fileName;
					}
				}
				return type;
			}
		}

		if( sizeMax >= 512 ) {
			/* checking if it's a tar file */
			unsigned checksum = 32*8;
			char chksum[ 9 ];
			for( int i=0; i != 512; i++ )
				checksum += ((unsigned char *)buffer)[ i ];
			for( int i=148; i != 156; i++ )
				checksum -= ((unsigned char *)buffer)[ i ];
			sprintf( chksum, "0%o", checksum );
			if( !memcmp( buffer + 148, chksum, strlen( chksum ) ) ) {
				int k = strlen( chksum );
				for(; k < 8; k++ )
					if( buffer[148+k] != 0 && buffer[148+k] != 32 )
						break;
				if( k==8 )
					return "tar";
			}
		}
	}

	if (fileName.endsWith(".tar.xz"))
	{
	  return "txz";
	}
	else if (fileName.endsWith(".xz"))
	{
	  return "xz";
	}

	return TQString();
}

void tdeio_krarcProtocol::checkOutputForPassword( TDEProcess *proc,char *buf,int len ) {
	TQByteArray d(len);
	d.setRawData(buf,len);
	TQString data =  TQString( d );
	d.resetRawData(buf,len);

	TQString checkable = lastData + data;

	TQStringList lines = TQStringList::split( '\n', checkable );
	lastData = lines[ lines.count() - 1 ];
	for( unsigned i=0; i != lines.count(); i++ ) {
		TQString line = lines[ i ].stripWhiteSpace().lower();
		int ndx = line.find( "testing" );
		if( ndx >=0 )
			line.truncate( ndx );
		if( line.isEmpty() )
			continue;

		if( line.contains( "password" ) && line.contains( "enter" ) ) {
			KRDEBUG( "Encrypted 7z archive found!" );
			encrypted = true;
			proc->kill();
		}
	}
}

void tdeio_krarcProtocol::invalidatePassword() {
	KRDEBUG( arcFile->url().path(-1) + "/" );

	if( !encrypted )
		return;

	TDEIO::AuthInfo authInfo;
	authInfo.caption= i18n( "Krarc Password Dialog" );
	authInfo.username= "archive";
	authInfo.readOnly = true;
	authInfo.keepPassword = true;
	authInfo.verifyPath = true;
	TQString fileName = arcFile->url().path(-1);
	authInfo.url = KURL::fromPathOrURL( "/" );
	authInfo.url.setHost( fileName /*.replace('/','_')*/ );
	authInfo.url.setProtocol( "krarc" );

	password = TQString();

	cacheAuthentication( authInfo );
}

TQString tdeio_krarcProtocol::getPassword() {
	KRDEBUG( encrypted );

	if( !password.isNull() )
		return password;
	if( !encrypted )
		return (password = "" );

	TDEIO::AuthInfo authInfo;
	authInfo.caption= i18n( "Krarc Password Dialog" );
	authInfo.username= "archive";
	authInfo.readOnly = true;
	authInfo.keepPassword = true;
	authInfo.verifyPath = true;
	TQString fileName = arcFile->url().path(-1);
	authInfo.url = KURL::fromPathOrURL( "/" );
	authInfo.url.setHost( fileName /*.replace('/','_')*/ );
	authInfo.url.setProtocol( "krarc" );

	if( checkCachedAuthentication( authInfo ) && !authInfo.password.isNull() ) {
		KRDEBUG( authInfo.password );
		return ( password = authInfo.password );
	}

	authInfo.password = TQString();

	if ( openPassDlg( authInfo, i18n("Accessing the file requires password.") ) && !authInfo.password.isNull() ) {
		KRDEBUG( authInfo.password );
		return ( password = authInfo.password );
	}

	KRDEBUG( password );
	return password;
}

TQString tdeio_krarcProtocol::fullPathName( TQString name ) {
	TQString supposedName = krConfig->readEntry( name, name );
	if( supposedName.isEmpty() )
		supposedName = name;
	return escape( supposedName );
}

TQString tdeio_krarcProtocol::convertFileName( TQString name ) {
	if( arcType == "zip" )
		name = name.replace( "[", "[[]" );
	return convertName( name );
}

TQString tdeio_krarcProtocol::convertName( TQString name ) {
	if( !name.contains( '\'' ) )
		return "'" + name + "'";
	if( !name.contains( '"' ) && !name.contains( '$' ) )
		return "\"" + name + "\"";
	return escape( name );
}

TQString tdeio_krarcProtocol::escape( TQString name ) {
	const TQString evilstuff = "\\\"'`()[]{}!?;$&<>| ";		// stuff that should get escaped

	for ( unsigned int i = 0; i < evilstuff.length(); ++i )
		name.replace( evilstuff[ i ], (TQString("\\") + evilstuff[ i ]) );

	return name;
}

#include "krarc.moc"
