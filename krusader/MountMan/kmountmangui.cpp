/***************************************************************************
                              kmountmangui.cpp
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



#include "kmountmangui.h"
#include "kmountman.h"
#include "../krusader.h"
#include "../Dialogs/krspecialwidgets.h"
#include "../kicons.h"
#include "../defaults.h"
#include "../VFS/vfs.h"
#include <tdelocale.h>
#include <tqpixmap.h>
#include <tdepopupmenu.h>
#include <tqbitmap.h>
#include <tdemessagebox.h>
#include <tqlayout.h>
#include <tqgroupbox.h>
#include <kprocess.h>
#include <tqcursor.h>
#include <kdebug.h>
#include <kguiitem.h>
#include <tqfileinfo.h>
#include <sys/param.h>

#if defined(BSD)
#include <kmountpoint.h>
#include <kmdcodec.h>
#else
#define MTAB	"/etc/mtab"
#endif

// use our version of it until kde fixes theirs
#include "kdiskfreesp.h"

KMountManGUI::KMountManGUI() : KDialogBase( krApp, 0, true, "Mount.Man" ),
info( 0 ), mountList( 0 ) {
	watcher = new TQTimer( this );
   connect( watcher, TQ_SIGNAL( timeout() ), this, TQ_SLOT( checkMountChange() ) );

   connect( this, TQ_SIGNAL( finishedGettingSpaceData() ), this, TQ_SLOT( updateList() ) );
   setButtonOK( i18n( "&Close" ) );
   showButtonApply( false ); showButtonCancel( false );
   setPlainCaption( i18n( "MountMan - Your Mount-Manager" ) );
   widget = new KJanusWidget( this, 0, KJanusWidget::Tabbed );
   createLayout();
   setMainWidget( widget );
   widget->setMinimumSize( widget->sizeHint().width() + mountList->columnWidth( 5 ),
                           widget->sizeHint().height() );
   setMinimumSize( widget->minimumSize().width(), widget->minimumSize().height() );
   resize( minimumSize() );

   // connections
   connect( mountList, TQ_SIGNAL( doubleClicked( TQListViewItem * ) ), this,
            TQ_SLOT( doubleClicked( TQListViewItem* ) ) );
   connect( mountList, TQ_SIGNAL( contextMenuRequested( TQListViewItem *, const TQPoint &, int ) ),
            this, TQ_SLOT( clicked( TQListViewItem*, const TQPoint&, int ) ) );
   connect( mountList, TQ_SIGNAL( clicked( TQListViewItem * ) ), this,
            TQ_SLOT( changeActive( TQListViewItem * ) ) );
   connect( mountList, TQ_SIGNAL( selectionChanged( TQListViewItem * ) ), this,
            TQ_SLOT( changeActive( TQListViewItem * ) ) );

   getSpaceData();
   exec();
}

KMountManGUI::~KMountManGUI() {
   watcher->stop();
   delete watcher;
}

void KMountManGUI::createLayout() {
   mainPage = widget->addPage( i18n( "Filesystems" ), 0 );
   createMainPage();
   widget->showPage( Filesystems );
}

void KMountManGUI::createMainPage() {
   // check if we need to clean up first!
   if ( mountList != 0 ) {
      mountList->hide();
      delete mountList;
      mountList = 0;
   }
   // clean up is finished...
   TQGridLayout *layout = new TQGridLayout( mainPage, 1, 1 );
   mountList = new TQListView( mainPage );  // create the main container
   krConfig->setGroup( "Look&Feel" );
   mountList->setFont( krConfig->readFontEntry( "Filelist Font", _FilelistFont ) );
   mountList->setAllColumnsShowFocus( true );
   mountList->setMultiSelection( false );
   mountList->setSelectionMode( TQListView::Single );
   mountList->setVScrollBarMode( TQScrollView::AlwaysOn );
   mountList->setHScrollBarMode( TQScrollView::Auto );
   mountList->setShowSortIndicator( true );
   int i = TQFontMetrics( mountList->font() ).width( "W" );
   int j = TQFontMetrics( mountList->font() ).width( "0" );
   j = ( i > j ? i : j );
   mountList->addColumn( i18n( "Name" ), j * 8 );
   mountList->addColumn( i18n( "Type" ), j * 4 );
   mountList->addColumn( i18n( "Mnt.Point" ), j * 6 );
   mountList->addColumn( i18n( "Total Size" ), j * 6 );
   mountList->addColumn( i18n( "Free Size" ), j * 6 );
   mountList->addColumn( i18n( "Free %" ), j * 5 );
   mountList->setColumnWidthMode( 0, TQListView::Maximum );
   mountList->setColumnWidthMode( 1, TQListView::Maximum );
   mountList->setColumnWidthMode( 2, TQListView::Maximum );
   mountList->setColumnWidthMode( 3, TQListView::Maximum );
   mountList->setColumnWidthMode( 4, TQListView::Maximum );
   mountList->setColumnWidthMode( 5, TQListView::Maximum );
   // now the list is created, time to fill it with data.
   //=>krMtMan.forceUpdate();
   TQGroupBox *box = new TQGroupBox( "MountMan.Info", mainPage );
   box->setAlignment( TQt::AlignHCenter );
   info = new KRFSDisplay( box );
   info->resize( info->width(), height() );
   layout->addWidget( box, 0, 0 );
   layout->addWidget( mountList, 0, 1 );
}

void KMountManGUI::getSpaceData() {
   fileSystemsTemp.clear();
	KrMountDetector::getInstance()->hasMountsChanged();
	
   mounted = KMountPoint::currentMountPoints();
	possible = KMountPoint::possibleMountPoints();
   if ( mounted.size() == 0 ) { // nothing is mounted
      updateList(); // let's continue
      return ;
   }

   numOfMountPoints = mounted.size();
   for ( KMountPoint::List::iterator it = mounted.begin(); it != mounted.end(); ++it ) {
   	  // don't bother with invalid file systems
      if (krMtMan.invalidFilesystem((*it)->mountType())) {
			--numOfMountPoints;
			continue;
		}
      KDiskFreeSp *sp = KDiskFreeSp::findUsageInfo( ( *it ) ->mountPoint() );
      connect( sp, TQ_SIGNAL( foundMountPoint( const TQString &, unsigned long, unsigned long, unsigned long ) ),
               this, TQ_SLOT( gettingSpaceData( const TQString&, unsigned long, unsigned long, unsigned long ) ) );
      connect( sp, TQ_SIGNAL( done() ), this, TQ_SLOT( gettingSpaceData() ) );
   }
}

// this decrements the counter, while the following uses the data
// used when certain filesystem (/dev, /sys) can't have the needed stats
void KMountManGUI::gettingSpaceData() {
   if ( --numOfMountPoints == 0 ) {
      fileSystems = fileSystemsTemp;
      emit finishedGettingSpaceData();
   }
}

void KMountManGUI::gettingSpaceData( const TQString &mountPoint, unsigned long kBSize,
                                     unsigned long /*kBUsed*/, unsigned long kBAvail ) {
   KMountPoint *m = KMountMan::findInListByMntPoint( mounted, mountPoint );
   if ( !m ) { // this should never never never happen!
      KMessageBox::error( 0, i18n( "Critical Error" ),
                          i18n( "Internal error in MountMan\nPlease email the developers" ) );
      exit( 1 );
   }
   fsData data;
   data.setMntPoint( mountPoint );
   data.setMounted( true );
   data.setTotalBlks( kBSize );
   data.setFreeBlks( kBAvail );
   data.setName( m->mountedFrom() );
   data.setType( m->mountType() );
   fileSystemsTemp.append( data );
}

void KMountManGUI::addItemToMountList( TQListView *lst, fsData &fs ) {
   bool mtd = fs.mounted();

   TQString tSize = TQString( "%1" ).arg( TDEIO::convertSizeFromKB( fs.totalBlks() ) );
   TQString fSize = TQString( "%1" ).arg( TDEIO::convertSizeFromKB( fs.freeBlks() ) );
   TQString sPrct = TQString( "%1%" ).arg( 100 - ( fs.usedPerct() ) );
   TQListViewItem *item = new TQListViewItem( lst, fs.name(),
                         fs.type(), fs.mntPoint(),
                         ( mtd ? tSize : TQString( "N/A" ) ), ( mtd ? fSize : TQString( "N/A" ) ),
                         ( mtd ? sPrct : TQString( "N/A" ) ) );
   
	TQString id = fs.name().left(7); // only works assuming devices start with  "/dev/XX"
   TQPixmap *icon = 0;
   if ( id == "/dev/fd") {
      icon = new TQPixmap( LOADICON( mtd ? "media-floppy-3_5-mounted" : "media-floppy-3_5-unmounted" ) );
	} else if ( id == "/dev/cd" || fs.type() == "iso9660" ) {
		icon = new TQPixmap( LOADICON( mtd ? "media-optical-cdrom-mounted" : "media-optical-cdrom-unmounted" ) );
   } else if ( fs.type() == "nfs" || fs.type() == "smbfs" ) {
		icon = new TQPixmap( LOADICON( mtd ? "nfs-mounted" : "nfs-unmounted" ) );
	} else icon = new TQPixmap( LOADICON( mtd ? "drive-harddisk-mounted" : "drive-harddisk-unmounted" ) );

   item->setPixmap( 0, *icon );
   delete icon;
}

void KMountManGUI::updateList() {
   mountList->clear();
   // this handles the mounted ones
	for ( TQValueList<fsData>::iterator it = fileSystems.begin(); it != fileSystems.end() ; ++it ) {
		if (krMtMan.invalidFilesystem((*it).type())) {
			continue;
		}
      addItemToMountList( mountList, *it );
   }
	
	// now, handle the non-mounted ones
	for (KMountPoint::List::iterator it = possible.begin(); it != possible.end(); ++it) {
		// make sure we don't add things we've already added
		if (KMountMan::findInListByMntPoint(mounted, (*it)->mountPoint())) {
			continue;
		} else {
			fsData data;
			data.setMntPoint((*it)->mountPoint());
			data.setMounted(false);
			data.setType((*it)->mountType());
			data.setName((*it)->mountedFrom());
			fileSystems.append(data);
			
			if (krMtMan.invalidFilesystem(data.type())) continue;
			addItemToMountList(mountList, data);
		}
	}
	
   mountList->clearSelection();
   if ( info ) {
      info->setEmpty( true );
      info->repaint();
   }
   watcher->start( WATCHER_DELAY, true );   // starting the watch timer ( single shot )
}

void KMountManGUI::checkMountChange() {
	if (KrMountDetector::getInstance()->hasMountsChanged())
		getSpaceData();
   watcher->start( WATCHER_DELAY, true );   // starting the watch timer ( single shot )
}

void KMountManGUI::doubleClicked( TQListViewItem *i ) {
   if ( !i )
		return; // we don't want to refresh to swap, do we ?
		 
   // change the active panel to this mountpoint
   connect( ( TQObject* ) this, TQ_SIGNAL( refreshPanel( const KURL & ) ), ( TQObject* ) SLOTS,
            TQ_SLOT( refresh( const KURL & ) ) );
   emit refreshPanel( vfs::fromPathOrURL( i->text(2) ) ); // text(2) ? so ugly ... 
   disconnect( this, TQ_SIGNAL( refreshPanel( const KURL & ) ), 0, 0 );
   slotClose();
}

// when user clicks on a filesystem, change information
void KMountManGUI::changeActive( TQListViewItem *i ) {
	if ( !i ) return ;
   fsData *system = 0;
	
	for (TQValueList<fsData>::Iterator it = fileSystems.begin(); it != fileSystems.end(); ++it) {
		// the only thing which is unique is the mount point
		if ((*it).mntPoint() == i->text(2)) { // text(2) ? ugly ugly ugly
			system = &(*it);
			break;
		}
	}
	
	if (system == 0) {
		KMessageBox::error(0, i18n("Critical Error"), i18n("Internal error in MountMan\nCall the developers"));
		exit(1);
	}
   info->setAlias( system->mntPoint() );
   info->setRealName( system->name() );
   info->setMounted( system->mounted() );
   info->setEmpty( false );
	info->setTotalSpace( system->totalBlks() );
   info->setFreeSpace( system->freeBlks() );
   info->repaint();
}

// called when right-clicked on a filesystem
void KMountManGUI::clicked( TQListViewItem *item, const TQPoint& pos, int /* col */ ) {
   // these are the values that will exist in the menu
#define MOUNT_ID       90
#define UNMOUNT_ID     91
#define FORMAT_ID      93
#define EJECT_ID       94
   //////////////////////////////////////////////////////////
	if ( !item ) return ;
	
	fsData *system = 0;
   for (TQValueList<fsData>::Iterator it = fileSystems.begin(); it != fileSystems.end(); ++it) {
		// the only thing which is unique is the mount point
		if ((*it).mntPoint() == item->text(2)) { // text(2) ? ugly ugly ugly
			system = &(*it);
			break;
		}
	}
	
   if ( !system ) {
      KMessageBox::error( 0, i18n( "MountMan has an internal error. Please notify the developers. Thank you." ) );
      exit( 0 );
   }
   // create the menu
   TDEPopupMenu popup;
   popup.insertTitle( i18n( "MountMan" ) );
   if ( !system->mounted() ) {
      popup.insertItem( i18n( "Mount" ), MOUNT_ID );
		bool enable = !(krMtMan.nonmountFilesystem(system->type(), system->mntPoint()));
		popup.setItemEnabled( MOUNT_ID, enable);
	} else {
		popup.insertItem( i18n( "Unmount" ), UNMOUNT_ID );
		bool enable = !(krMtMan.nonmountFilesystem(system->type(), system->mntPoint()));
		popup.setItemEnabled( UNMOUNT_ID, enable);
	}
   if ( krMtMan.ejectable( system->mntPoint() ) )
      //  if (system->type()=="iso9660" || krMtMan.followLink(system->name()).left(2)=="cd")
      popup.insertItem( i18n( "Eject" ), EJECT_ID );
   else {
      popup.insertItem( i18n( "Format" ), FORMAT_ID );
      popup.setItemEnabled( FORMAT_ID, false );
   }

   TQString mountPoint = system->mntPoint();

   int result = popup.exec( pos );
   // check out the user's option
   switch ( result ) {
         case - 1 : return ;     // the user clicked outside of the menu
         case MOUNT_ID :
         case UNMOUNT_ID :
         krMtMan.toggleMount( mountPoint );
         break;
         case FORMAT_ID :
         break;
         case EJECT_ID :
         KMountMan::eject( mountPoint );
         break;
   }
}

KrMountDetector::KrMountDetector() {
   hasMountsChanged();
}

bool KrMountDetector::hasMountsChanged() {
#if defined(BSD)
   KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::NeedRealDeviceName);
   KMD5 md5;
   for(KMountPoint::List::iterator i = mountPoints.begin(); i != mountPoints.end(); ++i) {
      md5.update((*i)->mountedFrom().utf8());
      md5.update((*i)->realDeviceName().utf8());
      md5.update((*i)->mountPoint().utf8());
      md5.update((*i)->mountType().utf8());
   }
   TQString s = md5.hexDigest();
   bool result = s != checksum;
   checksum = s;
#else
   bool result = TQFileInfo(MTAB).lastModified() != lastMtab;
   lastMtab = TQFileInfo(MTAB).lastModified();
#endif
   return result;
}

KrMountDetector krMountDetector;
KrMountDetector * KrMountDetector::getInstance() {
   return & krMountDetector;
}

#include "kmountmangui.moc"
