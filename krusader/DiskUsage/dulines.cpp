/***************************************************************************
                         dulines.cpp  -  description
                             -------------------
    copyright            : (C) 2004 by Csaba Karai
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

#include "dulines.h"
#include "../kicons.h"
#include "../krusader.h"
#include "../VFS/krpermhandler.h"
#include <tqheader.h>
#include <tdelocale.h>
#include <tqpen.h>
#include <tqpainter.h>
#include <tqfontmetrics.h>
#include <tqtimer.h>
#include <tqtooltip.h>
#include <tdepopupmenu.h>

class DULinesItem : public TQListViewItem
{
public:
  DULinesItem( DiskUsage *diskUsageIn, File *fileItem, TQListView * parent, TQString label1, 
               TQString label2, TQString label3, unsigned int italicPos ) : TQListViewItem( parent, label1, label2, label3 ), 
               diskUsage( diskUsageIn ), file( fileItem ), isTruncated( false ), italicTextPos( italicPos ) {}
  DULinesItem( DiskUsage *diskUsageIn, File *fileItem, TQListView * parent, TQListViewItem * after, 
               TQString label1, TQString label2, TQString label3, unsigned int italicPos ) : TQListViewItem( parent, after, label1, 
               label2, label3 ), diskUsage( diskUsageIn ), file( fileItem ), isTruncated( false ), italicTextPos( italicPos ) {}
  
  virtual int compare ( TQListViewItem * i, int col, bool ascending ) const 
  {
    if( text(0) == ".." ) return ascending ? -1 : 1;
    if( i->text(0) == "..") return ascending ? 1 : -1;
    
    DULinesItem *compWith = dynamic_cast< DULinesItem * >( i );
        
    TQString buf1,buf2;
    
    switch( col )
    {
    case 0:    
    case 1:
      buf1.sprintf("%025llu",file->size());
      buf2.sprintf("%025llu",compWith->file->size());
      return -TQString::compare( buf1, buf2 );
    default:    
      return TQListViewItem::compare( i, col, ascending );
    }
  }

  virtual void paintCell( TQPainter * p, const TQColorGroup & cg, int column, int width, int align )
  {
    if( column == 2 )
    {
      if ( isSelected() ) 
        p->fillRect( 0, 0, width, height(), cg.brush( TQColorGroup::Highlight ) );
      else
        p->fillRect( 0, 0, width, height(), cg.brush( TQColorGroup::Base ) );
        
      TQListView *lv = listView();
      
      int pos = lv->itemMargin();      
            
      const TQPixmap *icon = pixmap( column );
      if( icon )
      {
        int iconWidth = icon->width() + lv->itemMargin();
        int xo = pos;
        int yo = ( height() - icon->height() ) / 2;

        p->drawPixmap( xo, yo, *icon );
        
        pos += iconWidth;
      }
    
      TQFontMetrics fm( p->fontMetrics() );
      
      if( isSelected() )
        p->setPen( cg.highlightedText() );
      else
        p->setPen( cg.text() );
      
      TQString t = text( column );
      TQString b;
      
      if( t.length() > italicTextPos )
      {
        b = t.mid( italicTextPos );
        t.truncate( italicTextPos );
      }
      
      isTruncated = false;
      if( !t.isEmpty() )
      {
        int remWidth = width-pos;
        
        if( fm.width( t ) > remWidth )
        {
          while( !t.isEmpty() )
          {
            t.truncate( t.length() - 1 );
            if( fm.width( t + "..." ) <= remWidth )
              break;
          }          
          t += "...";
          isTruncated = true;
        }
        
        p->drawText( pos, 0, width, height(), align, t );
        pos += fm.width( t );
      }
                    
      if( !b.isEmpty() && !isTruncated )
      {
        TQFont font( p->font() );
        font.setItalic( true );
        p->setFont( font );

        TQFontMetrics fm2( p->fontMetrics() );
        
        int remWidth = width-pos;
        
        if( fm2.width( b ) > remWidth )
        {
          while( !b.isEmpty() )
          {
            b.truncate( b.length() - 1 );
            if( fm2.width( b + "..." ) <= remWidth )
              break;
          }          
          b += "...";
          isTruncated = true;
        }
        
        p->drawText( pos, 0, width, height(), align, b );
      }
    }
    else
      TQListViewItem::paintCell( p, cg, column, width, align );
  }
    
  inline File * getFile() { return file; }
  
private:
  DiskUsage *diskUsage;
  File *file;
  
  bool isTruncated;
  unsigned int italicTextPos;
};

class DULinesToolTip : public TQToolTip
{
public:
    DULinesToolTip( DiskUsage *usage, TQWidget *parent, TQListView *lv );
    void maybeTip( const TQPoint &pos );

    virtual ~DULinesToolTip() {}
private:
    TQListView *view;
    DiskUsage *diskUsage;
};

DULinesToolTip::DULinesToolTip( DiskUsage *usage, TQWidget *parent, TQListView *lv )
  : TQToolTip( parent ), view( lv ), diskUsage( usage )
{
}

void DULinesToolTip::maybeTip( const TQPoint &pos )
{
  TQListViewItem *item = view->itemAt( pos );
  TQPoint contentsPos = view->viewportToContents( pos );
  if ( !item )
    return;
    
  int col = view->header()->sectionAt( contentsPos.x() );

  int width = item->width( TQFontMetrics( view->font() ), view, col );
    
  TQRect r = view->itemRect( item );
  int headerPos = view->header()->sectionPos( col );
  r.setLeft( headerPos );
  r.setRight( headerPos + view->header()->sectionSize( col ) );
    
  if( col != 0 && width > view->columnWidth( col ) )
    tip( r, item->text( col ) );
  else if( col == 1 && item->text( 0 ) != ".." )
  {
    File *fileItem = ((DULinesItem *)item)->getFile();
    tip( r, diskUsage->getToolTip( fileItem ) );
  }
}

DULines::DULines( DiskUsage *usage, const char *name )
  : TQListView( usage, name ), diskUsage( usage ), refreshNeeded( false )
{
  setAllColumnsShowFocus(true);
  setVScrollBarMode(TQScrollView::Auto);
  setHScrollBarMode(TQScrollView::Auto);
  setShowSortIndicator(true);
  setTreeStepSize( 10 );

  int defaultSize = TQFontMetrics(font()).width("W");
  
  krConfig->setGroup( diskUsage->getConfigGroup() ); 

  showFileSize = krConfig->readBoolEntry( "L Show File Size", true );
  
  int lineWidth  = krConfig->readNumEntry("L Line Width",  defaultSize * 20 );    
  addColumn( i18n("Line View"), lineWidth );
  setColumnWidthMode(0,TQListView::Manual);
  int precentWidth  = krConfig->readNumEntry("L Percent Width",  defaultSize * 6 );    
  addColumn( i18n("Percent"), precentWidth );
  setColumnWidthMode(1,TQListView::Manual);
  int nameWidth  = krConfig->readNumEntry("L Name Width",  defaultSize * 20 );
  addColumn( i18n("Name"), nameWidth );
  setColumnWidthMode(2,TQListView::Manual);
  
  setColumnAlignment( 1, TQt::AlignRight );
  
  header()->setStretchEnabled( true, 0 );
  
  setSorting( 1 );
  
  toolTip = new DULinesToolTip( diskUsage, viewport(), this );

  connect( diskUsage, TQT_SIGNAL( enteringDirectory( Directory * ) ), this, TQT_SLOT( slotDirChanged( Directory * ) ) );
  connect( diskUsage, TQT_SIGNAL( clearing() ), this, TQT_SLOT( clear() ) );
  
  connect( header(), TQT_SIGNAL( sizeChange( int, int, int ) ), this, TQT_SLOT( sectionResized( int ) ) );

  connect( this, TQT_SIGNAL(rightButtonPressed(TQListViewItem *, const TQPoint &, int)),
           this, TQT_SLOT( slotRightClicked(TQListViewItem *) ) );
  connect( diskUsage, TQT_SIGNAL( changed( File * ) ), this, TQT_SLOT( slotChanged( File * ) ) );
  connect( diskUsage, TQT_SIGNAL( deleted( File * ) ), this, TQT_SLOT( slotDeleted( File * ) ) );
}

DULines::~DULines()
{
  krConfig->setGroup( diskUsage->getConfigGroup() ); 
  krConfig->writeEntry("L Line Width",      columnWidth( 0 ) );
  krConfig->writeEntry("L Percent Width",   columnWidth( 1 ) );
  krConfig->writeEntry("L Name Width",      columnWidth( 2 ) );
  krConfig->writeEntry("L Show File Size",  showFileSize );
  
  delete toolTip;
}

void DULines::slotDirChanged( Directory *dirEntry )
{
  clear();  
  
  TQListViewItem * lastItem = 0;
    
  if( ! ( dirEntry->parent() == 0 ) )
  {
    lastItem = new TQListViewItem( this, ".." );
    lastItem->setPixmap( 0, FL_LOADICON( "up" ) );
    lastItem->setSelectable( false );
  }
          
  int maxPercent = -1;
  for( Iterator<File> it = dirEntry->iterator(); it != dirEntry->end(); ++it )
  {
    File *item = *it;
    if( !item->isExcluded() && item->intPercent() > maxPercent )
      maxPercent = item->intPercent();
  }
  
  for( Iterator<File> it = dirEntry->iterator(); it != dirEntry->end(); ++it )
  { 
    File *item = *it;
    
    TQString fileName = item->name();
    
    unsigned int italicStart = fileName.length();
    
    if( showFileSize )
      fileName += "  [" + TDEIO::convertSize( item->size() ) + "]";
    
    if( lastItem == 0 )
      lastItem = new DULinesItem( diskUsage, item, this, "", item->percent() + "  ", fileName, italicStart );
    else
      lastItem = new DULinesItem( diskUsage, item, this, lastItem, "", item->percent() + "  ", fileName, italicStart );
   
    if( item->isExcluded() )
      lastItem->setVisible( false );
                                    
    lastItem->setPixmap( 2, diskUsage->getIcon( item->mime() ) );
    lastItem->setPixmap( 0, createPixmap( item->intPercent(), maxPercent, columnWidth( 0 ) - itemMargin() ) );
  }
  
  setCurrentItem( firstChild() );
}

TQPixmap DULines::createPixmap( int percent, int maxPercent, int maxWidth )
{
  if( percent < 0 || percent > maxPercent || maxWidth < 2 || maxPercent == 0 )
    return TQPixmap();
  maxWidth -= 2;

  int actualWidth = maxWidth*percent/maxPercent;
  if( actualWidth == 0 )
    return TQPixmap();
    
  TQPen pen;
  pen.setColor( TQt::black );  
  TQPainter painter;
  
  int size = TQFontMetrics(font()).height()-2;
  TQRect rect( 0, 0, actualWidth, size );
  TQPixmap pixmap( rect.width(), rect.height() );

  painter.begin( &pixmap );
  painter.setPen( pen );
  
  for( int i = 1; i < actualWidth - 1; i++ )
  {
    int color = (511*i/maxWidth);
    if( color < 256 )
      pen.setColor( TQColor( 255-color, 255, 0 ) );
    else
      pen.setColor( TQColor( color-256, 511-color, 0 ) );
    
    painter.setPen( pen );
    painter.drawLine( i, 1, i, size-1 );
  }
  
  pen.setColor( TQt::black );  
  painter.setPen( pen );
  painter.drawRect( rect );
  painter.end();
  pixmap.detach();
  return pixmap;
}

void DULines::sectionResized( int column )
{
  if( childCount() == 0 || column != 0 )
    return;
    
  Directory * currentDir = diskUsage->getCurrentDir();  
  if( currentDir == 0 )
    return;

  int maxPercent = -1;  
  for( Iterator<File> it = currentDir->iterator(); it != currentDir->end(); ++it )
  {
    File *item = *it;  
    
    if( !item->isExcluded() && item->intPercent() > maxPercent )
      maxPercent = item->intPercent();
  }
  
  DULinesItem *duItem = (DULinesItem *)firstChild();
  while( duItem )
  {
    if( duItem->text( 0 ) != ".." )
      duItem->setPixmap( 0, createPixmap( duItem->getFile()->intPercent(), maxPercent, columnWidth( 0 ) ) );
    duItem = (DULinesItem *)duItem->nextSibling();
  }
}

bool DULines::doubleClicked( TQListViewItem * item )
{
  if( item )
  {
    if( item->text( 0 ) != ".." )
    {
      File *fileItem = ((DULinesItem *)item)->getFile();
      if( fileItem->isDir() )
        diskUsage->changeDirectory( dynamic_cast<Directory *> ( fileItem ) );
      return true;
    }
    else
    {
      Directory *upDir = (Directory *)diskUsage->getCurrentDir()->parent();
    
      if( upDir )
        diskUsage->changeDirectory( upDir );
      return true;
    }
  }
  return false;
}

void DULines::contentsMouseDoubleClickEvent ( TQMouseEvent * e )
{
  if ( e || e->button() == Qt::LeftButton )
  {
    TQPoint vp = contentsToViewport(e->pos());
    TQListViewItem * item = itemAt( vp );

    if( doubleClicked( item ) )
      return;
    
  }
  TQListView::contentsMouseDoubleClickEvent( e );
}


void DULines::keyPressEvent( TQKeyEvent *e )
{
  switch ( e->key() )
  {
  case Key_Return :
  case Key_Enter :
    if( doubleClicked( currentItem() ) )
      return;
    break;
  case Key_Left :
  case Key_Right :
  case Key_Up :
  case Key_Down :
    if( e->state() == ShiftButton )
    {
      e->ignore();
      return;
    }
    break;
  case Key_Delete :
    e->ignore();
    return;
  }
  TQListView::keyPressEvent( e );
}
 
void DULines::slotRightClicked( TQListViewItem *item )
{
  File * file = 0;
  
  if ( item && item->text( 0 ) != ".." )
    file = ((DULinesItem *)item)->getFile();

  TDEPopupMenu linesPopup;    
  int lid = linesPopup.insertItem( i18n("Show file sizes"), this, TQT_SLOT( slotShowFileSizes() ) );
  linesPopup.setItemChecked( lid, showFileSize );
    
  diskUsage->rightClickMenu( file, &linesPopup, i18n( "Lines" ) );
}

void DULines::slotShowFileSizes()
{
  showFileSize = !showFileSize;
  slotDirChanged( diskUsage->getCurrentDir() );
}

File * DULines::getCurrentFile()
{
  TQListViewItem *item = currentItem();
  
  if( item == 0 || item->text( 0 ) == ".." )
    return 0;
  
  return ((DULinesItem *)item)->getFile();
}

void DULines::slotChanged( File * item )
{
  TQListViewItem *lvitem = firstChild();
  while( lvitem )
  {
    if( lvitem->text( 0 ) != ".." ) {
      DULinesItem *duItem = (DULinesItem *)( lvitem );
      if( duItem->getFile() == item )
      {
        duItem->setVisible( !item->isExcluded() );
        duItem->setText( 1, item->percent() );
        if( !refreshNeeded )
        {
          refreshNeeded = true;
          TQTimer::singleShot( 0, this, TQT_SLOT( slotRefresh() ) );
        }
        break;
      }
    }
    lvitem = lvitem->nextSibling();
  }
}

void DULines::slotDeleted( File * item )
{
  TQListViewItem *lvitem = firstChild();
  while( lvitem )
  {
    if( lvitem->text( 0 ) != ".." ) {
      DULinesItem *duItem = (DULinesItem *)( lvitem );
      if( duItem->getFile() == item )
      {
        delete duItem;
        break;
      }
    }
    lvitem = lvitem->nextSibling();
  }
}

#include "dulines.moc"
