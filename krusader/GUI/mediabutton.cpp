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

#include <klocale.h>
#include <kiconloader.h>

#include <kdeversion.h>
#include <kio/job.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kprotocolinfo.h>
#include <kfileitem.h>
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



MediaButton::MediaButton( TQWidget *tqparent, const char *name ) : TQToolButton( tqparent, name ),
		popupMenu( 0 ), rightMenu( 0 ), hasMedia( false ), waitingForMount( -1 ), mountCheckerTimer()
	 {
	KIconLoader * iconLoader = new KIconLoader();
	TQPixmap icon = iconLoader->loadIcon( "blockdevice", KIcon::Toolbar, 16 );

	setFixedSize( icon.width() + 4, icon.height() + 4 );
	setPixmap( icon );
	setTextLabel( i18n( "Open the available media list" ), true );
	setPopupDelay( 1 ); // immediate press
	setAcceptDrops( false );

	popupMenu = new TQPopupMenu( this );
	popupMenu->installEventFilter( this );
	Q_CHECK_PTR( popupMenu );

	setPopup( popupMenu );

	connect( popupMenu, TQT_SIGNAL( aboutToShow() ), this, TQT_SLOT( slotAboutToShow() ) );
	connect( popupMenu, TQT_SIGNAL( aboutToHide() ), this, TQT_SLOT( slotAboutToHide() ) );
	connect( popupMenu, TQT_SIGNAL( activated( int ) ), this, TQT_SLOT( slotPopupActivated( int ) ) );

	connect( &mountCheckerTimer, TQT_SIGNAL( timeout() ), this, TQT_SLOT( slotTimeout() ) );
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
	KIO::ListJob *job = KIO::listDir( KURL( "media:/" ), false );
	connect( job, TQT_SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ),
		this, TQT_SLOT( slotEntries( KIO::Job*, const KIO::UDSEntryList& ) ) );
	connect( job, TQT_SIGNAL( result( KIO::Job* ) ),
		this, TQT_SLOT( slotListResult( KIO::Job* ) ) );
	busy = true;
	
	if( !busy )
		tqApp->processEvents();
}

void MediaButton::slotEntries( KIO::Job *, const KIO::UDSEntryList& entries ) 
{
	KMountPoint::List mountList = KMountPoint::currentMountPoints();
	
	KIO::UDSEntryListConstIterator it = entries.begin();
	KIO::UDSEntryListConstIterator end = entries.end();
	
	while( it != end )
	{
		KURL url;
		TQString text;
		TQString mime;
		TQString localPath;
		bool mounted = false;
		
		KIO::UDSEntry::ConstIterator it2 = (*it).begin();
		
		for( ; it2 != (*it).end(); it2++ ) {
			switch ((*it2).m_uds) {
			case KIO::UDS_NAME:
				text = KURL::decode_string((*it2).m_str);
				break;
			case KIO::UDS_URL:
				url = KURL::fromPathOrURL(  (*it2).m_str );
				break;
			case KIO::UDS_MIME_TYPE:
				mime = (*it2).m_str;
				if( !mime.endsWith( "unmounted" ) )
					mounted = true;
				break;
#if KDE_IS_VERSION(3,4,0)
			case KIO::UDS_LOCAL_PATH:
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
				if( !text.tqcontains( url.path() ) )
					text += "  [" + url.path() + "]";
			}
			else if( mounted )
			{
				url = getLocalPath( url, &mountList );
				if( url.isLocalFile() && !text.tqcontains( url.path() ) )
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

void MediaButton::slotListResult( KIO::Job * ) {
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
		name = name.mid( name.tqfindRev( "/" ) + 1 );
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
	   is necessary. If transparency is set for the widget, QT produces weird
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
			if(buf.tqcontains("cdrom"))
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
	if (-1!=mp->mountedFrom().tqfind("/acd",0,FALSE)) typeName="cdrom";
	else if (-1!=mp->mountedFrom().tqfind("/scd",0,FALSE)) typeName="cdrom";
	else if (-1!=mp->mountedFrom().tqfind("/ad",0,FALSE)) typeName="hdd";
	else if (-1!=mp->mountedFrom().tqfind("/da",0,FALSE)) typeName="hdd";
	else if (-1!=mp->mountedFrom().tqfind("/afd",0,FALSE)) typeName="zip";
	else
#endif

    /* Guessing of cdrom and cd recorder devices */
	if (-1!=mp->mountPoint().tqfind("cdrom",0,FALSE)) typeName="cdrom";
	else if (-1!=mp->mountedFrom().tqfind("cdrom",0,FALSE)) typeName="cdrom";
	else if (-1!=mp->mountPoint().tqfind("cdwriter",0,FALSE)) typeName="cdwriter";
	else if (-1!=mp->mountedFrom().tqfind("cdwriter",0,FALSE)) typeName="cdwriter";
	else if (-1!=mp->mountedFrom().tqfind("cdrw",0,FALSE)) typeName="cdwriter";
	else if (-1!=mp->mountPoint().tqfind("cdrw",0,FALSE)) typeName="cdwriter";
	else if (-1!=mp->mountedFrom().tqfind("cdrecorder",0,FALSE)) typeName="cdwriter";
	else if (-1!=mp->mountPoint().tqfind("cdrecorder",0,FALSE)) typeName="cdwriter";
	else if (-1!=mp->mountedFrom().tqfind("dvdrecorder",0,FALSE)) typeName="dvdwriter";
	else if (-1!=mp->mountPoint().tqfind("dvdrecorder",0,FALSE)) typeName="dvdwriter";
	else if (-1!=mp->mountPoint().tqfind("dvdwriter",0,FALSE)) typeName="dvdwriter";
	else if (-1!=mp->mountedFrom().tqfind("dvdwriter",0,FALSE)) typeName="dvdwriter";
	else if (-1!=mp->mountPoint().tqfind("dvd",0,FALSE)) typeName="dvd";   
	else if (-1!=mp->mountedFrom().tqfind("dvd",0,FALSE)) typeName="dvd";   
	else if (-1!=mp->mountedFrom().tqfind("/dev/scd",0,FALSE)) typeName="cdrom";
	else if (-1!=mp->mountedFrom().tqfind("/dev/sr",0,FALSE)) typeName="cdrom";
	
	/* Guessing of floppy types */
	else if (-1!=mp->mountedFrom().tqfind("fd",0,FALSE)) {
		if (-1!=mp->mountedFrom().tqfind("360",0,FALSE)) typeName="floppy5";
			if (-1!=mp->mountedFrom().tqfind("1200",0,FALSE)) typeName="floppy5";
			else typeName="floppy";
		}
	else if (-1!=mp->mountPoint().tqfind("floppy",0,FALSE)) typeName="floppy";
	
	else if (-1!=mp->mountPoint().tqfind("zip",0,FALSE)) typeName="zip";
	else if (-1!=mp->mountType().tqfind("nfs",0,FALSE)) typeName="nfs";
	else if (-1!=mp->mountType().tqfind("smb",0,FALSE)) typeName="smb";
	else if (-1!=mp->mountedFrom().tqfind("//",0,FALSE)) typeName="smb";
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
	
	KIO::filesize_t size = kBSize;
	size *= 1024;
	TQString sizeText = KIO::convertSize( size );
	
	for( unsigned i=0; i != urls.size(); i++ ) {
		if( mediaURL.equals( urls[ i ], true ) ) {
			if( kBSize == 0 ) { // if df gives 0, it means the device is quasy umounted
				TQString mime = mimes[ i ];
				if( mimes[ i ].endsWith( "_mounted" ) ) {
					quasiMounted[ i ] = true;
					mimes[ i ] = mimes[ i ].tqreplace( "_mounted", "_unmounted" );
				}
				
				TQPixmap pixmap = FL_LOADICON( KMimeType::mimeType( mimes[ i ] ) ->icon( TQString(), true ) );
				popupMenu->changeItem( i, pixmap, popupMenu->text( i ) );
			}
			else if( mimes[ i ].tqcontains( "hdd_" ) )
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
		mp->mountPoint().tqfind( "/proc" ) == 0 )
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
		mp->mountedFrom().tqfind( "shm" )  != -1 )
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
	   which is buggy in QT when transparency is set */
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
		connect( sp, TQT_SIGNAL( foundMountPoint( const TQString &, unsigned long, unsigned long, unsigned long ) ),
		         this, TQT_SLOT( gettingSpaceData( const TQString&, unsigned long, unsigned long, unsigned long ) ) );
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
	if( TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(popupMenu) ) {
		if( e->type() == TQEvent::MouseButtonPress || e->type() == TQEvent::MouseButtonRelease ) {
			TQMouseEvent *m = TQT_TQMOUSEEVENT(e);
			if( m->button() == Qt::RightButton ) {
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
	bool ejectable = mime.tqcontains( "dvd_" ) || mime.tqcontains( "dvdwriter_" ) || mime.tqcontains( "cdrom_" ) || mime.tqcontains( "cdwriter_" );
	bool mounted = mime.tqcontains( "_mounted" );
	
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
			KProcess proc;
			proc << KrServices::fullPathName( "kio_media_mounthelper" ) << "-m" << mediaUrls[ index ].url();
			proc.start( KProcess::DontCare );
		} else {
			krMtMan.mount( urls[ index ].path(), false );
		}
	}
	return false;
}

bool MediaButton::umount( int index ) {
	if ( (unsigned)index < mimes.count() ) {
		if( !mediaUrls[ index ].isEmpty() ) {
			KProcess proc;
			proc << KrServices::fullPathName( "kio_media_mounthelper" ) << "-u" << mediaUrls[ index ].url();
			proc.start( KProcess::DontCare );
		} else {
			krMtMan.unmount( urls[ index ].path(), false );
		}
	}
	return false;
}

bool MediaButton::eject( int index ) {
	if ( (unsigned)index < mimes.count() ) {
		if( !mediaUrls[ index ].isEmpty() ) {
			KProcess proc;
			proc << KrServices::fullPathName( "kio_media_mounthelper" ) << "-e" << mediaUrls[ index ].url();
			proc.start( KProcess::DontCare );
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
				
				if( !text.tqcontains( uri.path() ) ) 
				{
					if( text.endsWith( "]" ) )
					{
						int ndx = text.tqfindRev( "  [" );
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
					int ndx = text.tqfindRev( "  [" );
					if( ndx >0 )
						text.truncate( ndx );
				}
			}
		}
		
		if( quasiMounted[ index ] ) // mounted but not listed with DF
			mounted = false;
		
		if( mimes[ index ].tqcontains( "_mounted" ) && !mounted )
			mimes[ index ] = mimes[ index ].tqreplace( "_mounted", "_unmounted" );
		if( mimes[ index ].tqcontains( "_unmounted" ) && mounted )
			mimes[ index ] = mimes[ index ].tqreplace( "_unmounted", "_mounted" );
		
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
