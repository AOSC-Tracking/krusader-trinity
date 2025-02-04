/***************************************************************************
                                krarchandler.cpp
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
#include <tqtextstream.h>
// TDE includes
#include <kprocess.h>
#include <tdetempfile.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <tdeio/passdlg.h>
#include <tqfile.h>
#include <kstandarddirs.h>
#include <ktar.h>
#include <tdeio/global.h>
// Krusader includes
#include "krarchandler.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../krservices.h"
#include "../Dialogs/krpleasewait.h"

static TQStringList arcProtocols = TQStringList::split(";", "tar;bzip;bzip2;gzip;krarc;zip;xz");

TDEWallet::Wallet * KRarcHandler::wallet = 0;

TQStringList KRarcHandler::supportedPackers() {
  TQStringList packers;

  // we will simply try to find the packers here..
  if ( KrServices::cmdExist( "tar" ) ) packers.append( "tar" );
  if ( KrServices::cmdExist( "gzip" ) ) packers.append( "gzip" );
  if ( KrServices::cmdExist( "bzip2" ) ) packers.append( "bzip2" );
  if ( KrServices::cmdExist( "unzip" ) ) packers.append( "unzip" );
  if ( KrServices::cmdExist( "zip" ) ) packers.append( "zip" );
  if ( KrServices::cmdExist( "lha" ) ) packers.append( "lha" );
  if ( KrServices::cmdExist( "cpio" ) ) packers.append( "cpio" );
  if ( KrServices::cmdExist( "unrar" ) ) packers.append( "unrar" );
  if ( KrServices::cmdExist( "rar" ) ) packers.append( "rar" );
  if ( KrServices::cmdExist( "arj" ) ) packers.append( "arj" );
  if ( KrServices::cmdExist( "unarj" ) ) packers.append( "unarj" );
  if ( KrServices::cmdExist( "unace" ) ) packers.append( "unace" );
  if ( KrServices::cmdExist( "dpkg"  ) ) packers.append( "dpkg"  );
  if ( KrServices::cmdExist( "7z"  ) || KrServices::cmdExist( "7za" ) ) packers.append( "7z"  );
  if ( KrServices::cmdExist( "rpm"   ) && KrServices::cmdExist( "rpm2cpio" ) ) packers.append( "rpm" );
  if ( KrServices::cmdExist( "xz" ) ) packers.append( "xz" );
  // kdDebug() << "Supported Packers:" << endl;
  //TQStringList::Iterator it;
  //for( it = packers.begin(); it != packers.end(); ++it )
  // kdDebug() << *it << endl;

  return packers;
  }

bool KRarcHandler::arcSupported( TQString type ) {
  // lst will contain the supported unpacker list...
  krConfig->setGroup( "Archives" );
  TQStringList lst = krConfig->readListEntry( "Supported Packers" );

  if ( type == "-zip" && lst.contains( "unzip" ) )
    return true;
  else if ( type == "-tar" && lst.contains( "tar" ) )
    return true;
  else if ( type == "-tbz" && lst.contains( "tar" ) )
    return true;
  else if ( type == "-tgz" && lst.contains( "tar" ) )
    return true;
  else if ( type == "-txz" && lst.contains( "tar" ) )
    return true;
  else if ( type == "tarz" && lst.contains( "tar" ) )
    return true;
  else if ( type == "gzip" && lst.contains( "gzip" ) )
    return true;
  else if ( type == "zip2" && lst.contains( "bzip2" ) )
    return true;
  else if ( type == "-lha" && lst.contains( "lha" ) )
    return true;
  else if ( type == "-ace" && lst.contains( "unace" ) )
    return true;
  else if ( type == "-rpm" && lst.contains( "cpio" ) )
    return true;
  else if ( type == "cpio" && lst.contains( "cpio" ) )
    return true;
  else if ( type == "-rar" && ( lst.contains( "unrar" ) || lst.contains( "rar" ) ) )
    return true;
  else if ( type == "-arj" && ( lst.contains( "unarj" ) || lst.contains( "arj" ) ) )
    return true;
  else if ( type == "-deb" && ( lst.contains( "dpkg"  ) && lst.contains( "tar" ) ) )
    return true;
  else if ( type == "-7z" && lst.contains( "7z" ) )
    return true;
  else if ( type == "-xz" && lst.contains( "xz" ) )
    return true;
  // not supported : (
  return false;
  }

bool KRarcHandler::arcHandled( TQString type ) {
  // first check if supported
  if ( !arcSupported( type ) ) return false;

  krConfig->setGroup( "Archives" );
  if ( ( type == "-tgz" && krConfig->readBoolEntry( "Do GZip" , _DoGZip ) ) ||
       ( type == "tarz" && krConfig->readBoolEntry( "Do GZip" , _DoGZip ) ) ||
       ( type == "-tar" && krConfig->readBoolEntry( "Do Tar"  , _DoTar ) ) ||
       ( type == "-tbz" && krConfig->readBoolEntry( "Do BZip2", _DoBZip2 ) ) ||
       ( type == "-txz" && krConfig->readBoolEntry( "Do Xz"   , _DoXz ) ) ||
       ( type == "gzip" && krConfig->readBoolEntry( "Do GZip" , _DoGZip ) ) ||
       ( type == "zip2" && krConfig->readBoolEntry( "Do BZip2", _DoBZip2 ) ) ||
       ( type == "-zip" && krConfig->readBoolEntry( "Do UnZip", _DoUnZip ) ) ||
       ( type == "-lha" && krConfig->readBoolEntry( "Do Lha"  , _DoUnZip ) ) ||
       ( type == "-rar" && krConfig->readBoolEntry( "Do UnRar", _DoUnRar ) ) ||
       ( type == "-arj" && krConfig->readBoolEntry( "Do UnArj", _DoUnarj ) ) ||
       ( type == "-ace" && krConfig->readBoolEntry( "Do UnAce", _DoUnAce ) ) ||
       ( type == "cpio" && krConfig->readBoolEntry( "Do RPM"  , _DoRPM ) ) ||
       ( type == "-rpm" && krConfig->readBoolEntry( "Do RPM"  , _DoRPM ) ) ||
       ( type == "-deb" && krConfig->readBoolEntry( "Do DEB"  , _DoDEB ) ) ||
       ( type == "-7z"  && krConfig->readBoolEntry( "Do 7z"   , _Do7z ) ) ||
       ( type == "-xz"  && krConfig->readBoolEntry( "Do Xz"   , _DoXz ) ) )
    return true;
  else
    return false;
  }


long KRarcHandler::arcFileCount( TQString archive, TQString type, TQString password ) {
  int divideWith = 1;

  // first check if supported
  if ( !arcSupported( type ) ) return 0;

  // bzip, gzip and xz archives contain only one file
  if ( type == "zip2" || type == "gzip" || type == "-xz") return 1L;

  // set the right lister to do the job
  TQString lister;

  if ( type == "-zip" ) lister = KrServices::fullPathName( "unzip" ) + " -ZTs";
  else if ( type == "-tar" ) lister = KrServices::fullPathName( "tar" ) + " -tvf";
  else if ( type == "-tgz" ) lister = KrServices::fullPathName( "tar" ) + " -tvzf";
  else if ( type == "tarz" ) lister = KrServices::fullPathName( "tar" ) + " -tvzf";
  else if ( type == "-tbz" ) lister = KrServices::fullPathName( "tar" ) + " -tjvf";
  else if ( type == "-txz" ) lister = KrServices::fullPathName( "tar" ) + " -tJvf";
  else if ( type == "-lha" ) lister = KrServices::fullPathName( "lha" ) + " l";
  else if ( type == "-rar" ) lister = KrServices::fullPathName( KrServices::cmdExist( "rar" ) ? "rar" : "unrar" ) + " l -v";
  else if ( type == "-ace" ) lister = KrServices::fullPathName( "unace" ) + " l";
  else if ( type == "-arj" ) { if( KrServices::cmdExist( "arj" ) )
                                 lister = KrServices::fullPathName( "arj" ) + " v -y -v",
                                 divideWith = 4;
                               else
                                 lister = KrServices::fullPathName( "unarj" ) + " l";
                             }
  else if ( type == "-rpm" ) lister = KrServices::fullPathName( "rpm" ) + " --dump -lpq";
  else if ( type == "-deb" ) lister = KrServices::fullPathName( "dpkg" ) + " -c";
  else if ( type == "-7z" )  lister = KrServices::fullPathName( "7z" ) + " -y l";
  else if ( type == "-xz" )  lister = KrServices::fullPathName( "xz" ) + " -l";
  else return 0L;

  if ( !password.isNull() ) {
    if ( type == "-arj" )
      lister += " -g'" + password + "'";
    if ( type == "-ace" || type == "-rar" || type == "-7z" )
      lister += " -p'" + password + "'";
  }

  // tell the user to wait
  krApp->startWaiting( i18n( "Counting files in archive" ), 0, true );

  // count the number of files in the archive
  long count = 1;
  KTempFile tmpFile( /*"tmp"*/ TQString(), "krusader-unpack" ); // commented out as it created files in the current dir!
  KrShellProcess list;
  list << lister << KrServices::quote( archive ) << ">" << tmpFile.name() ;
  if( type == "-ace" && TQFile( "/dev/ptmx" ).exists() )  // Don't remove, unace crashes if missing!!!
    list<< "<" << "/dev/ptmx";
  list.start( TDEProcess::NotifyOnExit, TDEProcess::AllOutput );
  while ( list.isRunning() ) {
    usleep( 1000 );
    tqApp->processEvents();
    if( krApp->wasWaitingCancelled() )
      list.kill();
    }
  ; // busy wait - need to find something better...

  krApp->stopWait();

  if( !list.normalExit() || !checkStatus( type, list.exitStatus() ) ) {
    KMessageBox::detailedError (krApp, i18n( "Failed to list the content of the archive (%1)!" ).arg( archive ),
                                list.getErrorMsg(), i18n("Error" ) );
    return 0;
  }

  TQTextStream *stream = tmpFile.textStream();
  while ( stream && stream->readLine() != TQString() ) ++count;
  tmpFile.unlink();

  //make sure you call stopWait after this function return...
  //krApp->stopWait();

  return count / divideWith;
  }

bool KRarcHandler::unpack( TQString archive, TQString type, TQString password, TQString dest ) {
  krConfig->setGroup( "Archives" );
  if ( krConfig->readBoolEntry( "Test Before Unpack", _TestBeforeUnpack ) ) {
    // test first - or be sorry later...
    if ( type != "-rpm" && type != "-deb" && !test( archive, type, password, 0 ) ) {
      KMessageBox::error( krApp, i18n( "Failed to unpack" ) + " \"" + archive + "\" !" );
      return false;
      }
    }

  // count the files in the archive
  long count = arcFileCount( archive, type, password );
  if ( count == 0 ) return false; // not supported
  if ( count == 1 ) count = 0 ;

  // choose the right packer for the job
  TQString packer, cpioName = TQString();

  // set the right packer to do the job
  if ( type == "-zip" ) packer = KrServices::fullPathName( "unzip" ) + " -o" ;
  else if ( type == "-tar" ) packer = KrServices::fullPathName( "tar" ) + " -xvf";
  else if ( type == "-tgz" ) packer = KrServices::fullPathName( "tar" ) + " -xvzf";
  else if ( type == "tarz" ) packer = KrServices::fullPathName( "tar" ) + " -xvzf";
  else if ( type == "-tbz" ) packer = KrServices::fullPathName( "tar" ) + " -xjvf";
  else if ( type == "-txz" ) packer = KrServices::fullPathName( "tar" ) + " -xJvf";
  else if ( type == "gzip" ) packer = KrServices::fullPathName( "gzip" ) + " -cd";
  else if ( type == "zip2" ) packer = KrServices::fullPathName( "bzip2" ) + " -cdk";
  else if ( type == "-lha" ) packer = KrServices::fullPathName( "lha" ) + " xf";
  else if ( type == "-rar" ) packer = KrServices::fullPathName( KrServices::cmdExist( "rar" ) ? "rar" : "unrar" ) + " -y x";
  else if ( type == "-ace" ) packer = KrServices::fullPathName( "unace" ) + " x";
  else if ( type == "-arj" ) packer = KrServices::cmdExist( "arj" ) ?
                                      KrServices::fullPathName( "arj" ) + " -y -v x" :
                                      KrServices::fullPathName( "unarj" ) + " x";
  else if ( type == "-7z" )  packer = KrServices::fullPathName( "7z" ) + " -y x";
  else if ( type == "-xz" )  packer = KrServices::fullPathName( "xz" ) + " -cdk";
  else if ( type == "-rpm" ) {
    TQString tempDir = locateLocal("tmp",TQString());

    cpioName = tempDir+"/contents.cpio";

    KrShellProcess cpio;
    cpio << KrServices::fullPathName( "rpm2cpio" ) << " " + KrServices::quote( archive ) << " > " << cpioName;
    cpio.start(TDEProcess::Block, TDEProcess::AllOutput );
    if( !cpio.normalExit() || !checkStatus( "cpio", cpio.exitStatus() ) ) {
      KMessageBox::detailedError (krApp, i18n( "Failed to convert rpm (%1) to cpio!" ).arg( archive ),
                                  cpio.getErrorMsg(), i18n("Error" ) );
      return 0;
    }

    archive = cpioName;
    packer = KrServices::fullPathName( "cpio" ) + " --force-local --no-absolute-filenames -iuvdF";
  }
  else if ( type == "-deb" ) {
    TQString tempDir = locateLocal("tmp",TQString());

    cpioName = tempDir+"/contents.tar";

    KrShellProcess dpkg;
    dpkg << KrServices::fullPathName( "dpkg" ) << " --fsys-tarfile " + KrServices::quote( archive ) << " > " << cpioName;
    dpkg.start(TDEProcess::Block, TDEProcess::AllOutput );
    if( !dpkg.normalExit() || !checkStatus( "-deb", dpkg.exitStatus() ) ) {
      KMessageBox::detailedError (krApp, i18n( "Failed to convert deb (%1) to tar!" ).arg( archive ),
                                  dpkg.getErrorMsg(), i18n("Error" ) );
      return 0;
    }

    archive = cpioName;
    packer = KrServices::fullPathName( "tar" ) + " xvf ";
  }
  else return false;

  if ( !password.isNull() ) {
    if ( type == "-zip" )
      packer += " -P '" + password + "'";
    if ( type == "-arj" )
      packer += " -g'" + password + "'";
    if ( type == "-ace" || type == "-rar" || type == "-7z" )
      packer += " -p'" + password + "'";
  }

  // unpack the files
  KrShellProcess proc;
  proc << packer << " " + KrServices::quote( archive );
  if( type == "zip2" || type=="gzip" ){
    TQString arcname = archive.mid(archive.findRev("/")+1);
    if( arcname.contains(".") ) arcname = arcname.left(arcname.findRev("."));
    proc << ">" << KrServices::quote( dest+"/"+arcname );
  }
  if( type == "-ace" && TQFile( "/dev/ptmx" ).exists() ) // Don't remove, unace crashes if missing!!!
    proc << "<" << "/dev/ptmx";

  TQString save = getcwd( 0, 0 );
  chdir( dest.local8Bit() );

  // tell the user to wait
  krApp->startWaiting( i18n( "Unpacking File(s)" ), count, true );
  if ( count != 0 ) {
    connect( &proc, TQ_SIGNAL( receivedStdout( TDEProcess*, char*, int ) ),
             krApp, TQ_SLOT( incProgress( TDEProcess*, char*, int ) ) );
    if( type == "-rpm" )
      connect( &proc, TQ_SIGNAL( receivedStderr( TDEProcess*, char*, int ) ),
               krApp, TQ_SLOT( incProgress( TDEProcess*, char*, int ) ) );
  }

  // start the unpacking process
  proc.start( TDEProcess::NotifyOnExit, TDEProcess::AllOutput );
  while ( proc.isRunning() ) {
    usleep( 1000 );
    tqApp->processEvents();
    if( krApp->wasWaitingCancelled() )
      proc.kill();
    }
  ; // busy wait - need to find something better...
  krApp->stopWait();
  chdir( save.local8Bit() );

  if( !cpioName.isEmpty() )
    TQFile( cpioName ).remove();    /* remove the cpio file */

  // check the return value
  if ( !proc.normalExit() || !checkStatus( type, proc.exitStatus() ) ) {
    KMessageBox::detailedError (krApp, i18n( "Failed to unpack %1!" ).arg( archive ),
                                krApp->wasWaitingCancelled() ? i18n( "User cancelled." ) :
                                proc.getErrorMsg(), i18n("Error" ) );
    return false;
    }
  return true; // SUCCESS
  }

bool KRarcHandler::test( TQString archive, TQString type, TQString password, long count ) {
  // choose the right packer for the job
  TQString packer;

  // set the right packer to do the job
  if ( type == "-zip" ) packer = KrServices::fullPathName( "unzip" ) + " -t";
  else if ( type == "-tar" ) packer = KrServices::fullPathName( "tar" ) + " -tvf";
  else if ( type == "-tgz" ) packer = KrServices::fullPathName( "tar" ) + " -tvzf";
  else if ( type == "tarz" ) packer = KrServices::fullPathName( "tar" ) + " -tvzf";
  else if ( type == "-tbz" ) packer = KrServices::fullPathName( "tar" ) + " -tjvf";
  else if ( type == "-txz" ) packer = KrServices::fullPathName( "tar" ) + " -tJvf";
  else if ( type == "gzip" ) packer = KrServices::fullPathName( "gzip" ) + " -tv";
  else if ( type == "zip2" ) packer = KrServices::fullPathName( "bzip2" ) + " -tv";
  else if ( type == "-rar" ) packer = KrServices::fullPathName( KrServices::cmdExist( "rar" ) ? "rar" : "unrar" ) + " t";
  else if ( type == "-ace" ) packer = KrServices::fullPathName( "unace" ) + " t";
  else if ( type == "-lha" ) packer = KrServices::fullPathName( "lha" ) + " t";
  else if ( type == "-arj" ) packer = KrServices::fullPathName( KrServices::cmdExist( "arj" ) ? "arj" : "unarj" ) + " t";
  else if ( type == "cpio" ) packer = KrServices::fullPathName( "cpio" ) + " --only-verify-crc -tvF" ;
  else if ( type == "-7z" )  packer = KrServices::fullPathName( "7z" ) + " -y t";
  else if ( type == "-xz" )  packer = KrServices::fullPathName( "xz" ) + " -tv";
  else return false;

  if ( !password.isNull() ) {
    if ( type == "-zip" )
      packer += " -P '" + password + "'";
    if ( type == "-arj" )
      packer += " -g'" + password + "'";
    if ( type == "-ace" || type == "-rar" || type == "-7z" )
      packer += " -p'" + password + "'";
  }

  // unpack the files
  KrShellProcess proc;
  proc << packer << KrServices::quote( archive );

  if( type == "-ace" && TQFile( "/dev/ptmx" ).exists() ) // Don't remove, unace crashes if missing!!!
    proc << "<" << "/dev/ptmx";

  // tell the user to wait
  krApp->startWaiting( i18n( "Testing Archive" ), count, true );
  if ( count != 0 ) connect( &proc, TQ_SIGNAL( receivedStdout( TDEProcess*, char*, int ) ),
                               krApp, TQ_SLOT( incProgress( TDEProcess*, char*, int ) ) );

  // start the unpacking process
  proc.start( TDEProcess::NotifyOnExit, TDEProcess::AllOutput );
  while ( proc.isRunning() ) {
    usleep( 1000 );
    tqApp->processEvents();
    if( krApp->wasWaitingCancelled() )
      proc.kill();
    }
  ; // busy wait - need to find something better...
  krApp->stopWait();

  // check the return value
  if ( !proc.normalExit() || !checkStatus( type, proc.exitStatus() ) )
    return false;

  return true; // SUCCESS
  }

bool KRarcHandler::pack( TQStringList fileNames, TQString type, TQString dest, long count, TQMap<TQString,TQString> extraProps ) {
  // set the right packer to do the job
  TQString packer;

  if      ( type == "zip" ) { packer = KrServices::fullPathName( "zip" ) + " -ry"; type = "-zip"; }
  else if ( type == "tar" ) { packer = KrServices::fullPathName( "tar" ) + " -cvf"; type = "-tar"; }
  else if ( type == "tar.gz" ) { packer = KrServices::fullPathName( "tar" ) + " -cvzf"; type = "-tgz"; }
  else if ( type == "tar.bz2" ) { packer = KrServices::fullPathName( "tar" ) + " -cvjf"; type = "-tbz"; }
  else if ( type == "tar.xz" ) { packer = KrServices::fullPathName( "tar" ) + " -cvJf"; type = "-txz"; }
  else if ( type == "rar" ) { packer = KrServices::fullPathName( "rar" ) + " -r a"; type = "-rar"; }
  else if ( type == "lha" ) { packer = KrServices::fullPathName( "lha" ) + " a"; type = "-lha"; }
  else if ( type == "arj" ) { packer = KrServices::fullPathName( "arj" ) + " -r -y a"; type = "-arj"; }
  else if ( type == "7z" ) {  packer = KrServices::fullPathName( "7z" ) + " -y a"; type = "-7z"; }
  else if ( type == "xz" ) {  packer = KrServices::fullPathName( "xz" ) + " -z"; type = "-xz"; }
  else return false;

  TQString password = TQString();

  if( extraProps.count( "Password" ) > 0 ) {
    password = extraProps[ "Password" ];

    if ( !password.isNull() ) {
      if ( type == "-zip" )
        packer += " -P '" + password + "'";
      else if ( type == "-arj" )
        packer += " -g'" + password + "'";
      else if ( type == "-ace" || type == "-7z" )
        packer += " -p'" + password + "'";
      else if ( type == "-rar" ) {
        if( extraProps.count( "EncryptHeaders" ) > 0 )
          packer += " -hp'" + password + "'";
        else
          packer += " -p'" + password + "'";
      }
      else
        password = TQString();
    }
  }

  if( extraProps.count( "VolumeSize" ) > 0 ) {
     TQString sizeStr = extraProps[ "VolumeSize" ];
     TDEIO::filesize_t size = sizeStr.toLongLong();

     if( size >= 10000 ) {
       if( type == "-arj" || type == "-rar" )
           packer += TQString( " -v%1b" ).arg( sizeStr );
     }
  }

  if( extraProps.count( "CompressionLevel" ) > 0 ) {
     int level = extraProps[ "CompressionLevel" ].toInt() - 1;
     if ( level < 0 )
       level = 0;
     if ( level > 8 )
       level = 8;

     if( type == "-rar" ) {
       static const int rarLevels[] = { 0, 1, 2, 2, 3, 3, 4, 4, 5 };
       packer += TQString( " -m%1" ).arg( rarLevels[ level ] );
     }
     else if( type == "-arj" ) {
       static const int arjLevels[] = { 0, 4, 4, 3, 3, 2, 2, 1, 1 };
       packer += TQString( " -m%1" ).arg( arjLevels[ level ] );
     }
     else if( type == "-zip" ) {
       static const int zipLevels[] = { 0, 1, 2, 4, 5, 6, 7, 8, 9 };
       packer += TQString( " -%1" ).arg( zipLevels[ level ] );
     }
     else if( type == "-7z" ) {
       static const int sevenZipLevels[] = { 0, 1, 2, 4, 5, 6, 7, 8, 9 };
       packer += TQString( " -mx%1" ).arg( sevenZipLevels[ level ] );
     }
     else if( type == "-xz" ) {
       static const int xzLevels[] = { 0, 1, 2, 4, 5, 6, 7, 8, 9 };
       packer += TQString( " -%1" ).arg( xzLevels[ level ] );
     }
  }

  if( extraProps.count( "CommandLineSwitches" ) > 0 )
     packer += TQString( " %1" ).arg( extraProps[ "CommandLineSwitches" ] );

  // prepare to pack
  KrShellProcess proc;
  proc << packer << KrServices::quote( dest );

  for ( TQStringList::Iterator file = fileNames.begin(); file != fileNames.end(); ++file ) {
    proc << KrServices::quote( *file );
    }

  // tell the user to wait
  krApp->startWaiting( i18n( "Packing File(s)" ), count, true );
  if ( count != 0 )
    connect( &proc, TQ_SIGNAL( receivedStdout( TDEProcess*, char*, int ) ),
             krApp, TQ_SLOT( incProgress( TDEProcess*, char*, int ) ) );

  // start the packing process
  proc.start( TDEProcess::NotifyOnExit, TDEProcess::AllOutput );
  while ( proc.isRunning() ) {
    usleep( 1000 );
    tqApp->processEvents();
    if( krApp->wasWaitingCancelled() )
      proc.kill();
    }
  ; // busy wait - need to find something better...
  krApp->stopWait();

  // check the return value
  if ( !proc.normalExit() || !checkStatus( type, proc.exitStatus() ) ) {
    KMessageBox::detailedError (krApp, i18n( "Failed to pack %1!" ).arg( dest ),
                                krApp->wasWaitingCancelled() ? i18n( "User cancelled." ) : proc.getErrorMsg(),
                                i18n("Error" ) );
    return false;
    }

  krConfig->setGroup( "Archives" );
  if ( krConfig->readBoolEntry( "Test Archives", _TestArchives ) &&
       !test( dest, type, password, count ) ) {
    KMessageBox::error( krApp, i18n( "Failed to pack: " ) + dest, i18n( "Error" ) );
    return false;
    }
  return true; // SUCCESS
  }

TQString KRarcHandler::getPassword( TQString path ) {
	TQString password;

	TQString key = "krarc-" + path;

	if( !TDEWallet::Wallet::keyDoesNotExist(TDEWallet::Wallet::NetworkWallet(), TDEWallet::Wallet::PasswordFolder(), key ) ) {
		if( !TDEWallet::Wallet::isOpen( TDEWallet::Wallet::NetworkWallet() )  && wallet != 0 ) {
			delete wallet;
			wallet = 0;
		}
		if( wallet == 0 )
			wallet = TDEWallet::Wallet::openWallet( TDEWallet::Wallet::NetworkWallet() );
		if ( wallet && wallet->hasFolder( TDEWallet::Wallet::PasswordFolder() ) ) {
			wallet->setFolder( TDEWallet::Wallet::PasswordFolder() );
			TQMap<TQString,TQString> map;
			if ( wallet->readMap( key, map ) == 0 ) {
				TQMap<TQString, TQString>::ConstIterator it = map.find( "password" );
				if ( it != map.end() )
					password = it.data();
			}
		}
	}

	bool keep = true;
	TQString user = "archive";
	TDEIO::PasswordDialog passDlg( i18n("This archive is encrypted, please supply the password:"),
	                             user, true );
	passDlg.setPassword( password );
	if (passDlg.exec() == TDEIO::PasswordDialog::Accepted) {
		password = passDlg.password();
		if ( keep ) {
			if( !TDEWallet::Wallet::isOpen( TDEWallet::Wallet::NetworkWallet() ) && wallet != 0 ) {
				delete wallet;
				wallet = 0;
			}
			if ( !wallet )
				wallet = TDEWallet::Wallet::openWallet( TDEWallet::Wallet::NetworkWallet() );
			if ( wallet ) {
				bool ok = true;
				if ( !wallet->hasFolder( TDEWallet::Wallet::PasswordFolder() ) )
					ok = wallet->createFolder( TDEWallet::Wallet::PasswordFolder() );
				if ( ok ) {
					wallet->setFolder( TDEWallet::Wallet::PasswordFolder() );
					TQMap<TQString,TQString> map;
					map.insert( "login", "archive" );
					map.insert( "password", password );
					wallet->writeMap( key, map );
				}
			}
		}
		return password;
	}

	return "";
}

bool KRarcHandler::isArchive(const KURL& url) {
	TQString protocol = url.protocol();
	if (arcProtocols.find(protocol) != arcProtocols.end())
		return true;
	else return false;
}

TQString KRarcHandler::getType( bool &encrypted, TQString fileName, TQString mime, bool checkEncrypted ) {
	TQString result = detectArchive( encrypted, fileName, checkEncrypted );
	if( result.isNull() )
		result = mime;
	else
		result = "-" + result;

	if (result.endsWith("-7z"))
	{
		result = "-7z";
	}
  if (result.endsWith("-xz"))
  {
    result = "-xz";
  }

	return result.right( 4 );
}


bool KRarcHandler::checkStatus( TQString type, int exitCode ) {
	if( type == "-zip" || type == "-rar" || type == "-7z" )
		return exitCode == 0 || exitCode == 1;
	else if( type == "-ace" || type == "zip2" || type == "-lha" || type == "-rpm" || type == "cpio" ||
	         type == "-tar" || type == "tarz" || type == "-tbz" || type == "-tgz" || type == "-arj" ||
	         type == "-deb" || type == "-txz" || type == "-xz")
		return exitCode == 0;
	else if (type == "gzip" || type == "-xz")
		return exitCode == 0 || exitCode == 2;
	else
		return exitCode == 0;
}

struct AutoDetectParams {
	TQString type;
	int location;
	TQString detectionString;
};

TQString KRarcHandler::detectArchive( bool &encrypted, TQString fileName, bool checkEncrypted ) {
	static AutoDetectParams autoDetectParams[] = {{"zip",  0, "PK\x03\x04"},
	                                              {"rar",  0, "Rar!\x1a" },
	                                              {"arj",  0, "\x60\xea" },
	                                              {"rpm",  0, "\xed\xab\xee\xdb"},
	                                              {"ace",  7, "**ACE**" },
	                                              {"bzip2",0, "\x42\x5a\x68\x39\x31" },
	                                              {"gzip", 0, "\x1f\x8b"},
	                                              {"deb",  0, "!<arch>\ndebian-binary   " },
	                                              {"7z",   0, "7z\xbc\xaf\x27\x1c" },
	                                              {"xz",   0, "\xfd" "7zXZ\x00" } };
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
					if( checkEncrypted ) { // encryption check is expensive
					                       // check only if it's necessary
						Kr7zEncryptionChecker proc;
						proc << KrServices::fullPathName( "7z" ) << " -y t";
						proc << KrServices::quote( fileName );
						proc.start(TDEProcess::Block,TDEProcess::AllOutput);
						encrypted = proc.isEncrypted();
					}
				}
				else if( type == "xz" && (fileName.endsWith(".tar.xz") || fileName.endsWith(".txz")) )
				    type = "txz";
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

	return TQString();
}

#include "krarchandler.moc"

