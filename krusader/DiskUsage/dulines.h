/***************************************************************************
                           dulines.h  -  description
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

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __DU_LINES_H__
#define __DU_LINES_H__

#include <tqlistview.h>
#include <tqpixmap.h>
#include "diskusage.h"

class DULinesToolTip;

class DULines : public TQListView
{
  TQ_OBJECT
  
  
public:
  DULines( DiskUsage *usage, const char *name );
  ~DULines();

  File * getCurrentFile();
  
public slots:
  void slotDirChanged( Directory *dirEntry );
  void sectionResized( int );
  void slotRightClicked(TQListViewItem *);
  void slotChanged( File * );
  void slotDeleted( File * );
  void slotShowFileSizes();
  void slotRefresh() { refreshNeeded = false; sectionResized( 0 ); }
  
protected:
  DiskUsage *diskUsage;  
  
  virtual void contentsMouseDoubleClickEvent ( TQMouseEvent * e );
  virtual void keyPressEvent( TQKeyEvent *e );
  
private:
  TQPixmap createPixmap( int percent, int maxPercent, int maxWidth );
  
  bool doubleClicked( TQListViewItem * item );
  
  bool refreshNeeded;
  
  bool showFileSize;
  
  DULinesToolTip *toolTip;
};

#endif /* __DU_LINES_H__ */

