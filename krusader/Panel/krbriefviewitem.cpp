#include "krbriefview.h"
#include "krbriefviewitem.h"
#include "../krusaderview.h"
#include "../defaults.h"
#include "../kicons.h"
#include "listpanel.h"
#include <tdeconfig.h>
#include "krcolorcache.h"

#include <tqpainter.h>
#include <tqpointarray.h>

#define PROPS	_viewProperties
#define VF	getVfile()

#ifdef FASTER
int KrBriefViewItem::expHeight = 0;
#endif // FASTER

KrBriefViewItem::KrBriefViewItem(KrBriefView *parent, TQIconViewItem *after, vfile *vf):
	TDEIconViewItem(parent, after), KrViewItem(vf, parent->properties()) {
#ifdef FASTER
	initiated = false;
	// get the expected height of an item - should be done only once
	if (expHeight == 0) {
		TDEConfigGroupSaver svr(krConfig, "Look&Feel");
  		expHeight = 2 + (krConfig->readEntry("Filelist Icon Size",_FilelistIconSize)).toInt();
	}
	if( PROPS->displayIcons )
		itemIcon = TQPixmap( expHeight, expHeight );
#endif // FASTER

	// there's a special case, where if _vf is null, then we've got the ".." (updir) item
	// in that case, create a special vfile for that item, and delete it, if needed
	if (!_vf) {
		dummyVfile = true;
		_vf = new vfile("..", 0, "drw-r--r--", 0, false, 0, 0, TQString(), TQString(), 0);
		
		setText("..");
		if ( PROPS->displayIcons )
			setPixmap( FL_LOADICON( "go-up" ) );
		setSelectable( false );
		setDragEnabled( false );
		setDropEnabled( false );
#ifdef FASTER
		initiated = true;
#endif // FASTER
	}
	
	setRenameEnabled( false );
	setDragEnabled( true );
	setDropEnabled( true );
	repaintItem();
}


int KrBriefViewItem::compare(TQIconViewItem *i ) const {
  bool ignoreCase = (PROPS->sortMode & KrViewProperties::IgnoreCase);

  KrBriefViewItem *other = (KrBriefViewItem *)i;
  int asc = iconView()->sortDirection() ? -1 : 1;

  bool thisDir = VF->vfile_isDir();
  bool otherDir = other->VF->vfile_isDir();

  // handle directory sorting
  if ( thisDir ){
    if ( !otherDir ) return 1*asc;
  } else if( otherDir ) return -1*asc;

  if ( isDummy() ) return 1*asc;
  if ( other->isDummy() ) return -1*asc;

  TQString text0 = name();
  if (text0 == "..") return 1*asc;
  
  TQString itext0 = other->name();
  if (itext0 == "..") return -1*asc;

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
}

void KrBriefViewItem::paintItem(TQPainter *p, const TQColorGroup &cg) {
#ifdef FASTER
  if (!initiated && !dummyVfile) {
     // display an icon if needed
     initiated = true;
     if (PROPS->displayIcons)
       itemIcon =KrView::getIcon(_vf);
  }
#endif

  TQColorGroup _cg(cg);

  KrColorItemType colorItemType;
  colorItemType.m_activePanel = (dynamic_cast<KrView *>(iconView()) == ACTIVE_PANEL->view);
  
  int gridX = iconView()->gridX();
  int xpos = x() / gridX;
  int ypos = y() / height();
  
  colorItemType.m_alternateBackgroundColor = (xpos & 1) ^ (ypos & 1) ;
  colorItemType.m_currentItem = (iconView()->currentItem() == this);
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
  
  if( _cg.background() != iconView()->paletteBackgroundColor() )
  {
     p->save();
     p->setPen( TQt::NoPen );
     p->setBrush( TQBrush( _cg.background() ) );
     p->drawRect( rect() );
     p->restore();
  }

  TQIconViewItem::paintItem(p, _cg);

  paintFocus( p, cg );
}

void KrBriefViewItem::paintFocus(TQPainter *p, const TQColorGroup &cg) {
  if ( ( iconView()->hasFocus() && this == iconView()->currentItem() ) || 
     ((KrBriefView *)iconView())->_currDragItem == this )
  {
      p->save();
      TQPen pen( cg.shadow(), 0, TQt::DotLine );
      p->setPen( pen );

      // we manually draw the focus rect by points
      TQRect rec = rect();
      TQPointArray points( rec.right() - rec.left() + rec.bottom() - rec.top() + 4 );
      
      int ndx = 0;
      for( int x=rec.left(); x <= rec.right(); x+=2 )
      {
        points.setPoint( ndx++, x, rec.top() );
        points.setPoint( ndx++, x, rec.bottom() );
      }
      for( int y=rec.top(); y <= rec.bottom(); y+=2 )
      {
        points.setPoint( ndx++, rec.left(), y );
        points.setPoint( ndx++, rec.right(), y );
      }

      p->drawPoints( points );
      p->restore();

//    --- That didn't work with all themes
//    iconView()->style().drawPrimitive(TQStyle::PE_FocusRect, p,
//       TQRect( rect().x(), rect().y(), rect().width(), rect().height() ), cg,
//              TQStyle::Style_Default, cg.base() );
  }
}

void KrBriefViewItem::itemHeightChanged() {
#ifdef FASTER
	expHeight = 0;
#endif // FASTER
}

void KrBriefViewItem::repaintItem()
{
   if ( dummyVfile ) return;

#ifndef FASTER
   if (PROPS->displayIcons)
     setPixmap(KrView::getIcon(_vf));
#endif // FASTER
   setText( _vf->vfile_getName() );
}

// for keeping the exact item heights...
void KrBriefViewItem::calcRect ( const TQString & text_ )
{
   TDEIconViewItem::calcRect( text_ );
   TQRect rec = rect();
   
   int gridX = iconView()->gridX();
   int minX  = ( rec.x() / gridX ) * gridX;
   rec.setX( minX );
   rec.setWidth( gridX );
   rec.setHeight( expHeight );   
   setItemRect( rec );
   
   rec = pixmapRect();
   if( rec.height() > expHeight )
   {
     rec.setHeight( expHeight );
     setPixmapRect( rec );
   }   
   
   rec = textRect();
   if( rec.height() > expHeight )
   {
     rec.setHeight( expHeight );
     setTextRect( rec );
   }   
}
