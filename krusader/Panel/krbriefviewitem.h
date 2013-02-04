/***************************************************************************
                            krbriefviewitem.h
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

#ifndef KRBRIEFVIEWITEM_H
#define KRBRIEFVIEWITEM_H

#include "krviewitem.h"
#include <sys/types.h>
#include "../VFS/vfile.h"
#include <kiconview.h>
#include <tqguardedptr.h>

#define FASTER

class TQPixmap;
class KrBriefView;

class KrBriefViewItem : public TDEIconViewItem, public KrViewItem {
friend class KrBriefView;
friend class KrCalcSpaceDialog;
public:
	KrBriefViewItem(KrBriefView *parent, TQIconViewItem *after, vfile *vf);
	inline bool isSelected() const { return TDEIconViewItem::isSelected(); }
	inline void setSelected(bool s) { TDEIconViewItem::setSelected(s); }
	inline void cancelRename() { removeRenameBox(); }
	int compare(TQIconViewItem *i) const;
	virtual void repaintItem();
	static void itemHeightChanged(); // force the items to resize when icon/font size change
	// TODO: virtual void setup(); // called when iconview needs to know the height of the item
#ifdef FASTER
	virtual TQPixmap * pixmap() const {return const_cast<TQPixmap *>(&itemIcon);}
	virtual void setPixmap ( const TQPixmap &icon ) { itemIcon = icon;}
#endif

protected:
	virtual void paintItem(TQPainter *p, const TQColorGroup &cg);
	virtual void paintFocus(TQPainter *p, const TQColorGroup &cg);
	virtual void calcRect ( const TQString & text_ = TQString() );

private:
#ifdef FASTER
	bool initiated;
	static int expHeight;
	TQPixmap itemIcon;
#endif // FASTER
	// TODO:
	static const TQColor & setColorIfContrastIsSufficient(const TQColor & /* background */, const TQColor & /* color1 */, const TQColor & /* color2 */ ) {static TQColor col; return col;}
	
//	static int expHeight;
};

#endif
