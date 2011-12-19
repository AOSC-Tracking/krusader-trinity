/***************************************************************************
                                 krarchandler.h
                             -------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description: this class will supply static archive handling functions.
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
#ifndef KRARCHANDLER_H
#define KRARCHANDLER_H

#include <tqstringlist.h>
#include <tqobject.h>
#include <kprocess.h>
#include <kurl.h>
#include <kwallet.h>

class KRarcHandler: public TQObject {
  Q_OBJECT
  TQ_OBJECT
public:
  // return the number of files in the archive
  static long arcFileCount(TQString archive, TQString type, TQString password);
  // unpack an archive to destination directory
  static bool unpack(TQString archive, TQString type, TQString password, TQString dest );
  // pack an archive to destination directory
  static bool pack(TQStringList fileNames, TQString type, TQString dest, long count, TQMap<TQString,TQString> extraProps );
  // test an archive
  static bool test(TQString archive, TQString type, TQString password, long count = 0L );
  // true - if the right unpacker exist in the system
  static bool arcSupported(TQString type);
  // true - if supported and the user want us to handle this kind of archive
  static bool arcHandled(TQString type);
  // return the a list of supported packers
  static TQStringList supportedPackers();
  // true - if the url is an archive (ie: tar:/home/test/file.tar.bz2)
  static bool isArchive(const KURL& url);
  // used to determine the type of the archive
  static TQString getType( bool &encrypted, TQString fileName, TQString mime, bool checkEncrypted = true );
  // queries the password from the user
  static TQString getPassword( TQString path );
  // detects the archive type
  static TQString detectArchive( bool &encrypted, TQString fileName, bool checkEncrypted = true );
private:
  // checks if the returned status is correct
  static bool checkStatus( TQString type, int exitCode );

  static KWallet::Wallet * wallet;
};

class KrShellProcess : public KShellProcess {
	Q_OBJECT
  TQ_OBJECT
public:
	KrShellProcess() : KShellProcess(), errorMsg( TQString() ), outputMsg( TQString() ) {
		connect(this,TQT_SIGNAL(receivedStderr(KProcess*,char*,int)),
				this,TQT_SLOT(receivedErrorMsg(KProcess*,char*,int)) );
		connect(this,TQT_SIGNAL(receivedStdout(KProcess*,char*,int)),
				this,TQT_SLOT(receivedOutputMsg(KProcess*,char*,int)) );
	}
	
	TQString getErrorMsg() {
		if( errorMsg.stripWhiteSpace().isEmpty() )
			return outputMsg.right( 500 );
		else
			return errorMsg.right( 500 );
	}
	
public slots:
	void receivedErrorMsg(KProcess*, char *buf, int len) {
		errorMsg += TQString::fromLocal8Bit( buf, len );
		if( errorMsg.length() > 500 )
			errorMsg = errorMsg.right( 500 );
		receivedOutputMsg( 0, buf, len );
	}
	
	void receivedOutputMsg(KProcess*, char *buf, int len) {
		outputMsg += TQString::fromLocal8Bit( buf, len );
		if( outputMsg.length() > 500 )
			outputMsg = outputMsg.right( 500 );
	}
	
private:
	TQString errorMsg;
	TQString outputMsg;
};

class Kr7zEncryptionChecker : public KrShellProcess {
	Q_OBJECT
  TQ_OBJECT
	
public:
	Kr7zEncryptionChecker() : KrShellProcess(), encrypted( false ), lastData() {
		connect(this,TQT_SIGNAL(receivedStdout(KProcess*,char*,int)),
				this,TQT_SLOT(processStdout(KProcess*,char*,int)) );
	}

public slots:
	void processStdout( KProcess *proc,char *buf,int len ) {
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
				encrypted = true;
				proc->kill();
			}
		}
	}

	bool isEncrypted() { return encrypted; }
private:
	bool encrypted;
	TQString lastData;
};

#endif
