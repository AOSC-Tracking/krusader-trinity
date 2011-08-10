/***************************************************************************
                                 krmaskchoice.h
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
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
#ifndef KRMASKCHOICE_H
#define KRMASKCHOICE_H

#include <tqdialog.h>
class TQVBoxLayout; 
class TQHBoxLayout; 
class TQGridLayout; 
class TQComboBox;
class TQGroupBox;
class TQLabel;
class TQListBox;
class TQListBoxItem;
class TQPushButton;

class KRMaskChoice : public TQDialog
{ 
    Q_OBJECT
  TQ_OBJECT

public:
    KRMaskChoice( TQWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~KRMaskChoice();

    TQComboBox* selection;
    TQLabel* PixmapLabel1;
    TQLabel* label;
    TQGroupBox* GroupBox1;
    TQListBox* preSelections;
    TQPushButton* PushButton7;
    TQPushButton* PushButton7_2;
    TQPushButton* PushButton7_3;
    TQPushButton* PushButton3;
    TQPushButton* PushButton3_2;

public slots:
    virtual void addSelection();
    virtual void clearSelections();
    virtual void deleteSelection();
    virtual void acceptFromList(TQListBoxItem *);

protected:
    TQHBoxLayout* hbox;
    TQHBoxLayout* hbox_2;
    TQHBoxLayout* hbox_3;
    TQVBoxLayout* vbox;
};

#endif // KRMASKCHOICE_H
