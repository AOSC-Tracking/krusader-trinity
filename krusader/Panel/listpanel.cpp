/***************************************************************************
                         listpanel.cpp
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

#include <unistd.h>
#include <sys/param.h>
// QT includes
#include <tqbitmap.h>
#include <tqwhatsthis.h>
#include <tqstringlist.h>
#include <tqstrlist.h>
#include <kurldrag.h>
#include <tqheader.h>
#include <tqtimer.h>
#include <tqregexp.h> 
#include <tqsplitter.h>
// KDE includes
#include <kpopupmenu.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kurl.h>
#include <ktrader.h>
#include <kopenwith.h>
#include <kuserprofile.h>
#include <kiconloader.h>
#include <kcursor.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <tqtooltip.h>
#include <kdeversion.h>
#include <tqimage.h>
#include <tqtabbar.h>
#include <kdebug.h>
#include <kurlrequester.h>
#include <kurl.h> 
#include <kmountpoint.h>

// Krusader includes
#include "../krusader.h"
#include "../krslots.h"
#include "panelfunc.h"
#include "../kicons.h"
#include "../VFS/krpermhandler.h"
#include "listpanel.h"
#include "../krusaderview.h"
#include "../panelmanager.h"
#include "../defaults.h"
#include "../resources.h"
#include "../MountMan/kmountman.h"
#include "../Dialogs/krdialogs.h"
#include "../BookMan/krbookmarkbutton.h"
#include "../Dialogs/krspwidgets.h"
#include "../Dialogs/krspecialwidgets.h"
#include "../GUI/kcmdline.h"
#include "../Dialogs/percentalsplitter.h"
#include "krdetailedview.h"
#include "krbriefview.h"
#include "krpreviewpopup.h"
#include "../GUI/dirhistorybutton.h"
#include "../GUI/dirhistoryqueue.h"
#include "../GUI/mediabutton.h"
#include "../GUI/syncbrowsebutton.h"
#include "../krservices.h"
#include "panelpopup.h" 
#include "../UserAction/useractionpopupmenu.h"
#include "../Dialogs/popularurls.h"
#include "krpopupmenu.h"

#ifdef __LIBKONTQ__
#include <konq_popupmenu.h>
#include <konqbookmarkmanager.h>
#endif

typedef TQValueList<KServiceOffer> OfferList;

#define URL(X) KURL::fromPathOrURL(X)

/////////////////////////////////////////////////////
// 					The list panel constructor       //
/////////////////////////////////////////////////////
ListPanel::ListPanel( TQString typeIn, TQWidget *tqparent, bool &left, const char *name ) :
      TQWidget( tqparent, name ), panelType( typeIn ), colorMask( 255 ), compareMode( false ), currDragItem( 0 ), statsAgent( 0 ), 
		quickSearch( 0 ), cdRootButton( 0 ), cdUpButton( 0 ), popupBtn(0), popup(0),inlineRefreshJob(0), _left( left ) {

   func = new ListPanelFunc( this );
   setAcceptDrops( true );
   tqlayout = new TQGridLayout( this, 3, 3 );

   mediaButton = new MediaButton( this, "mediaButton" );
   connect( mediaButton, TQT_SIGNAL( pressed() ), this, TQT_SLOT( slotFocusOnMe() ) );
   connect( mediaButton, TQT_SIGNAL( openUrl( const KURL& ) ), func, TQT_SLOT( openUrl( const KURL& ) ) );

   status = new KrSqueezedTextLabel( this );
   krConfig->setGroup( "Look&Feel" );
   status->setFont( krConfig->readFontEntry( "Filelist Font", _FilelistFont ) );
   status->setBackgroundMode( PaletteBackground );
   status->setFrameStyle( TQFrame::Box | TQFrame::Raised );
   status->setLineWidth( 1 );		// a nice 3D touch :-)
   status->setText( "" );        // needed for initialization code!
   status->enableDrops( true );
   int sheight = TQFontMetrics( status->font() ).height() + 4;
   status->setMaximumHeight( sheight );
   TQWhatsThis::add
      ( status, i18n( "The statusbar displays information about the FILESYSTEM "
                      "which holds your current directory: Total size, free space, "
                      "type of filesystem, etc." ) );
   connect( status, TQT_SIGNAL( clicked() ), this, TQT_SLOT( slotFocusOnMe() ) );
   connect( status, TQT_SIGNAL( dropped( TQDropEvent *) ), this, TQT_SLOT( handleDropOntqStatus(TQDropEvent *) ) );

   // ... create the history button
   dirHistoryQueue = new DirHistoryQueue( this );
   historyButton = new DirHistoryButton( dirHistoryQueue, this, "historyButton" );
   connect( historyButton, TQT_SIGNAL( pressed() ), this, TQT_SLOT( slotFocusOnMe() ) );
   connect( historyButton, TQT_SIGNAL( openUrl( const KURL& ) ), func, TQT_SLOT( openUrl( const KURL& ) ) );

	bookmarksButton = new KrBookmarkButton(this);
	connect( bookmarksButton, TQT_SIGNAL( pressed() ), this, TQT_SLOT( slotFocusOnMe() ) );
   connect( bookmarksButton, TQT_SIGNAL( openUrl( const KURL& ) ), func, TQT_SLOT( openUrl( const KURL& ) ) );
	TQWhatsThis::add
      ( bookmarksButton, i18n( "Open menu with bookmarks. You can also add "
                               "current location to the list, edit bookmarks "
                               "or add subfolder to the list." ) );
										 
   TQHBoxLayout *totalsLayout = new TQHBoxLayout;
	totals = new KrSqueezedTextLabel( this );
   krConfig->setGroup( "Look&Feel" );
   totals->setFont( krConfig->readFontEntry( "Filelist Font", _FilelistFont ) );
   totals->setFrameStyle( TQFrame::Box | TQFrame::Raised );
   totals->setBackgroundMode( PaletteBackground );
   totals->setLineWidth( 1 );		// a nice 3D touch :-)
   totals->setMaximumHeight( sheight );
   totals->enableDrops( true );
   TQWhatsThis::add
      ( totals, i18n( "The totals bar shows how many files exist, "
                      "how many selected and the bytes math" ) );
   connect( totals, TQT_SIGNAL( clicked() ), this, TQT_SLOT( slotFocusOnMe() ) );
   connect( totals, TQT_SIGNAL( dropped( TQDropEvent *) ), this, TQT_SLOT( handleDropOnTotals(TQDropEvent *) ) );  
   
	// a cancel button for the inplace refresh mechanism
	inlineRefreshCancelButton = new KPushButton(this);
	inlineRefreshCancelButton->setFixedSize( 22, 20 );
	inlineRefreshCancelButton->setPixmap(krLoader->loadIcon("cancel", KIcon::Toolbar, 16));
	connect(inlineRefreshCancelButton, TQT_SIGNAL(clicked()), this, TQT_SLOT(inlineRefreshCancel()));

	// a quick button to open the popup panel
	popupBtn = new TQToolButton( this, "popupbtn" );
	popupBtn->setFixedSize( 22, 20 );
	popupBtn->setPixmap(krLoader->loadIcon("1uparrow", KIcon::Toolbar, 16));
	connect(popupBtn, TQT_SIGNAL(clicked()), this, TQT_SLOT(togglePanelPopup()));
	TQToolTip::add(  popupBtn, i18n( "Open the popup panel" ) );
	totalsLayout->addWidget(totals);
	totalsLayout->addWidget(inlineRefreshCancelButton); inlineRefreshCancelButton->hide();
	totalsLayout->addWidget(popupBtn);
   
   quickSearch = new KrQuickSearch( this );
   krConfig->setGroup( "Look&Feel" );
   quickSearch->setFont( krConfig->readFontEntry( "Filelist Font", _FilelistFont ) );
   quickSearch->setFrameStyle( TQFrame::Box | TQFrame::Raised );
   quickSearch->setLineWidth( 1 );		// a nice 3D touch :-)
   quickSearch->setMaximumHeight( sheight );

   TQHBox * hbox = new TQHBox( this );

	// clear-origin button
	bool clearButton = krConfig->readBoolEntry("Clear Location Bar Visible", _ClearLocation);
	if (clearButton){
		clearOrigin = new TQToolButton(hbox, "clearorigin");
		clearOrigin->setPixmap(krLoader->loadIcon("locationbar_erase", KIcon::Toolbar, 16));
		TQToolTip::add(  clearOrigin, i18n( "Clear the location bar" ) );
	}
	
	QuickNavLineEdit *qnle = new QuickNavLineEdit(this);
   origin = new KURLRequester( qnle, hbox );
   TQPixmap pixMap = origin->button() ->iconSet() ->pixmap( TQIconSet::Small, TQIconSet::Normal );
   origin->button() ->setFixedSize( pixMap.width() + 4, pixMap.height() + 4 );
   TQWhatsThis::add
      ( origin, i18n( "Use superb KDE file dialog to choose location. " ) );
   origin->setShowLocalProtocol( false );
   origin->lineEdit() ->setURLDropsEnabled( true );
   origin->lineEdit() ->installEventFilter( this );
   TQWhatsThis::add
      ( origin->lineEdit(), i18n( "Name of directory where you are. You can also "
                                  "enter name of desired location to move there. "
                                  "Use of Net protocols like ftp or fish is possible." ) );
   origin->setMode( KFile::Directory | KFile::ExistingOnly );
   connect( origin, TQT_SIGNAL( returnPressed( const TQString& ) ), func, TQT_SLOT( openUrl( const TQString& ) ) );
   connect( origin, TQT_SIGNAL( returnPressed( const TQString& ) ), this, TQT_SLOT( slotFocusOnMe() ) );
   connect( origin, TQT_SIGNAL( urlSelected( const TQString& ) ), func, TQT_SLOT( openUrl( const TQString& ) ) );
   connect( origin, TQT_SIGNAL( urlSelected( const TQString& ) ), this, TQT_SLOT( slotFocusOnMe() ) );
   
	// this is here on purpose, do not move up!
	if (clearButton) {
		clearOrigin->setFixedSize( 20, origin->button() ->height() );
		connect(clearOrigin, TQT_SIGNAL(clicked()), origin->lineEdit(), TQT_SLOT(clear()));
		connect(clearOrigin, TQT_SIGNAL(clicked()), origin->lineEdit(), TQT_SLOT(setFocus()));
	}
	//
   
   cdOtherButton = new TQToolButton( hbox, "cdOtherButton" );
   cdOtherButton->setFixedSize( 20, origin->button() ->height() );
   cdOtherButton->setText( i18n( "=" ) );
	TQToolTip::add(  cdOtherButton, i18n( "Equal" ) );
   connect( cdOtherButton, TQT_SIGNAL( clicked() ), this, TQT_SLOT( slotFocusAndCDOther() ) );

   cdUpButton = new TQToolButton( hbox, "cdUpButton" );
   cdUpButton->setFixedSize( 20, origin->button() ->height() );
   cdUpButton->setText( i18n( ".." ) );
	TQToolTip::add(  cdUpButton, i18n( "Up" ) );
   connect( cdUpButton, TQT_SIGNAL( clicked() ), this, TQT_SLOT( slotFocusAndCDup() ) );

   cdHomeButton = new TQToolButton( hbox, "cdHomeButton" );
   cdHomeButton->setFixedSize( 20, origin->button() ->height() );
   cdHomeButton->setText( i18n( "~" ) );
	TQToolTip::add(  cdHomeButton, i18n( "Home" ) );
   connect( cdHomeButton, TQT_SIGNAL( clicked() ), this, TQT_SLOT( slotFocusAndCDHome() ) );

   cdRootButton = new TQToolButton( hbox, "cdRootButton" );
   cdRootButton->setFixedSize( 20, origin->button() ->height() );
   cdRootButton->setText( i18n( "/" ) );
	TQToolTip::add(  cdRootButton, i18n( "Root" ) );
   connect( cdRootButton, TQT_SIGNAL( clicked() ), this, TQT_SLOT( slotFocusAndCDRoot() ) );

   // ... creates the button for sync-browsing
   syncBrowseButton = new SyncBrowseButton( hbox );

   setPanelToolbar();

   header = new TQHeader( this );
   header->hide();

	// create a splitter to hold the view and the popup
	splt = new PercentalSplitter(this);
	splt->setChildrenCollapsible(true);
	splt->setOrientation(Qt::Vertical);

	createView();
	
   // make sure that a focus/path change reflects in the command line and activePanel
   connect( this, TQT_SIGNAL( cmdLineUpdate( TQString ) ), SLOTS, TQT_SLOT( slotCurrentChanged( TQString ) ) );
   connect( this, TQT_SIGNAL( activePanelChanged( ListPanel * ) ), SLOTS, TQT_SLOT( slotSetActivePanel( ListPanel * ) ) );
	
	// add a popup
	popup = new PanelPopup(splt, left);
	connect(popup, TQT_SIGNAL(selection(const KURL&)), SLOTS, TQT_SLOT(refresh(const KURL&)));
	connect(popup, TQT_SIGNAL(hideMe()), this, TQT_SLOT(togglePanelPopup()));
	popup->hide();
	
   // finish the tqlayout
	tqlayout->addMultiCellWidget( hbox, 0, 0, 0, 3 );
   tqlayout->addWidget( mediaButton, 1, 0 );
   tqlayout->addWidget( status, 1, 1 );
   tqlayout->addWidget( historyButton, 1, 2 );
   tqlayout->addWidget( bookmarksButton, 1, 3 );
   tqlayout->addMultiCellWidget( header, 2, 2, 0, 3 );
   tqlayout->addMultiCellWidget( splt, 3, 3, 0, 3 );
   tqlayout->addMultiCellWidget( quickSearch, 4, 4, 0, 3 );
   quickSearch->hide();
   tqlayout->addMultiCellLayout( totalsLayout, 5, 5, 0, 3 );
   //filter = ALL;
}

void ListPanel::createView()
{
	header->hide();
	if( panelType == "Brief" )
	{
		view = new KrBriefView( header, splt, _left, krConfig );
		view->init();
		
		connect( dynamic_cast<KrBriefView*>( view ), TQT_SIGNAL( middleButtonClicked( KrViewItem * ) ), SLOTS, TQT_SLOT( newTab( KrViewItem * ) ) );
		connect( dynamic_cast<KrBriefView*>( view ), TQT_SIGNAL( currentChanged( KrViewItem * ) ), 
			SLOTS, TQT_SLOT( updatePopupPanel( KrViewItem* ) ) );

		// connect quicksearch
		connect( quickSearch, TQT_SIGNAL( textChanged( const TQString& ) ),
			dynamic_cast<KrBriefView*>( view ), TQT_SLOT( quickSearch( const TQString& ) ) );
		connect( quickSearch, TQT_SIGNAL( otherMatching( const TQString&, int ) ),
			dynamic_cast<KrBriefView*>( view ), TQT_SLOT( quickSearch( const TQString& , int ) ) );
		connect( quickSearch, TQT_SIGNAL( stop( TQKeyEvent* ) ),
			dynamic_cast<KrBriefView*>( view ), TQT_SLOT( stopQuickSearch( TQKeyEvent* ) ) );
		connect( quickSearch, TQT_SIGNAL( process( TQKeyEvent* ) ),
			dynamic_cast<KrBriefView*>( view ), TQT_SLOT( handleQuickSearchEvent( TQKeyEvent* ) ) );
	} else { /* Detailed */
		panelType = "Detailed";
		view = new KrDetailedView( splt, _left, krConfig );
		view->init();
		connect( dynamic_cast<KrDetailedView*>( view ), TQT_SIGNAL( middleButtonClicked( KrViewItem * ) ), SLOTS, TQT_SLOT( newTab( KrViewItem * ) ) );
		connect( dynamic_cast<KrDetailedView*>( view ), TQT_SIGNAL( currentChanged( KrViewItem * ) ), 
			SLOTS, TQT_SLOT( updatePopupPanel( KrViewItem * ) ) );
		// connect quicksearch
		connect( quickSearch, TQT_SIGNAL( textChanged( const TQString& ) ),
			dynamic_cast<KrDetailedView*>( view ), TQT_SLOT( quickSearch( const TQString& ) ) );
		connect( quickSearch, TQT_SIGNAL( otherMatching( const TQString&, int ) ),
			dynamic_cast<KrDetailedView*>( view ), TQT_SLOT( quickSearch( const TQString& , int ) ) );
		connect( quickSearch, TQT_SIGNAL( stop( TQKeyEvent* ) ),
			dynamic_cast<KrDetailedView*>( view ), TQT_SLOT( stopQuickSearch( TQKeyEvent* ) ) );
		connect( quickSearch, TQT_SIGNAL( process( TQKeyEvent* ) ),
			dynamic_cast<KrDetailedView*>( view ), TQT_SLOT( handleQuickSearchEvent( TQKeyEvent* ) ) );
	}

   connect( view->op(), TQT_SIGNAL( renameItem( const TQString &, const TQString & ) ),
            func, TQT_SLOT( rename( const TQString &, const TQString & ) ) );
   connect( view->op(), TQT_SIGNAL( executed( TQString& ) ), func, TQT_SLOT( execute( TQString& ) ) );
   connect( view->op(), TQT_SIGNAL( needFocus() ), this, TQT_SLOT( slotFocusOnMe() ) );
   connect( view->op(), TQT_SIGNAL( selectionChanged() ), this, TQT_SLOT( slotUpdateTotals() ) );
   connect( view->op(), TQT_SIGNAL( itemDescription( TQString& ) ), krApp, TQT_SLOT( statusBarUpdate( TQString& ) ) );
   connect( view->op(), TQT_SIGNAL( contextMenu( const TQPoint & ) ), this, TQT_SLOT( popRightClickMenu( const TQPoint & ) ) );
   connect( view->op(), TQT_SIGNAL( emptyContextMenu( const TQPoint &) ), 
   	this, TQT_SLOT( popEmptyRightClickMenu( const TQPoint & ) ) );
   connect( view->op(), TQT_SIGNAL( letsDrag( TQStringList, TQPixmap ) ), this, TQT_SLOT( startDragging( TQStringList, TQPixmap ) ) );
   connect( view->op(), TQT_SIGNAL( gotDrop( TQDropEvent * ) ), this, TQT_SLOT( handleDropOnView( TQDropEvent * ) ) );
}

void ListPanel::changeType( const TQString & type )
{
   if( panelType != type )
   {
      panelType = type;
      delete view;
      createView();

      slotStartUpdate();

      if( panelType == "Brief" )
         dynamic_cast<KrBriefView*>( view )->show();
      else /* Detailed */
         dynamic_cast<KrDetailedView*>( view )->show();
   }
}

ListPanel::~ListPanel() {
   delete func;
   delete view;
   delete status;
   delete bookmarksButton;
   delete totals;
   delete quickSearch;
   delete origin;
   delete cdRootButton;
   delete cdHomeButton;
   delete cdUpButton;
   delete cdOtherButton;
   delete syncBrowseButton;
   delete tqlayout;
}

int ListPanel::getProperties()
{
   int props = 0;
   if( syncBrowseButton->state() == SYNCBROWSE_CD )
      props |= PROP_SYNC_BUTTON_ON;
   return props;
}

void ListPanel::setProperties( int prop )
{
   if( prop & PROP_SYNC_BUTTON_ON )
       syncBrowseButton->setOn( true );
   else
       syncBrowseButton->setOn( false );
}

bool ListPanel::eventFilter ( TQObject * watched, TQEvent * e ) {
	if( e->type() == TQEvent::KeyPress && TQT_BASE_OBJECT(origin->lineEdit()) == TQT_BASE_OBJECT(watched) ) {
		TQKeyEvent *ke = (TQKeyEvent *)e;
		
		if( ( ke->key() ==  Key_Down ) && ( ke->state() == ControlButton ) ) {
			slotFocusOnMe();
			return true;
		}
	}
	return false;
}


void ListPanel::togglePanelPopup() {
	if (popup->isHidden()) {
		if (popupSizes.count() > 0) {
			dynamic_cast<TQSplitter*>(popup->tqparent())->setSizes(popupSizes);
		} else { // on the first time, resize to 50%
			TQValueList<int> lst;
			lst << height()/2 << height()/2;
			dynamic_cast<TQSplitter*>(popup->tqparent())->setSizes(lst);
		}
		
		popup->show();
		popupBtn->setPixmap(krLoader->loadIcon("1downarrow", KIcon::Toolbar, 16));
		TQToolTip::add(  popupBtn, i18n( "Close the popup panel" ) );
	} else {
		popupSizes.clear();
		popupSizes = dynamic_cast<TQSplitter*>(popup->tqparent())->sizes();
		popup->hide();
		popupBtn->setPixmap(krLoader->loadIcon("1uparrow", KIcon::Toolbar, 16));
		TQToolTip::add(  popupBtn, i18n( "Open the popup panel" ) );
		
		TQValueList<int> lst;
		lst << height() << 0;
		dynamic_cast<TQSplitter*>(popup->tqparent())->setSizes(lst);
		if( ACTIVE_PANEL )
			ACTIVE_PANEL->slotFocusOnMe();
	}
}

KURL ListPanel::virtualPath() const {
	return func->files()->vfs_getOrigin(); 
}

TQString ListPanel::realPath() const { 
	return _realPath.path(); 
}


void ListPanel::setPanelToolbar() {
   krConfig->setGroup( "Look&Feel" );

   bool panelToolBarVisible = krConfig->readBoolEntry( "Panel Toolbar visible", _PanelToolBar );

   if ( panelToolBarVisible && ( krConfig->readBoolEntry( "Root Button Visible", _cdRoot ) ) )
      cdRootButton->show();
   else
      cdRootButton->hide();

   if ( panelToolBarVisible && ( krConfig->readBoolEntry( "Home Button Visible", _cdHome ) ) )
      cdHomeButton->show();
   else
      cdHomeButton->hide();

   if ( panelToolBarVisible && ( krConfig->readBoolEntry( "Up Button Visible", _cdUp ) ) )
      cdUpButton->show();
   else
      cdUpButton->hide();

   if ( panelToolBarVisible && ( krConfig->readBoolEntry( "Equal Button Visible", _cdOther ) ) )
      cdOtherButton->show();
   else
      cdOtherButton->hide();

   if ( !panelToolBarVisible || ( krConfig->readBoolEntry( "Open Button Visible", _Open ) ) )
      origin->button() ->show();
   else
      origin->button() ->hide();

   if ( panelToolBarVisible && ( krConfig->readBoolEntry( "SyncBrowse Button Visible", _syncBrowseButton ) ) )
      syncBrowseButton->show();
   else
      syncBrowseButton->hide();
}

void ListPanel::slotUpdateTotals() {
   totals->setText( view->statistics() );
}

void ListPanel::slotFocusAndCDOther() {
   slotFocusOnMe();
   func->openUrl( otherPanel->func->files() ->vfs_getOrigin() );

}

void ListPanel::slotFocusAndCDHome() {
   slotFocusOnMe();
   func->openUrl( TQString( "~" ), TQString() );
}

void ListPanel::slotFocusAndCDup() {
   slotFocusOnMe();
   func->dirUp();
}

void ListPanel::slotFocusAndCDRoot() {
   slotFocusOnMe();
   func->openUrl( TQString( "/" ), TQString() );
}

void ListPanel::select( KRQuery query, bool select) {
   if ( !query.isNull() ) {
      if ( select )
         view->select( query );
      else
         view->unselect( query );
   }
}

void ListPanel::select( bool select, bool all ) {
   if ( all )
   {
      if ( select )
         view->select( KRQuery( "*" ) );
      else
         view->unselect( KRQuery( "*" ) );
   }
   else {
      KRQuery query = KRSpWidgets::getMask( ( select ? i18n( " Select Files " ) : i18n( " Unselect Files " ) ) );
      // if the user canceled - quit
      if ( query.isNull() )
         return ;
      if ( select )
         view->select( query );
      else
         view->unselect( query );
   }
}

void ListPanel::invertSelection() {
   view->invertSelection();
}

void ListPanel::compareDirs() {
   krConfig->setGroup( "Private" );
   int compareMode = krConfig->readNumEntry( "Compare Mode", 0 );
   krConfig->setGroup( "Look&Feel" );
   bool selectDirs = krConfig->readBoolEntry( "Mark Dirs", false );
  
   KrViewItem *item, *otherItem;
  
   for( item = view->getFirst(); item != 0; item = view->getNext( item ) )
   {
      if( item->name() == ".." )
         continue;
      
      for( otherItem = otherPanel->view->getFirst(); otherItem != 0 && otherItem->name() != item->name() ;
         otherItem = otherPanel->view->getNext( otherItem ) );
           
      bool isSingle = ( otherItem == 0 ), isDifferent = false, isNewer = false;
   
      if( func->getVFile(item)->vfile_isDir() && !selectDirs )
      {
         item->setSelected( false );
         continue;
      }
      
      if( otherItem )
      {
         if( !func->getVFile(item)->vfile_isDir() )
            isDifferent = ITEM2VFILE(otherPanel,otherItem)->vfile_getSize() != func->getVFile(item)->vfile_getSize();
         isNewer = func->getVFile(item)->vfile_getTime_t() > ITEM2VFILE(otherPanel, otherItem)->vfile_getTime_t();
      }

      switch( compareMode )
      {
      case 0:
         item->setSelected( isNewer || isSingle );
         break;
      case 1:
         item->setSelected( isNewer );
         break;
      case 2:
         item->setSelected( isSingle );
         break;
      case 3:
         item->setSelected( isDifferent || isSingle );
         break;
      case 4:
         item->setSelected( isDifferent );
         break;
      }
   }
   
   view->updateView();
}

void ListPanel::slotFocusOnMe() {
   // give this VFS the focus (the path bar)
   // we start by calling the KVFS function
   krConfig->setGroup( "Look&Feel" );

   // take care of the 'otherpanel'
   TQPalette q( otherPanel->status->palette() );
   q.setColor( TQColorGroup::Foreground, KGlobalSettings::textColor() );
   q.setColor( TQColorGroup::Background, KGlobalSettings::baseColor() );

   otherPanel->status->setPalette( q );
   otherPanel->totals->setPalette( q );
   otherPanel->view->prepareForPassive();

   // now, take care of this panel
   TQPalette p( status->palette() );
   p.setColor( TQColorGroup::Foreground, KGlobalSettings::highlightedTextColor() );
   p.setColor( TQColorGroup::Background, KGlobalSettings::highlightColor() );
   status->setPalette( p );
   totals->setPalette( p );

   view->prepareForActive();
   emit cmdLineUpdate( realPath() );
   emit activePanelChanged( this );

   func->refreshActions();
   
   Krusader::actDetailedView->setEnabled( panelType != "Detailed" ); // enable/disable the detailed view action
   Krusader::actBriefView->setEnabled( panelType != "Brief" );       // enable/disable the brief view action

   if( panelType == "Brief" )
   {
      KrBriefView * v = dynamic_cast<KrBriefView *>( view );
      if ( v )
         v->refreshColors();
   }
   else /* detailed */
   {
      KrDetailedView * v = dynamic_cast<KrDetailedView *>( view );
      if ( v )
         v->refreshColors();
   }
   
   if( otherPanel->panelType == "Brief" )
   {
      KrBriefView * v = dynamic_cast<KrBriefView *>( otherPanel->view );
      if ( v )
         v->refreshColors();
   }
   else /* detailed */
   {   
      KrDetailedView *v = dynamic_cast<KrDetailedView *>( otherPanel->view );
      if ( v )
         v->refreshColors();
   }
}

// this is used to start the panel, AFTER setOther() has been used
//////////////////////////////////////////////////////////////////
void ListPanel::start( KURL url, bool immediate ) {
   KURL virt;
   
   virt = url;
   
   if ( !virt.isValid() )
      virt = URL("/");
   if( virt.isLocalFile() ) _realPath = virt;
   else _realPath = URL("/");

   if( immediate )
     func->immediateOpenUrl( virt );
   else
     func->openUrl( virt );

   slotFocusOnMe();
   setJumpBack( virt );
}

void ListPanel::slotStartUpdate() {
   while ( func->inRefresh )
      ; // wait until the last refresh finish
   func->inRefresh = true;  // make sure the next refresh wait for this one
	if (inlineRefreshJob)
		inlineRefreshListResult(0);

	if( this == ACTIVE_PANEL ){
		slotFocusOnMe();
	}

   setCursor( KCursor::workingCursor() );
   view->clear();

   if ( func->files() ->vfs_getType() == vfs::NORMAL )
      _realPath = virtualPath();
   this->origin->setURL( vfs::pathOrURL( virtualPath() ) );
   emit pathChanged( this );
   emit cmdLineUpdate( realPath() );	// update the command line

   slotGetStats( virtualPath() );
   slotUpdate();
   if ( compareMode ) {
      otherPanel->view->clear();
      otherPanel->slotUpdate();
   }
   // return cursor to normal arrow
   setCursor( KCursor::arrowCursor() );
   slotUpdateTotals();
	krApp->popularUrls->addUrl(virtualPath());
}

void ListPanel::slotUpdate() {
   // if we are not at the root add the ".." entery
   TQString protocol = func->files() ->vfs_getOrigin().protocol();
   bool isFtp = ( protocol == "ftp" || protocol == "smb" || protocol == "sftp" || protocol == "fish" );

   TQString origin = virtualPath().prettyURL(-1);
   if ( origin.right( 1 ) != "/" && !( ( func->files() ->vfs_getType() == vfs::FTP ) && isFtp &&
                                       origin.tqfind( '/', origin.tqfind( ":/" ) + 3 ) == -1 ) ) {
      view->addItems( func->files() );
   } else
      view->addItems( func->files(), false );

   func->inRefresh = false;
}


void ListPanel::slotGetStats( const KURL& url ) {
   if ( !url.isLocalFile() ) {
      status->setText( i18n( "No space information on non-local filesystems" ) );
      return ;
   }

	// check for special filesystems;
	TQString path = url.path(); // must be local url
	if ( path.left(4) == "/dev") {
		status->setText(i18n( "No space information on [dev]" ));
		return;
	}
#if defined(BSD)
	if ( path.left( 5 ) == "/procfs" ) { // /procfs is a special case - no volume information
		status->setText(i18n( "No space information on [procfs]" ));
		return;
	}
#else
	if ( path.left( 5 ) == "/proc" ) { // /proc is a special case - no volume information
		status->setText(i18n( "No space information on [proc]" ));
		return;
	}
#endif

   status->setText( i18n( "Mt.Man: working ..." ) );
	statsAgent = KDiskFreeSp::findUsageInfo( path );
   connect( statsAgent, TQT_SIGNAL( foundMountPoint( const TQString &, unsigned long, unsigned long, unsigned long ) ),
            this, TQT_SLOT( gotStats( const TQString &, unsigned long, unsigned long, unsigned long ) ) );
}

void ListPanel::gotStats( const TQString &mountPoint, unsigned long kBSize,
                          unsigned long,  unsigned long kBAvail ) {
	int perc = 0;
	if (kBSize != 0) { // make sure that if totalsize==0, then perc=0
		perc = (int)(((float)kBAvail / (float)kBSize)*100.0);
	}
	// mount point information - find it in the list first
	KMountPoint::List lst = KMountPoint::currentMountPoints();
	TQString fstype = i18n("unknown");
   for (KMountPoint::List::iterator it = lst.begin(); it != lst.end(); ++it) {
		if ((*it)->mountPoint() == mountPoint) {
			fstype = (*it)->mountType();
			break;
		}
	}
	
	TQString stats = i18n( "%1 free out of %2 (%3%) on %4 [ (%5) ]" )
        .tqarg( KIO::convertSizeFromKB( kBAvail ) )
        .tqarg( KIO::convertSizeFromKB( kBSize ) ).tqarg( perc )
        .tqarg( mountPoint ).tqarg( fstype );
	status->setText( stats );
}

void ListPanel::handleDropOnTotals( TQDropEvent *e ) {
  handleDropOnView( e, totals );
}

void ListPanel::handleDropOntqStatus( TQDropEvent *e ) {
  handleDropOnView( e, status );
}

void ListPanel::handleDropOnView( TQDropEvent *e, TQWidget *widget ) {
   // if copyToPanel is true, then we call a simple vfs_addfiles
   bool copyToDirInPanel = false;
   bool dragFromOtherPanel = false;
   bool dragFromThisPanel = false;
   bool isWritable = func->files() ->vfs_isWritable();

   vfs* tempFiles = func->files();
   vfile *file;
   KrViewItem *i = 0;
   if( widget == 0 )
   {
      i = view->getKrViewItemAt( e->pos() );
      widget = this;
   }

   if ( e->source() == otherPanel )
      dragFromOtherPanel = true;
   if ( e->source() == this )
      dragFromThisPanel = true;

   if ( i ) {
      file = func->files() ->vfs_search( i->name() );

      if ( !file ) { // trying to drop on the ".."
            copyToDirInPanel = true;
      } else {
         if ( file->vfile_isDir() ) {
            copyToDirInPanel = true;
            isWritable = file->vfile_isWriteable();
            if ( isWritable ) {
               // keep the folder_open icon until we're finished, do it only
               // if the folder is writeable, to avoid flicker
            }
         } else
            if ( e->source() == this )
               return ; // no dragging onto ourselves
      }
   } else    // if dragged from this panel onto an empty spot in the panel...
      if ( dragFromThisPanel ) {  // leave!
         e->ignore();
         return ;
      }

   if ( !isWritable && getuid() != 0 ) {
      e->ignore();
      KMessageBox::sorry( 0, i18n( "Can't drop here, no write permissions." ) );
      return ;
   }
   //////////////////////////////////////////////////////////////////////////////
   // decode the data
   KURL::List URLs;
   if ( !KURLDrag::decode( e, URLs ) ) {
      e->ignore(); // not for us to handle!
      return ;
   }

   bool isLocal = true;
   for( unsigned u = 0; u != URLs.count(); u++ )
     if( !URLs[ u ].isLocalFile() ) {
       isLocal = false;
       break;
     }

   KIO::CopyJob::CopyMode mode = KIO::CopyJob::Copy;

   // the KURL::List is finished, let's go
   // --> display the COPY/MOVE/LINK menu
   TQPopupMenu popup( this );
   popup.insertItem( i18n( "Copy Here" ), 1 );
   if ( func->files() ->vfs_isWritable() )
      popup.insertItem( i18n( "Move Here" ), 2 );
   if ( func->files() ->vfs_getType() == vfs::NORMAL &&
         isLocal )
      popup.insertItem( i18n( "Link Here" ), 3 );
   popup.insertItem( i18n( "Cancel" ), 4 );
   TQPoint tmp = widget->mapToGlobal( e->pos() );
   int result = popup.exec( tmp );
   switch ( result ) {
         case 1 :
         mode = KIO::CopyJob::Copy;
         break;
         case 2 :
         mode = KIO::CopyJob::Move;
         break;
         case 3 :
         mode = KIO::CopyJob::Link;
         break;
         case - 1 :         // user pressed outside the menu
         case 4:
         return ; // cancel was pressed;
   }

   TQString dir = "";
   if ( copyToDirInPanel ) {
      dir = i->name();
   }
   TQWidget *notify = ( !e->source() ? 0 : e->source() );
   tempFiles->vfs_addFiles( &URLs, mode, TQT_TQOBJECT(notify), dir );
}

void ListPanel::startDragging( TQStringList names, TQPixmap px ) {
   KURL::List * urls = func->files() ->vfs_getFiles( &names );

   if ( urls->isEmpty() ) { // avoid draging empty urls
      delete urls;
      return ;
   }

	KURLDrag *d = new KURLDrag(*urls, this);
   d->setPixmap( px, TQPoint( -7, 0 ) );
   d->dragCopy();

   delete urls; // free memory
}

// pops a right-click menu for items
void ListPanel::popRightClickMenu( const TQPoint &loc ) {
   // run it, on the mouse location
   int j = TQFontMetrics( font() ).height() * 2;
   KrPopupMenu::run(TQPoint( loc.x() + 5, loc.y() + j ), this);
}

void ListPanel::popEmptyRightClickMenu( const TQPoint &loc ) {
	KrPopupMenu::run(loc, this);
}

void ListPanel::setFilter( KrViewProperties::FilterSpec f ) {
   switch ( f ) {
         case KrViewProperties::All :
         //case KrView::EXEC:
         break;
         case KrViewProperties::Custom :
         filterMask = KRSpWidgets::getMask( i18n( " Select Files " ) );
         // if the user canceled - quit
         if ( filterMask.isNull() )
            return;
         view->setFilterMask( filterMask );
         break;
         default:
         return ;
   }
   view->setFilter( f ); // do that in any case
   func->files()->vfs_tqinvalidate();
   func->refresh();
}

TQString ListPanel::getCurrentName() {
   TQString name = view->getCurrentItem();
   if ( name != ".." )
      return name;
   else
      return TQString();
}

void ListPanel::prepareToDelete() {
   view->setNameToMakeCurrent( view->firstUnmarkedBelowCurrent() );
}

void ListPanel::keyPressEvent( TQKeyEvent *e ) {
   switch ( e->key() ) {
         case Key_Enter :
         case Key_Return :
         if ( e->state() & ControlButton ) {
         	if (e->state() & AltButton) {
         		vfile *vf = func->files()->vfs_search(view->getCurrentKrViewItem()->name());
         		if (vf && vf->vfile_isDir()) SLOTS->newTab(vf->vfile_getUrl());
         	} else {
					SLOTS->insertFileName( ( e->state() & ShiftButton ) != 0 );
            }
         } else e->ignore();
         break;
         case Key_Right :
         case Key_Left :
         if ( e->state() == ControlButton ) {
            // user pressed CTRL+Right/Left - refresh other panel to the selected path if it's a
            // directory otherwise as this one
            if ( ( _left && e->key() == Key_Right ) || ( !_left && e->key() == Key_Left ) ) {
               KURL newPath;
               KrViewItem *it = view->getCurrentKrViewItem();

               if( it->name() == ".." ) {
                  newPath = func->files()->vfs_getOrigin().upURL();
               } else {
                  vfile *v = func->getVFile( it );
                  if ( v && v->vfile_isDir() && v->vfile_getName() != ".." ) {
                     newPath = v->vfile_getUrl();
                  } else {
                     newPath = func->files() ->vfs_getOrigin();
                  }
               }
               otherPanel->func->openUrl( newPath );
            } else func->openUrl( otherPanel->func->files() ->vfs_getOrigin() );
            return ;
         } else
            e->ignore();
         break;

         case Key_Down :
         if ( e->state() == ControlButton ) { // give the keyboard focus to the command line
            if ( MAIN_VIEW->cmdLine->isVisible() )
               MAIN_VIEW->cmdLineFocus();
            else 
               MAIN_VIEW->focusTerminalEmulator();
            return ;
         } else if ( e->state() == ( ControlButton | ShiftButton ) ) { // give the keyboard focus to TE
           MAIN_VIEW->focusTerminalEmulator();
         } else
            e->ignore();
         break;

			case Key_Up :
          if ( e->state() == ControlButton ) { // give the keyboard focus to the command line
            origin->lineEdit()->setFocus();
            return ;
          } else
             e->ignore();
			break;

         default:
         // if we got this, it means that the view is not doing
         // the quick search thing, so send the characters to the commandline, if normal key
         if ( e->state() == Qt::NoButton )
            MAIN_VIEW->cmdLine->addText( e->text() );

         //e->ignore();
   }
}

void ListPanel::slotItemAdded(vfile *vf) {
	view->addItem(vf);
}

void ListPanel::slotItemDeleted(const TQString& name) {
	view->delItem(name);
}

void ListPanel::slotItemUpdated(vfile *vf) {
	view->updateItem(vf);
}

void ListPanel::slotCleared() {
	view->clear();
}

void ListPanel::showEvent( TQShowEvent *e ) {
  panelActive();
  TQWidget::showEvent(e);
}

void ListPanel::hideEvent( TQHideEvent *e ) {
  panelInactive();
  TQWidget::hideEvent(e);
}

void ListPanel::panelActive() {
	// don't refresh when not active (ie: hidden, application isn't focussed ...)
	func->files()->vfs_enableRefresh(true);
}

void ListPanel::panelInactive() {
	func->files()->vfs_enableRefresh(false);
}

void ListPanel::slotJobStarted(KIO::Job* job) {
	// disable the parts of the panel we don't want touched
	//static_cast<KrDetailedView*>(view)->setEnabled(false);
	status->setEnabled(false);
	origin->setEnabled(false);
	cdRootButton->setEnabled(false);
   cdHomeButton->setEnabled(false);
   cdUpButton->setEnabled(false);
   cdOtherButton->setEnabled(false);
	popupBtn->setEnabled(false);
	popup->setEnabled(false);
   bookmarksButton->setEnabled(false);
   historyButton->setEnabled(false);
   syncBrowseButton->setEnabled(false);

	// connect to the job interface to provide in-panel refresh notification
	connect( job, TQT_SIGNAL( infoMessage( KIO::Job*, const TQString & ) ),
		TQT_SLOT( inlineRefreshInfoMessage( KIO::Job*, const TQString & ) ) );
	connect( job, TQT_SIGNAL( percent( KIO::Job*, unsigned long ) ),
      TQT_SLOT( inlineRefreshPercent( KIO::Job*, unsigned long ) ) );		
	connect(job,TQT_SIGNAL(result(KIO::Job*)),
         this,TQT_SLOT(inlineRefreshListResult(KIO::Job*)));
	connect(job,TQT_SIGNAL(canceled(KIO::Job*)),
         this,TQT_SLOT(inlineRefreshListResult(KIO::Job*)));
			
	inlineRefreshJob = job;
	
	totals->setText(i18n(">> Reading..."));
	inlineRefreshCancelButton->show();
}

void ListPanel::inlineRefreshCancel() {
	if (inlineRefreshJob) {
		inlineRefreshJob->kill(false);
		inlineRefreshJob = 0;
	}
}

void ListPanel::inlineRefreshPercent( KIO::Job*, unsigned long perc) {
	TQString msg = TQString(">> %1: %2 % complete...").tqarg(i18n("Reading")).tqarg(perc);
	totals->setText(msg);
}

void ListPanel::inlineRefreshInfoMessage( KIO::Job*, const TQString &msg ) {
	totals->setText(">> " + i18n("Reading: ") + msg);
}

void ListPanel::inlineRefreshListResult(KIO::Job*) {
	inlineRefreshJob = 0;
	// reenable everything
	//static_cast<KrDetailedView*>(view)->setEnabled(true);
	status->setEnabled(true);
	origin->setEnabled(true);
	cdRootButton->setEnabled(true);
   cdHomeButton->setEnabled(true);
   cdUpButton->setEnabled(true);
   cdOtherButton->setEnabled(true);
	popupBtn->setEnabled(true);
	popup->setEnabled(true);
   bookmarksButton->setEnabled(true);
   historyButton->setEnabled(true);
   syncBrowseButton->setEnabled(true);
	
	inlineRefreshCancelButton->hide();
}

void ListPanel::jumpBack() {
	func->openUrl( _jumpBackURL );
}

void ListPanel::setJumpBack( KURL url ) {
	_jumpBackURL = url;
}

#include "listpanel.moc"
