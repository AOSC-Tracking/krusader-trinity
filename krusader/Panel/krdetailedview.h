/***************************************************************************
                            krdetailedview.h
                           -------------------
  copyright            : (C) 2000-2002 by Shie Erlich & Rafi Yanai
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
#ifndef KRDETAILEDVIEW_H
#define KRDETAILEDVIEW_H

#include <tdelistview.h>
#include <ksqueezedtextlabel.h>
#include <tdelocale.h>
#include <tqwidget.h>
#include <tqtimer.h>
#include <tqdatetime.h>
#include "krview.h"
#include "krviewitem.h"

// extends KrViewProperties to add detailedview-only properties
class KrDetailedViewProperties: public KrViewProperties {
public:
	enum ColumnType { Unused = -1, Name = 0x0, Extention = 0x1, Mime = 0x2, Size = 0x3, DateTime = 0x4,
                     Permissions = 0x5, KrPermissions = 0x6, Owner = 0x7, Group = 0x8 };
	static const int MAX_COLUMNS = 9;
   int column[ MAX_COLUMNS ];	// column[ColumnType] contains the number of the requested column.
										// This is used by column() and whenever item uses text() or setText()
	bool numericPermissions; // show full permission column as octal numbers

	KrDetailedViewProperties() {
		for ( int i = 0; i < MAX_COLUMNS; i++ ) column[i] = Unused;
		filter = KrViewProperties::All;
		filterMask = KRQuery( "*" );
	}
};


class TQDragMoveEvent;
class KrRenameTimerObject;
class ListPanel;
class KrDetailedViewItem;

/**
 * KrDetailedView implements everthing and anything regarding a detailed view in a filemananger.
 * IT MUST USE KrViewItem as the children to it's *TDEListView. KrDetailedView and KrViewItem are
 * tightly coupled and the view will not work with other kinds of items.
 * Apart from this, the view is self-reliant and you can use the vast interface to get whatever
 * information is necessery from it.
 */
class KrDetailedView : public TDEListView, public KrView {
   TQ_OBJECT
  
   friend class KrDetailedViewItem;

public:
   KrDetailedView( TQWidget *parent, bool &left, TDEConfig *cfg = krConfig, const char *name = 0 );
   virtual ~KrDetailedView();
   virtual int column( KrDetailedViewProperties::ColumnType type );
   virtual inline KrViewItem *getFirst() { return dynamic_cast<KrViewItem*>( firstChild() ); }
   virtual inline KrViewItem *getLast() { return dynamic_cast<KrViewItem*>( lastChild() ); }
   virtual inline KrViewItem *getNext( KrViewItem *current ) { return dynamic_cast<KrViewItem*>( dynamic_cast<TDEListViewItem*>( current ) ->itemBelow() ); }
   virtual inline KrViewItem *getPrev( KrViewItem *current ) { return dynamic_cast<KrViewItem*>( dynamic_cast<TDEListViewItem*>( current ) ->itemAbove() ); }
   virtual inline KrViewItem *getCurrentKrViewItem() { return dynamic_cast<KrViewItem*>( currentItem() ); }
   virtual KrViewItem *getKrViewItemAt( const TQPoint &vp );
   virtual inline KrViewItem *findItemByName( const TQString &name ) { return dynamic_cast<KrViewItem*>( findItem( name, 0 ) ); }
   virtual void addItems( vfs *v, bool addUpDir = true );
 	virtual TQString getCurrentItem() const;
   virtual void makeItemVisible( const KrViewItem *item );	
   virtual void setCurrentItem( const TQString& name );
   virtual void updateView();
   virtual void updateItem(KrViewItem* item);
   virtual void clear();
   virtual void sort() { TDEListView::sort(); }
   virtual void setSortMode( KrViewProperties::SortSpec mode );
   virtual void prepareForActive();
   virtual void prepareForPassive();
   virtual inline void saveSettings() { TDEListView::saveLayout( _config, nameInTDEConfig() ); }
   virtual inline void restoreSettings() { TDEListView::restoreLayout( _config, nameInTDEConfig() ); }

signals:
   void middleButtonClicked( KrViewItem *item );
   void currentChanged( KrViewItem *item );

protected:
	virtual void setup();
	virtual void initProperties();
	virtual void initOperator();
	virtual KrViewItem *preAddItem(vfile *vf);
	virtual bool preDelItem(KrViewItem *item);

   void newColumn( KrDetailedViewProperties::ColumnType type );
   void selectColumns();
   
   virtual void keyPressEvent( TQKeyEvent *e );
   virtual void imStartEvent( TQIMEvent* e );
   virtual void imEndEvent( TQIMEvent *e );
   virtual void imComposeEvent( TQIMEvent *e );
   virtual void contentsMousePressEvent( TQMouseEvent *e );
   virtual void contentsMouseReleaseEvent (TQMouseEvent *e);
   virtual void contentsMouseMoveEvent ( TQMouseEvent * e );
   virtual void contentsWheelEvent( TQWheelEvent *e );
   virtual bool acceptDrag( TQDropEvent* e ) const;
   virtual void contentsDropEvent( TQDropEvent *e );
   virtual void contentsDragMoveEvent( TQDragMoveEvent *e );
   virtual TQRect drawItemHighlighter(TQPainter *painter, TQListViewItem *item);
   virtual void startDrag() { op()->startDrag(); }
   virtual bool event( TQEvent *e );
   virtual bool eventFilter( TQObject * watched, TQEvent * e );

protected slots:
   void rename( TQListViewItem *item, int c );
   void slotClicked( TQListViewItem *item );
   void slotDoubleClicked( TQListViewItem *item );
   void slotItemDescription( TQListViewItem *item );
   void slotCurrentChanged( TQListViewItem *item );
   void handleContextMenu( TQListViewItem*, const TQPoint&, int );
   virtual void renameCurrentItem();
   virtual void showContextMenu( );
   void inplaceRenameFinished( TQListViewItem *it, int col );
   void setNameToMakeCurrent( TQListViewItem *it );
	void sortOrderChanged(int);
	void slotRightButtonPressed(TQListViewItem*, const TQPoint& point, int);
	void slotSortOrderChanged(int col);
   void transformCurrentChanged( TQListViewItem * item ) { emit currentChanged( dynamic_cast<KrViewItem *>(item ) ); }

   /**
    * used internally to produce the signal middleButtonClicked()
    */
   void slotMouseClicked( int button, TQListViewItem * item, const TQPoint & pos, int c );
   inline void slotExecuted( TQListViewItem* i ) {
      TQString tmp = dynamic_cast<KrViewItem*>( i ) ->name();
      op()->emitExecuted( tmp );
   }

public slots:
   void refreshColors();
   void quickSearch( const TQString &, int = 0 );
   void stopQuickSearch( TQKeyEvent* );
   void handleQuickSearchEvent( TQKeyEvent* );

private:
   static TQString ColumnName[ KrDetailedViewProperties::MAX_COLUMNS ];
   bool swushSelects;
   TQPoint dragStartPos;
   TQListViewItem *lastSwushPosition;
   bool caseSensitiveSort;
   KrViewItem *_currDragItem;
   bool singleClicked;
   bool modifierPressed;
   TQTime clickTime;
   TQListViewItem *clickedItem;
   TQTimer renameTimer;
   TQTimer contextMenuTimer;
   TQPoint contextMenuPoint;
   TQListViewItem *currentlyRenamedItem;
   TQListViewItem *pressedItem;
};

#endif /* KRDETAILEDVIEW_H */
