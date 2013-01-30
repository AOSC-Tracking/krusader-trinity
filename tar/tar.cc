
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include <tqfile.h>
#include <tqfileinfo.h>
#include <tqptrqueue.h>
#include <kurl.h>
#include <kdebug.h>
#include <kinstance.h>
#include <ktar.h>
#include <kzip.h>
#include <kar.h>
#include <kmimemagic.h>
#include <klocale.h>
#include <tdeversion.h>

#include <errno.h> // to be removed

#include "tar.h"

using namespace TDEIO;

#if KDE_IS_VERSION(3,4,0)
extern "C" { int KDE_EXPORT kdemain( int argc, char **argv ); }
#else
extern "C" { int kdemain( int argc, char **argv ); }
#endif

int kdemain( int argc, char **argv ) {
	TDEInstance instance( "tdeio_tar" );

	kdDebug( 7109 ) << "Starting " << getpid() << endl;

	if ( argc != 4 ) {
		fprintf( stderr, "Usage: tdeio_tar protocol domain-socket1 domain-socket2\n" );
		exit( -1 );
	}

	ArchiveProtocol slave( argv[ 2 ], argv[ 3 ] );
	slave.dispatchLoop();

	kdDebug( 7109 ) << "Done" << endl;
	return 0;
}

ArchiveProtocol::ArchiveProtocol( const TQCString &pool, const TQCString &app ) : SlaveBase( "tar", pool, app ) {
	kdDebug( 7109 ) << "ArchiveProtocol::ArchiveProtocol" << endl;
	m_archiveFile = 0L;
}

ArchiveProtocol::~ArchiveProtocol() {
	delete m_archiveFile;
}

void ArchiveProtocol::put( const KURL& url, int, bool, bool resume ){
	if( resume ){
		error(ERR_UNSUPPORTED_ACTION,i18n("This protocol does not support resuming") );	
		return;
	}
	
	TQByteArray  tempBuffer;
	TQPtrQueue<TQByteArray> buffer;
	buffer.setAutoDelete(true);
	int readResult=0;
	int size = 0;
	TQByteArray* temp;
	do{
		dataReq();
		temp = new TQByteArray();
		readResult = readData(*temp);
		buffer.enqueue(temp);
		size += temp->size();
 	} while( readResult > 0 );
	
	TQString filename = url.path().mid(m_archiveName.length()+1);

	if( !m_archiveFile->prepareWriting(filename,user,group,size) ){
		error(ERR_UNSUPPORTED_ACTION,
		      i18n("Writing to %1 is not supported").arg(filename) );
		return;
	}
	while( (temp=buffer.dequeue()) ){
		m_archiveFile->writeData(temp->data(),temp->size());
	}
	m_archiveFile->doneWriting(size);

	finished();
}

void ArchiveProtocol::mkdir(const KURL& url,int){
	TQString filename = url.path().mid(m_archiveName.length()+1);
	m_archiveFile->writeDir(filename,user,group);
	finished();
}

bool ArchiveProtocol::checkNewFile( const KURL & url, TQString & path ) {
	TQString fullPath = url.path();
	kdDebug( 7109 ) << "ArchiveProtocol::checkNewFile " << fullPath << endl;


	// Are we already looking at that file ?
	if ( m_archiveFile && m_archiveName == fullPath.left( m_archiveName.length() ) ) {
		// Has it changed ?
		struct stat statbuf;
		if ( ::stat( TQFile::encodeName( m_archiveName ), &statbuf ) == 0 ) {
			if ( m_mtime == statbuf.st_mtime ) {
				path = fullPath.mid( m_archiveName.length() );
				kdDebug( 7109 ) << "ArchiveProtocol::checkNewFile returning " << path << endl;
				return true;
			}
		}
	}
	kdDebug( 7109 ) << "Need to open a new file" << endl;

	// Close previous file
	if ( m_archiveFile ) {
		m_archiveFile->close();
		delete m_archiveFile;
		m_archiveFile = 0L;
	}

	// Find where the tar file is in the full path
	int pos = 0;
	TQString archiveFile;
	path = TQString();

	int len = fullPath.length();
	if ( len != 0 && fullPath[ len - 1 ] != '/' )
		fullPath += '/';

	kdDebug( 7109 ) << "the full path is " << fullPath << endl;
	while ( ( pos = fullPath.find( '/', pos + 1 ) ) != -1 ) {
		TQString tryPath = fullPath.left( pos );
		kdDebug( 7109 ) << fullPath << "  trying " << tryPath << endl;
		struct stat statbuf;
		if ( ::stat( TQFile::encodeName( tryPath ), &statbuf ) == 0 && !S_ISDIR( statbuf.st_mode ) ) {
			archiveFile = tryPath;
			m_mtime = statbuf.st_mtime;
			user = TQFileInfo(archiveFile).owner();
			group = TQFileInfo(archiveFile).group();
			path = fullPath.mid( pos + 1 );
			kdDebug( 7109 ) << "fullPath=" << fullPath << " path=" << path << endl;
			len = path.length();
			if ( len > 1 ) {
				if ( path[ len - 1 ] == '/' )
					path.truncate( len - 1 );
			} else
				path = TQString::fromLatin1( "/" );
			kdDebug( 7109 ) << "Found. archiveFile=" << archiveFile << " path=" << path << endl;
			break;
		}
	}
	if ( archiveFile.isEmpty() ) {
		kdDebug( 7109 ) << "ArchiveProtocol::checkNewFile: not found" << endl;
		return false;
	}

	// Open new file
	if ( url.protocol() == "tar" ) {
		kdDebug( 7109 ) << "Opening KTar on " << archiveFile << endl;
		m_archiveFile = new KTar( archiveFile );
	} else if ( url.protocol() == "ar" ) {
		kdDebug( 7109 ) << "Opening KAr on " << archiveFile << endl;
		m_archiveFile = new KAr( archiveFile );
	} else if ( url.protocol() == "zip" ) {
		kdDebug( 7109 ) << "Opening KZip on " << archiveFile << endl;
		m_archiveFile = new KZip( archiveFile );
	} else {
		kdWarning( 7109 ) << "Protocol " << url.protocol() << " not supported by this IOSlave" << endl;
		return false;
	}

	if ( !m_archiveFile->open( IO_ReadWrite ) ) {
		kdDebug( 7109 ) << "Opening " << archiveFile << "failed." << endl;
		delete m_archiveFile;
		m_archiveFile = 0L;
		return false;
	}

	m_archiveName = archiveFile;
	return true;
}


void ArchiveProtocol::createUDSEntry( const KArchiveEntry * archiveEntry, UDSEntry & entry ) {
	UDSAtom atom;
	entry.clear();
	atom.m_uds = UDS_NAME;
	atom.m_str = archiveEntry->name();
	entry.append( atom );

	atom.m_uds = UDS_FILE_TYPE;
	atom.m_long = archiveEntry->permissions() & S_IFMT; // keep file type only
	entry.append( atom );

	atom.m_uds = UDS_SIZE;
	atom.m_long = archiveEntry->isFile() ? ( ( KArchiveFile * ) archiveEntry ) ->size() : 0L ;
	entry.append( atom );

	atom.m_uds = UDS_MODIFICATION_TIME;
	atom.m_long = archiveEntry->date();
	entry.append( atom );

	atom.m_uds = UDS_ACCESS;
	atom.m_long = archiveEntry->permissions() & 07777; // keep permissions only
	entry.append( atom );

	atom.m_uds = UDS_USER;
	atom.m_str = archiveEntry->user();
	entry.append( atom );

	atom.m_uds = UDS_GROUP;
	atom.m_str = archiveEntry->group();
	entry.append( atom );

	atom.m_uds = UDS_LINK_DEST;
	atom.m_str = archiveEntry->symlink();
	entry.append( atom );
}

void ArchiveProtocol::listDir( const KURL & url ) {
	kdDebug( 7109 ) << "ArchiveProtocol::listDir " << url.url() << endl;

	TQString path;
	if ( !checkNewFile( url, path ) ) {
		TQCString _path( TQFile::encodeName( url.path() ) );
		kdDebug( 7109 ) << "Checking (stat) on " << _path << endl;
		struct stat buff;
		if ( ::stat( _path.data(), &buff ) == -1 || !S_ISDIR( buff.st_mode ) ) {
			error( TDEIO::ERR_DOES_NOT_EXIST, url.prettyURL() );
			return ;
		}
		// It's a real dir -> redirect
		KURL redir;
		redir.setPath( url.path() );
		kdDebug( 7109 ) << "Ok, redirection to " << redir.url() << endl;
		redirection( redir );
		finished();
		// And let go of the tar file - for people who want to unmount a cdrom after that
		delete m_archiveFile;
		m_archiveFile = 0L;
		return ;
	}

	if ( path.isEmpty() ) {
		KURL redir( url.protocol() + TQString::fromLatin1( ":/" ) );
		kdDebug( 7109 ) << "url.path()==" << url.path() << endl;
		redir.setPath( url.path() + TQString::fromLatin1( "/" ) );
		kdDebug( 7109 ) << "ArchiveProtocol::listDir: redirection " << redir.url() << endl;
		redirection( redir );
		finished();
		return ;
	}

	kdDebug( 7109 ) << "checkNewFile done" << endl;
	const KArchiveDirectory* root = m_archiveFile->directory();
	const KArchiveDirectory* dir;
	if ( !path.isEmpty() && path != "/" ) {
		kdDebug( 7109 ) << TQString( "Looking for entry %1" ).arg( path ) << endl;
		const KArchiveEntry* e = root->entry( path );
		if ( !e ) {
			error( TDEIO::ERR_DOES_NOT_EXIST, url.prettyURL() );
			return ;
		}
		if ( ! e->isDirectory() ) {
			error( TDEIO::ERR_IS_FILE, url.prettyURL() );
			return ;
		}
		dir = ( KArchiveDirectory* ) e;
	} else {
		dir = root;
	}

	TQStringList l = dir->entries();
	totalSize( l.count() );

	UDSEntry entry;
	TQStringList::Iterator it = l.begin();
	for ( ; it != l.end(); ++it ) {
		kdDebug( 7109 ) << ( *it ) << endl;
		const KArchiveEntry* archiveEntry = dir->entry( ( *it ) );

		createUDSEntry( archiveEntry, entry );

		listEntry( entry, false );
	}

	listEntry( entry, true ); // ready

	finished();

	kdDebug( 7109 ) << "ArchiveProtocol::listDir done" << endl;
}

void ArchiveProtocol::stat( const KURL & url ) {
	TQString path;
	UDSEntry entry;
	if ( !checkNewFile( url, path ) ) {
		// We may be looking at a real directory - this happens
		// when pressing up after being in the root of an archive
		TQCString _path( TQFile::encodeName( url.path() ) );
		kdDebug( 7109 ) << "ArchiveProtocol::stat (stat) on " << _path << endl;
		struct stat buff;
		if ( ::stat( _path.data(), &buff ) == -1 || !S_ISDIR( buff.st_mode ) ) {
			kdDebug( 7109 ) << "isdir=" << S_ISDIR( buff.st_mode ) << "  errno=" << strerror( errno ) << endl;
			error( TDEIO::ERR_DOES_NOT_EXIST, url.path() );
			return ;
		}
		// Real directory. Return just enough information for KRun to work
		UDSAtom atom;
		atom.m_uds = TDEIO::UDS_NAME;
		atom.m_str = url.fileName();
		entry.append( atom );
		kdDebug( 7109 ) << "ArchiveProtocol::stat returning name=" << url.fileName() << endl;

		atom.m_uds = TDEIO::UDS_FILE_TYPE;
		atom.m_long = buff.st_mode & S_IFMT;
		entry.append( atom );

		statEntry( entry );

		finished();

		// And let go of the tar file - for people who want to unmount a cdrom after that
		delete m_archiveFile;
		m_archiveFile = 0L;
		return ;
	}

	const KArchiveDirectory* root = m_archiveFile->directory();
	const KArchiveEntry* archiveEntry;
	if ( path.isEmpty() ) {
		path = TQString::fromLatin1( "/" );
		archiveEntry = root;
	} else {
		archiveEntry = root->entry( path );
	}
	if ( !archiveEntry ) {
		error( TDEIO::ERR_DOES_NOT_EXIST, url.prettyURL() );
		return ;
	}

	createUDSEntry( archiveEntry, entry );
	statEntry( entry );

	finished();
}

void ArchiveProtocol::get( const KURL & url ) {
	kdDebug( 7109 ) << "ArchiveProtocol::get" << url.url() << endl;

	TQString path;
	if ( !checkNewFile( url, path ) ) {
		error( TDEIO::ERR_DOES_NOT_EXIST, url.prettyURL() );
		return ;
	}

	const KArchiveDirectory* root = m_archiveFile->directory();
	const KArchiveEntry* archiveEntry = root->entry( path );

	if ( !archiveEntry ) {
		error( TDEIO::ERR_DOES_NOT_EXIST, url.prettyURL() );
		return ;
	}
	if ( archiveEntry->isDirectory() ) {
		error( TDEIO::ERR_IS_DIRECTORY, url.prettyURL() );
		return ;
	}
	const KArchiveFile* archiveFileEntry = static_cast<const KArchiveFile *>( archiveEntry );
	if ( !archiveEntry->symlink().isEmpty() ) {
		kdDebug( 7109 ) << "Redirection to " << archiveEntry->symlink() << endl;
		KURL realURL( url, archiveEntry->symlink() );
		kdDebug( 7109 ) << "realURL= " << realURL.url() << endl;
		redirection( realURL );
		finished();
		return ;
	}

	totalSize( archiveFileEntry->size() );

	TQByteArray completeData = archiveFileEntry->data();

	KMimeMagicResult * result = KMimeMagic::self() ->findBufferFileType( completeData, path );
	kdDebug( 7109 ) << "Emitting mimetype " << result->mimeType() << endl;
	mimeType( result->mimeType() );

	data( completeData );

	processedSize( archiveFileEntry->size() );
	data( TQByteArray() );

	finished();
}

/*
  In case someone wonders how the old filter stuff looked like :    :)
void TARProtocol::slotData(void *_p, int _len)
{
  switch (m_cmd) {
    case CMD_PUT:
      assert(m_pFilter);
      m_pFilter->send(_p, _len);
      break;
    default:
      abort();
      break;
    }
}
 
void TARProtocol::slotDataEnd()
{
  switch (m_cmd) {
    case CMD_PUT:
      assert(m_pFilter && m_pJob);
      m_pFilter->finish();
      m_pJob->dataEnd();
      m_cmd = CMD_NONE;
      break;
    default:
      abort();
      break;
    }
}
 
void TARProtocol::jobData(void *_p, int _len)
{
  switch (m_cmd) {
  case CMD_GET:
    assert(m_pFilter);
    m_pFilter->send(_p, _len);
    break;
  case CMD_COPY:
    assert(m_pFilter);
    m_pFilter->send(_p, _len);
    break;
  default:
    abort();
  }
}
 
void TARProtocol::jobDataEnd()
{
  switch (m_cmd) {
  case CMD_GET:
    assert(m_pFilter);
    m_pFilter->finish();
    dataEnd();
    break;
  case CMD_COPY:
    assert(m_pFilter);
    m_pFilter->finish();
    m_pJob->dataEnd();
    break;
  default:
    abort();
  }
}
 
void TARProtocol::filterData(void *_p, int _len)
{
debug("void TARProtocol::filterData");
  switch (m_cmd) {
  case CMD_GET:
    data(_p, _len);
    break;
  case CMD_PUT:
    assert (m_pJob);
    m_pJob->data(_p, _len);
    break;
  case CMD_COPY:
    assert(m_pJob);
    m_pJob->data(_p, _len);
    break;
  default:
    abort();
  }
}
*/

