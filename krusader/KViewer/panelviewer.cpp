#include <kurl.h>
#include <tqstring.h>
#include <tqwidgetstack.h>
#include <tqapplication.h>
#include <tdeparts/part.h>
#include <tdeparts/browserextension.h>
#include <tdemessagebox.h>
#include <tqdict.h>
#include <tqlabel.h>
#include <kmimetype.h>
#include <tdetempfile.h>
#include <tdelocale.h>
#include <klibloader.h>
#include <kuserprofile.h>
#include <kdebug.h>
#include <tdefileitem.h>
#include <tdeio/netaccess.h>
#include <tqfile.h>
#include <klargefile.h>
#include "panelviewer.h"

#define DICTSIZE 211

/* ----==={ PanelViewerBase }===---- */

PanelViewerBase::PanelViewerBase( TQWidget *parent ) :
TQWidgetStack( parent ), mimes( 0 ), cpart( 0 ) {
	setSizePolicy( TQSizePolicy( TQSizePolicy::Preferred, TQSizePolicy::Ignored ) );

	mimes = new TQDict<KParts::ReadOnlyPart>( DICTSIZE, false );
	mimes->setAutoDelete( true );
	cpart = 0;
	fallback = new TQLabel( i18n( "No file selected or selected file can't be displayed." ), this );
	fallback->setAlignment( AlignCenter | ExpandTabs | WordBreak );
	addWidget( fallback );
	raiseWidget( fallback );
}

PanelViewerBase::~PanelViewerBase() {
//	cpart->queryClose();
	closeURL();
	mimes->clear();
	delete mimes;
	delete fallback;
}

/* ----==={ PanelViewer }===---- */

PanelViewer::PanelViewer( TQWidget *parent ) :
PanelViewerBase( parent ) {
}

PanelViewer::~PanelViewer() {
}

KParts::ReadOnlyPart* PanelViewer::openURL( const KURL &url, KrViewer::Mode mode ) {
	emit urlChanged( this, url );
	closeURL();
	curl = url;

	if( mode == KrViewer::Generic ){
		cmimetype = KMimeType::findByURL( curl ) ->name();
		cpart = ( *mimes ) [ cmimetype ];
		if ( !cpart ){
			cpart = getPart( cmimetype );
			mimes->insert( cmimetype, cpart );
		}
	}

	KTempFile tmpFile;

	if( mode == KrViewer::Hex ){
		if ( !cpart ) cpart = getHexPart();
		if ( !cpart ) oldHexViewer(tmpFile);
	}
	
	if ( !cpart ) cpart = getPart( "text/plain" );
	if ( !cpart ) cpart = getPart( "all/allfiles" );

	if ( cpart ) {
		addWidget( cpart->widget() );
		raiseWidget( cpart->widget() );
	}
	if ( cpart && cpart->openURL( curl ) ){
		curl = url; /* needed because of the oldHexViewer */
		return cpart;
	}
	else {
		raiseWidget( fallback );
		return 0;
	}
}

bool PanelViewer::closeURL() {
	raiseWidget( fallback );
	if ( cpart && cpart->closeURL() ) {
		cpart = 0;
		return true;
	}
	return false;
}

KParts::ReadOnlyPart* PanelViewer::getPart( TQString mimetype ) {
	KParts::ReadOnlyPart * part = 0L;
	KLibFactory *factory = 0;
	KService::Ptr ptr = KServiceTypeProfile::preferredService( mimetype, "KParts/ReadOnlyPart" );
	if ( ptr ) {
		TQStringList args;
		TQVariant argsProp = ptr->property( "X-TDE-BrowserView-Args" );
		if ( argsProp.isValid() ) {
			TQString argStr = argsProp.toString();
			args = TQStringList::split( " ", argStr );
		}
		TQVariant prop = ptr->property( "X-TDE-BrowserView-AllowAsDefault" );
		if ( !prop.isValid() || prop.toBool() )   // defaults to true
		{
			factory = KLibLoader::self() ->factory( ptr->library().latin1() );
			if ( factory ) {
				part = static_cast<KParts::ReadOnlyPart *>( factory->create( this,
				        ptr->name().latin1(), TQString( "KParts::ReadOnlyPart" ).latin1(), args ) );
			}
		}
	}
	if ( part ) {
		KParts::BrowserExtension * ext = KParts::BrowserExtension::childObject( part );
		if ( ext ) {
			connect( ext, TQ_SIGNAL( openURLRequestDelayed( const KURL &, const KParts::URLArgs & ) ), this, TQ_SLOT( openURL( const KURL & ) ) );
			connect( ext, TQ_SIGNAL( openURLRequestDelayed( const KURL &, const KParts::URLArgs & ) ), this, TQ_SIGNAL( openURLRequest( const KURL & ) ) );
		}
	}
	return part;
}

KParts::ReadOnlyPart* PanelViewer::getHexPart(){
	KParts::ReadOnlyPart * part = 0L;

	KLibFactory * factory = KLibLoader::self() ->factory( "libkhexedit2part" );
	if ( factory ) {
		// Create the part
		part = ( KParts::ReadOnlyPart * ) factory->create( this, "hexedit2part","KParts::ReadOnlyPart" );
	}

	return part;
}

void PanelViewer::oldHexViewer(KTempFile& tmpFile) {
	TQString file;
	// files that are not local must first be downloaded
	if ( !curl.isLocalFile() ) {
		if ( !TDEIO::NetAccess::download( curl, file,this ) ) {
			KMessageBox::sorry( this, i18n( "KrViewer is unable to download: " ) + curl.url() );
			return ;
		}
	} else file = curl.path();


	// create a hex file
	TQFile f_in( file );
	f_in.open( IO_ReadOnly );
	TQDataStream in( &f_in );

	FILE *out = KDE_fopen( tmpFile.name().local8Bit(), "w" );

	TDEIO::filesize_t fileSize = f_in.size();
	TDEIO::filesize_t address = 0;
	char buf[ 16 ];
	unsigned int* pBuff = ( unsigned int* ) buf;

	while ( address < fileSize ) {
		memset( buf, 0, 16 );
		int bufSize = ( ( fileSize - address ) > 16 ) ? 16 : ( fileSize - address );
		in.readRawBytes( buf, bufSize );
		fprintf( out, "0x%8.8llx: ", address );
		for ( int i = 0; i < 4; ++i ) {
			if ( i < ( bufSize / 4 ) ) fprintf( out, "%8.8x ", pBuff[ i ] );
			else fprintf( out, "         " );
		}
		fprintf( out, "| " );

		for ( int i = 0; i < bufSize; ++i ) {
			if ( buf[ i ] > ' ' && buf[ i ] < '~' ) fputc( buf[ i ], out );
			else fputc( '.', out );
		}
		fputc( '\n', out );

		address += 16;
	}
	// clean up
	f_in.close();
	fclose( out );
	if ( !curl.isLocalFile() )
	TDEIO::NetAccess::removeTempFile( file );

	curl = tmpFile.name();
}

/* ----==={ PanelEditor }===---- */

PanelEditor::PanelEditor( TQWidget *parent ) :
PanelViewerBase( parent ) {
}

PanelEditor::~PanelEditor() {
}

KParts::ReadOnlyPart* PanelEditor::openURL( const KURL &url, KrViewer::Mode mode ) {
	emit urlChanged( this, url );
	closeURL();
	curl = url;

	if( mode == KrViewer::Generic ){
		cmimetype = KMimeType::findByURL( curl ) ->name();
		cpart = ( *mimes ) [ cmimetype ];
		if ( !cpart ){ 
			cpart = getPart( cmimetype );
			mimes->insert( cmimetype, cpart );
		}
	}

	if ( !cpart ) cpart = getPart( "text/plain" );
	if ( !cpart ) cpart = getPart( "all/allfiles" );

	if ( cpart ) {
		addWidget( cpart->widget() );
		raiseWidget( cpart->widget() );
	}
	else {
		raiseWidget( fallback );
		return 0;
	}

	bool create = true;
	TDEIO::StatJob* statJob = TDEIO::stat( url, false );
	connect( statJob, TQ_SIGNAL( result( TDEIO::Job* ) ), this, TQ_SLOT( slotStatResult( TDEIO::Job* ) ) );
	busy = true;
	while ( busy ) tqApp->processEvents();
	if( !entry.isEmpty() ) {
		KFileItem file( entry, url );
		if( file.isReadable() ) create = false;
	}
	
	if( create ){
		if( static_cast<KParts::ReadWritePart *>(cpart)->saveAs( curl ) ) return cpart;
	}
	else {
		if ( cpart->openURL( curl ) ) return cpart;
	}
	return 0;
}

bool PanelEditor::queryClose() {
	if ( !cpart ) return true;
	return static_cast<KParts::ReadWritePart *>(cpart)->queryClose();
}

bool PanelEditor::closeURL() {
	if ( !cpart ) return false;
	
	static_cast<KParts::ReadWritePart *>(cpart)->closeURL( false );
	
	raiseWidget( fallback );
	cpart = 0;
	return true;
}

KParts::ReadWritePart* PanelEditor::getPart( TQString mimetype ) {
	KParts::ReadWritePart * part = 0L;
	KLibFactory *factory = 0;
	KService::Ptr ptr = KServiceTypeProfile::preferredService( mimetype, "KParts/ReadWritePart" );
	if ( ptr ) {
		TQStringList args;
		TQVariant argsProp = ptr->property( "X-TDE-BrowserView-Args" );
		if ( argsProp.isValid() ) {
			TQString argStr = argsProp.toString();
			args = TQStringList::split( " ", argStr );
		}
		TQVariant prop = ptr->property( "X-TDE-BrowserView-AllowAsDefault" );
		if ( !prop.isValid() || prop.toBool() )  // defaults to true
		{
			factory = KLibLoader::self() ->factory( ptr->library().latin1() );
			if ( factory ) {
				part = static_cast<KParts::ReadWritePart *>( factory->create( this,
				        ptr->name().latin1(), TQString( "KParts::ReadWritePart" ).latin1(), args ) );
			}
		}
	}
	if ( part ) {
		KParts::BrowserExtension * ext = KParts::BrowserExtension::childObject( part );
		if ( ext ) {
			connect( ext, TQ_SIGNAL( openURLRequestDelayed( const KURL &, const KParts::URLArgs & ) ), this, TQ_SLOT( openURL( const KURL & ) ) );
			connect( ext, TQ_SIGNAL( openURLRequestDelayed( const KURL &, const KParts::URLArgs & ) ), this, TQ_SIGNAL( openURLRequest( const KURL & ) ) );
		}
	}
	return part;
}

void PanelEditor::slotStatResult( TDEIO::Job* job ) {
  if( !job || job->error() ) entry = TDEIO::UDSEntry();
  else entry = static_cast<TDEIO::StatJob*>(job)->statResult();
  busy = false;
}

bool PanelEditor::isModified(){
	return static_cast<KParts::ReadWritePart *>(cpart)->isModified();
}

#include "panelviewer.moc"
