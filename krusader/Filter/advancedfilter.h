/***************************************************************************
                        advancedfilter.h  -  description
                             -------------------
    copyright            : (C) 2003 + by Shie Erlich & Rafi Yanai & Csaba Karai
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

#ifndef ADVANCEDFILTER_H
#define ADVANCEDFILTER_H

#include "filterbase.h"
#include "../VFS/krquery.h"

#include <tqwidget.h>
#include <tqcheckbox.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <tqradiobutton.h>
#include <tqtoolbutton.h>

class AdvancedFilter : public TQWidget, public FilterBase
{
  Q_OBJECT
  

public:
  AdvancedFilter( FilterTabs *tabs, TQWidget *parent = 0, const char *name = 0 );

  virtual void          queryAccepted() {}
  virtual bool          fillQuery( KRQuery *query );
  virtual void          loadFromProfile( TQString name );
  virtual void          saveToProfile( TQString name );
  virtual TQString       name() { return "AdvancedFilter";}
  virtual FilterTabs *  filterTabs() { return fltTabs; }

public slots:
  void modifiedBetweenSetDate1();
  void modifiedBetweenSetDate2();
  void notModifiedAfterSetDate();

public:
  TQCheckBox* smallerThanEnabled;
  TQLineEdit* smallerThanAmount;
  KComboBox* smallerThanType;

  TQCheckBox* biggerThanEnabled;
  KComboBox* biggerThanType;
  TQLineEdit* biggerThanAmount;

  TQRadioButton* modifiedBetweenEnabled;
  TQRadioButton* notModifiedAfterEnabled;
  TQRadioButton* modifiedInTheLastEnabled;

  TQLineEdit* modifiedBetweenData1;
  TQLineEdit* modifiedBetweenData2;

  TQToolButton* modifiedBetweenBtn1;
  TQToolButton* modifiedBetweenBtn2;
  TQToolButton* notModifiedAfterBtn;

  TQLineEdit* notModifiedAfterData;
  TQLineEdit* modifiedInTheLastData;
  TQLineEdit* notModifiedInTheLastData;
  TQComboBox* modifiedInTheLastType;
  TQComboBox* notModifiedInTheLastType;

  TQCheckBox* belongsToUserEnabled;
  TQComboBox* belongsToUserData;
  TQCheckBox* belongsToGroupEnabled;
  TQComboBox* belongsToGroupData;

  TQCheckBox* permissionsEnabled;

  TQComboBox* ownerW;
  TQComboBox* ownerR;
  TQComboBox* ownerX;
  TQComboBox* groupW;
  TQComboBox* groupR;
  TQComboBox* groupX;
  TQComboBox* allW;
  TQComboBox* allX;
  TQComboBox* allR;

  FilterTabs *fltTabs;

private:
  void changeDate(TQLineEdit *p);
  void fillList(TQComboBox *list, TQString filename);
  void qdate2time_t(time_t *dest, TQDate d, bool start);
  void invalidDateMessage(TQLineEdit *p);
};

#endif /* ADVANCEDFILTER_H */
