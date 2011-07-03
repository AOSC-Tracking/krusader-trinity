/***************************************************************************
                            krdetailedviewitem.cpp
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

#include "../krusader.h"
#include "../defaults.h"
#include "../kicons.h"
#include "../krusaderview.h"
#include "krdetailedviewitem.h"
#include "krdetailedview.h"
#include "krcolorcache.h"
#include "listpanel.h"
#include "../VFS/krpermhandler.h"
#include <sys/types.h>
#include <time.h>
#include <tqpainter.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <tqpalette.h>
#include <kdebug.h>
#include <kmimetype.h>

#define COLUMN(X)	static_cast<const KrDetailedViewProperties*>(_viewProperties)->	\
	column[ KrDetailedViewProperties::X ]
#define PROPS	static_cast<const KrDetailedViewProperties*>(_viewProperties)
#define PERM_BITMASK (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)
#define VF	getVfile()

#ifdef FASTER
int KrDetailedViewItem::expHeight = 0;
#endif // FASTER

KrDetailedViewItem::KrDetailedViewItem(KrDetailedView *tqparent, TQListViewItem *after, vfile *vf):
	KListViewItem(tqparent, after), KrViewItem(vf, tqparent->properties()) {
#ifdef FASTER
	initiated = false;
	// get the expected height of an item - should be done only once
	if (expHeight == 0) {
		KConfigGroupSaver svr(krConfig, "Look&Feel");
  		expHeight = 2 + (krConfig->readEntry("Filelist Icon Size",_FilelistIconSize)).toInt();
	}

#endif	
	// there's a special case, where if _vf is null, then we've got the ".." (updir) item
	// in that case, create a special vfile for that item, and delete it, if needed
	if (!_vf) {
		dummyVfile = true;
		_vf = new vfile("..", 0, "drw-r--r--", 0, false, 0, 0, TQString(), TQString(), 0);
		
		setText(COLUMN(Name), "..");
		setText(COLUMN(Size), i18n("<DIR>") );
      if ( PROPS->displayIcons )
         setPixmap( COLUMN(Name), FL_LOADICON( "up" ) );
      setSelectable( false );
#ifdef FASTER
		initiated = true;
#endif
	}
	
	tqrepaintItem();
}

#ifdef FASTER
void KrDetailedViewItem::setup() {
	// idea: when not having pixmaps in the first place, the height of the item is smaller then with
	// the pixmap. when the pixmap is inserted, the item resizes, thereby making ensureItemVisible()
	// become 'confused' and stop working. therefore, we set the correct height here and avoid the issue
	KListViewItem::setup();
	setHeight(expHeight);
}
#endif

void KrDetailedViewItem::tqrepaintItem() {
    if ( dummyVfile ) return;
    TQString tmp;
    // set text in columns, according to what columns are available
    int id = KrDetailedViewProperties::Unused;
    if ((id = COLUMN(Mime)) != -1) {
      tmp = KMimeType::mimeType(_vf->vfile_getMime())->comment();
      setText( id, tmp );
    }
    if ((id = COLUMN(Size)) != -1) {
		 if (_vf->vfile_isDir() && _vf->vfile_getSize() <= 0) setText(id, i18n("<DIR>"));
	    else setText(id, PROPS->humanReadableSize ? KIO::convertSize(_vf->vfile_getSize())+"  " :
		 						KRpermHandler::parseSize(_vf->vfile_getSize())+" ");
    }

    if ((id = COLUMN(DateTime)) != -1)
      setText(id, dateTime());
    if ((id = COLUMN(KrPermissions)) != -1) {
      // first, build the krusader permissions
      tmp=TQString();
      switch (_vf->vfile_isReadable()){
        case ALLOWED_PERM: tmp+='r'; break;
        case UNKNOWN_PERM: tmp+='?'; break;
        case NO_PERM:      tmp+='-'; break;
      }
   	  switch (_vf->vfile_isWriteable()){
        case ALLOWED_PERM: tmp+='w'; break;
        case UNKNOWN_PERM: tmp+='?'; break;
        case NO_PERM:      tmp+='-'; break;
      }
   	  switch (_vf->vfile_isExecutable()){
        case ALLOWED_PERM: tmp+='x'; break;
        case UNKNOWN_PERM: tmp+='?'; break;
        case NO_PERM:      tmp+='-'; break;
      }
      setText(id, tmp);
    }
    if ((id = COLUMN(Permissions) ) != -1) {
		if (PROPS->numericPermissions) {
      	setText(id, tmp.sprintf("%.4o", _vf->vfile_getMode() & PERM_BITMASK));
		} else setText(id, _vf->vfile_getPerm());
	 }
    if ((id = COLUMN(Owner)) != -1) {
      setText(id, _vf->vfile_getOwner());
    }
    if ((id = COLUMN(Group)) != -1) {
      setText(id, _vf->vfile_getGroup());
    }
    // if we've got an extention column, clip the name accordingly
    TQString name = this->name(), ext = "";
    if ((id = COLUMN(Extention)) != -1 && !_vf->vfile_isDir()) {
    	ext = this->extension();
	name = this->name(false); // request name without extension
      	setText(id, ext);
    }
    setText(COLUMN(Name), name);
#ifndef FASTER
    // display an icon if needed
    if (PROPS->displayIcons)
      setPixmap(COLUMN(Name),KrView::getIcon(_vf));
#endif
}

TQString num2qstring(KIO::filesize_t num){
  TQString buf;
  buf.sprintf("%025llu",num);
  return buf;
}

void KrDetailedViewItem::paintCell(TQPainter *p, const TQColorGroup &cg, int column, int width, int align) {
#ifdef FASTER
	if (!initiated && !dummyVfile) {
		// display an icon if needed
		if (PROPS->displayIcons)
			setPixmap(COLUMN(Name),KrView::getIcon(_vf));
		
		initiated = true;
	}
#endif
  
  TQColorGroup _cg(cg);

   // This is ugly! I had to dublicate KListViewItem::paintCell() code, as the
   // KListViewItem::paintCell() overwrites my color settings. So KrDetailedViewItem::paintCell
   // must dublicate the KListViewItem::paintCell() code, do the required color settings
   // and call TQListViewItem::paintCell() afterwards (the base class of KListViewItem).
   // This tabooed in the object oriented heaven, but necessary here. Blame the KDE team for
   // this really poor paintCell implementation!
   
   const TQPixmap *pm = listView()->viewport()->backgroundPixmap();
   if (pm && !pm->isNull())
   {
         _cg.setBrush(TQColorGroup::Base, TQBrush(backgroundColor(), *pm));
         p->setBrushOrigin( -listView()->contentsX(), -listView()->contentsY() );
   }
   else if (isAlternate())
        if (listView()->viewport()->backgroundMode()==TQt::FixedColor)
             _cg.setColor(TQColorGroup::Background, static_cast< KListView* >(listView())->alternateBackground());
       else
             _cg.setColor(TQColorGroup::Base, static_cast< KListView* >(listView())->alternateBackground());

  // end of uglyness

  KrColorItemType colorItemType;
  colorItemType.m_activePanel = (dynamic_cast<KrView *>(listView()) == ACTIVE_PANEL->view);
  colorItemType.m_alternateBackgroundColor = isAlternate();
  colorItemType.m_currentItem = (listView()->currentItem() == this);
  colorItemType.m_selectedItem = isSelected();
  if (VF->vfile_isSymLink())
  {
     if (_vf->vfile_getMime() == "Broken Link !" )
        colorItemType.m_fileType = KrColorItemType::InvalidSymlink;
     else
        colorItemType.m_fileType = KrColorItemType::Symlink;
  }
  else if (VF->vfile_isDir())
     colorItemType.m_fileType = KrColorItemType::Directory;
  else if (VF->vfile_isExecutable())
     colorItemType.m_fileType = KrColorItemType::Executable;
  else
     colorItemType.m_fileType = KrColorItemType::File;
  KrColorCache::getColorCache().getColors(_cg, colorItemType);
	// center the <DIR> thing if needed
	if(column != COLUMN(Size))
		TQListViewItem::paintCell(p, _cg, column, width, align);
	else {
  		if (dummyVfile) {
			TQListViewItem::paintCell(p, _cg, column, width, TQt::AlignHCenter); // updir
  		} else {
    		if (_vf->vfile_isDir() && _vf->vfile_getSize()<=0)
      		TQListViewItem::paintCell(p, _cg, column, width, TQt::AlignHCenter);
    		else TQListViewItem::paintCell(p, _cg, column, width, align); // size
  		}
	}
}

const TQColor & KrDetailedViewItem::setColorIfContrastIsSufficient(const TQColor & background, const TQColor & color1, const TQColor & color2)
{
   #define sqr(x) ((x)*(x))
   int contrast = sqr(color1.red() - background.red()) + sqr(color1.green() - background.green()) + sqr(color1.blue() - background.blue());

   // if the contrast between background and color1 is too small, take color2 instead.
   if (contrast < 1000)
      return color2;
   return color1;
}

int KrDetailedViewItem::compare(TQListViewItem *i,int col,bool ascending ) const {
  bool ignoreCase = (PROPS->sortMode & KrViewProperties::IgnoreCase);
	bool alwaysSortDirsByName = (PROPS->sortMode & KrViewProperties::AlwaysSortDirsByName);
  int asc = ( ascending ? -1 : 1 );
  KrDetailedViewItem *other = (KrDetailedViewItem *)(i);

  bool thisDir = VF->vfile_isDir();
  bool otherDir = other->VF->vfile_isDir();

  // handle directory sorting
  if ( thisDir ){
    if ( !otherDir ) return 1*asc;
  } else if( otherDir ) return -1*asc;

  if ( isDummy() ) return 1*asc;
  if ( other->isDummy() ) return -1*asc;
		
  if (col == COLUMN(Name) ||
			(alwaysSortDirsByName && thisDir && otherDir )) {
      // localeAwareCompare doesn't handle names that start with a dot
		TQString text0 = name();
		TQString itext0 = other->name();

		if( ignoreCase )
		{
			text0  = text0.lower();
			itext0 = itext0.lower();
		}

		if ( isHidden() ) {
			if ( !other->isHidden() ) return 1*asc;
		} else if ( other->isHidden() ) return -1*asc;
		if (!ignoreCase && !PROPS->localeAwareCompareIsCaseSensitive) {
			// sometimes, localeAwareCompare is not case sensative. in that case,
			// we need to fallback to a simple string compare (KDE bug #40131)
			return TQString::compare(text0, itext0);
		} else return TQString::localeAwareCompare(text0,itext0);
  } else if (col == COLUMN(Size) ) {
      if( VF->vfile_getSize() == other->VF->vfile_getSize() )
        return 0;
      return (VF->vfile_getSize() > other->VF->vfile_getSize() ? 1 : -1);
  } else if (col == COLUMN(DateTime) ) {
      if( VF->vfile_getTime_t() == other->VF->vfile_getTime_t() )
        return 0;
      return (VF->vfile_getTime_t() > other->VF->vfile_getTime_t() ? 1 : -1);
  } else if (col == COLUMN(Permissions) && PROPS->numericPermissions) {
		int thisPerm = VF->vfile_getMode() & PERM_BITMASK;
		int otherPerm = other->VF->vfile_getMode() & PERM_BITMASK;
		if( thisPerm == otherPerm )
			return 0;
		return ((thisPerm > otherPerm) ? 1 : -1);
  } else {
      TQString e1 = (!ignoreCase ? text(col) : text(col).lower());
      TQString e2 = (!ignoreCase ? i->text(col) : i->text(col).lower());
		if (!ignoreCase && !PROPS->localeAwareCompareIsCaseSensitive) {
			// sometimes, localeAwareCompare is not case sensative. in that case,
			// we need to fallback to a simple string compare (KDE bug #40131)
			return TQString::compare(e1, e2);
		} else return TQString::localeAwareCompare(e1, e2);
  }
}

void KrDetailedViewItem::itemHeightChanged() {
#ifdef FASTER
	expHeight = 0;
#endif
}
