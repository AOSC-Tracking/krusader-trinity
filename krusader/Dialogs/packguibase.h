/***************************************************************************
                                 packguibase.h
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
#ifndef PACKGUIBASE_H
#define PACKGUIBASE_H

#include <klocale.h>
#include <tqdialog.h>
#include <tqmap.h>


class TQVBoxLayout; 
class TQHBoxLayout; 
class TQGridLayout; 
class TQCheckBox;
class TQComboBox;
class TQLabel;
class TQLineEdit;
class TQPushButton;
class TQToolButton;
class TQSpinBox;
class TQSlider;
class KHistoryCombo;

class PackGUIBase : public TQDialog
{ 
    Q_OBJECT
  TQ_OBJECT

public:
    PackGUIBase( TQWidget* tqparent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~PackGUIBase();

    TQLabel* TextLabel3;
    TQLineEdit* nameData;
    TQComboBox* typeData;
    TQLabel* TextLabel5;
    TQLineEdit* dirData;
    TQToolButton* browseButton;
    TQWidget* advancedWidget;
    TQLabel* PixmapLabel1;
    TQLabel* TextLabel1;
    TQLabel* TextLabel4;
    TQLabel* TextLabel6;
    TQLabel* TextLabel7;
    TQLabel* TextLabel8;
    TQLabel* minLabel;
    TQLabel* maxLabel;
    TQLineEdit* password;
    TQLineEdit* passwordAgain;
    TQLabel* passwordConsistencyLabel;
    TQPushButton* okButton;
    TQPushButton* cancelButton;
    TQPushButton* advancedButton;
    TQCheckBox* encryptHeaders;
    TQCheckBox* multipleVolume;
    TQSpinBox* volumeSpinBox;
    TQComboBox* volumeUnitCombo;
    TQCheckBox* setCompressionLevel;
    TQSlider*   compressionSlider;
    KHistoryCombo *commandLineSwitches;

public slots:
    virtual void browse();
    virtual bool extraProperties( TQMap<TQString,TQString> & );

    void expand();
    void checkConsistency();

protected:
    TQHBoxLayout* hbox;
    TQHBoxLayout* hbox_2;
    TQHBoxLayout* hbox_3;
    TQHBoxLayout* hbox_4;
    TQGridLayout* hbox_5;
    TQHBoxLayout* hbox_6;
    TQHBoxLayout* hbox_7;
    TQGridLayout* grid;

private:
    bool expanded;
};

#endif // PACKGUIBASE_H
