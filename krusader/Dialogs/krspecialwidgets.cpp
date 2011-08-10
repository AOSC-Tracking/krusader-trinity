/***************************************************************************
                            krspecialwidgets.cpp
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



#include "krspecialwidgets.h"
#include "krmaskchoice.h"
#include "newftpgui.h"
#include "../krusader.h"
#include "../MountMan/kmountman.h"
#include <math.h>
#include <kfileitem.h>
#include <klocale.h>
#include <klineedit.h>
#include <kdebug.h>

/////////////////////////////////////////////////////////////////////////////
/////////////////////// Pie related widgets /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// The pie-related widgets use hard-coded coordinates to create the look.
// This is ok since the whole widget is fitted into an existing view and thus
// no re-alignments are needed.

#define LEFT       10
#define BOTTOM     150
#define WIDTH      120
#define HEIGHT     40
#define Z_HEIGHT   10
#define STARTANGLE 0
#define DEG(x)     (16*(x))

TQColor KRPie::colors[ 12 ] = {TQt::red, TQt::blue, TQt::green, TQt::cyan, TQt::magenta, TQt::gray,
                              TQt::black, TQt::white, TQt::darkRed, TQt::darkBlue, TQt::darkMagenta,
                              TQt::darkCyan};

//////////////////////////////////////////////////////////////////////////////
/////////////// KRFSDisplay - Filesystem / Freespace Display /////////////////
//////////////////////////////////////////////////////////////////////////////
// This is the full constructor: use it for a mounted filesystem
KRFSDisplay::KRFSDisplay( TQWidget *parent, TQString _alias, TQString _realName,
                          KIO::filesize_t _total, KIO::filesize_t _free ) : TQWidget( parent ), totalSpace( _total ),
      freeSpace( _free ), alias( _alias ), realName( _realName ), mounted( true ),
empty( false ), supermount( false ) {
   resize( 150, 200 );
   show();
}

// Use this one for an unmounted filesystem
KRFSDisplay::KRFSDisplay( TQWidget *parent, TQString _alias, TQString _realName, bool sm ) :
      TQWidget( parent ), alias( _alias ), realName( _realName ), mounted( false ),
empty( false ), supermount( sm ) {
   resize( 150, 200 );
   show();
}

// This is used only when an empty widget needs to be displayed (for example:
// when filesystem statistics haven't been calculated yet)
KRFSDisplay::KRFSDisplay( TQWidget *parent ) : TQWidget( parent ), empty( true ) {
   resize( 150, 200 );
   show();
}


// The main painter!
void KRFSDisplay::paintEvent( TQPaintEvent * ) {
   TQPainter paint( this );
   if ( !empty ) {
      // create the text
      // first, name and location
      paint.setFont( TQFont( "helvetica", 12, TQFont::Bold ) );
      paint.drawText( 10, 20, alias );
      paint.setFont( TQFont( "helvetica", 12, TQFont::Normal ) );
      paint.drawText( 10, 37, "(" + realName + ")" );
      if ( mounted ) {  // incase the filesystem is already mounted
         // second, the capacity
         paint.drawText( 10, 70, i18n( "Capacity: " ) + KIO::convertSizeFromKB( totalSpace ) );
         // third, the 2 boxes (used, free)
         TQPen systemPen = paint.pen();
         paint.setPen( TQt::black );
         paint.drawRect( 10, 90, 10, 10 );
         paint.fillRect( 11, 91, 8, 8, TQBrush( TQt::gray ) );
         paint.drawRect( 10, 110, 10, 10 );
         paint.fillRect( 11, 111, 8, 8, TQBrush( TQt::white ) );
         // now, the text for the boxes
         paint.setPen( systemPen );
         paint.drawText( 25, 100, i18n( "Used: " ) + KIO::convertSizeFromKB( totalSpace - freeSpace ) );
         paint.drawText( 25, 120, i18n( "Free: " ) + KIO::convertSizeFromKB( freeSpace ) );
         // first, create the empty pie
         // bottom...
         paint.setPen( TQt::black );
         paint.setBrush( TQt::white );
         paint.drawPie( LEFT, BOTTOM, WIDTH, HEIGHT, STARTANGLE, DEG( 360 ) );
         // body...
         paint.setPen( TQt::lightGray );
         for ( int i = 1; i < Z_HEIGHT; ++i )
            paint.drawPie( LEFT, BOTTOM - i, WIDTH, HEIGHT, STARTANGLE, DEG( 360 ) );
         // side lines...
         paint.setPen( TQt::black );
         paint.drawLine( LEFT, BOTTOM + HEIGHT / 2, LEFT, BOTTOM + HEIGHT / 2 - Z_HEIGHT );
         paint.drawLine( LEFT + WIDTH, BOTTOM + HEIGHT / 2, LEFT + WIDTH, BOTTOM + HEIGHT / 2 - Z_HEIGHT );
         // top of the pie
         paint.drawPie( LEFT, BOTTOM - Z_HEIGHT, WIDTH, HEIGHT, STARTANGLE, DEG( 360 ) );
         // the "used space" slice
         float i = ( ( float ) ( totalSpace - freeSpace ) / ( totalSpace ) ) * 360.0;
         paint.setBrush( TQt::gray );
         paint.drawPie( LEFT, BOTTOM - Z_HEIGHT, WIDTH, HEIGHT, STARTANGLE, ( int ) DEG( i ) );
         // if we need to draw a 3d stripe ...
         if ( i > 180.0 ) {
            for ( int j = 1; j < Z_HEIGHT; ++j )
               paint.drawArc( LEFT, BOTTOM - j, WIDTH, HEIGHT, STARTANGLE - 16 * 180, ( int ) ( DEG( i - 180.0 ) ) );
         }
      } else {  // if the filesystem is unmounted...
         paint.setFont( TQFont( "helvetica", 12, TQFont::Bold ) );
         paint.drawText( 10, 60, i18n( "Not mounted." ) );
      }
   } else {  // if the widget is in empty situation...

   }
}

////////////////////////////////////////////////////////////////////////////////
KRPie::KRPie( KIO::filesize_t _totalSize, TQWidget *parent ) : TQWidget( parent, 0 ), totalSize( _totalSize ) {
   slices.setAutoDelete( true ); // kill items when they are removed
   slices.append( new KRPieSlice( 100, TQt::yellow, "DEFAULT" ) );
   sizeLeft = totalSize;
   resize( 300, 300 );
}

void KRPie::paintEvent( TQPaintEvent * ) {
   TQPainter paint( this );
   // now create the slices
   KRPieSlice *slice;
   float sAngle = STARTANGLE;
   for ( slice = slices.first(); slice != 0; slice = slices.next() ) {
      paint.setBrush( slice->getColor() );
      paint.setPen( slice->getColor() );
      // angles are negative to create a clock-wise drawing of slices
      float angle = -( slice->getPerct() / 100 * 360 ) * 16;
      for ( int i = 1; i < Z_HEIGHT; ++i )
         paint.drawPie( LEFT, BOTTOM + i, WIDTH, HEIGHT, ( int ) sAngle, ( int ) angle );
      sAngle += angle;
   }
   paint.setPen( TQt::yellow );   // pen
   paint.setBrush( TQt::yellow ); // fill
   // for (int i=1; i<Z_HEIGHT; ++i)
   //  paint.drawPie(LEFT,BOTTOM+i,WIDTH,HEIGHT,sAngle,360*16-(STARTANGLE-sAngle));
   sAngle = STARTANGLE;
   for ( slice = slices.first(); slice != 0; slice = slices.next() ) {
      paint.setBrush( slice->getColor() );
      paint.setPen( slice->getColor() );
      // angles are negative to create a clock-wise drawing of slices
      float angle = -( slice->getPerct() / 100 * 360 ) * 16;
      paint.drawPie( LEFT, BOTTOM, WIDTH, HEIGHT, ( int ) sAngle, ( int ) angle );
      sAngle += angle;
   }


   paint.setPen( TQt::black );
   // the pie
   //  paint.drawPie(LEFT,BOTTOM,WIDTH,HEIGHT,STARTANGLE,360*16);
   ///////////////////////// end of empty pie /////////////////////////
   // now, the pie is ready to draw slices on...
   // to make a good look on the perimiter, draw another black circle
   paint.setPen( TQt::black );
   paint.drawArc( LEFT, BOTTOM, WIDTH, HEIGHT, STARTANGLE, 360 * 16 );

}

void KRPie::addSlice( KIO::filesize_t size, TQString label ) {
   int i = ( slices.count() % 12 );
   slices.removeLast();
   slices.append( new KRPieSlice( size * 100 / totalSize, colors[ i ], label ) );
   sizeLeft -= size;
   slices.append( new KRPieSlice( sizeLeft * 100 / totalSize, TQt::yellow, "DEFAULT" ) );
}

////////////////////////////////////////////////////
/////////////////// KrQuickSearch  /////////////////
////////////////////////////////////////////////////
KrQuickSearch::KrQuickSearch( TQWidget *parent, const char * name ) : KLineEdit( parent, name ) {}

void KrQuickSearch::myKeyPressEvent( TQKeyEvent *e ) {
   switch ( e->key() ) {
         case Key_Escape:
         emit stop( 0 );
         break;
         case Key_Return:
         case Key_Enter:
         case Key_Tab:
         case Key_Right:
         case Key_Left:
         emit stop( e );
         break;
         case Key_Down:
         otherMatching( text(), 1 );
         break;
         case Key_Up:
         otherMatching( text(), -1 );
         break;
         case Key_Insert:
         case Key_Home:
         case Key_End:
         process( e );
         break;
         default:
         keyPressEvent( e );
   }
}



#include "krspecialwidgets.moc"
