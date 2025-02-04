
/***************************************************************************
                                krusader.h
                           -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
 The main application ! what's more to say ?
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

#ifndef KRUSADER_H
#define KRUSADER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// TDE includes
#include <tdeapplication.h>
#include <tdeparts/mainwindow.h>
#include <kstdaction.h>
#include <tdeaction.h>
#include <tdeconfig.h>
#include <tdeaccel.h>
#include <tqstringlist.h>
#include <tqtextstream.h>
#include <kdebug.h>
#include <dcopobject.h>

#ifdef __KJSEMBED__
class KrJS;
#endif

class KrusaderStatus;
class KRPleaseWaitHandler;
class KrusaderView;
class TDEProcess;
class KRslots;
class TDEIconLoader;
class KSystemTray;
class UserMenu;
class UserAction;
class Expander;
class KMountMan;
class KrBookmarkHandler;
class PopularUrls;
class QueueManager;

//static TQTextOStream krOut(stdout);
#define krOut kdDebug(50010)

class Krusader : public KParts::MainWindow, public DCOPObject {
    TQ_OBJECT
  
  public:
    Krusader();
    virtual ~Krusader();
    void refreshView();				 // re-create the main view
    void configChanged();
    /**
     * This returns a defferent icon if krusader runs with root-privileges
     * @return a character string with the specitif icon-name
     */
    static const char* privIcon();
    static TQStringList supportedTools(); // find supported tools

    virtual bool process (const TQCString &fun, const TQByteArray &data, TQCString &replyType, TQByteArray &replyData);
    void moveToTop();

  public slots:
    // increase the internal progress bar
    void incProgress( TDEProcess *, char *buffer, int buflen );
    void statusBarUpdate( TQString& mess );
    // in use by Krusader only
    void saveSettings();
    void savePosition();
    void updateGUI( bool enforce = false );
    void slotClose();
    void setDirectExit() { directExit = true; }

  protected:
    bool queryExit() {
      config->sync();
      return true;
    }
    bool queryClose();
    void setupActions();
    void setupAccels();
    bool versionControl();  // handle version differences in krusaderrc
    void showEvent ( TQShowEvent * );
    void hideEvent ( TQHideEvent * );
    void moveEvent ( TQMoveEvent * );
    void resizeEvent ( TQResizeEvent * );

  public:
    static Krusader *App;       // a kApp style pointer
    KMountMan *mountMan;  // krusader's Mount Manager
    KrusaderView *mainView;  // The GUI
    TDEConfig *config;    // allow everyone to access the config
    TDEIconLoader *iconLoader; // the app's icon loader
	 PopularUrls *popularUrls; // holds a sorted list of the most popular urls visited
	 QueueManager *queueManager;
    // Actions
    static TDEAction *actProperties, *actPack, *actUnpack, *actTest, *actCompare, *actCmdlinePopup;
    static TDEAction *actCalculate, *actSelect, *actUnselect, *actSelectAll, *actLocate, *actSwitchFullScreenTE;
    static TDEAction *actUnselectAll, *actInvert, *actSync, *actDiskUsage, *actSavePosition, *actCompDirs;
    static TDEAction *actHomeTerminal, *actFTPConnect, *actFTPNewConnect, *actFTPDisconnect, *actProfiles;
    static TDEAction *actExecFilter, *actCustomFilter, *actMountMan, *actNewTool, *actSwapPanels, *actSwapSides;
    static TDEAction *actKonfigurator, *actToolsSetup, *actBack, *actRoot, *actFind, *actDirUp;
    static TDEAction *actSelectColorMask, *actMultiRename, *actAllFilter, *actOpenLeftBm, *actOpenRightBm;
    static TDEAction *actNewTab, *actDupTab, *actCloseTab, *actPreviousTab, *actNextTab, *actSplit; 
    static TDEAction *actCombine, *actUserMenu, *actManageUseractions, *actSyncDirs, *actSyncBrowse;
    static TDEAction *actF2, *actF3, *actF4, *actF5, *actF6, *actF7, *actF8, *actF9, *actF10;
    static TDEAction *actPopularUrls, *actLocationBar, *actJumpBack, *actSetJumpBack, *actCreateChecksum, *actMatchChecksum;
    static TDEAction *actDetailedView, *actBriefView, *actCopy, *actPaste;
    static TDEToggleAction *actToggleTerminal, *actVerticalMode;
    static TDERadioAction *actSelectNewerAndSingle, *actSelectNewer, *actSelectSingle, 
                        *actSelectDifferentAndSingle, *actSelectDifferent;
    /** actions for setting the execution mode of commands from commanddline */
    static TDERadioAction *actExecStartAndForget,
                        *actExecCollectSeparate,*actExecCollectTogether,
                        *actExecTerminalExternal,*actExecTerminalEmbedded;
    TDEToggleAction *actToggleFnkeys, *actToggleCmdline, *actShowToolBar,
                  *actShowStatusBar, *actToggleHidden, *actCompareDirs;

    static TDERadioAction **compareArray[];
    /** actions for setting the execution mode of commands from commanddline */
    static TDERadioAction **execTypeArray[];

    // return a path to a temp dir or file we can use.
    TQString getTempDir();
    TQString getTempFile();

    // the internal progress bar variales + functions
    KRPleaseWaitHandler* plzWait;
    void startWaiting( TQString msg = "Please Wait", int count = 0 , bool cancel = false );
    void stopWait();
	 bool wasWaitingCancelled() const;

    KrusaderStatus *status;
    KRslots *slot;
    TDEAccel *accels; // global accelerators
    
    static TDEPopupMenu *userActionMenu;
    static UserMenu *userMenu;
    static UserAction *userAction;
	 static KrBookmarkHandler *bookman;

    #ifdef __KJSEMBED__
    static KrJS *js;
    static TDEAction *actShowJSConsole;
    #endif

  signals:
    void changeMessage( TQString );

  private:
    KSystemTray *sysTray;
    TQPoint       oldPos;
    TQSize        oldSize;
    bool         isStarting;
    bool         isExiting;
    bool         directExit;
};

// main modules
#define krApp        Krusader::App
#define krConfig     Krusader::App->config
#define krMtMan      (*(Krusader::App->mountMan))
#define krBookMan    Krusader::App->bookman
#define SLOTS        Krusader::App->slot
#define krLoader     Krusader::App->iconLoader

#define MAIN_VIEW    (krApp->mainView)
#define ACTIVE_MNG   (MAIN_VIEW->activeManager())
#define ACTIVE_PANEL (MAIN_VIEW->activePanel)
#define ACTIVE_FUNC  (ACTIVE_PANEL->func)
#define OTHER_MNG		(MAIN_VIEW->inactiveManager())
#define OTHER_PANEL	(ACTIVE_PANEL->otherPanel)
#define OTHER_FUNC	(OTHER_PANEL->func)
#define LEFT_PANEL	(MAIN_VIEW->left)
#define LEFT_FUNC		(LEFT_PANEL->func)
#define LEFT_MNG		(MAIN_VIEW->leftMng)
#define RIGHT_PANEL  (MAIN_VIEW->right)
#define RIGHT_FUNC	(RIGHT_PANEL->func)
#define RIGHT_MNG		(MAIN_VIEW->rightMng)

// krusader's actions - things krusader can do!
#define krProperties      Krusader::App->actProperties     // file properties
#define krPack            Krusader::App->actPack           // pack files into an archive
#define krUnpack          Krusader::App->actUnpack         // unpack archive
#define krTest            Krusader::App->actTest           // test archive
#define krCompare         Krusader::App->actCompare        // compare 2 files by content
#define krCalculate       Krusader::App->actCalculate      // calculate occupied space
#define krCreateCS	  Krusader::App->actCreateChecksum
#define krMatchCS	  Krusader::App->actMatchChecksum
#define krSelect          Krusader::App->actSelect         // select a group by filter
#define krSelectAll       Krusader::App->actSelectAll      // select all files
#define krUnselect        Krusader::App->actUnselect       // unselect by filter
#define krUnselectAll     Krusader::App->actUnselectAll    // remove all selections
#define krInvert          Krusader::App->actInvert         // invert the selection
#define krSyncDirs        Krusader::App->actSync           // syncronize directories
#define krHomeTerm        Krusader::App->actHomeTerminal   // open terminal@home dir
#define krFTPConnect      Krusader::App->actFTPConnect     // connect to an ftp
#define krFTPNew          Krusader::App->actFTPNewConnect  // create a new connection
#define krFTPDiss         Krusader::App->actFTPDisconnect  // disconnect an FTP session
#define krAllFiles        Krusader::App->actAllFilter      // show all files in list
#define krExecFiles       Krusader::App->actExecFilter     // show only executables
#define krCustomFiles     Krusader::App->actCustomFilter   // show a custom set of files
#define krMountMan        Krusader::App->actMountMan       // run Mount-manager
#define krNewTool         Krusader::App->actNewTool        // Add a new tool to menu
#define krKonfigurator    Krusader::App->actKonfigurator
#define krToolsSetup      Krusader::App->actToolsSetup     // setup the tools menu
#define krBack            Krusader::App->actBack
#define krRoot            Krusader::App->actRoot
#define krFind            Krusader::App->actFind           // find files
#define krMultiRename     Krusader::App->actMultiRename
#define krToggleTerminal  Krusader::App->actToggleTerminal
#define krToggleSortByExt Krusader::App->actToggleSortByExt// Sort by extention
#define krOpenLeftBm      Krusader::App->actOpenLeftBm     // open left bookmarks
#define krOpenRightBm     Krusader::App->actOpenRightBm    // open left bookmarks
#define krDirUp           Krusader::App->actDirUp
#define krCmdlinePopup    Krusader::App->actCmdlinePopup
#define krNewTab          Krusader::App->actNewTab
#define krDupTab          Krusader::App->actDupTab
#define krCloseTab        Krusader::App->actCloseTab
#define krNextTab         Krusader::App->actNextTab
#define krPreviousTab     Krusader::App->actPreviousTab
#define krSplit           Krusader::App->actSplit
#define krCombine         Krusader::App->actCombine
#define krUserMenu        Krusader::App->actUserMenu
#define krUserActionMenu      Krusader::App->userActionMenu
#define krUserAction      Krusader::App->userAction
#define krF2				  Krusader::App->actF2
#define krF3				  Krusader::App->actF3
#define krF4				  Krusader::App->actF4
#define krF5				  Krusader::App->actF5
#define krF6				  Krusader::App->actF6
#define krF7				  Krusader::App->actF7
#define krF8				  Krusader::App->actF8
#define krF9				  Krusader::App->actF9
#define krF10				  Krusader::App->actF10
#define krPopularUrls	  Krusader::App->actPopularUrls
#define krLocationBar	  Krusader::App->actLocationBar
#define krJumpBack	  Krusader::App->actJumpBack
#define krSetJumpBack	  Krusader::App->actSetJumpBack

#ifdef __KJSEMBED__
#define krJS			Krusader::App->js
#define krJSConsole		Krusader::App->actShowJSConsole
#endif

#endif
