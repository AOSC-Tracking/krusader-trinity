/***************************************************************************
                          kgcolors.h  -  description
                             -------------------
    copyright            : (C) 2004 + by Csaba Karai
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

#ifndef __KGCOLORS_H__
#define __KGCOLORS_H__

#include "konfiguratorpage.h"
#include <tqptrlist.h>
#include <tqvaluelist.h>
#include <tqlistview.h>
#include <tqwidgetstack.h>

class KgColors : public KonfiguratorPage
{
  Q_OBJECT
  TQ_OBJECT

public:
  KgColors( bool first, TQWidget* parent=0,  const char* name=0 );

  bool apply();

public slots:
  void slotDisable();
  void slotForegroundChanged();
  void slotBackgroundChanged();
  void slotAltBackgroundChanged();
  void slotActiveChanged();
  void slotMarkedBackgroundChanged();
  void slotInactiveForegroundChanged();
  void slotInactiveBackgroundChanged();
  void slotInactiveAltBackgroundChanged();
  void slotInactiveMarkedBackgroundChanged();
  void generatePreview();

protected slots:
  void slotImportColors();
  void slotExportColors();

private:
  class PreviewItem;

  int                        addColorSelector( TQString cfgName, TQString name, TQColor dflt, TQString dfltName = TQString(),
                                               ADDITIONAL_COLOR *addColor = 0, int addColNum = 0);
  KonfiguratorColorChooser  *getColorSelector( TQString name );
  TQLabel                    *getSelectorLabel( TQString name );
  void                       serialize(class TQDataStream &);
  void                       deserialize(class TQDataStream &);
  void                       serializeItem(class TQDataStream &, const char * name);
  void                       setColorWithDimming(PreviewItem * item, TQColor foreground, TQColor background, bool dimmed );

private:
  TQWidget                            *colorsGrp;
  TQGridLayout                        *colorsGrid;
  int                                 offset;
  int                                 endOfActiveColors;
  int                                 endOfPanelColors;

  TQGroupBox                          *previewGrp;
  TQGridLayout                        *previewGrid;
  TQTabWidget                         *colorTabWidget;

  TQWidgetStack                       *inactiveColorStack;
  TQWidget                            *normalInactiveWidget;
  TQWidget                            *dimmedInactiveWidget;
  KonfiguratorSpinBox                *dimFactor;

  KonfiguratorCheckBoxGroup          *generals;

  TQPtrList<TQLabel>                    labelList;
  TQPtrList<KonfiguratorColorChooser>  itemList;
  TQValueList<TQString>                 itemNames;

  TQListView                          *preview;
  KPushButton *importBtn, *exportBtn;

  class PreviewItem : public TQListViewItem
  {
  private:
    TQColor  defaultBackground;
    TQColor  defaultForeground;
    TQString label;

  public:
    PreviewItem( TQListView * parent, TQString name ) : TQListViewItem( parent, name )
    {
      defaultBackground = TQColor( 255, 255, 255 );
      defaultForeground = TQColor( 0, 0, 0 );
      label = name;
    }

    void setColor( TQColor foregnd, TQColor backgnd )
    {
      defaultForeground = foregnd;
      defaultBackground = backgnd;
      listView()->repaintItem( this );
    }

    TQString text()
    {
      return label;
    }

    void paintCell ( TQPainter * p, const TQColorGroup & cg, int column, int width, int align )
    {
      TQColorGroup _cg( cg );
      _cg.setColor( TQColorGroup::Base, defaultBackground );
      _cg.setColor( TQColorGroup::Text, defaultForeground );
      TQListViewItem::paintCell(p, _cg, column, width, align);
    }
  };
};
#endif /* __KGCOLORS_H__ */
