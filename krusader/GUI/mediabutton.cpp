/***************************************************************************
                         mediabutton.cpp  -  description
                             -------------------
    copyright            : (C) 2006 + by Csaba Karai
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

#include "mediabutton.h"
#include "../krusader.h"
#include "../krservices.h"
#include "../kicons.h"
#include "../krslots.h"
#include "../MountMan/kdiskfreesp.h"
#include "../MountMan/kmountman.h"

#include <tqpopupmenu.h>
#include <tqfile.h>
#include <tqfontmetrics.h>

#include <tdelocale.h>
#include <kiconloader.h>

#include <tdeversion.h>
#include <tdeio/job.h>
#include <tdemessagebox.h>
#include <kmimetype.h>
#include <kprotocolinfo.h>
#include <tdefileitem.h>
#include <kprocess.h>
#include <tqcursor.h>

#ifdef Q_OS_LINUX
// For CD/DVD drive detection
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>
#define CDROM_GET_CAPABILITY    0x5331
#define CDSL_CURRENT            ((int) (~0U>>1))
#define CDC_DVD_R               0x10000 /* drive can write DVD-R */
#define CDC_DVD_RAM             0x20000 /* drive can write DVD-RAM */
#define CDC_CD_R                0x2000  /* drive is a CD-R */
#define CDC_CD_RW               0x4000  /* drive is a CD-RW */
#define CDC_DVD                 0x8000  /* drive is a DVD */
#include <tqfile.h>
#endif



MediaButton::MediaButton( TQWidget *parent, const char *name ) : TQToolButton( parent, name ),
		popupMenu( 0 ), rightMenu( 0 ), hasMedia( false ), waitingForMount( -1 ), mountCheckerTimer()
	 {
	TDEIconLoader * iconLoader = new TDEIconLoader();
	TQPixmap icon = iconLoader->loadIcon( "blockdevice", TDEIcon::Toolbar, 16 );

	setFixedSize( icon.width() + 4, icon.height() + 4 );
	setPixmap( icon );
	setTextLabel( i18n( "Open the available media list" ), true );
	setPopupDelay( 1 ); // immediate press
	setAcceptDrops( false );

	popupMenu = new TQPopupMenu( this );
	popupMenu->installEventFilter( this );
	TQ_CHECK_PTR( popupMenu );

	setPopup( popupMenu );

	connect( popupMenu, TQ_SIGNAL( aboutToShow() ), this, TQ_SLOT( slotAboutToShow() ) );
	connect( popupMenu, TQ_SIGNAL( aboutToHide() ), this, TQ_SLOT( slotAboutToHide() ) );
	connect( popupMenu, TQ_SIGNAL( activated( int ) ), this, TQ_SLOT( slotPopupActivated( int ) ) );

	connect( &mountCheckerTimer, TQ_SIGNAL( timeout() ), this, TQ_SLOT( slotTimeout() ) );
}

MediaButton::~MediaButton() {
	busy = false;
}

void MediaButton::slotAboutToShow() {
	hasMedia = KProtocolInfo::isKnownProtocol( TQString( "media" ) );
	krConfig->setGroup( "Advanced" );
	if( krConfig->readBoolEntry( "DontUseMediaProt", !hasMedia ) )
		hasMedia = false;
	
	popupMenu->clear();
	urls.clear();
	mediaUrls.clear();
	mimes.clear();
	quasiMounted.clear();
	waitingForMount = -1;
	
	if( hasMedia )
		createListWithMedia();
	else
		createListWithoutMedia();
	
	mountCheckerTimer.start( 1000, true );
}

void MediaButton::slotAboutToHide() {
	if( rightMenu )
		rightMenu->close();
	
	if( waitingForMount < 0 )
		mountCheckerTimer.stop();
}

void MediaButton::createListWithMedia() {
	TDEIO::ListJob *job = TDEIO::listDir( KURL( "media:/" ), false );
	connect( job, TQ_SIGNAL( entries( TDEIO::Job*, const TDEIO::UDSEntryList& ) ),
		this, TQ_SLOT( slotEntries( TDEIO::Job*, const TDEIO::UDSEntryList& ) ) );
	connect( job, TQ_SIGNAL( result( TDEIO::Job* ) ),
		this, TQ_SLOT( slotListResult( TDEIO::Job* ) ) );
	busy = true;
	
	if( !busy )
		tqApp->processEvents();
}

void MediaButton::slotEntries( TDEIO::Job *, const TDEIO::UDSEntryList& entries ) 
{
	KMountPoint::List mountList = KMountPoint::currentMountPoints();
	
	TDEIO::UDSEntryListConstIterator it = entries.begin();
	TDEIO::UDSEntryListConstIterator end = entries.end();
	
	while( it != end )
	{
		KURL url;
		TQString text;
		TQString mime;
		TQString localPath;
		bool mounted = false;
		
		TDEIO::UDSEntry::ConstIterator it2 = (*it).begin();
		
		for( ; it2 != (*it).end(); it2++ ) {
			switch ((*it2).m_uds) {
			case TDEIO::UDS_NAME:
				text = KURL::decode_string((*it2).m_str);
				break;
			case TDEIO::UDS_URL:
				url = KURL::fromPathOrURL(  (*it2).m_str );
				break;
			case TDEIO::UDS_MIME_TYPE:
				mime = (*it2).m_str;
				if( !mime.endsWith( "unmounted" ) )
					mounted = true;
				break;
#if KDE_IS_VERSION(3,4,0)
			case TDEIO::UDS_LOCAL_PATH:
				localPath = (*it2).m_str;
				break;
#endif
			}
		}
		
		if( text != "." && text != ".." ) {
			int index = popupMenu->count();
			TQPixmap pixmap = FL_LOADICON( KMimeType::mimeType( mime ) ->icon( TQString(), true ) );
			
			mediaUrls.append( url );
			
			if( mounted && !localPath.isEmpty() )
			{
				url = KURL::fromPathOrURL( localPath );
				if( !text.contains( url.path() ) )
					text += "  [" + url.path() + "]";
			}
			else if( mounted )
			{
				url = getLocalPath( url, &mountList );
				if( url.isLocalFile() && !text.contains( url.path() ) )
					text += "  [" + url.path() + "]";
			}
			
			popupMenu->insertItem( pixmap, text, index, index );
			
			urls.append( url );
			mimes.append( mime );
			quasiMounted.append( false );
		}
		++it;
	}
}

void MediaButton::slotListResult( TDEIO::Job * ) {
	busy = false;
}

KURL MediaButton::getLocalPath( const KURL &url, KMountPoint::List *mountList ) {
	KMountPoint::List mountListRef;
	if( mountList == 0 ) {
		mountListRef = KMountPoint::currentMountPoints();
		mountList = &mountListRef;
	}
	
	for (KMountPoint::List::iterator it = mountList->begin(); it != mountList->end(); ++it) {
		TQString name = (*it)->mountedFrom();
		name = name.mid( name.findRev( "/" ) + 1 );
		if( name == url.fileName() ) {
			TQString point = (*it)->mountPoint();
			if( !point.isEmpty() )
				return KURL::fromPathOrURL( point );
		}
	}
	return url;
}


void MediaButton::createListWithoutMedia() {
	/* WORKAROUND CODE START */
	
	/* 1. the menu is drawn when we know all the mount points
	   2. the menu is corrected, when the HDD volume sizes arrive from df
	
	   when the volume sizes are added to the items, some cases widget resize
	   is necessary. If transparency is set for the widget, TQt produces weird
	   looking widgets, and that's why this workaround is used.
	   Here we add additional spaces to the mounted HDD elements for avoiding
	   the buggy widget resize. These are extra spaces. */

	extraSpaces = "";
	TQFontMetrics fm( popupMenu->font() );
	int requiredWidth = fm.width( "999.9 GB " );
	while( fm.width( extraSpaces ) < requiredWidth )
		extraSpaces+=" ";
	/* WORKAROUND CODE END */
	
	KMountPoint::List possibleMountList = KMountPoint::possibleMountPoints();
	for (KMountPoint::List::iterator it = possibleMountList.begin(); it != possibleMountList.end(); ++it) {
		addMountPoint( *it, false );
	}
	
	KMountPoint::List mountList = KMountPoint::currentMountPoints();
	for (KMountPoint::List::iterator it = mountList.begin(); it != mountList.end(); ++it) {
		addMountPoint( *it, true );
	}
}

TQString MediaButton::detectType( KMountPoint *mp )
{
	TQString typeName = TQString();
#ifdef Q_OS_LINUX
	// Guessing device types by mount point is not exactly accurate...
	// Do something accurate first, and fall back if necessary.
	
	bool isCd=false;
	TQString devname=mp->mountedFrom().section('/', -1);
	if(devname.startsWith("scd") || devname.startsWith("sr"))
	{
		// SCSI CD/DVD drive
		isCd=true;
	}
	else if(devname.startsWith("hd"))
	{
		// IDE device -- we can't tell if this is a
		// CD/DVD drive or harddisk by just looking at the
		// filename
		TQFile m(TQString("/proc/ide/") + devname + "/media");
		if(m.open(IO_ReadOnly))
		{
			TQTextStream in(&m);
			TQString buf=in.readLine();
			if(buf.contains("cdrom"))
				isCd=true;
			m.close();
		}
	}
	if(isCd)
	{
		int device=::open((const char *)TQFile::encodeName(mp->mountedFrom()), O_RDONLY | O_NONBLOCK );
		if(device>=0)
		{
			int drv=::ioctl(device, CDROM_GET_CAPABILITY, CDSL_CURRENT);
			if(drv>=0)
			{
				if((drv & CDC_DVD_R) || (drv & CDC_DVD_RAM))
					typeName = "dvdwriter";
				else if((drv & CDC_CD_R) || (drv & CDC_CD_RW))
					typeName = "cdwriter";
				else if(drv & CDC_DVD)
					typeName = "dvd";
				else
					typeName = "cdrom";
			}
			
			::close(device);
		}
	}
	if( !typeName.isNull() )
		return typeName;

#elif defined(__FreeBSD__)
	if (-1!=mp->mountedFrom().find("/acd",0,FALSE)) typeName="cdrom";
	else if (-1!=mp->mountedFrom().find("/scd",0,FALSE)) typeName="cdrom";
	else if (-1!=mp->mountedFrom().find("/ad",0,FALSE)) typeName="hdd";
	else if (-1!=mp->mountedFrom().find("/da",0,FALSE)) typeName="hdd";
	else if (-1!=mp->mountedFrom().find("/afd",0,FALSE)) typeName="zip";
	else
#endif

    /* Guessing of cdrom and cd recorder devices */
	if (-1!=mp->mountPoint().find("cdrom",0,FALSE)) typeName="cdrom";
	else if (-1!=mp->mountedFrom().find("cdrom",0,FALSE)) typeName="cdrom";
	else if (-1!=mp->mountPoint().find("cdwriter",0,FALSE)) typeName="cdwriter";
	else if (-1!=mp->mountedFrom().find("cdwriter",0,FALSE)) typeName="cdwriter";
	else if (-1!=mp->mountedFrom().find("cdrw",0,FALSE)) typeName="cdwriter";
	else if (-1!=mp->mountPoint().find("cdrw",0,FALSE)) typeName="cdwriter";
	else if (-1!=mp->mountedFrom().find("cdrecorder",0,FALSE)) typeName="cdwriter";
	else if (-1!=mp->mountPoint().find("cdrecorder",0,FALSE)) typeName="cdwriter";
	else if (-1!=mp->mountedFrom().find("dvdrecorder",0,FALSE)) typeName="dvdwriter";
	else if (-1!=mp->mountPoint().find("dvdrecorder",0,FALSE)) typeName="dvdwriter";
	else if (-1!=mp->mountPoint().find("dvdwriter",0,FALSE)) typeName="dvdwriter";
	else if (-1!=mp->mountedFrom().find("dvdwriter",0,FALSE)) typeName="dvdwriter";
	else if (-1!=mp->mountPoint().find("dvd",0,FALSE)) typeName="dvd";   
	else if (-1!=mp->mountedFrom().find("dvd",0,FALSE)) typeName="dvd";   
	else if (-1!=mp->mountedFrom().find("/dev/scd",0,FALSE)) typeName="cdrom";
	else if (-1!=mp->mountedFrom().find("/dev/sr",0,FALSE)) typeName="cdrom";
	
	/* Guessing of floppy types */
	else if (-1!=mp->mountedFrom().find("fd",0,FALSE)) {
		if (-1!=mp->mountedFrom().find("360",0,FALSE)) typeName="floppy5";
			if (-1!=mp->mountedFrom().find("1200",0,FALSE)) typeName="floppy5";
			else typeName="floppy";
		}
	else if (-1!=mp->mountPoint().find("floppy",0,FALSE)) typeName="floppy";
	
	else if (-1!=mp->mountPoint().find("zip",0,FALSE)) typeName="zip";
	else if (-1!=mp->mountType().find("nfs",0,FALSE)) typeName="nfs";
	else if (-1!=mp->mountType().find("smb",0,FALSE)) typeName="smb";
	else if (-1!=mp->mountedFrom().find("//",0,FALSE)) typeName="smb";
	else typeName="hdd";
	
	return typeName;
}

void MediaButton::slotPopupActivated( int elem ) {
	if( !quasiMounted[ elem ] && mimes[ elem ].endsWith( "_unmounted" ) ) {
		mount( elem );
		waitingForMount = elem;
		maxMountWait = 20;
		newTabAfterMount = false;
		mountCheckerTimer.start( 1000, true );
		return;
	}
	emit openUrl( urls[ elem ] );
}

void MediaButton::gettingSpaceData(const TQString &mountPoint, unsigned long kBSize, unsigned long, unsigned long ) {
	KURL mediaURL = KURL::fromPathOrURL( mountPoint );
	
	TDEIO::filesize_t size = kBSize;
	size *= 1024;
	TQString sizeText = TDEIO::convertSize( size );
	
	for( unsigned i=0; i != urls.size(); i++ ) {
		if( mediaURL.equals( urls[ i ], true ) ) {
			if( kBSize == 0 ) { // if df gives 0, it means the device is quasy umounted
				TQString mime = mimes[ i ];
				if( mimes[ i ].endsWith( "_mounted" ) ) {
					quasiMounted[ i ] = true;
					mimes[ i ] = mimes[ i ].replace( "_mounted", "_unmounted" );
				}
				
				TQPixmap pixmap = FL_LOADICON( KMimeType::mimeType( mimes[ i ] ) ->icon( TQString(), true ) );
				popupMenu->changeItem( i, pixmap, popupMenu->text( i ) );
			}
			else if( mimes[ i ].contains( "hdd_" ) )
				popupMenu->changeItem( i, sizeText + " " + popupMenu->text( i ).stripWhiteSpace() );
			return;
		}
	}
}

void MediaButton::openPopup() {
	TQPopupMenu * pP = popup();
	if ( pP ) {
		popup() ->exec( mapToGlobal( TQPoint( 0, height() ) ) );
	}
}

void MediaButton::addMountPoint( KMountPoint * mp, bool isMounted ) {
	TQString mountString = isMounted ? "_mounted" : "_unmounted";
	if( mp->mountPoint() == "/dev/swap" || 
		mp->mountPoint() == "/dev/pts"  ||
		mp->mountPoint().startsWith( "/sys/kernel" ) ||
		mp->mountPoint().find( "/proc" ) == 0 )
		return;
	if( mp->mountType() == "swap" ||
		mp->mountType() == "sysfs" ||
		mp->mountType() == "tmpfs" ||
		mp->mountType() == "kernfs" ||
		mp->mountType() == "usbfs" ||
		mp->mountType() == "unknown" ||
		mp->mountType() == "none" ||
		mp->mountType() == "sunrpc" )
		return;
	if( mp->mountedFrom() == "none" ||
		mp->mountedFrom() == "tmpfs" ||
		mp->mountedFrom().find( "shm" )  != -1 )
		return;
	
	int overwrite = -1;
	KURL mountURL = KURL::fromPathOrURL( mp->mountPoint() );
	
	for( unsigned i=0; i != urls.size(); i++ ) 
		if( urls[ i ].equals( mountURL, true ) ) {
			overwrite = i;
			break;
		}
	
	TQString name;
	TQString type = detectType( mp );
	
	/* WORKAROUND CODE START */
	/* add spaces to avoid widget resize in gettingSpaceData,
	   which is buggy in TQt when transparency is set */
	TQString extSpc = ( isMounted && type == "hdd" ) ? extraSpaces : "";
	/* WORKAROUND CODE END */
		
#if KDE_IS_VERSION(3,4,0)
	TQString mimeBase = "media/";
#else
	TQString mimeBase = "kdedevice/";
#endif
	
	TQString mime = mimeBase + type + mountString;
	
	if( type == "hdd" )
		name =  i18n( "Hard Disk" ) ;
	else if( type == "cdrom" )
		name = i18n( "CD-ROM" );
	else if( type == "cdwriter" )
		name = i18n( "CD Recorder" );
	else if( type == "dvdwriter" ) {
		mime = mimeBase + "cdwriter" + mountString;
		name = i18n( "DVD Recorder" );
	}
	else if( type == "dvd" )
		name = i18n( "DVD" );
	else if( type == "smb" )
		name = i18n( "Remote Share" );
	else if( type == "nfs" )
		name = i18n( "Remote Share" );
	else if( type == "floppy" )
		name = i18n( "Floppy" );
	else if( type == "floppy5" )
		name = i18n( "Floppy" );
	else if( type == "zip" )
		name = i18n( "Zip Disk" );
	else {
		mime = mimeBase + "hdd" + mountString;
		name = i18n( "Unknown" );
	}
	
	if( isMounted ) {
		KDiskFreeSp *sp = KDiskFreeSp::findUsageInfo( mp->mountPoint() );
		connect( sp, TQ_SIGNAL( foundMountPoint( const TQString &, unsigned long, unsigned long, unsigned long ) ),
		         this, TQ_SLOT( gettingSpaceData( const TQString&, unsigned long, unsigned long, unsigned long ) ) );
	}
	
	TQPixmap pixmap = FL_LOADICON( KMimeType::mimeType( mime ) ->icon( TQString(), true ) );
	
	if( overwrite == -1 ) {
		int index = popupMenu->count();
		urls.append( KURL::fromPathOrURL( mp->mountPoint() ) );
		mimes.append( mime );
		mediaUrls.append( KURL() );
		quasiMounted.append( false );
		popupMenu->insertItem( pixmap, name + "  [" + mp->mountPoint() + "]" + extSpc, index, index );
	}
	else {
		mimes[ overwrite ] = mime;
		popupMenu->changeItem( overwrite, pixmap, name + "  [" + mp->mountPoint() + "]" + extSpc );
	}
}

bool MediaButton::eventFilter( TQObject *o, TQEvent *e ) {
	if( o == popupMenu ) {
		if( e->type() == TQEvent::MouseButtonPress || e->type() == TQEvent::MouseButtonRelease ) {
			TQMouseEvent *m = static_cast<TQMouseEvent*>(e);
			if( m->button() == TQt::RightButton ) {
				if( e->type() == TQEvent::MouseButtonPress ) {
					int id = popupMenu->idAt( m->pos() );
					if( id != -1 )
						rightClickMenu( id );
				}
 				m->accept();
				return true;
			}
		}
	}
	return false;
}

void MediaButton::rightClickMenu( int index ) {
	if( rightMenu )
		rightMenu->close();
	
	TQString mime = mimes[ index ];
	bool ejectable = mime.contains( "dvd_" ) || mime.contains( "dvdwriter_" ) || mime.contains( "cdrom_" ) || mime.contains( "cdwriter_" );
	bool mounted = mime.contains( "_mounted" );
	
	TQPopupMenu * myMenu = rightMenu = new TQPopupMenu( popupMenu );
	myMenu->insertItem( i18n( "Open" ), 1 );
	myMenu->insertItem( i18n( "Open in a new tab" ), 2 );
	myMenu->insertSeparator();
	if( !mounted )
		myMenu->insertItem( i18n( "Mount" ), 3 );
	else
		myMenu->insertItem( i18n( "Unmount" ), 4 );
	if( ejectable )
		myMenu->insertItem( i18n( "Eject" ), 5 );
	int result = myMenu->exec( TQCursor::pos() );
	delete myMenu;
	if( rightMenu == myMenu )
		rightMenu = 0;
	else
		return;
	
	switch ( result ) {
	case 1:
	case 2:
		popupMenu->close();
		if( mounted || quasiMounted[ index ] ) {
			if( result == 1 )
				emit openUrl( urls[ index ] );
			else
				SLOTS->newTab( urls[ index ] );
		} else {
			mount( index ); // mount first, when mounted open the tab
			waitingForMount = index;
			maxMountWait = 20;
			newTabAfterMount = ( result == 2 );
			mountCheckerTimer.start( 1000, true );
		}
		break;
	case 3:
		mount( index );
		break;
	case 4:
		umount( index );
		break;
	case 5:
		eject( index );
		break;
	default:
		break;
	}
}

bool MediaButton::mount( int index ) {
	if ( (unsigned)index < mimes.count() ) {
		if( !mediaUrls[ index ].isEmpty() ) {
			TDEProcess proc;
			proc << KrServices::fullPathName( "tdeio_media_mounthelper" ) << "-m" << mediaUrls[ index ].url();
			proc.start( TDEProcess::DontCare );
		} else {
			krMtMan.mount( urls[ index ].path(), false );
		}
	}
	return false;
}

bool MediaButton::umount( int index ) {
	if ( (unsigned)index < mimes.count() ) {
		if( !mediaUrls[ index ].isEmpty() ) {
			TDEProcess proc;
			proc << KrServices::fullPathName( "tdeio_media_mounthelper" ) << "-u" << mediaUrls[ index ].url();
			proc.start( TDEProcess::DontCare );
		} else {
			krMtMan.unmount( urls[ index ].path(), false );
		}
	}
	return false;
}

bool MediaButton::eject( int index ) {
	if ( (unsigned)index < mimes.count() ) {
		if( !mediaUrls[ index ].isEmpty() ) {
			TDEProcess proc;
			proc << KrServices::fullPathName( "tdeio_media_mounthelper" ) << "-e" << mediaUrls[ index ].url();
			proc.start( TDEProcess::DontCare );
		} else {
			krMtMan.eject( urls[ index ].path() );
		}
	}
	return false;
}

void MediaButton::slotTimeout() {
	if( isHidden() && ( waitingForMount < 0 ) )
		return;
	
	KMountPoint::List mountList = KMountPoint::currentMountPoints();
	
	for( unsigned index = 0; index < urls.count(); index++ ) {
		bool mounted = false;
		
		TQString text = popupMenu->text( index );
		
		if( mediaUrls[ index ].isEmpty() ) {
			for (KMountPoint::List::iterator it = mountList.begin(); it != mountList.end(); ++it)
				if( (*it)->mountPoint() == urls[ index ].path() ) {
					mounted = true;;
					break;
				}
		} else {
			KURL uri = getLocalPath( mediaUrls[ index ], &mountList );
			if(  uri.isLocalFile() ) {
				urls[ index ] = uri;
				mounted = true;
				
				if( !text.contains( uri.path() ) ) 
				{
					if( text.endsWith( "]" ) )
					{
						int ndx = text.findRev( "  [" );
						if( ndx >0 )
							text.truncate( ndx );
					}
					
					text += "  [" + uri.path() + "]";
				}
			}
			else
			{
				if( text.endsWith( "]" ) )
				{
					int ndx = text.findRev( "  [" );
					if( ndx >0 )
						text.truncate( ndx );
				}
			}
		}
		
		if( quasiMounted[ index ] ) // mounted but not listed with DF
			mounted = false;
		
		if( mimes[ index ].contains( "_mounted" ) && !mounted )
			mimes[ index ] = mimes[ index ].replace( "_mounted", "_unmounted" );
		if( mimes[ index ].contains( "_unmounted" ) && mounted )
			mimes[ index ] = mimes[ index ].replace( "_unmounted", "_mounted" );
		
		TQPixmap pixmap = FL_LOADICON( KMimeType::mimeType( mimes[ index ] ) ->icon( TQString(), true ) );
		popupMenu->changeItem( index, pixmap, text );
		
		if( ((int)index == waitingForMount) && mounted ) {
			waitingForMount = -1;
			if( newTabAfterMount )
				SLOTS->newTab( urls[ index ] );
			else
				emit openUrl( urls[ index ] );
		}
	}
	
	if( waitingForMount >= 0 ) {         // maximum wait for mounting expired ?
		if( --maxMountWait < 0 ) {
			waitingForMount = -1;
			return;
		}
	}
	
	mountCheckerTimer.start( 1000, true );
}


#include "mediabutton.moc"
