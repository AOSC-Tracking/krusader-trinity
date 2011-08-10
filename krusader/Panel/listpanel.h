/***************************************************************************
                               listpanel.h
                            -------------------
   begin                : Thu May 4 2000
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

                                                    H e a d e r    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#ifndef LISTPANEL_H
#define LISTPANEL_H

#include <kpropsdlg.h>
#include <kfileitem.h>
#include <kurl.h>
#include <tqwidget.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqstring.h>
#include <tqpixmap.h>
#include <tqtoolbutton.h>
#include <tqpopupmenu.h>
#include <tqdir.h>
#include <tqpixmapcache.h>
#include <tqiconset.h>
#include <tqptrstack.h>
#include <tqtextbrowser.h>
#include <keditcl.h>
#include <klineedit.h>
#include <tqguardedptr.h>
#include "krview.h"
#include "../Dialogs/krsqueezedtextlabel.h"

// use our version of it, until kde fixes theirs
#include "../MountMan/kdiskfreesp.h"

#define PROP_SYNC_BUTTON_ON               1

class vfs;
class vfile;
class KRdirWatch;
class KrView;
class KURLRequester;
class BookmarksButton;
class KrQuickSearch;
class DirHistoryButton;
class DirHistoryQueue;
class MediaButton;
class PanelPopup;
class SyncBrowseButton;
class KrBookmarkButton;
class KPushButton;
class ListPanelFunc;
class TQHeader;

class ListPanel : public TQWidget {
   friend class ListPanelFunc;
   Q_OBJECT
  TQ_OBJECT
public:
	#define ITEM2VFILE(PANEL_PTR, KRVIEWITEM)		PANEL_PTR->func->files()->vfs_search(KRVIEWITEM->name())
	#define NAME2VFILE(PANEL_PTR, STRING_NAME)	PANEL_PTR->func->files()->vfs_search(STRING_NAME)
   // constructor create the panel, but DOESN'T fill it with data, use start()
   ListPanel( TQString panelType, TQWidget *parent, bool &left, const char *name = 0 );
   ~ListPanel();
   void start( KURL url = KURL(), bool immediate = false );
   
   const TQString & getType() { return panelType; }
   void changeType( const TQString & );
   
   KURL virtualPath() const;
	TQString realPath() const;
   TQString getCurrentName();
   void getSelectedNames( TQStringList* fileNames ) {
      view->getSelectedItems( fileNames );
   }
   void setPanelToolbar();
   bool isLeft() {return _left;}
   void jumpBack();
   void setJumpBack( KURL url );

   int  getProperties();
   void setProperties( int );

public slots:
   void gotStats( const TQString &mountPoint, unsigned long kBSize, unsigned long kBUsed, unsigned long kBAvail); // displays filesystem status
   void popRightClickMenu( const TQPoint& );
   void popEmptyRightClickMenu( const TQPoint & );
   void select( KRQuery query, bool select);
   void select( bool, bool );      // see doc in ListPanel
   void invertSelection();       // see doc in ListPanel
   void compareDirs();
   void slotFocusOnMe(); // give this VFS the focus (the path bar)
   void slotUpdate();			                  // when the vfs finish to update...
   void slotUpdateTotals();
   void slotStartUpdate();                   // internal
   void slotGetStats( const KURL& url );          // get the disk-free stats
   void setFilter( KrViewProperties::FilterSpec f );
   void slotFocusAndCDRoot();
   void slotFocusAndCDHome();
   void slotFocusAndCDup();
   void slotFocusAndCDOther();
	void togglePanelPopup();
	// for signals from vfs' dirwatch
	void slotItemAdded(vfile *vf);
	void slotItemDeleted(const TQString& name);
	void slotItemUpdated(vfile *vf);
	void slotCleared();        
	void panelActive(); // called when the panel becomes active
	void panelInactive(); // called when panel becomes inactive
	

   ///////////////////////// service functions - called internally ////////////////////////
   inline void setOther( ListPanel *panel ) {
      otherPanel = panel;
   }
   void prepareToDelete();                   // internal use only

protected:
   virtual void keyPressEvent( TQKeyEvent *e );
   virtual void showEvent( TQShowEvent * );
   virtual void hideEvent( TQHideEvent * );
   virtual bool eventFilter ( TQObject * watched, TQEvent * e );
   
   void createView();

protected slots:
   void handleDropOnView(TQDropEvent *, TQWidget *destWidget=0); // handles drops on the view only
   void handleDropOnTotals( TQDropEvent * );                   // handles drops on the totals line
   void handleDropOntqStatus( TQDropEvent * );                   // handles drops on the status line
   void startDragging( TQStringList, TQPixmap );
	// those handle the in-panel refresh notifications
	void slotJobStarted(KIO::Job* job);
	void inlineRefreshInfoMessage( KIO::Job* job, const TQString &msg );
	void inlineRefreshListResult(KIO::Job* job);
	void inlineRefreshPercent( KIO::Job*, unsigned long );
	void inlineRefreshCancel();

signals:
   void signaltqStatus( TQString msg );       // emmited when we need to update the status bar
   void cmdLineUpdate( TQString p );	      // emitted when we need to update the command line
   void pathChanged( ListPanel *panel );
   void activePanelChanged( ListPanel *p ); // emitted when the user changes panels
   void finishedDragging();              // currently

public:
   TQString panelType;
   ListPanelFunc	*func;
   KrView *view;
   ListPanel	*otherPanel;
   int colorMask;
   bool compareMode;
   //FilterSpec	   filter;
   KRQuery filterMask;
   TQPixmap currDragPix;
   TQListViewItem *currDragItem;
   KDiskFreeSp* statsAgent;
   KrSqueezedTextLabel *status, *totals;
   KrQuickSearch *quickSearch;
   KURLRequester *origin;
   TQGridLayout *tqlayout;
   TQToolButton *cdRootButton;
   TQToolButton *cdHomeButton;
   TQToolButton *cdUpButton;
   TQToolButton *cdOtherButton;
	TQToolButton *popupBtn;
	TQToolButton *clearOrigin;
	PanelPopup *popup;
   KrBookmarkButton *bookmarksButton;
   DirHistoryQueue* dirHistoryQueue;
   DirHistoryButton* historyButton;
   MediaButton *mediaButton;
   SyncBrowseButton *syncBrowseButton;
	KPushButton *inlineRefreshCancelButton;
	KIO::Job *inlineRefreshJob;
	TQSplitter *splt;
   TQHeader * header;

protected:
   KURL _realPath; // named with _ to keep realPath() compatability
   KURL _jumpBackURL;
   
	
private:
   bool &_left;
	TQValueList<int> popupSizes;
};

#endif
