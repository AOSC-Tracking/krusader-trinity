//Author:    Max Howell <max.howell@methylblue.com>, (C) 2003-4
//Copyright: See COPYING file that comes with this distribution

#include <kcursor.h>        //ctor
#include <tdelocale.h>
#include <kurl.h>
#include <tqapplication.h>   //sendEvent
#include <tqbitmap.h>        //ctor - finding cursor size
#include <tqcursor.h>        //slotPostMouseEvent()
#include <tqtimer.h>         //member

#include "Config.h"
#include "debug.h"
#include "fileTree.h"
#include "radialMap.h" //constants
#include "widget.h"



RadialMap::Widget::Widget( TQWidget *parent, const char *name )
   : TQWidget( parent, name, TQt::WNoAutoErase )
   , m_tree( 0 )
   , m_focus( 0 )
   , m_tip( KCursor::handCursor().bitmap()->height() ) //needs to know cursor height
   , m_rootSegment( 0 ) //TODO we don't delete it, *shrug*
{
   setBackgroundColor( TQt::white );

   connect( this, TQ_SIGNAL(created( const Directory* )), TQ_SLOT(sendFakeMouseEvent()) );
   connect( this, TQ_SIGNAL(created( const Directory* )), TQ_SLOT(update()) );
   connect( &m_timer, TQ_SIGNAL(timeout()), TQ_SLOT(resizeTimeout()) );
}

TQString
RadialMap::Widget::path() const
{
   if( m_tree == 0 )
      return TQString();
   return m_tree->fullPath();
}

KURL
RadialMap::Widget::url( File const * const file ) const
{
   if( file == 0 && m_tree == 0 )
      return KURL();

   return KURL::fromPathOrURL( file ? file->fullPath() : m_tree->fullPath() );
}

void
RadialMap::Widget::invalidate( const bool b )
{
   if( isValid() )
   {
      //**** have to check that only way to invalidate is this function frankly
      //**** otherwise you may get bugs..

      //disable mouse tracking
      setMouseTracking( false );

      KURL urlInv = url();

      //ensure this class won't think we have a map still
      m_tree  = 0;
      m_focus = 0;

      delete m_rootSegment;
      m_rootSegment = 0;

      //FIXME move this disablement thing no?
      //      it is confusing in other areas, like the whole createFromCache() thing
      m_map.invalidate( b ); //b signifies whether the pixmap is made to look disabled or not
      if( b )
         update();

      //tell rest of Filelight
      emit invalidated( urlInv );
   }
}

void
RadialMap::Widget::create( const Directory *tree )
{
   //it is not the responsibility of create() to invalidate first
   //skip invalidation at your own risk

   //FIXME make it the responsibility of create to invalidate first

   if( tree )
   {
      m_focus = 0;
      //generate the filemap image
      m_map.make( tree );

      //this is the inner circle in the center
      m_rootSegment = new Segment( tree, 0, 16*360 );

      setMouseTracking( true );
   }

   m_tree = tree;

   //tell rest of Filelight
   emit created( tree );
}

void
RadialMap::Widget::createFromCache( const Directory *tree )
{
    //no scan was necessary, use cached tree, however we MUST still emit invalidate
    invalidate( false );
    create( tree );
}

void
RadialMap::Widget::sendFakeMouseEvent() //slot
{
   TQMouseEvent me( TQEvent::MouseMove, mapFromGlobal( TQCursor::pos() ), TQt::NoButton, TQt::NoButton );
   TQApplication::sendEvent( this, &me );
}

void
RadialMap::Widget::resizeTimeout() //slot
{
   // the segments are about to erased!
   // this was a horrid bug, and proves the OO programming should be obeyed always!
   m_focus = 0;
   if( m_tree )
      m_map.make( m_tree, true );
   update();
}

void
RadialMap::Widget::refresh( int filth )
{
   //TODO consider a more direct connection

   if( !m_map.isNull() )
   {
      switch( filth )
      {
      case 1:
         m_focus = 0;
         if( m_tree )
             m_map.make( m_tree, true ); //true means refresh only
         break;

      case 2:
         m_map.aaPaint();
         break;

      case 3:
         m_map.colorise(); //FALL THROUGH!
      case 4:
         m_map.paint();

      default:
         break;
      }

      update();
   }
}

void
RadialMap::Widget::zoomIn() //slot
{
   if( m_map.m_visibleDepth > MIN_RING_DEPTH )
   {
      m_focus = 0;
      --m_map.m_visibleDepth;
      if( m_tree )
          m_map.make( m_tree );
      Config::defaultRingDepth = m_map.m_visibleDepth;
      update();
   }
}

void
RadialMap::Widget::zoomOut() //slot
{
   m_focus = 0;
   ++m_map.m_visibleDepth;
   if( m_tree )
       m_map.make( m_tree );
   if( m_map.m_visibleDepth > Config::defaultRingDepth )
      Config::defaultRingDepth = m_map.m_visibleDepth;
   update();
}


RadialMap::Segment::~Segment()
{
   if( isFake() )
      delete m_file; //created by us in Builder::build()
}

#include "widget.moc"
