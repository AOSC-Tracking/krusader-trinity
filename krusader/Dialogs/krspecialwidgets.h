/***************************************************************************
                              krspecialwidgets.h
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

                                                   H e a d e r    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#ifndef KRSPECIALWIDGETS_H
#define KRSPECIALWIDGETS_H

#include <tqwidget.h>
#include <tqpainter.h>
#include <tqcolor.h>
#include <tqptrlist.h>
#include <klineedit.h>
#include <tqevent.h>
#include <kio/global.h>

class KRPieSlice;

class KRPie : public TQWidget {
    Q_OBJECT
  
  public:
    KRPie( TDEIO::filesize_t _totalSize, TQWidget *parent = 0 );
    void addSlice( TDEIO::filesize_t size, TQString label );

  protected:
    void paintEvent( TQPaintEvent * );

  private:
    TQList<KRPieSlice> slices;
    TDEIO::filesize_t totalSize, sizeLeft;
    static TQColor colors[ 12 ];
};

class KRFSDisplay : public TQWidget {
    Q_OBJECT
  
  public:
    // this constructor is used for a mounted filesystem
    KRFSDisplay( TQWidget *parent, TQString _alias, TQString _realName,
                 TDEIO::filesize_t _total, TDEIO::filesize_t _free );
    // this one is for an unmounted/supermount file system
    KRFSDisplay( TQWidget *parent, TQString _alias, TQString _realName, bool sm = false );
    // the last one is used inside MountMan(R), when no filesystem is selected
    KRFSDisplay( TQWidget *parent );
    inline void setTotalSpace( TDEIO::filesize_t t ) { totalSpace = t; }
    inline void setFreeSpace( TDEIO::filesize_t t ) { freeSpace = t; }
    inline void setAlias( TQString a ) { alias = a; }
    inline void setRealName( TQString r ) { realName = r; }
    inline void setMounted( bool m ) { mounted = m; }
    inline void setEmpty( bool e ) { empty = e; }
    inline void setSupermount( bool s ) { supermount = s; }

  protected:
    void paintEvent( TQPaintEvent * );

  private:
    TDEIO::filesize_t totalSpace, freeSpace;
    TQString alias, realName;
    bool mounted, empty, supermount;
};

class KRPieSlice {
  public:
    KRPieSlice( float _perct, TQColor _color, TQString _label ) :
    perct( _perct ), color( _color ), label( _label ) {}
    inline TQColor getColor() { return color; }
    inline float getPerct() { return perct; }
    inline TQString getLabel() { return label; }
    inline void setPerct( float _perct ) { perct = _perct; }
    inline void setLabel( TQString _label ) { label = _label; }

  private:
    float perct;
    TQColor color;
    TQString label;
};

class KrQuickSearch: public KLineEdit {
      Q_OBJECT
  
   public:
      KrQuickSearch(TQWidget *parent, const char * name = 0);
      void addText(const TQString &str) { setText(text()+str); }
      void myKeyPressEvent(TQKeyEvent *e);
      void myIMStartEvent(TQIMEvent* e) {
        imStartEvent(e);
      }
      void myIMEndEvent(TQIMEvent* e) {
        imEndEvent(e);
      }
      void myIMComposeEvent(TQIMEvent* e) {
        imComposeEvent(e);
      }

   signals:
      void stop(TQKeyEvent *e);
      void process(TQKeyEvent *e);
      void otherMatching(const TQString &, int);

};

#endif
