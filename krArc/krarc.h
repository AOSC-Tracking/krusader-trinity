/***************************************************************************
                                   krarc.h
                             -------------------
    begin                : Sat Jun 14 14:42:49 IDT 2003
    copyright            : (C) 2003 by Rafi Yanai & Shie Erlich
    email                : yanai@users.sf.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __krarc_h__
#define __krarc_h__

#include <tqstring.h>
#include <tqcstring.h>
#include <tqdict.h>
#include <tqfile.h>
#include <sys/types.h>

#include <kurl.h>
#include <tdeio/global.h>
#include <tdeio/slavebase.h>
#include <kprocess.h>

class TDEProcess;
class KFileItem;
class TQCString;

class kio_krarcProtocol : public TQObject, public TDEIO::SlaveBase {
Q_OBJECT
  
public:
	kio_krarcProtocol(const TQCString &pool_socket, const TQCString &app_socket);
	virtual ~kio_krarcProtocol();
	virtual void stat( const KURL & url );
	virtual void get(const KURL& url);
	virtual void put(const KURL& url,int permissions,bool overwrite,bool resume);
	virtual void mkdir(const KURL& url,int permissions);
	virtual void listDir(const KURL& url);
	virtual void del(KURL const & url, bool isFile);
	virtual void copy (const KURL &src, const KURL &dest, int permissions, bool overwrite);

public slots:
	void receivedData(TDEProcess* proc,char* buf,int len);
	void checkOutputForPassword( TDEProcess*,char*,int );

protected:
	virtual bool   initDirDict(const KURL& url,bool forced = false);
	virtual bool   initArcParameters();
	TQString detectArchive( bool &encrypted, TQString fileName );
	virtual void parseLine(int lineNo, TQString line, TQFile* temp);
	virtual bool setArcFile(const KURL& url);
	virtual TQString getPassword();
	virtual void invalidatePassword();

	// archive specific commands
	TQString cmd;     ///< the archiver name.
	TQString listCmd; ///< list files. 
	TQString getCmd;  ///< unpack files command.
	TQString delCmd;  ///< delete files command.
	TQString putCmd;  ///< add file command.
	TQString copyCmd; ///< copy to file command.

private:
	void get(const KURL& url, int tries);
	/** checks if the exit code is OK. */
	bool checkStatus( int exitCode );
	/** service function for parseLine. */
	TQString nextWord(TQString &s,char d=' ');
	/** translate permittion string to mode_t. */
	mode_t parsePermString(TQString perm);
	/** return the name of the directory inside the archive. */
	TQString findArcDirectory(const KURL& url);
	/** find the UDSEntry of a file in a directory. */
	TDEIO::UDSEntry* findFileEntry(const KURL& url);
	/** add a new directory (file list container). */
	TDEIO::UDSEntryList* addNewDir(TQString path);
	TQString fullPathName( TQString name );
	TQString convertFileName( TQString name );
	static TQString convertName( TQString name );
	static TQString escape( TQString name );
	
	TQDict<TDEIO::UDSEntryList> dirDict; //< the directoris data structure.
	bool encrypted;                   //< tells whether the archive is encrypted
	bool archiveChanged;              //< true if the archive was changed.
	bool archiveChanging;             //< true if the archive is currently changing.
	bool newArchiveURL;               //< true if new archive was entered for the protocol
	TDEIO::filesize_t decompressedLen;  //< the number of the decompressed bytes
	KFileItem* arcFile;               //< the archive file item.
	TQString arcPath;                  //< the archive location
	TQString arcTempDir;               //< the currently used temp directory.
	TQString arcType;                  //< the archive type.
	bool extArcReady;                 //< Used for RPM & DEB files.
	TQString password;                 //< Password for the archives
	TDEConfig *krConfig;                //< The configuration file for krusader
	
	TQString lastData;
	TQString encryptedArchPath;
};

class KrShellProcess : public KShellProcess {
	Q_OBJECT
  
public:
	KrShellProcess() : KShellProcess(), errorMsg( TQString() ), outputMsg( TQString() ) {
		connect(this,TQT_SIGNAL(receivedStderr(TDEProcess*,char*,int)),
				this,TQT_SLOT(receivedErrorMsg(TDEProcess*,char*,int)) );
		connect(this,TQT_SIGNAL(receivedStdout(TDEProcess*,char*,int)),
				this,TQT_SLOT(receivedOutputMsg(TDEProcess*,char*,int)) );
	}
	
	TQString getErrorMsg() {
		if( errorMsg.stripWhiteSpace().isEmpty() )
			return outputMsg.right( 500 );
		else
			return errorMsg.right( 500 );
	}
	
public slots:
	void receivedErrorMsg(TDEProcess*, char *buf, int len) {
		errorMsg += TQString::fromLocal8Bit( buf, len );
		if( errorMsg.length() > 500 )
			errorMsg = errorMsg.right( 500 );
		receivedOutputMsg( 0, buf, len );
	}
	
	void receivedOutputMsg(TDEProcess*, char *buf, int len) {
		outputMsg += TQString::fromLocal8Bit( buf, len );
		if( outputMsg.length() > 500 )
			outputMsg = outputMsg.right( 500 );
	}
	
private:
	TQString errorMsg;
	TQString outputMsg;
};

#endif
