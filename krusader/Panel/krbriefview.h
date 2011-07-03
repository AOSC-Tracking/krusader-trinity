/***************************************************************************
                              krbriefview.h
                           -------------------
  copyright            : (C) 2000-2007 by Shie Erlich & Rafi Yanai & Csaba Karai
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
#ifndef KRBRIEFVIEW_H
#define KRBRIEFVIEW_H

#include "krview.h"
#include "krviewitem.h"
#include <kiconview.h>
#include <tqtimer.h>

// extends KrViewProperties to add detailedview-only properties
class KrBriefViewProperties: public KrViewProperties {
public:
	int numberOfColumns; // the number of columns in the view
};

class KrBriefViewItem;
class TQDragMoveEvent;
class TQToolTip;
class TQHeader;

/**
 * KrBriefView implements everthing and anything regarding a brief view in a filemananger.
 * IT MUST USE KrViewItem as the tqchildren to it's *KIconView. KrBriefView and KrViewItem are
 * tightly coupled and the view will not work with other kinds of items.
 * Apart from this, the view is self-reliant and you can use the vast interface to get whatever
 * information is necessery from it.
 */
class KrBriefView: public KIconView, public KrView {
	friend class KrBriefViewItem;
	Q_OBJECT
  TQ_OBJECT
public:
	KrBriefView( TQHeader *header, TQWidget *tqparent, bool &left, KConfig *cfg = krConfig, const char *name = 0 );
	virtual ~KrBriefView();
	virtual inline KrViewItem *getFirst() { return dynamic_cast<KrViewItem*>( firstItem() ); }
	virtual inline KrViewItem *getLast() { return dynamic_cast<KrViewItem*>( lastItem() ); }
	virtual inline KrViewItem *getNext( KrViewItem *current ) { return dynamic_cast<KrViewItem*>( dynamic_cast<TQIconViewItem*>( current ) ->nextItem() ); }
	virtual inline KrViewItem *getPrev( KrViewItem *current ) { return dynamic_cast<KrViewItem*>( dynamic_cast<TQIconViewItem*>( current ) ->prevItem() ); }
	virtual inline KrViewItem *getCurrentKrViewItem() { return dynamic_cast<KrViewItem*>( currentItem() ); }
	virtual KrViewItem *getKrViewItemAt(const TQPoint &vp);
	virtual inline KrViewItem *findItemByName(const TQString &name) { return dynamic_cast<KrViewItem*>( tqfindItem( name, TQt::ExactMatch ) ); }
	virtual void addItems(vfs* v, bool addUpDir = true);
	virtual void delItem(const TQString &);
	virtual TQString getCurrentItem() const;
	virtual void makeItemVisible(const KrViewItem * item );
	virtual void setCurrentItem(const TQString& name );
	virtual void updateView();
	virtual void updateItem(KrViewItem* item );
	virtual void clear();
	virtual void sort()                        { if( sortDirection() ) sortOrderChanged();KIconView::sort( true ); }
	virtual void sort( bool ascending )        { if( sortDirection() != ascending ) sortOrderChanged();KIconView::sort( ascending ); }
	virtual void prepareForActive();
	virtual void prepareForPassive();
	virtual void saveSettings() {}
	virtual void restoreSettings() {}
	virtual TQString nameInKConfig() {return _nameInKConfig;}
	virtual void resizeEvent ( TQResizeEvent * );

signals:
	void middleButtonClicked( KrViewItem *item );
	void currentChanged( KrViewItem *item );

protected:
	virtual void setup();
	virtual void initProperties();
	virtual void initOperator();
	virtual KrViewItem *preAddItem(vfile * vf);
	virtual bool preDelItem(KrViewItem * item );

	void setColumnNr();
	void redrawColumns();

	virtual void keyPressEvent( TQKeyEvent *e );
	virtual void imStartEvent( TQIMEvent* e );
	virtual void imEndEvent( TQIMEvent *e );
	virtual void imComposeEvent( TQIMEvent *e );
	virtual void contentsMousePressEvent( TQMouseEvent *e );
	virtual void contentsMouseReleaseEvent (TQMouseEvent *e);
	virtual void contentsMouseMoveEvent ( TQMouseEvent * e );
	virtual void contentsMouseDoubleClickEvent ( TQMouseEvent * e );
	virtual bool acceptDrag( TQDropEvent* e ) const;
	virtual void contentsDropEvent( TQDropEvent *e );
	virtual void contentsDragMoveEvent( TQDragMoveEvent *e );
	virtual void contentsDragLeaveEvent ( TQDragLeaveEvent * );
	virtual void startDrag() { op()->startDrag(); }
	virtual bool event( TQEvent *e );
	virtual bool eventFilter( TQObject * watched, TQEvent * e );
	TQMouseEvent * transformMouseEvent( TQMouseEvent * );

protected slots:
	void rename( TQIconViewItem *item );
	void slotClicked( TQIconViewItem *item );
	void slotDoubleClicked( TQIconViewItem *item );
	void slotItemDescription( TQIconViewItem * );
	void slotCurrentChanged( TQIconViewItem *item );
	void handleContextMenu( TQIconViewItem*, const TQPoint& );
	virtual void renameCurrentItem();
	virtual void showContextMenu( );
	void inplaceRenameFinished( TQIconViewItem *it );
	void setNameToMakeCurrent( TQIconViewItem *it );
	void sortOrderChanged();
	void slotRightButtonPressed(TQIconViewItem*, const TQPoint& point);
	void transformCurrentChanged( TQIconViewItem * item ) { emit currentChanged( dynamic_cast<KrViewItem *>(item ) ); }

	/**
	  * used internally to produce the signal middleButtonClicked()
	 */
	void slotMouseClicked( int button, TQIconViewItem * item, const TQPoint & pos );
	inline void slotExecuted( TQIconViewItem* i ) {
		TQString tmp = dynamic_cast<KrViewItem*>( i ) ->name();
		op()->emitExecuted( tmp );
	}

public slots:
	void refreshColors();
	void quickSearch( const TQString &, int = 0 );
	void stopQuickSearch( TQKeyEvent* );
	void handleQuickSearchEvent( TQKeyEvent* );
	void changeSortOrder();

  
signals:
	void letsDrag(TQStringList items, TQPixmap icon);
	void gotDrop(TQDropEvent *);

private:
	TQHeader * header;
	bool swushSelects;
	TQPoint dragStartPos;
	TQIconViewItem *lastSwushPosition;
	KrViewItem *_currDragItem;
	bool singleClicked;
	bool modifierPressed;
	TQTime clickTime;
	TQIconViewItem *clickedItem;
	TQTimer renameTimer;
	TQTimer contextMenuTimer;
	TQPoint contextMenuPoint;
	KrBriefViewItem *currentlyRenamedItem;
	TQIconViewItem *pressedItem;
	TQMouseEvent *mouseEvent;
	TQToolTip *toolTip;
};

#endif
