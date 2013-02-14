/***************************************************************************
                          krusader.cpp
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
#include <sys/types.h>
#include <sys/stat.h> 
#include <sys/param.h>
#include <unistd.h>
#ifdef BSD
#include <sys/types.h>
#endif
// KDE includes
#include <kmessagebox.h>
#include <tdeaction.h>
#include <kcursor.h>
#include <ksystemtray.h>
#include <kmenubar.h>
#include <tdeapplication.h>
#include <tdecmdlineargs.h>
#include <kglobal.h>
#include <klocale.h>
#include <tdeaccelmanager.h>
#include <twin.h>

#if KDE_IS_VERSION(3,2,0)
#include <tdeactionclasses.h>
#endif

#include <tdeversion.h> 
// QT includes
#include <tqpixmap.h>
#include <tqstringlist.h>
#include <tqdir.h>
#include <tqprinter.h>
#include <tqprogressdialog.h>
#include <tqvaluelist.h>
#include <tqwhatsthis.h> 
#include <tqwidgetlist.h>
#include <tqdatetime.h>
#include <dcopclient.h>
// Krusader includes
#include "krusader.h"
#include "kicons.h"
#include "VFS/krpermhandler.h"
#include "GUI/krusaderstatus.h"
#include "RemoteMan/remoteman.h"
#include "Dialogs/krpleasewait.h"
#include "krusaderview.h"
#include "Panel/listpanel.h"
#include "Panel/panelfunc.h"
#include "Konfigurator/konfigurator.h"
#include "Konfigurator/kgprotocols.h"
#include "MountMan/kmountman.h"
#include "Panel/panelpopup.h"
#include "Queue/queue_mgr.h"
#include "defaults.h"
#include "resources.h"
#include "GUI/kfnkeys.h"
#include "GUI/kcmdline.h"
#include "krslots.h"
#include "krservices.h"
#include "UserAction/useraction.h"
// This makes gcc-4.1 happy. Warning about possible problem with KrAction's dtor not called
#include "UserAction/kraction.h"
#include "UserAction/expander.h"
#include "UserMenu/usermenu.h"
#include "panelmanager.h"
#include "MountMan/kmountman.h"
#include "BookMan/krbookmarkhandler.h"
#include "Dialogs/popularurls.h"
#include "GUI/krremoteencodingmenu.h"
#include "Dialogs/checksumdlg.h"

#ifdef __KJSEMBED__
#include "KrJS/krjs.h"
#endif

// define the static members
Krusader *Krusader::App = 0;
TDEAction *Krusader::actProperties = 0;
TDEAction *Krusader::actPack = 0;
TDEAction *Krusader::actUnpack = 0;
TDEAction *Krusader::actTest = 0;
TDEAction *Krusader::actCopy = 0;
TDEAction *Krusader::actPaste = 0;
TDEAction *Krusader::actCompare = 0;
TDEAction *Krusader::actCalculate = 0;
TDEAction *Krusader::actCreateChecksum = 0;
TDEAction *Krusader::actMatchChecksum = 0;
TDEAction *Krusader::actSelect = 0;
TDEAction *Krusader::actSelectAll = 0;
TDEAction *Krusader::actUnselect = 0;
TDEAction *Krusader::actUnselectAll = 0;
TDEAction *Krusader::actInvert = 0;
TDEAction *Krusader::actCompDirs = 0;
TDEAction *Krusader::actSync = 0;
TDEAction *Krusader::actDiskUsage = 0;
TDEAction *Krusader::actHomeTerminal = 0;
TDEAction *Krusader::actFTPConnect = 0;
TDEAction *Krusader::actFTPNewConnect = 0;
TDEAction *Krusader::actFTPDisconnect = 0;
TDEAction *Krusader::actProfiles = 0;
TDEAction *Krusader::actMultiRename = 0;
TDEAction *Krusader::actAllFilter = 0;
TDEAction *Krusader::actExecFilter = 0;
TDEAction *Krusader::actCustomFilter = 0;
TDEAction *Krusader::actMountMan = 0;
TDEAction *Krusader::actNewTool = 0;
TDEAction *Krusader::actKonfigurator = 0;
TDEAction *Krusader::actToolsSetup = 0;
TDEAction *Krusader::actSwapPanels = 0;
TDEAction *Krusader::actSwapSides = 0;
TDEAction *Krusader::actBack = 0;
TDEAction *Krusader::actRoot = 0;
TDEAction *Krusader::actFind = 0;
TDEAction *Krusader::actLocate = 0;
TDEAction *Krusader::actSwitchFullScreenTE = 0;
//TDEAction *Krusader::actAddBookmark = 0;
TDEAction *Krusader::actSavePosition = 0;
TDEAction *Krusader::actSelectColorMask = 0;
TDEAction *Krusader::actOpenLeftBm = 0;
TDEAction *Krusader::actOpenRightBm = 0;
TDEAction *Krusader::actDirUp = 0;
TDEAction *Krusader::actCmdlinePopup = 0;
TDEAction *Krusader::actNewTab = 0;
TDEAction *Krusader::actDupTab = 0;
TDEAction *Krusader::actCloseTab = 0;
TDEAction *Krusader::actNextTab = 0;
TDEAction *Krusader::actPreviousTab = 0;
TDEAction *Krusader::actSplit = 0;
TDEAction *Krusader::actCombine = 0;
TDEAction *Krusader::actUserMenu = 0;
TDEAction *Krusader::actManageUseractions = 0;
TDEAction *Krusader::actSyncDirs = 0;
TDEAction *Krusader::actSyncBrowse = 0;
TDEAction *Krusader::actF2 = 0;
TDEAction *Krusader::actF3 = 0;
TDEAction *Krusader::actF4 = 0;
TDEAction *Krusader::actF5 = 0;
TDEAction *Krusader::actF6 = 0;
TDEAction *Krusader::actF7 = 0;
TDEAction *Krusader::actF8 = 0;
TDEAction *Krusader::actF9 = 0;
TDEAction *Krusader::actF10 = 0;
TDEAction *Krusader::actLocationBar = 0;
TDEAction *Krusader::actPopularUrls = 0;
TDEAction *Krusader::actJumpBack = 0;
TDEAction *Krusader::actSetJumpBack = 0;
TDEAction *Krusader::actDetailedView = 0;
TDEAction *Krusader::actBriefView = 0;

TDEToggleAction *Krusader::actToggleTerminal = 0;
TDEToggleAction *Krusader::actVerticalMode = 0;
TDERadioAction  *Krusader::actSelectNewerAndSingle = 0;
TDERadioAction  *Krusader::actSelectSingle = 0;
TDERadioAction  *Krusader::actSelectNewer = 0;
TDERadioAction  *Krusader::actSelectDifferentAndSingle = 0;
TDERadioAction  *Krusader::actSelectDifferent = 0;
TDERadioAction  **Krusader::compareArray[] = {&actSelectNewerAndSingle, &actSelectNewer, &actSelectSingle, 
                                            &actSelectDifferentAndSingle, &actSelectDifferent, 0};
TDERadioAction *Krusader::actExecStartAndForget = 0;
TDERadioAction *Krusader::actExecCollectSeparate = 0;
TDERadioAction *Krusader::actExecCollectTogether = 0;
TDERadioAction *Krusader::actExecTerminalExternal = 0;
TDERadioAction *Krusader::actExecTerminalEmbedded = 0;
TDERadioAction **Krusader::execTypeArray[] =
       {&actExecStartAndForget, &actExecCollectSeparate, &actExecCollectTogether,
        &actExecTerminalExternal, &actExecTerminalEmbedded, 0};

TDEPopupMenu *Krusader::userActionMenu = 0;
UserAction *Krusader::userAction = 0;
UserMenu *Krusader::userMenu = 0;
KrBookmarkHandler *Krusader::bookman = 0;
//TQTextOStream *Krusader::_krOut = TQTextOStream(::stdout);

#ifdef __KJSEMBED__
KrJS *Krusader::js = 0;
TDEAction *Krusader::actShowJSConsole = 0;
#endif

// construct the views, statusbar and menu bars and prepare Krusader to start
Krusader::Krusader() : KParts::MainWindow(0,0,WType_TopLevel|WDestructiveClose|TQt::WStyle_ContextHelp),
   DCOPObject("Krusader-Interface"), status(NULL), sysTray( 0 ), isStarting( true ), isExiting( false ), directExit( false ) {
   // parse command line arguments
   TDECmdLineArgs * args = TDECmdLineArgs::parsedArgs();

   kapp->ref(); // FIX: krusader exits at closing the viewer when minimized to tray

   // create the "krusader"
   App = this;
   slot = new KRslots(TQT_TQOBJECT(this));
   setXMLFile( "krusaderui.rc" ); // kpart-related xml file

   plzWait = new KRPleaseWaitHandler();

   bool runKonfig = versionControl();

   TQString message;
   switch ( config->getConfigState() ) {
         case TDEConfigBase::NoAccess :
         message = "Krusader's configuration file can't be found. Default values will be used.";
         break;
         case TDEConfigBase::ReadOnly :
         message = "Krusader's configuration file is in READ ONLY mode (why is that!?) Changed values will not be saved";
         break;
         case TDEConfigBase::ReadWrite :
         message = "";
         break;
   }
   if ( message != "" ) {
      KMessageBox::error( krApp, message );
   }

   // create an icon loader
   iconLoader = TDEGlobal::iconLoader();

   // create MountMan
   mountMan = new KMountMan();

   // create bookman
   bookman = new KrBookmarkHandler();

   popularUrls = new PopularUrls(TQT_TQOBJECT(this));

   queueManager = new QueueManager();

   // create the main view
   mainView = new KrusaderView( this );

   // setup all the krusader's actions
   setupActions();

   // init the permmision handler class
   KRpermHandler::init();

   // init the protocol handler
   KgProtocols::init();

   // init the checksum tools
   initChecksumModule();

   krConfig->setGroup( "Look&Feel" );
   TQString defaultType = krConfig->readEntry( "Default Panel Type", _DefaultPanelType );

   krConfig->setGroup( "Startup" );
   TQStringList     leftTabs = krConfig->readPathListEntry( "Left Tab Bar" );
   TQStringList     rightTabs = krConfig->readPathListEntry( "Right Tab Bar" );
   TQStringList     leftTabTypes = krConfig->readListEntry( "Left Tab Bar Types" );
   TQStringList     rightTabTypes = krConfig->readListEntry( "Right Tab Bar Types" );
   TQValueList<int> leftTabProps = krConfig->readIntListEntry( "Left Tab Bar Props" );
   TQValueList<int> rightTabProps = krConfig->readIntListEntry( "Right Tab Bar Props" );
   int             leftActiveTab = krConfig->readNumEntry( "Left Active Tab", 0 );
   int             rightActiveTab = krConfig->readNumEntry( "Right Active Tab", 0 );
   TQString         startProfile = krConfig->readEntry("Starter Profile Name", TQString() );
   bool            leftActive = krConfig->readBoolEntry( "Left Side Is Active", false );
   
   // get command-line arguments
   if ( args->isSet( "left" ) ) {
      leftTabs = TQStringList::split( ',', args->getOption( "left" ) );
      leftTabTypes.clear();
      leftTabProps.clear();

      leftActiveTab = 0;

      // make sure left or right are not relative paths
      for(unsigned int i = 0; i != leftTabs.count(); i++ )
      {
        leftTabs[ i ] = leftTabs[ i ].stripWhiteSpace();
        if( !leftTabs[ i ].startsWith( "/" ) && leftTabs[ i ].find( ":/" ) < 0 )
          leftTabs[ i ] = TQDir::currentDirPath() + "/" + leftTabs[ i ];
      }
      startProfile = TQString();
   }
   if ( args->isSet( "right" ) ) {
      rightTabs = TQStringList::split( ',', args->getOption( "right" ) );
      rightTabTypes.clear();
      rightTabProps.clear();

      rightActiveTab = 0;

      // make sure left or right are not relative paths
      for(unsigned int i = 0; i != rightTabs.count(); i++ )
      {
        rightTabs[ i ] = rightTabs[ i ].stripWhiteSpace();
        if( !rightTabs[ i ].startsWith( "/" ) && rightTabs[ i ].find( ":/" ) < 0 )
          rightTabs[ i ] = TQDir::currentDirPath() + "/" + rightTabs[ i ];
      }
      startProfile = TQString();
   }

   while( leftTabTypes.count() < leftTabs.count() )
      leftTabTypes += defaultType;
   while( rightTabTypes.count() < rightTabs.count() )
      rightTabTypes += defaultType;
   while( leftTabProps.count() < leftTabs.count() )
      leftTabProps += 0;
   while( rightTabProps.count() < rightTabs.count() )
      rightTabProps += 0;

   if ( args->isSet( "profile" ) )
    startProfile = args->getOption( "profile" );

   if( !startProfile.isEmpty() ) {
      leftTabs.clear();
      leftTabTypes.clear();
      leftTabProps.clear();
      rightTabs.clear();   
      rightTabTypes.clear();   
      rightTabProps.clear();
      leftActiveTab = rightActiveTab = 0;
   }

   if( leftTabs.count() == 0 )
   {
     leftTabs.push_back( TQDir::homeDirPath() );
     leftTabTypes.push_back( defaultType );
     leftTabProps.push_back( 0 );
   }
   if( rightTabs.count() == 0 )
   {
     rightTabs.push_back( TQDir::homeDirPath() );
     rightTabTypes.push_back( defaultType );
     rightTabProps.push_back( 0 );
   }

   // starting the panels
   mainView->start( leftTabs, leftTabTypes, leftTabProps, leftActiveTab, rightTabs, 
                    rightTabTypes, rightTabProps, rightActiveTab, leftActive );

   // create the user menu
   userMenu = new UserMenu( this );
   userMenu->hide();

   // setup keyboard accelerators
   setupAccels();

   // create a status bar
   status = new KrusaderStatus( this );
   TQWhatsThis::add( status, i18n( "Statusbar will show basic information "
                                          "about file below mouse pointer." ) );

   // This enables Krusader to show a tray icon
   sysTray = new KSystemTray( this );
   // Krusader::privIcon() returns either "krusader_blue" or "krusader_red" if the user got root-privileges
   sysTray->setPixmap( iconLoader->loadIcon( privIcon(), TDEIcon::Panel, 22 ) );
   sysTray->hide();

   connect( sysTray, TQT_SIGNAL( quitSelected() ), TQT_TQOBJECT(this), TQT_SLOT( setDirectExit() ) );

   setCentralWidget( mainView );
   config->setGroup( "Startup" );
   bool startToTray = config->readBoolEntry( "Start To Tray", _StartToTray );
   config->setGroup( "Look&Feel" );
   bool minimizeToTray = config->readBoolEntry( "Minimize To Tray", _MinimizeToTray );
   bool singleInstanceMode = config->readBoolEntry( "Single Instance Mode", _SingleInstanceMode );

   startToTray = startToTray && minimizeToTray;

   if( singleInstanceMode && minimizeToTray )
     sysTray->show();


   // manage our keyboard short-cuts
   //TDEAcceleratorManager::manage(this,true);

   setCursor( KCursor::arrowCursor() );

   if ( ! startProfile.isEmpty() )
       mainView->profiles( startProfile );
   // let the good times rool :)
   updateGUI( true );

	if ( runKonfig )
		slot->runKonfigurator( true );
	
   if (!runKonfig) {
		config->setGroup( "Private" );
		if ( krConfig->readBoolEntry( "Maximized" ) )
			restoreWindowSize(config);
		else {
			move( oldPos = krConfig->readPointEntry( "Start Position", _StartPosition ) );
			resize( oldSize = krConfig->readSizeEntry( "Start Size", _StartSize ));
		}
	}
	
	if( startToTray ) {
		sysTray->show();
		hide();
	}
	else
		show();


   isStarting = false;
}

Krusader::~Krusader() {
   if( !isExiting )   // save the settings if it was not saved (SIGTERM)
      saveSettings();

   delete mainView;
   delete queueManager;
   mainView = 0;
   App = 0;
}

bool Krusader::versionControl() {
#define FIRST_RUN	"First Time"
   bool retval = false;
   // create config file
   config = kapp->config();
   bool firstRun = config->readBoolEntry(FIRST_RUN, true);

#if 0      
	TQString oldVerText = config->readEntry( "Version", "10.0" );
   oldVerText.truncate( oldVerText.findRev( "." ) ); // remove the third dot
   float oldVer = oldVerText.toFloat();
   // older icompatible version
   if ( oldVer <= 0.9 ) {
      KMessageBox::information( krApp, i18n( "A configuration of 1.51 or older was detected. Krusader has to reset your configuration to default values.\nNote: Your bookmarks and keybindings will remain intact.\n Krusader will now run Konfigurator." ) );
      /*if ( !TQDir::home().remove( ".kde/share/config/krusaderrc" ) ) {
         KMessageBox::error( krApp, i18n( "Unable to remove your krusaderrc file! Please remove it manually and rerun Krusader." ) );
         exit( 1 );
      }*/
      retval = true;
      config->reparseConfiguration();
   }
#endif

   // first installation of krusader
   if ( firstRun ) {
      KMessageBox::information( krApp, i18n( "<qt><b>Welcome to Krusader!</b><p>As this is your first run, your machine will now be checked for external applications. Then the Konfigurator will be launched where you can customize Krusader to your needs.</p></qt>" ) );
      retval = true;
   }
   config->writeEntry( "Version", VERSION );
   config->writeEntry( FIRST_RUN, false);
   config->sync();
   return retval;
}

void Krusader::statusBarUpdate( TQString& mess ) {
   // change the message on the statusbar for 2 seconds
   if (status) // ugly!!!! But as statusBar() creates a status bar if there is no, I have to ask status to prevent 
               // the creation of the TDE default status bar instead of KrusaderStatus.
      statusBar() ->message( mess, 5000 );
}

void Krusader::showEvent ( TQShowEvent * ) {
   if( isExiting )
     return;
   config->setGroup( "Look&Feel" );
   bool showTrayIcon = krConfig->readBoolEntry( "Minimize To Tray", _MinimizeToTray );
   bool singleInstanceMode = krConfig->readBoolEntry( "Single Instance Mode", _SingleInstanceMode );
   
   if( showTrayIcon && !singleInstanceMode )
     sysTray->hide();
   show(); // needed to make sure krusader is removed from
   // the taskbar when minimizing (system tray issue)
}

void Krusader::hideEvent ( TQHideEvent *e ) {
   if( isExiting ) {
     KParts::MainWindow::hideEvent( e );
     sysTray->hide();
     return;
   }
   TQString lastGroup = config->group();
   config->setGroup( "Look&Feel" );
   bool showTrayIcon = krConfig->readBoolEntry( "Minimize To Tray", _MinimizeToTray );
   config->setGroup ( lastGroup );

   bool isModalTopWidget = false;

   TQWidget *actWnd = TQT_TQWIDGET(tqApp->activeWindow());
   if ( actWnd )
      isModalTopWidget = actWnd->isModal();

   if ( showTrayIcon  && !isModalTopWidget  && KWin::windowInfo( winId() ).isOnCurrentDesktop() ) {
      sysTray->show();
      hide(); // needed to make sure krusader is removed from
      // the taskbar when minimizing (system tray issue)
   } else KParts::MainWindow::hideEvent( e );
}

void Krusader::moveEvent ( TQMoveEvent *e ) {
   oldPos = e->oldPos();
   KParts::MainWindow::moveEvent( e );
}

void Krusader::resizeEvent ( TQResizeEvent *e ) {
   oldSize = e->oldSize();
   KParts::MainWindow::resizeEvent( e );
}

void Krusader::setupAccels() {
	 accels = new TDEAccel( this );
	 // SHIFT+F3
   accels->insert( "F3_ViewDlg", i18n( "F3 View Dialog" ), TQString(),
                   SHIFT + Key_F3, SLOTS, TQT_SLOT( viewDlg() ) );
   // Tab
   accels->insert( "Tab-Switch panel", i18n( "Tab: switch panel" ), TQString(),
                   Key_Tab, TQT_TQOBJECT(mainView), TQT_SLOT( panelSwitch() ) );

}

// <patch> Moving from Pixmap actions to generic filenames - thanks to Carsten Pfeiffer
void Krusader::setupActions() {
   // first come the TODO actions
   //actSync =       0;//new TDEAction(i18n("S&yncronize Dirs"),                         0, this, 0, actionCollection(), "sync dirs");
   //actNewTool =    0;//new TDEAction(i18n("&Add a new tool"),                          0, this, 0, actionCollection(), "add tool");
   //actToolsSetup = 0;//new TDEAction(i18n("&Tools Menu Setup"),                        0, 0, this, 0, actionCollection(), "tools setup");
   //KStdAction::print(SLOTS, 0,actionCollection(),"std_print");
   //KStdAction::showMenubar( SLOTS, TQT_SLOT( showMenubar() ), actionCollection(), "std_menubar" );


   // second, the TDE standard action
   //KStdAction::up( SLOTS, TQT_SLOT( dirUp() ), actionCollection(), "std_up" )->setShortcut(Key_Backspace);
   /* Shortcut disabled because of the Terminal Emulator bug. */
   krConfig->setGroup( "Private" );
   int compareMode = krConfig->readNumEntry( "Compare Mode", 0 );
   int cmdExecMode =  krConfig->readNumEntry( "Command Execution Mode", 0 );

   KStdAction::home( SLOTS, TQT_SLOT( home() ), actionCollection(), "std_home" )->setText( i18n("Home") ); /*->setShortcut(Key_QuoteLeft);*/
   new TDEAction( i18n( "&Reload" ), "reload", CTRL + Key_R, SLOTS, TQT_SLOT( refresh() ), actionCollection(), "std_redisplay" );
   actShowToolBar = KStdAction::showToolbar( SLOTS, TQT_SLOT( toggleToolbar() ), actionCollection(), "std_toolbar" );
   new TDEToggleAction( i18n("Show Actions Toolbar"), 0, SLOTS, TQT_SLOT( toggleActionsToolbar() ),
                      actionCollection(), "toggle actions toolbar" );
   actShowStatusBar = KStdAction::showStatusbar( SLOTS, TQT_SLOT( toggleStatusbar() ), actionCollection(), "std_statusbar" );
   KStdAction::quit( TQT_TQOBJECT(this), TQT_SLOT( slotClose() ), actionCollection(), "std_quit" );
   KStdAction::configureToolbars( SLOTS, TQT_SLOT( configToolbar() ), actionCollection(), "std_config_toolbar" );
   KStdAction::keyBindings( SLOTS, TQT_SLOT( configKeys() ), actionCollection(), "std_config_keys" );

   KStdAction::cut( SLOTS, TQT_SLOT( cut() ), actionCollection(), "std_cut" )->setText( i18n("Cut to Clipboard") );
   (actCopy = KStdAction::copy( SLOTS, TQT_SLOT( copy() ), actionCollection(), "std_copy" ))->setText( i18n("Copy to Clipboard") );
   (actPaste = KStdAction::paste( SLOTS, TQT_SLOT( paste() ), actionCollection(), "std_paste" ))->setText( i18n("Paste from Clipboard") );

   // the toggle actions
   actToggleFnkeys = new TDEToggleAction( i18n( "Show &FN Keys Bar" ), 0, SLOTS,
                                        TQT_SLOT( toggleFnkeys() ), actionCollection(), "toggle fn bar" );
   actToggleFnkeys->setChecked( true );
   actToggleCmdline = new TDEToggleAction( i18n( "Show &Command Line" ), 0, SLOTS,
                                         TQT_SLOT( toggleCmdline() ), actionCollection(), "toggle command line" );
   actToggleCmdline->setChecked( true );
   actToggleTerminal = new TDEToggleAction( i18n( "Show Terminal &Emulator" ), ALT + CTRL + Key_T, SLOTS,
                                          TQT_SLOT( toggleTerminal() ), actionCollection(), "toggle terminal emulator" );
   actToggleTerminal->setChecked( false );

   actDetailedView = new TDEAction( i18n( "&Detailed View" ), ALT + SHIFT + Key_D, SLOTS,
                                TQT_SLOT( setDetailedView() ), actionCollection(), "detailed_view" );

   actBriefView = new TDEAction( i18n( "&Brief View" ), ALT + SHIFT + Key_B, SLOTS,
                                TQT_SLOT( setBriefView() ), actionCollection(), "brief_view" );

   actToggleHidden = new TDEToggleAction( i18n( "Show &Hidden Files" ), CTRL + Key_Period, SLOTS,
                                        TQT_SLOT( toggleHidden() ), actionCollection(), "toggle hidden files" );
   actSwapPanels = new TDEAction( i18n( "S&wap Panels" ), CTRL + Key_U, SLOTS,
                                TQT_SLOT( swapPanels() ), actionCollection(), "swap panels" );
   actSwapSides = new TDEAction( i18n( "Sw&ap Sides" ), CTRL + SHIFT + Key_U, SLOTS,
                                TQT_SLOT( toggleSwapSides() ), actionCollection(), "toggle swap sides" );
   krConfig->setGroup( "Look&Feel" );
   actToggleHidden->setChecked( krConfig->readBoolEntry( "Show Hidden", _ShowHidden ) );

   // and then the DONE actions
   actCmdlinePopup = new TDEAction( i18n( "popup cmdline" ), 0, CTRL + Key_Slash, SLOTS,
                                  TQT_SLOT( cmdlinePopup() ), actionCollection(), "cmdline popup" );
   /* Shortcut disabled because of the Terminal Emulator bug. */
   actDirUp = new TDEAction( i18n( "Up" ), "up", CTRL+Key_PageUp /*Key_Backspace*/, SLOTS, TQT_SLOT( dirUp() ), actionCollection(), "dirUp" );
   new TDEAction( i18n( "&New Text File..." ), "filenew", SHIFT + Key_F4, SLOTS, TQT_SLOT( editDlg() ), actionCollection(), "edit_new_file" );
   new TDEAction( i18n( "Start &Root Mode Krusader" ), "krusader_root", ALT + Key_K, SLOTS, TQT_SLOT( rootKrusader() ), actionCollection(), "root krusader" );

   actTest = new TDEAction( i18n( "T&est Archive" ), "ark", ALT + Key_E,
                          SLOTS, TQT_SLOT( testArchive() ), actionCollection(), "test archives" );
   //actFTPConnect = new TDEAction( i18n( "&Net Connections" ), "domtreeviewer", 0,
   //                             SLOTS, TQT_SLOT( runRemoteMan() ), actionCollection(), "ftp connect" );
   actFTPNewConnect = new TDEAction( i18n( "New Net &Connection..." ), "connect_creating", CTRL + Key_N,
                                   SLOTS, TQT_SLOT( newFTPconnection() ), actionCollection(), "ftp new connection" );
   actProfiles = new TDEAction( i18n( "Pro&files" ), "kr_profile", ALT + Key_L,
                                   TQT_TQOBJECT(MAIN_VIEW), TQT_SLOT( profiles() ), actionCollection(), "profile" );
   actCalculate = new TDEAction( i18n( "Calculate &Occupied Space" ), "kcalc", 0,
                               SLOTS, TQT_SLOT( calcSpace() ), actionCollection(), "calculate" );
   actCreateChecksum = new TDEAction( i18n( "Create Checksum..." ), "binary", 0,
                               SLOTS, TQT_SLOT( createChecksum() ), actionCollection(), "create checksum" );
   actMatchChecksum = new TDEAction( i18n( "Verify Checksum..." ), "match_checksum", 0,
                               SLOTS, TQT_SLOT( matchChecksum() ), actionCollection(), "match checksum" );
   actProperties = new TDEAction( i18n( "&Properties..." ), 0, ALT+Key_Enter,
                                SLOTS, TQT_SLOT( properties() ), actionCollection(), "properties" );
   actPack = new TDEAction( i18n( "Pac&k..." ), "kr_arc_pack", ALT + Key_P,
                          SLOTS, TQT_SLOT( slotPack() ), actionCollection(), "pack" );
   actUnpack = new TDEAction( i18n( "&Unpack..." ), "kr_arc_unpack", ALT + Key_U,
                            SLOTS, TQT_SLOT( slotUnpack() ), actionCollection() , "unpack" );
   actSplit = new TDEAction( i18n( "Sp&lit File..." ), "kr_split", CTRL + Key_P,
                           SLOTS, TQT_SLOT( slotSplit() ), actionCollection(), "split" );
   actCombine = new TDEAction( i18n( "Com&bine Files..." ), "kr_combine", CTRL + Key_B,
                             SLOTS, TQT_SLOT( slotCombine() ), actionCollection() , "combine" );
   actSelect = new TDEAction( i18n( "Select &Group..." ), "kr_select", CTRL + Key_Plus,
                            SLOTS, TQT_SLOT( markGroup() ), actionCollection(), "select group" );
   actSelectAll = new TDEAction( i18n( "&Select All" ), "kr_selectall", ALT + Key_Plus,
                               SLOTS, TQT_SLOT( markAll() ), actionCollection(), "select all" );
   actUnselect = new TDEAction( i18n( "&Unselect Group..." ), "kr_unselect", CTRL + Key_Minus,
                              SLOTS, TQT_SLOT( unmarkGroup() ), actionCollection(), "unselect group" );
   actUnselectAll = new TDEAction( i18n( "U&nselect All" ), "kr_unselectall", ALT + Key_Minus,
                                 SLOTS, TQT_SLOT( unmarkAll() ), actionCollection(), "unselect all" );
   actInvert = new TDEAction( i18n( "&Invert Selection" ), "kr_invert", ALT + Key_Asterisk,
                            SLOTS, TQT_SLOT( invert() ), actionCollection(), "invert" );
   actCompDirs = new TDEAction( i18n( "&Compare Directories" ), "view_left_right", ALT + Key_C,
                              SLOTS, TQT_SLOT( compareDirs() ), actionCollection(), "compare dirs" );
   actSelectNewerAndSingle = new TDERadioAction( i18n( "&Select Newer and Single" ), 0,
                                 SLOTS, TQT_SLOT( compareSetup() ), actionCollection(), "select_newer_and_single" );
   actSelectNewer = new TDERadioAction( i18n( "Select &Newer" ), 0,
                                 SLOTS, TQT_SLOT( compareSetup() ), actionCollection(), "select_newer" );
   actSelectSingle = new TDERadioAction( i18n( "Select &Single" ), 0,
                                 SLOTS, TQT_SLOT( compareSetup() ), actionCollection(), "select_single" );
   actSelectDifferentAndSingle = new TDERadioAction( i18n( "Select Different &and Single" ), 0,
                                 SLOTS, TQT_SLOT( compareSetup() ), actionCollection(), "select_different_and_single" );
   actSelectDifferent = new TDERadioAction( i18n( "Select &Different" ), 0,
                                 SLOTS, TQT_SLOT( compareSetup() ), actionCollection(), "select_different" );
   actSelectNewerAndSingle->setExclusiveGroup( "the_select_group" );
   actSelectNewer->setExclusiveGroup( "the_select_group" );
   actSelectSingle->setExclusiveGroup( "the_select_group" );
   actSelectDifferentAndSingle->setExclusiveGroup( "the_select_group" );
   actSelectDifferent->setExclusiveGroup( "the_select_group" );
   if( compareMode < (int)( sizeof( compareArray ) / sizeof( TDERadioAction ** ) ) -1 )
     (*compareArray[ compareMode ])->setChecked( true );
   actExecStartAndForget = new TDERadioAction(
                                 i18n( "Start and &Forget" ), 0,
                                 SLOTS, TQT_SLOT( execTypeSetup() ),
                                 actionCollection(), "exec_start_and_forget" );
   actExecCollectSeparate = new TDERadioAction(
                                 i18n( "Display &Separated Standard and Error Output" ), 0,
                                 SLOTS, TQT_SLOT( execTypeSetup() ),
                                 actionCollection(), "exec_collect_separate" );
   actExecCollectTogether = new TDERadioAction(
                                 i18n( "Display &Mixed Standard and Error Output" ), 0,
                                 SLOTS, TQT_SLOT( execTypeSetup() ),
                                 actionCollection(), "exec_collect_together" );
   actExecTerminalExternal = new TDERadioAction(
                                 i18n( "Start in &New Terminal" ), 0,
                                 SLOTS, TQT_SLOT( execTypeSetup() ),
                                 actionCollection(), "exec_terminal_external" );
   actExecTerminalEmbedded = new TDERadioAction(
                                 i18n( "Send to &Embedded Terminal Emulator" ), 0,
                                 SLOTS, TQT_SLOT( execTypeSetup() ),
                                 actionCollection(), "exec_terminal_embedded" );
   actExecStartAndForget->setExclusiveGroup("the_exec_type_group");
   actExecCollectSeparate->setExclusiveGroup("the_exec_type_group");
   actExecCollectTogether->setExclusiveGroup("the_exec_type_group");
   actExecTerminalExternal->setExclusiveGroup("the_exec_type_group");
   actExecTerminalEmbedded->setExclusiveGroup("the_exec_type_group");
   if( cmdExecMode < (int)( sizeof( execTypeArray ) / sizeof( TDERadioAction ** ) ) -1 )
     (*execTypeArray[ cmdExecMode ])->setChecked( true );


   actHomeTerminal = new TDEAction( i18n( "Start &Terminal" ), "terminal", 0,
                                  SLOTS, TQT_SLOT( homeTerminal() ), actionCollection(), "terminal@home" );
   actFTPDisconnect = new TDEAction( i18n( "Disconnect &from Net" ), "kr_ftp_disconnect", SHIFT + CTRL + Key_F,
                                   SLOTS, TQT_SLOT( FTPDisconnect() ), actionCollection(), "ftp disconnect" );
#if KDE_IS_VERSION(3,2,0)	/* new mountman feature is available in kde 3.2 only! */
   actMountMan = new TDEToolBarPopupAction( i18n( "&MountMan..." ), "kr_mountman", ALT + Key_Slash,
                                          SLOTS, TQT_SLOT( runMountMan() ), actionCollection(), "mountman" );
   connect( ( ( TDEToolBarPopupAction* ) actMountMan ) ->popupMenu(), TQT_SIGNAL( aboutToShow() ),
            mountMan, TQT_SLOT( quickList() ) );
#else
   actMountMan = new TDEAction( i18n( "&MountMan..." ), "kr_mountman", ALT + Key_Slash,
                              SLOTS, TQT_SLOT( runMountMan() ), actionCollection(), "mountman" );
#endif /* KDE 3.2 */

   actFind = new TDEAction( i18n( "&Search..." ), "filefind", CTRL + Key_S,
                          SLOTS, TQT_SLOT( search() ), actionCollection(), "find" );
   actLocate = new TDEAction( i18n( "&Locate..." ), "find", SHIFT+CTRL + Key_L,
                            SLOTS, TQT_SLOT( locate() ), actionCollection(), "locate" );
   actSyncDirs = new TDEAction( i18n( "Synchronize &Directories..." ), "kr_syncdirs", CTRL + Key_Y,
                              SLOTS, TQT_SLOT( slotSynchronizeDirs() ), actionCollection(), "sync dirs" );
   actSyncBrowse = new TDEAction( i18n( "S&ynchron Directory Changes" ), "kr_syncbrowse_off", ALT + Key_Y,
                              SLOTS, TQT_SLOT( slotSyncBrowse() ), actionCollection(), "sync browse" );
   actDiskUsage = new TDEAction( i18n( "D&isk Usage..." ), "kr_diskusage", ALT + Key_D,
                              SLOTS, TQT_SLOT( slotDiskUsage() ), actionCollection(), "disk usage" );
   actKonfigurator = new TDEAction( i18n( "Configure &Krusader..." ), "configure", 0,
                                  SLOTS, TQT_SLOT( startKonfigurator() ), actionCollection(), "konfigurator" );
   actBack = new TDEAction( i18n( "Back" ), "back", 0,
                          SLOTS, TQT_SLOT( back() ), actionCollection(), "back" );
   actRoot = new TDEAction( i18n( "Root" ), "top", CTRL + Key_Backspace,
                          SLOTS, TQT_SLOT( root() ), actionCollection(), "root" );
   actSavePosition = new TDEAction( i18n( "Save &Position" ), 0,
                                  TQT_TQOBJECT(krApp), TQT_SLOT( savePosition() ), actionCollection(), "save position" );   
   actAllFilter = new TDEAction( i18n( "&All Files" ), SHIFT + Key_F10,
                               SLOTS, TQT_SLOT( allFilter() ), actionCollection(), "all files" );
   //actExecFilter = new TDEAction( i18n( "&Executables" ), SHIFT + Key_F11,
   //                             SLOTS, TQT_SLOT( execFilter() ), actionCollection(), "exec files" );
   actCustomFilter = new TDEAction( i18n( "&Custom" ), SHIFT + Key_F12,
                                  SLOTS, TQT_SLOT( customFilter() ), actionCollection(), "custom files" );
   actCompare = new TDEAction( i18n( "Compare b&y Content..." ), "tdemultiple", 0,
                             SLOTS, TQT_SLOT( compareContent() ), actionCollection(), "compare" );
   actMultiRename = new TDEAction( i18n( "Multi &Rename..." ), "krename", SHIFT + Key_F9,
                                 SLOTS, TQT_SLOT( multiRename() ), actionCollection(), "multirename" );
   new TDEAction( i18n( "Right-click Menu" ), Key_Menu,
                SLOTS, TQT_SLOT( rightclickMenu() ), actionCollection(), "rightclick menu" );
   new TDEAction( i18n( "Right Bookmarks" ), ALT + Key_Right,
                SLOTS, TQT_SLOT( openRightBookmarks() ), actionCollection(), "right bookmarks" );
   new TDEAction( i18n( "Left Bookmarks" ), ALT + Key_Left,
                SLOTS, TQT_SLOT( openLeftBookmarks() ), actionCollection(), "left bookmarks" );
   new TDEAction( i18n( "Bookmarks" ), CTRL + Key_D,
                SLOTS, TQT_SLOT( openBookmarks() ), actionCollection(), "bookmarks" );
   new TDEAction( i18n( "Bookmark Current" ), CTRL + SHIFT + Key_D,
                SLOTS, TQT_SLOT( bookmarkCurrent() ), actionCollection(), "bookmark current" );
   new TDEAction( i18n( "History" ), CTRL + Key_H,
                SLOTS, TQT_SLOT( openHistory() ), actionCollection(), "history" );
   new TDEAction( i18n( "Sync Panels" ), ALT + Key_O,
                SLOTS, TQT_SLOT( syncPanels() ), actionCollection(), "sync panels");
   new TDEAction( i18n( "Left History" ), ALT + CTRL + Key_Left,
                SLOTS, TQT_SLOT( openLeftHistory() ), actionCollection(), "left history" );
   new TDEAction( i18n( "Right History" ), ALT + CTRL + Key_Right,
                SLOTS, TQT_SLOT( openRightHistory() ), actionCollection(), "right history" );
   new TDEAction( i18n( "Media" ), CTRL + Key_M,
                SLOTS, TQT_SLOT( openMedia() ), actionCollection(), "media" );
   new TDEAction( i18n( "Left Media" ), CTRL + SHIFT + Key_Left,
                SLOTS, TQT_SLOT( openLeftMedia() ), actionCollection(), "left media" );
   new TDEAction( i18n( "Right Media" ), CTRL + SHIFT + Key_Right,
                SLOTS, TQT_SLOT( openRightMedia() ), actionCollection(), "right media" );
   new TDEAction( i18n( "New Symlink..." ), CTRL + ALT + Key_S,
                SLOTS, TQT_SLOT( newSymlink() ), actionCollection(), "new symlink");
   new TDEToggleAction( i18n( "Toggle Popup Panel" ), ALT + Key_Down, SLOTS,
                            TQT_SLOT( togglePopupPanel() ), actionCollection(), "toggle popup panel" );
   actVerticalMode = new TDEToggleAction( i18n( "Vertical Mode" ), "view_top_bottom", ALT + CTRL + Key_R, TQT_TQOBJECT(MAIN_VIEW), 
                                        TQT_SLOT( toggleVerticalMode() ), actionCollection(), "toggle vertical mode" );
   actNewTab = new TDEAction( i18n( "New Tab" ), "tab_new", ALT + CTRL + Key_N, SLOTS,
                            TQT_SLOT( newTab() ), actionCollection(), "new tab" );
   actDupTab = new TDEAction( i18n( "Duplicate Current Tab" ), "tab_duplicate", ALT + CTRL + SHIFT + Key_N, SLOTS,
                            TQT_SLOT( duplicateTab() ), actionCollection(), "duplicate tab" );
   actCloseTab = new TDEAction( i18n( "Close Current Tab" ), "tab_remove", CTRL + Key_W, SLOTS,
                              TQT_SLOT( closeTab() ), actionCollection(), "close tab" );
   actNextTab  = new TDEAction( i18n( "Next Tab" ), SHIFT + Key_Right, SLOTS,
                              TQT_SLOT( nextTab() ), actionCollection(), "next tab" );
   actPreviousTab  = new TDEAction( i18n( "Previous Tab" ), SHIFT + Key_Left, SLOTS,
                                  TQT_SLOT( previousTab() ), actionCollection(), "previous tab" );
/*
   actUserMenu = new TDEAction( i18n( "User Menu" ), ALT + Key_QuoteLeft, SLOTS,
                              TQT_SLOT( userMenu() ), actionCollection(), "user menu" );
*/
   actManageUseractions = new TDEAction( i18n( "Manage User Actions..." ), 0, SLOTS,
                              TQT_SLOT( manageUseractions() ), actionCollection(), "manage useractions" );
   new KrRemoteEncodingMenu(i18n("Select Remote Charset"), "charset", actionCollection(), "changeremoteencoding");

   // setup the Fn keys
   actF2 = new TDEAction( i18n( "Start Terminal Here" ), "terminal", Key_F2,
                        SLOTS, TQT_SLOT( terminal() ) , actionCollection(), "F2_Terminal" );
   actF3 = new TDEAction( i18n( "View File" ), Key_F3,
                        SLOTS, TQT_SLOT( view() ) , actionCollection(), "F3_View" );
   actF4 = new TDEAction( i18n( "Edit File" ), Key_F4,
                        SLOTS, TQT_SLOT( edit() ) , actionCollection(), "F4_Edit" );
   actF5 = new TDEAction( i18n( "Copy..." ), Key_F5,
                        SLOTS, TQT_SLOT( copyFiles() ) , actionCollection(), "F5_Copy" );
   actF6 = new TDEAction( i18n( "Move..." ), Key_F6,
                        SLOTS, TQT_SLOT( moveFiles() ) , actionCollection(), "F6_Move" );
   actF7 = new TDEAction( i18n( "New Directory..." ), "folder_new", Key_F7,
                        SLOTS, TQT_SLOT( mkdir() ) , actionCollection(), "F7_Mkdir" );
   actF8 = new TDEAction( i18n( "Delete" ), "editdelete", Key_F8,
                        SLOTS, TQT_SLOT( deleteFiles() ) , actionCollection(), "F8_Delete" );
   actF9 = new TDEAction( i18n( "Rename" ), Key_F9,
                        SLOTS, TQT_SLOT( rename() ) , actionCollection(), "F9_Rename" );
   actF10 = new TDEAction( i18n( "Quit" ), Key_F10,
                         TQT_TQOBJECT(this), TQT_SLOT( slotClose() ) , actionCollection(), "F10_Quit" );
   actPopularUrls = new TDEAction( i18n("Popular URLs..."), CTRL+Key_Z,
                                 popularUrls, TQT_SLOT( showDialog() ), actionCollection(), "Popular_Urls");
   actLocationBar = new TDEAction( i18n("Go to Location Bar"), CTRL+Key_L,
                                 SLOTS, TQT_SLOT( slotLocationBar() ), actionCollection(), "location_bar");
   actJumpBack = new TDEAction( i18n("Jump Back"), "kr_jumpback", CTRL+Key_J,
                              SLOTS, TQT_SLOT( slotJumpBack() ), actionCollection(), "jump_back");
   actSetJumpBack = new TDEAction( i18n("Set Jump Back Point"), "kr_setjumpback", CTRL+SHIFT+Key_J,
                                 SLOTS, TQT_SLOT( slotSetJumpBack() ), actionCollection(), "set_jump_back");
   actSwitchFullScreenTE = new TDEAction( i18n( "Toggle Fullwidget Terminal Emulator" ), 0, CTRL + Key_F,
                                        TQT_TQOBJECT(MAIN_VIEW), TQT_SLOT( switchFullScreenTE() ), actionCollection(), "switch_fullscreen_te" );

   // and at last we can set the tool-tips
   actSelect->setToolTip( i18n( "Select files using a filter" ) );
   actSelectAll->setToolTip( i18n("Select all files in the current directory" ) );
   actUnselectAll->setToolTip( i18n( "Unselect all selected files" ) );
   actKonfigurator->setToolTip( i18n( "Setup Krusader the way you like it" ) );
   actBack->setToolTip( i18n( "Back to the place you came from" ) );
   actRoot->setToolTip( i18n( "ROOT (/)" ) );
   actFind->setToolTip( i18n( "Search for files" ) );

   // setup all UserActions
   userAction = new UserAction();

   #ifdef __KJSEMBED__
   actShowJSConsole = new TDEAction( i18n( "JavaScript Console..." ), ALT + CTRL + Key_J, SLOTS, TQT_SLOT( jsConsole() ), actionCollection(), "JS_Console" );
   #endif
}

///////////////////////////////////////////////////////////////////////////
//////////////////// implementation of slots //////////////////////////////
///////////////////////////////////////////////////////////////////////////

void Krusader::savePosition() {
   config->setGroup( "Private" );
   config->writeEntry( "Maximized", isMaximized() );
   if (isMaximized())
      saveWindowSize(config);
   else {
      config->writeEntry( "Start Position", isMaximized() ? oldPos : pos() );
      config->writeEntry( "Start Size", isMaximized() ? oldSize : size() );
   }
   config->writeEntry( "Panel Size", mainView->vert_splitter->sizes() [ 0 ] );
   config->writeEntry( "Terminal Size", mainView->vert_splitter->sizes() [ 1 ] );
   TQValueList<int> lst = mainView->horiz_splitter->sizes();
   config->writeEntry( "Splitter Sizes", lst );
   mainView->left->popup->saveSizes();
   mainView->right->popup->saveSizes();
   if( !MAIN_VIEW->getTerminalEmulatorSplitterSizes().isEmpty() )
     config->writeEntry( "Terminal Emulator Splitter Sizes", MAIN_VIEW->getTerminalEmulatorSplitterSizes() );
   
   // save view settings ---> fix when we have tabbed-browsing
   mainView->left->view->saveSettings();
   mainView->right->view->saveSettings();
   
   config->setGroup( "Startup" );
   config->writeEntry( "Vertical Mode", actVerticalMode->isChecked());
   config->sync();
}

void Krusader::saveSettings() {
   toolBar() ->saveSettings( krConfig, "Private" );
   toolBar("actionsToolBar")->saveSettings( krConfig, "Actions Toolbar" );
   config->setGroup( "Startup" );   
   config->writeEntry( "Left Active Tab", mainView->leftMng->activeTab() );
   config->writeEntry( "Right Active Tab", mainView->rightMng->activeTab() );
   config->writeEntry( "Left Side Is Active", MAIN_VIEW->activePanel->isLeft() );
   mainView->leftMng->saveSettings( krConfig, "Left Tab Bar" );
   mainView->rightMng->saveSettings( krConfig, "Right Tab Bar" );
   
   bool rememberpos = config->readBoolEntry( "Remember Position", _RememberPos );
   bool uisavesettings = config->readBoolEntry( "UI Save Settings", _UiSave );

   // save the popup panel's page of the CURRENT tab
   config->writeEntry( "Left Panel Popup", mainView->left->popup->currentPage() );
   config->writeEntry( "Right Panel Popup", mainView->right->popup->currentPage() );

   // save size and position
   if ( rememberpos || uisavesettings ) {
      savePosition();
   }

   // save the gui
   if ( uisavesettings ) {
      config->setGroup( "Startup" );
      config->writeEntry( "Show status bar", actShowStatusBar->isChecked() );
      config->writeEntry( "Show tool bar", actShowToolBar->isChecked() );
      config->writeEntry( "Show FN Keys", actToggleFnkeys->isChecked() );
      config->writeEntry( "Show Cmd Line", actToggleCmdline->isChecked() );
      config->writeEntry( "Show Terminal Emulator", actToggleTerminal->isChecked() );
      config->writeEntry( "Vertical Mode", actVerticalMode->isChecked());
      config->writeEntry( "Start To Tray", isHidden());
   }

   // save popular links
   popularUrls->save();

   config->sync();
}

void Krusader::refreshView() {
   delete mainView;
   mainView = new KrusaderView( this );
   setCentralWidget( mainView );
   config->setGroup( "Private" );
   resize( krConfig->readSizeEntry( "Start Size", _StartSize ) );
   move( krConfig->readPointEntry( "Start Position", _StartPosition ) );
   mainView->show();
   show();
}

void Krusader::configChanged() {
   config->setGroup( "Look&Feel" );
   bool minimizeToTray = config->readBoolEntry( "Minimize To Tray", _MinimizeToTray );
   bool singleInstanceMode = config->readBoolEntry( "Single Instance Mode", _SingleInstanceMode );
   
   if( !isHidden() ) {
     if( singleInstanceMode && minimizeToTray )
       sysTray->show();
     else
       sysTray->hide();
   } else {
     if( minimizeToTray )
       sysTray->show();
   }
}

void Krusader::slotClose() {
   directExit = true;
   close();
}

bool Krusader::queryClose() {
   if( isStarting || isExiting )
     return false;
   
   if( kapp->sessionSaving() ) // KDE is logging out, accept the close
   { 
     saveSettings();

     kapp->dcopClient()->registerAs( TDEApplication::kApplication()->name(), true );

     kapp->deref(); // FIX: krusader exits at closing the viewer when minimized to tray
     kapp->deref(); // and close the application
     return isExiting = true;              // this will also kill the pending jobs
   }
   
   krConfig->setGroup( "Look&Feel" );
   if( !directExit && krConfig->readBoolEntry( "Single Instance Mode", _SingleInstanceMode ) && 
                      krConfig->readBoolEntry( "Minimize To Tray", _MinimizeToTray ) ) {
     hide();
     return false;
   }

   // the shutdown process can be cancelled. That's why
   // the directExit variable is set to normal here.
   directExit = false;

   bool quit = true;
   
   if ( krConfig->readBoolEntry( "Warn On Exit", _WarnOnExit ) ) {
      switch ( KMessageBox::warningYesNo( this,
                                          i18n( "Are you sure you want to quit?" ) ) ) {
            case KMessageBox::Yes :
            quit = true;
            break;
            case KMessageBox::No :
            quit = false;
            break;
            default:
            quit = false;
      }
   }
   if ( quit ) {
      /* First try to close the child windows, because it's the safer
         way to avoid crashes, then close the main window.
         If closing a child is not successful, then we cannot let the
         main window close. */

      for(;;) {
        TQWidgetList * list = TQApplication::topLevelWidgets();
        TQWidget *activeModal = TQT_TQWIDGET(TQApplication::activeModalWidget());
        TQWidget *w = list->first();

        if( activeModal && activeModal != this && activeModal != menuBar() && activeModal != sysTray && list->contains( activeModal ) && !activeModal->isHidden() )
          w = activeModal;
        else {
          while(w && (w==this || w==sysTray || w->isHidden() || w==menuBar()) )
            w = list->next();
        }
        delete list;

        if(!w) break;
        bool hid = false;

        if( w->inherits( "KDialogBase" ) ) { // KDE is funny and rejects the close event for
          w->hide();                         // playing a fancy animation with the CANCEL button.
          hid = true;                        // if we hide the widget, KDialogBase accepts the close event
        }

        if( !w->close() ) {
          if( hid )
            w->show();

          if( w->inherits( TQDIALOG_OBJECT_NAME_STRING ) )
            fprintf( stderr, "Failed to close: %s\n", w->className() );
  
          return false;
        }
      }
   
      saveSettings();

      isExiting = true;
      hide();        // hide

      // Changes the name of the application. Single instance mode requires unique appid.
      // As Krusader is exiting, we release that unique appid, so new Krusader instances
      // can be started.
      kapp->dcopClient()->registerAs( TDEApplication::kApplication()->name(), true );

      kapp->deref(); // FIX: krusader exits at closing the viewer when minimized to tray
      kapp->deref(); // and close the application
      return false;  // don't let the main widget close. It stops the pendig copies!
   } else
      return false;
}

// the please wait dialog functions
void Krusader::startWaiting( TQString msg, int count , bool cancel ) {
   plzWait->startWaiting( msg , count, cancel );
}

bool Krusader::wasWaitingCancelled() const { 
	return plzWait->wasCancelled(); 
}

void Krusader::incProgress( TDEProcess *, char *buffer, int buflen ) {
   int howMuch = 0;
   for ( int i = 0 ; i < buflen; ++i )
      if ( buffer[ i ] == '\n' )
         ++howMuch;

   plzWait->incProgress( howMuch );
}

void Krusader::stopWait() {
   plzWait->stopWait();
}

void Krusader::updateGUI( bool enforce ) {
   // now, check if we need to create a konsole_part
   config->setGroup( "Startup" );

   // call the XML GUI function to draw the UI
   createGUI( mainView->konsole_part );
   
   // this needs to be called AFTER createGUI() !!!
   userActionMenu = (TDEPopupMenu*) guiFactory()->container( "useractionmenu", this );
   if ( userActionMenu )
      userAction->populateMenu( userActionMenu );
   
   toolBar() ->applySettings( krConfig, "Private" );
	
	toolBar("actionsToolBar") ->applySettings( krConfig, "Actions Toolbar" );
	static_cast<TDEToggleAction*>(actionCollection()->action("toggle actions toolbar"))->
		setChecked(toolBar("actionsToolBar")->isVisible());
	
   if ( enforce ) {
      // now, hide what need to be hidden
      if ( !krConfig->readBoolEntry( "Show tool bar", _ShowToolBar ) ) {
         toolBar() ->hide();
         actShowToolBar->setChecked( false );
      } else {
         toolBar() ->show();
         actShowToolBar->setChecked( true );
      }
      if ( !krConfig->readBoolEntry( "Show status bar", _ShowStatusBar ) ) {
         statusBar() ->hide();
         actShowStatusBar->setChecked( false );
      } else {
         statusBar() ->show();
         actShowStatusBar->setChecked( true );
      }
      if ( !krConfig->readBoolEntry( "Show Cmd Line", _ShowCmdline ) ) {
         mainView->cmdLine->hide();
         actToggleCmdline->setChecked( false );
      } else {
         mainView->cmdLine->show();
         actToggleCmdline->setChecked( true );
      }

      // update the Fn bar to the shortcuts selected by the user
      mainView->fnKeys->updateShortcuts();
      if ( !krConfig->readBoolEntry( "Show FN Keys", _ShowFNkeys ) ) {
         mainView->fnKeys->hide();
         actToggleFnkeys->setChecked( false );
      } else {
         mainView->fnKeys->show();
         actToggleFnkeys->setChecked( true );
      }
      // set vertical mode
      if (krConfig->readBoolEntry( "Vertical Mode", false)) {
      	actVerticalMode->setChecked(true);
			mainView->toggleVerticalMode();
      }
      if ( config->readBoolEntry( "Show Terminal Emulator", _ShowTerminalEmulator ) ) {
        mainView->slotTerminalEmulator( true ); // create konsole_part
        TDEConfigGroup grp(krConfig, "Private" );
        TQValueList<int> lst;
        lst.append( grp.readNumEntry( "Panel Size", _PanelSize ) );
        lst.append( grp.readNumEntry( "Terminal Size", _TerminalSize ) );
        mainView->vert_splitter->setSizes( lst );
        config->setGroup( "Startup" );
      } else if ( actExecTerminalEmbedded->isChecked() ) {
        //create (but not show) terminal emulator,
        //if command-line commands are to be run there
        mainView->createTE();
      }
   }
	// popular urls
	popularUrls->load();

}

// return a list in the format of TOOLS,PATH. for example
// DIFF,kdiff,TERMINAL,konsole,...
//
// currently supported tools: DIFF, MAIL, RENAME
//
// to use it: TQStringList lst = supportedTools();
//            int i = lst.findIndex("DIFF");
//            if (i!=-1) pathToDiff=lst[i+1];
TQStringList Krusader::supportedTools() {
   TQStringList tools;

   // first, a diff program: kdiff
   if ( KrServices::cmdExist( "kdiff3" ) ) {
      tools.append( "DIFF" );
      tools.append( KrServices::fullPathName( "kdiff3", "diff utility" ) );
   } else if ( KrServices::cmdExist( "kompare" ) ) {
      tools.append( "DIFF" );
      tools.append( KrServices::fullPathName( "kompare", "diff utility" ) );
   } else if ( KrServices::cmdExist( "xxdiff" ) ) {
      tools.append( "DIFF" );
      tools.append( KrServices::fullPathName( "xxdiff", "diff utility" ) );
   }
   // a mailer: kmail
   if ( KrServices::cmdExist( "kmail" ) ) {
      tools.append( "MAIL" );
      tools.append( KrServices::fullPathName( "kmail" ) );
   }
   // rename tool: krename
   if ( KrServices::cmdExist( "krename" ) ) {
      tools.append( "RENAME" );
      tools.append( KrServices::fullPathName( "krename" ) );
   }
  // checksum utility
  if (KrServices::cmdExist("md5deep")) {
      tools.append("MD5");
      tools.append(KrServices::fullPathName("md5deep", "checksum utility"));
  } else if (KrServices::cmdExist("md5sum")) {
      tools.append("MD5");
      tools.append(KrServices::fullPathName("md5sum", "checksum utility"));
  } else if (KrServices::cmdExist("sha1deep")) {
      tools.append("MD5");
      tools.append(KrServices::fullPathName("sha1deep", "checksum utility"));
  } else if (KrServices::cmdExist("sha256deep")) {
      tools.append("MD5");
      tools.append(KrServices::fullPathName("sha256deep", "checksum utility"));
  } else if (KrServices::cmdExist("tigerdeep")) {
      tools.append("MD5");
      tools.append(KrServices::fullPathName("tigerdeep", "checksum utility"));
  } else if (KrServices::cmdExist("whirlpooldeep")) {
      tools.append("MD5");
      tools.append(KrServices::fullPathName("whirlpooldeep", "checksum utility"));
  } else if (KrServices::cmdExist("cfv")) {
      tools.append("MD5");
      tools.append(KrServices::fullPathName("cfv", "checksum utility"));
  }

   return tools;
}

TQString Krusader::getTempDir() {
   // try to make krusader temp dir
   krConfig->setGroup( "General" );
   TQString tmpDir = krConfig->readEntry( "Temp Directory", _TempDirectory );

   if ( ! TQDir( tmpDir ).exists() ) {
      for ( int i = 1 ; i != -1 ; i = tmpDir.find( '/', i + 1 ) )
         TQDir().mkdir( tmpDir.left( i ) );
      TQDir().mkdir( tmpDir );
      chmod( tmpDir.local8Bit(), 0777 );
   }

   // add a secure sub dir under the user UID
   TQString uid;
   uid.sprintf( "%d", getuid() );
   TQDir( tmpDir ).mkdir( uid );
   tmpDir = tmpDir + "/" + uid + "/";
   chmod( tmpDir.local8Bit(), S_IRUSR | S_IWUSR | S_IXUSR );
   // add a random sub dir to use
   while ( TQDir().exists( tmpDir ) )
      tmpDir = tmpDir + kapp->randomString( 8 );
   TQDir().mkdir( tmpDir );

   if ( !TQDir( tmpDir ).isReadable() ) {
      KMessageBox::error( krApp, "Could not create a temporary directory. Handling of Archives will not be possible until this is fixed." );
      return TQString();
   }
   return tmpDir;
}

TQString Krusader::getTempFile() {
   // try to make krusader temp dir
   krConfig->setGroup( "General" );
   TQString tmpDir = krConfig->readEntry( "Temp Directory", _TempDirectory );

   if ( ! TQDir( tmpDir ).exists() ) {
      for ( int i = 1 ; i != -1 ; i = tmpDir.find( '/', i + 1 ) )
         TQDir().mkdir( tmpDir.left( i ) );
      TQDir().mkdir( tmpDir );
      chmod( tmpDir.local8Bit(), 0777 );
   }

   // add a secure sub dir under the user UID
   TQString uid;
   uid.sprintf( "%d", getuid() );
   TQDir( tmpDir ).mkdir( uid );
   tmpDir = tmpDir + "/" + uid + "/";
   chmod( tmpDir.local8Bit(), S_IRUSR | S_IWUSR | S_IXUSR );

   while ( TQDir().exists( tmpDir ) )
      tmpDir = tmpDir + kapp->randomString( 8 );
   return tmpDir;
}

const char* Krusader::privIcon() {
   if ( geteuid() )
      return "krusader_user";
   else
      return "krusader_root";
}

bool Krusader::process(const TQCString &fun, const TQByteArray &/* data */, TQCString &/* replyType */, TQByteArray &/* replyData */) {
   if (fun == "moveToTop()") {
      moveToTop();
      return true;
   } else {
      fprintf( stderr, "Processing DCOP call failed. Function unknown!\n" );
      return false;
   }
}

void Krusader::moveToTop() {
   if( isHidden() )
     show();

   KWin::forceActiveWindow( winId() );
}

#include "krusader.moc"
