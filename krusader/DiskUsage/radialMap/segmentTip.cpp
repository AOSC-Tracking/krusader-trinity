//Author:    Max Howell <max.howell@methylblue.com>, (C) 2003-4
//Copyright: See COPYING file that comes with this distribution

#include "fileTree.h"
#include "segmentTip.h"

#include <tdeapplication.h>    //installing eventFilters
#include <tdeglobal.h>
#include <tdeglobalsettings.h>
#include <tdelocale.h>
#include <kpixmapeffect.h>
#include <tqpainter.h>
#include <tqtooltip.h>        //for its palette



namespace RadialMap {

SegmentTip::SegmentTip( uint h )
  : TQWidget( 0, 0, WNoAutoErase | WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WStyle_StaysOnTop | WX11BypassWM )
  , m_cursorHeight( -h )
{
   setBackgroundMode( TQt::NoBackground );
}

void
SegmentTip::moveto( TQPoint p, const TQWidget &canvas, bool placeAbove )
{
  //**** this function is very slow and seems to be visibly influenced by operations like mapFromGlobal() (who knows why!)
  //  ** so any improvements are much desired

  //TODO uints could improve the class
  p.rx() -= rect().center().x();
  p.ry() -= (placeAbove ? 8 + height() : m_cursorHeight - 8);

  const TQRect screen = TDEGlobalSettings::desktopGeometry( parentWidget() );

  const int x  = p.x();
  const int y  = p.y();
  const int x2 = x + width();
  const int y2 = y + height(); //how's it ever gunna get below screen height?! (well you never know I spose)
  const int sw = screen.width();
  const int sh = screen.height();

  if( x  < 0  ) p.setX( 0 );
  if( y  < 0  ) p.setY( 0 );
  if( x2 > sw ) p.rx() -= x2 - sw;
  if( y2 > sh ) p.ry() -= y2 - sh;


  //I'm using this TQPoint to determine where to offset the bitBlt in m_pixmap
  TQPoint offset = canvas.mapToGlobal( TQPoint() ) - p;
  if( offset.x() < 0 ) offset.setX( 0 );
  if( offset.y() < 0 ) offset.setY( 0 );


  const TQRect alphaMaskRect( canvas.mapFromGlobal( p ), size() );
  const TQRect intersection( alphaMaskRect.intersect( canvas.rect() ) );

  m_pixmap.resize( size() ); //move to updateTip once you are sure it can never be null
  bitBlt( &m_pixmap, offset, &canvas, intersection, TQt::CopyROP );

  TQPainter paint( &m_pixmap );
    paint.setPen( TQt::black );
    paint.setBrush( TQt::NoBrush );
    paint.drawRect( rect() );
    paint.end();

  m_pixmap = KPixmapEffect::fade( m_pixmap, 0.6, TQToolTip::palette().color( TQPalette::Active, TQColorGroup::Background ) );

  paint.begin( &m_pixmap );
  paint.drawText( rect(), AlignCenter, m_text );
  paint.end();

  p += screen.topLeft(); //for Xinerama users

  move( x, y );
  show();
  update();
}

void
SegmentTip::updateTip( const File* const file, const Directory* const root )
{
    const TQString s1  = file->fullPath( root );
    TQString s2        = file->humanReadableSize();
    TDELocale *loc      = TDEGlobal::locale();
    const uint MARGIN = 3;
    const uint pc     = 100 * file->size() / root->size();
    uint maxw         = 0;
    uint h            = fontMetrics().height()*2 + 2*MARGIN;

    if( pc > 0 ) s2 += TQString( " (%1%)" ).arg( loc->formatNumber( pc, 0 ) );

    m_text  = s1;
    m_text += '\n';
    m_text += s2;

    if( file->isDir() )
    {
        double files  = static_cast<const Directory*>(file)->fileCount();
        const uint pc = uint((100 * files) / (double)root->fileCount());
        TQString s3    = i18n( "Files: %1" ).arg( loc->formatNumber( files, 0 ) );

        if( pc > 0 ) s3 += TQString( " (%1%)" ).arg( loc->formatNumber( pc, 0 ) );

        maxw    = fontMetrics().width( s3 );
        h      += fontMetrics().height();
        m_text += '\n';
        m_text += s3;
    }

    uint
    w = fontMetrics().width( s1 ); if( w > maxw ) maxw = w;
    w = fontMetrics().width( s2 ); if( w > maxw ) maxw = w;

    resize( maxw + 2 * MARGIN, h );
}

bool
SegmentTip::event( TQEvent *e )
{
    switch( e->type() )
    {
    case TQEvent::Show:
        kapp->installEventFilter( this );
        break;
    case TQEvent::Hide:
        kapp->removeEventFilter( this );
        break;
    case TQEvent::Paint:
    {
        //TQPainter( this ).drawPixmap( 0, 0, m_pixmap );
        bitBlt( this, 0, 0, &m_pixmap );
        return true;
    }
    default:
        ;
    }

    return false/*TQWidget::event( e )*/;
}

bool
SegmentTip::eventFilter( TQObject*, TQEvent *e )
{
    switch ( e->type() )
    {
    case TQEvent::Leave:
//    case TQEvent::MouseButtonPress:
//    case TQEvent::MouseButtonRelease:
    case TQEvent::KeyPress:
    case TQEvent::KeyRelease:
    case TQEvent::FocusIn:
    case TQEvent::FocusOut:
    case TQEvent::Wheel:
        hide(); //FALL THROUGH
    default:
        return false; //allow this event to passed to target
    }
}

} //namespace RadialMap
