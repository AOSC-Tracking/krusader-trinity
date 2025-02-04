//Author:    Max Howell <max.howell@methylblue.com>, (C) 2003-4
//Copyright: See COPYING file that comes with this distribution

#include "fileTree.h"
#include "radialMap.h"   //class Segment
#include "widget.h"

#include <kcursor.h>     //::mouseMoveEvent()
#include <kiconeffect.h> //::mousePressEvent()
#include <kiconloader.h> //::mousePressEvent()
#include <tdeio/job.h>     //::mousePressEvent()
#include <tdelocale.h>
#include <tdemessagebox.h> //::mousePressEvent()
#include <tdepopupmenu.h>  //::mousePressEvent()
#include <krun.h>        //::mousePressEvent()
#include <math.h>        //::segmentAt()
#include <tqapplication.h>//TQApplication::setOverrideCursor()
#include <tqpainter.h>
#include <tqtimer.h>      //::resizeEvent()



void
RadialMap::Widget::resizeEvent( TQResizeEvent* )
{
    if( m_map.resize( rect() ) )
       m_timer.start( 500, true ); //will cause signature to rebuild for new size

    //always do these as they need to be initialised on creation
    m_offset.rx() = (width() - m_map.width()) / 2;
    m_offset.ry() = (height() - m_map.height()) / 2;
}

void
RadialMap::Widget::paintEvent( TQPaintEvent* )
{
    //bltBit for some TQt setups will bitBlt _after_ the labels are painted. Which buggers things up!
    //shame as bitBlt is faster, possibly TQt bug? Should report the bug? - seems to be race condition
    //bitBlt( this, m_offset, &m_map );

    TQPainter paint( this );

    paint.drawPixmap( m_offset, m_map );

    //vertical strips
    if( m_map.width() < width() )
    {
        paint.eraseRect( 0, 0, m_offset.x(), height() );
        paint.eraseRect( m_map.width() + m_offset.x(), 0, m_offset.x() + 1, height() );
    }
    //horizontal strips
    if( m_map.height() < height() )
    {
        paint.eraseRect( 0, 0, width(), m_offset.y() );
        paint.eraseRect( 0, m_map.height() + m_offset.y(), width(), m_offset.y() + 1 );
    }

    //exploded labels
    if( !m_map.isNull() && !m_timer.isActive() )
       paintExplodedLabels( paint );
}

const RadialMap::Segment*
RadialMap::Widget::segmentAt( TQPoint &e ) const
{
    //determine which segment TQPoint e is above

    e -= m_offset;

    if( e.x() <= m_map.width() && e.y() <= m_map.height() )
    {
        //transform to cartesian coords
        e.rx() -= m_map.width() / 2; //should be an int
        e.ry()  = m_map.height() / 2 - e.y();

        double length = hypot( e.x(), e.y() );

        if( length >= m_map.m_innerRadius ) //not hovering over inner circle
        {
            uint depth  = ((int)length - m_map.m_innerRadius) / m_map.m_ringBreadth;

            if( depth <= m_map.m_visibleDepth ) //**** do earlier since you can //** check not outside of range
            {
                //vector calculation, reduces to simple trigonometry
                //cos angle = (aibi + ajbj) / albl
                //ai = x, bi=1, aj=y, bj=0
                //cos angle = x / (length)

                uint a  = (uint)(acos( (double)e.x() / length ) * 916.736);  //916.7324722 = #radians in circle * 16

                //acos only understands 0-180 degrees
                if( e.y() < 0 ) a = 5760 - a;

                #define ring (m_map.m_signature + depth)
                for( ConstIterator<Segment> it = ring->constIterator(); it != ring->end(); ++it )
                    if( (*it)->intersects( a ) )
                        return *it;
                #undef ring
            }
        }
        else return m_rootSegment; //hovering over inner circle
    }

    return 0;
}

void
RadialMap::Widget::mouseMoveEvent( TQMouseEvent *e )
{
   //set m_focus to what we hover over, update UI if it's a new segment

   Segment const * const oldFocus = m_focus;
   TQPoint p = e->pos();

   m_focus = segmentAt( p ); //NOTE p is passed by non-const reference

   if( m_focus && m_focus->file() != m_tree )
   {
      if( m_focus != oldFocus ) //if not same as last time
      {
         setCursor( KCursor::handCursor() );
         m_tip.updateTip( m_focus->file(), m_tree );
         emit mouseHover( m_focus->file()->fullPath() );

         //repaint required to update labels now before transparency is generated
         repaint( false );
      }

      m_tip.moveto( e->globalPos(), *this, ( p.y() < 0 ) ); //updates tooltip psuedo-tranparent background
   }
   else if( oldFocus && oldFocus->file() != m_tree )
   {
      unsetCursor();
      m_tip.hide();
      update();

      emit mouseHover( TQString() );
   }
}

void
RadialMap::Widget::mousePressEvent( TQMouseEvent *e )
{
   //m_tip is hidden already by event filter
   //m_focus is set correctly (I've been strict, I assure you it is correct!)

   if( m_focus && !m_focus->isFake() )
   {
      const KURL url   = Widget::url( m_focus->file() );
      const bool isDir = m_focus->file()->isDir();

      if( e->button() == TQt::RightButton )
      {
         TDEPopupMenu popup;
         popup.insertTitle( m_focus->file()->fullPath( m_tree ) );

         if( isDir )
         {
            popup.insertItem( SmallIconSet( "konqueror" ), i18n( "Open &Konqueror Here" ), 0 );
            if( url.protocol() == "file" )
               popup.insertItem( SmallIconSet( "konsole" ), i18n( "Open &Konsole Here" ), 1 );

            if( m_focus->file() != m_tree )
            {
               popup.insertSeparator();
               popup.insertItem( SmallIconSet( "viewmag" ), i18n( "&Center Map Here" ), 2 );
            }
         }
         else popup.insertItem( SmallIconSet( "document-open" ), i18n( "&Open" ), 3 );

         popup.insertSeparator();
         popup.insertItem( SmallIconSet( "edit-delete" ), i18n( "&Delete" ), 4 );

         switch( popup.exec( e->globalPos(), 1 ) ) {
         case 0:
            //KRun::runCommand will show an error message if there was trouble
            KRun::runCommand( TQString( "kfmclient openURL '%1'" ).arg( url.url() ) );
            break;

         case 1:
            KRun::runCommand( TQString( "konsole --workdir '%1'" ).arg( url.url() ) );
            break;

         case 2:
         case 3:
            goto sectionTwo;

         case 4:
         {
            const KURL url = Widget::url( m_focus->file() );
            const TQString message = ( m_focus->file()->isDir()
               ? i18n( "<qt>The directory at <i>'%1'</i> will be <b>recursively</b> and <b>permanently</b> deleted!</qt>" )
               : i18n( "<qt><i>'%1'</i> will be <b>permanently</b> deleted!</qt>" )).arg( url.prettyURL() );
            const int userIntention = KMessageBox::warningContinueCancel( this, message, TQString(), KGuiItem( i18n("&Delete"), "edit-delete" ) );

            if( userIntention == KMessageBox::Continue ) {
               TDEIO::Job *job = TDEIO::del( url );
               job->setWindow( this );
               connect( job, TQ_SIGNAL(result( TDEIO::Job* )), TQ_SLOT(deleteJobFinished( TDEIO::Job* )) );
               TQApplication::setOverrideCursor( KCursor::workingCursor() );
            }
         }

         default:
            //ensure m_focus is set for new mouse position
            sendFakeMouseEvent();
         }

      } else {

      sectionTwo:

         const TQRect rect( e->x() - 20, e->y() - 20, 40, 40 );
         
         m_tip.hide(); //user expects this

         if( !isDir || e->button() == TQt::MidButton )
         {
            TDEIconEffect::visualActivate( this, rect );
            new KRun( url, this, true ); //FIXME see above
         }
         else if( m_focus->file() != m_tree ) //is left mouse button
         {
            TDEIconEffect::visualActivate( this, rect );
            emit activated( url ); //activate first, this will cause UI to prepare itself
            if( m_focus )
              createFromCache( (Directory *)m_focus->file() );
         }
      }
   }
}

void
RadialMap::Widget::deleteJobFinished( TDEIO::Job *job )
{
   TQApplication::restoreOverrideCursor();
   if( !job->error() )
      invalidate();
   else
      job->showErrorDialog( this );
}
