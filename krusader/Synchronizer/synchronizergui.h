/***************************************************************************
                       synchronizergui.h  -  description
                             -------------------
    copyright            : (C) 2003 + by Csaba Karai
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

#ifndef __SYNCHRONIZERGUI_H__
#define __SYNCHRONIZERGUI_H__

#include "synchronizer.h"
#include "../GUI/profilemanager.h"
#include "../Filter/filtertabs.h"
#include "../Filter/generalfilter.h"
#include <tqdialog.h>
#include <tqlistview.h>
#include <kcombobox.h>
#include <tqpixmap.h>
#include <tqcheckbox.h>
#include <tqmap.h>
#include <tqlabel.h>
#include <tqtabwidget.h>

class TQSpinBox;

class SynchronizerGUI : TQDialog
{
   TQ_OBJECT
  

public:
  class SyncViewItem : public TQListViewItem
  {
    private:
      SynchronizerFileItem *syncItemRef;
      SyncViewItem         *lastItemRef;
      TQColor                textColor;
      TQColor                baseColor;
            
    public:
      SyncViewItem( SynchronizerFileItem *item, TQColor txt, TQColor base, TQListView * parent, TQListViewItem *after, TQString label1,
                    TQString label2 = TQString(), TQString label3 = TQString(), TQString label4 = TQString(),
                    TQString label5 = TQString(), TQString label6 = TQString(),
                    TQString label7 = TQString(), TQString label8 = TQString() ) :
                      TQListViewItem( parent, after, label1, label2, label3, label4, label5, label6,
                                     label7, label8 ), syncItemRef( item ), lastItemRef( 0 ), textColor( txt ), baseColor( base )
      {
        item->setUserData( (void *)this );
        setDragEnabled( true );
      }
      
      SyncViewItem( SynchronizerFileItem *item, TQColor txt, TQColor base, TQListViewItem * parent, TQListViewItem *after, TQString label1,
                    TQString label2 = TQString(), TQString label3 = TQString(), TQString label4 = TQString(),
                    TQString label5 = TQString(), TQString label6 = TQString(),
                    TQString label7 = TQString(), TQString label8 = TQString() ) :
                      TQListViewItem( parent, after, label1, label2, label3, label4, label5, label6,
                                     label7, label8 ), syncItemRef( item ), lastItemRef( 0 ), textColor( txt ), baseColor( base )
      {
        item->setUserData( (void *)this );
        setDragEnabled( true );
      }

      ~SyncViewItem()
      {
        syncItemRef->setUserData( 0 );
      }

      inline SynchronizerFileItem * synchronizerItemRef()       {return syncItemRef;}
      inline SyncViewItem         * lastItem()                  {return lastItemRef;}
      inline void                   setLastItem(SyncViewItem*s) {lastItemRef = s;}
      
      void setColors( TQColor fore, TQColor back ) {
        textColor = fore;
        baseColor = back;
      }
      
      void paintCell(TQPainter *p, const TQColorGroup &cg, int column, int width, int align)
      {
        TQColorGroup _cg = cg;
        if( textColor.isValid() )
          _cg.setColor(TQColorGroup::Text, textColor );
        if( baseColor.isValid() )
          _cg.setColor(TQColorGroup::Base, baseColor );
        TQListViewItem::paintCell( p, _cg, column, width, align );
      };
  };
   
public:
  // if rightDirectory is null, leftDirectory is actually the profile name to load
  SynchronizerGUI(TQWidget* parent,  KURL leftDirectory, KURL rightDirectory = TQString(), TQStringList selList = TQStringList() );
  SynchronizerGUI(TQWidget* parent,  TQString profile );
  ~SynchronizerGUI();

  inline bool wasSynchronization()    {return wasSync;}

public slots:
  void rightMouseClicked(TQListViewItem *);
  void doubleClicked(TQListViewItem *);
  void compare();
  void synchronize();
  void stop();
  void feedToListBox();
  void closeDialog();
  void refresh();
  void swapSides();
  void loadFromProfile( TQString );
  void saveToProfile( TQString );
  
protected slots:
  void reject();
  void addFile( SynchronizerFileItem * );
  void markChanged( SynchronizerFileItem *, bool );
  void setScrolling( bool );
  void statusInfo( TQString );
  void subdirsChecked( bool );
  void setPanelLabels();
  void setCompletion();
  void checkExcludeURLValidity( TQString &text, TQString &error );
  void connectFilters( const TQString & );  

private:
  void initGUI(TQWidget* parent, TQString profile, KURL leftURL, KURL rightURL, TQStringList selList);
  
  TQString convertTime(time_t time) const;
  void    setMarkFlags();
  void    disableMarkButtons();
  void    enableMarkButtons();
  void    copyToClipboard( bool isLeft );
  
  int     convertToSeconds( int time, int unit );
  void    convertFromSeconds( int &time, int &unit, int second );

protected:
  virtual void keyPressEvent( TQKeyEvent * );
  virtual void resizeEvent( TQResizeEvent *e );
  
  ProfileManager *profileManager;
  FilterTabs     *filterTabs;
  GeneralFilter  *generalFilter;
  
  TQTabWidget    *synchronizerTabs;  
  
  KHistoryCombo *leftLocation;
  KHistoryCombo *rightLocation;
  KHistoryCombo *fileFilter;
  
  TQListView     *syncList;
  Synchronizer   synchronizer;
  
  TQCheckBox     *cbSubdirs;
  TQCheckBox     *cbSymlinks;
  TQCheckBox     *cbByContent;
  TQCheckBox     *cbIgnoreDate;
  TQCheckBox     *cbAsymmetric;
  TQCheckBox     *cbIgnoreCase;
  
  TQPushButton   *btnSwapSides;
  TQPushButton   *btnCompareDirs;
  TQPushButton   *btnStopComparing;
  TQPushButton   *btnSynchronize;
  TQPushButton   *btnFeedToListBox;
  TQPushButton   *btnScrollResults;
  
  TQPushButton   *btnLeftToRight;
  TQPushButton   *btnEquals;
  TQPushButton   *btnDifferents;
  TQPushButton   *btnRightToLeft;
  TQPushButton   *btnDeletable;
  TQPushButton   *btnDuplicates;
  TQPushButton   *btnSingles;

  TQLabel        *statusLabel;
  TQLabel        *leftDirLabel;
  TQLabel        *rightDirLabel;
  
  TQStringList    selectedFiles;
  
  TQSpinBox      *parallelThreadsSpinBox;
  TQSpinBox      *equalitySpinBox;
  TQComboBox     *equalityUnitCombo;
  TQSpinBox      *timeShiftSpinBox;
  TQComboBox     *timeShiftUnitCombo;
  TQCheckBox     *ignoreHiddenFilesCB;

private:
  TQPixmap        fileIcon;
  TQPixmap        folderIcon;
  bool           isComparing;
  bool           wasClosed;
  bool           wasSync;
  bool           firstResize;
  bool           hasSelectedFiles;
  SyncViewItem  *lastItem;
  
  int            sizeX;
  int            sizeY;
  
  TQColor         foreGrounds[ TT_MAX ];
  TQColor         backGrounds[ TT_MAX ];
};

#endif /* __SYNCHRONIZERGUI_H__ */
