/***************************************************************************
                              virt.cpp
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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include <tqfile.h>
#include <kurl.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdeversion.h>
#include <kinstance.h>
#include <tdemessagebox.h>

#include "virt.h"

using namespace TDEIO;

#define VIRT_VFS_DB "virt_vfs.db"
#define VIRT_PROTOCOL "virt"

#if KDE_IS_VERSION(3,4,0)
extern "C" { int TDE_EXPORT kdemain( int argc, char **argv ); }
#else
extern "C" { int kdemain( int argc, char **argv ); }
#endif

#define KrDEBUG(X...){\
	FILE* f = fopen("/tmp/tdeio_virt.log","a+");\
	fprintf(f,X);\
	fclose(f);\
}

TQDict<KURL::List> VirtProtocol::kioVirtDict;
TDEConfig* VirtProtocol::tdeio_virt_db;

int kdemain( int argc, char **argv ) {
	TDEInstance instance( "tdeio_virt" );

	if ( argc != 4 ) {
		fprintf( stderr, "Usage: tdeio_virt protocol domain-socket1 domain-socket2\n" );
		exit( -1 );
	}

	VirtProtocol slave( argv[ 2 ], argv[ 3 ] );
	slave.dispatchLoop();

	return 0;
}

VirtProtocol::VirtProtocol( const TQCString &pool, const TQCString &app ) : SlaveBase( "virt", pool, app ) {
	tdeio_virt_db = new TDEConfig(VIRT_VFS_DB,false,"data");
}

VirtProtocol::~VirtProtocol() {
	delete tdeio_virt_db;
}

void VirtProtocol::del(KURL const & /*url */, bool /* isFile */ ){
//	KRDEBUG(url.path());
	
	messageBox(TDEIO::SlaveBase::QuestionYesNo,
	                         "",
	                         i18n("Virtulal delete"),
	                         i18n("remove from virtual space"),
	                         i18n("really delete")
	                         );

	finished();
}

void VirtProtocol::copy( const KURL &src, const KURL &dest, int /* permissions */, bool /* overwrite */ ){
	TQString path = dest.path( -1 ).mid( 1 );
	path = path.left(path.findRev("/"));
	if ( path.isEmpty() ) path = "/";

	if( addDir(path) ){
		kioVirtDict[ path ]->append(src);
		save();
	}

	finished();
}

bool VirtProtocol::addDir(TQString& path){

	if( kioVirtDict[ path ] ) return true;

	TQString updir;
	if( !path.contains("/") ) updir = "/";
	else updir = path.left(path.findRev("/"));
	TQString name = path.mid(path.findRev("/")+1);

	if( addDir(updir) ){ 
		KURL url;
		if( updir == "/" ) url = TQString("virt:/")+name;
		else url = TQString("virt:/")+updir+"/"+name;
		kioVirtDict[ updir ]->append( url );

		KURL::List* temp = new KURL::List();
		kioVirtDict.replace( path, temp );

		return true;
	}
	return false;
}

void VirtProtocol::mkdir(const KURL& url,int){
	if( url.protocol() != VIRT_PROTOCOL ){
		redirection(url);
		finished();
		return;
	}

	TQString path = url.path( -1 ).mid( 1 );
	if ( path.isEmpty() ) path = "/";

	if( kioVirtDict[ path ] ){
		error( TDEIO::ERR_DIR_ALREADY_EXIST, url.path() );
		return;
	}

	addDir(path);

	save();

	finished();
}

void VirtProtocol::listDir( const KURL & url ) {
	if( url.protocol() != VIRT_PROTOCOL ){
		redirection(url);
		finished();
		return;
	}

	load();	

	TQString path = url.path( -1 ).mid( 1 );
	if ( path.isEmpty() ) path = "/";

	KURL::List* urlList = kioVirtDict[ path ];
	if ( !urlList ) {
		error(ERR_CANNOT_ENTER_DIRECTORY,url.path());
		return;
	}

	UDSEntryList dirList;
	KURL::List::iterator it;
	for ( it = urlList->begin() ; it != urlList->end() ; ++it ) {
		KURL entry_url = *it;
		// translate url->UDS_ENTRY
		UDSEntry entry;
		if( entry_url.protocol() == VIRT_PROTOCOL ){
			local_entry(entry_url,entry);
		} else {
			UDSAtom atom;

			atom.m_uds = UDS_NAME;
			atom.m_str = url.isLocalFile() ? url.path() : entry_url.prettyURL();
			entry.append(atom);
	
			atom.m_uds = UDS_URL;
			atom.m_str = entry_url.url();
			entry.append(atom);
		}

		dirList.append(entry);
	}

	totalSize(dirList.size());
	listEntries(dirList);

	finished();
}

void VirtProtocol::stat( const KURL & url ) {
	if( url.protocol() != VIRT_PROTOCOL ){
		redirection(url);
		finished();
		return;
	}
	
	UDSEntry entry;
	local_entry(url,entry);	

	statEntry(entry);

	finished();
}

void VirtProtocol::get( const KURL & url ) {
	if( url.protocol() != VIRT_PROTOCOL ){
		redirection(url);
		finished();
		return;
	}

	finished();
}

bool VirtProtocol::rewriteURL(const KURL& /* src */, KURL&){ 
	return true; 
}

bool VirtProtocol::save(){
	lock();

	TDEConfig* db = new TDEConfig(VIRT_VFS_DB,false,"data");;
	
	db->setGroup("virt_db");
	TQDictIterator<KURL::List> it( kioVirtDict ); // See TQDictIterator
	for( ; it.current(); ++it ){
		KURL::List::iterator url;
		TQStringList entry;
		for ( url = it.current()->begin() ; url != it.current()->end() ; ++url ) {
			entry.append( (*url).url() );
		}
		db->writeEntry(it.currentKey(),entry);
	}
	
	db->sync();
	delete(db);
	
	unlock();

	return true;
}

bool VirtProtocol::load(){
	lock();

	TDEConfig* db = new TDEConfig(VIRT_VFS_DB,false,"data");
	db->setGroup("virt_db");
	
	TQMap<TQString, TQString> map = db->entryMap("virt_db");
	TQMap<TQString, TQString>::Iterator it;
	KURL::List* urlList;
	for ( it = map.begin(); it != map.end(); ++it ) {
		urlList = new KURL::List( db->readListEntry(it.key()) );
		kioVirtDict.replace( it.key(),urlList );
	}

	if( !kioVirtDict["/" ]){
		urlList = new KURL::List();
		kioVirtDict.replace( "/", urlList );	
	}

	unlock();

	delete(db);
		
	return true;
}

void VirtProtocol::local_entry(const KURL& url,UDSEntry& entry){
	TQString path = url.path( -1 ).mid( 1 );
	if ( path.isEmpty() ) path = "/";

	UDSAtom atom;

	atom.m_uds = UDS_NAME;
	atom.m_str = url.fileName();
	entry.append(atom);

	atom.m_uds = UDS_URL;
	atom.m_str = url.url();
	entry.append(atom);
	
	atom.m_uds = UDS_FILE_TYPE;
	atom.m_long = S_IFDIR;
	entry.append(atom);

	atom.m_uds = UDS_ACCESS;
	atom.m_long = 0700;
	entry.append(atom);

	atom.m_uds = UDS_MIME_TYPE;
	atom.m_str = "inode/system_directory";
	entry.append(atom);
}

bool VirtProtocol::lock(){
	return true;
}

bool VirtProtocol::unlock(){
	return true;
}
