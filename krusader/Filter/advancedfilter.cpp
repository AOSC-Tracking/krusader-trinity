/***************************************************************************
                      advancedfilter.cpp  -  description
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

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../krusader.h"
#include "advancedfilter.h"
#include "../Dialogs/krdialogs.h"

#include <tqgroupbox.h>
#include <tdelocale.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqbuttongroup.h>
#include <tqfile.h>
#include <kdebug.h>
#include <tdemessagebox.h>
#include <time.h>
#include <kiconloader.h>

#define USERSFILE  TQString("/etc/passwd")
#define GROUPSFILE TQString("/etc/group")

AdvancedFilter::AdvancedFilter( FilterTabs *tabs, TQWidget *parent, const char *name ) : TQWidget( parent, name ), fltTabs( tabs )
{
  TQGridLayout *filterLayout = new TQGridLayout( this );
  filterLayout->setSpacing( 6 );
  filterLayout->setMargin( 11 );

  // Options for size

  TQGroupBox *sizeGroup = new TQGroupBox( this, "sizeGroup" );
  sizeGroup->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)5, (TQSizePolicy::SizeType)1, sizeGroup->sizePolicy().hasHeightForWidth() ) );
  sizeGroup->setTitle( i18n( "Size" ) );
  sizeGroup->setColumnLayout(0, Qt::Vertical );
  sizeGroup->layout()->setSpacing( 0 );
  sizeGroup->layout()->setMargin( 0 );
  TQGridLayout *sizeLayout = new TQGridLayout( sizeGroup->layout() );
  sizeLayout->setAlignment( TQt::AlignTop );
  sizeLayout->setSpacing( 6 );
  sizeLayout->setMargin( 11 );

  biggerThanEnabled = new TQCheckBox( sizeGroup, "biggerThanEnabled" );
  biggerThanEnabled->setText( i18n( "&Bigger than" ) );
  sizeLayout->addWidget( biggerThanEnabled, 0, 0 );

  biggerThanAmount = new TQLineEdit( sizeGroup, "biggerThanAmount" );
  biggerThanAmount->setEnabled( false );
  biggerThanAmount->setSizePolicy( TQSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed, biggerThanAmount->sizePolicy().hasHeightForWidth() ) );
  sizeLayout->addWidget( biggerThanAmount, 0, 1 );

  biggerThanType = new KComboBox( false, sizeGroup, "biggerThanType" );
  biggerThanType->insertItem( i18n( "Bytes" ) );
  biggerThanType->insertItem( i18n( "KB" ) );
  biggerThanType->insertItem( i18n( "MB" ) );
  biggerThanType->setEnabled( false );
  sizeLayout->addWidget( biggerThanType, 0, 2 );

  smallerThanEnabled = new TQCheckBox( sizeGroup, "smallerThanEnabled" );
  smallerThanEnabled->setText( i18n( "&Smaller than" ) );
  sizeLayout->addWidget( smallerThanEnabled, 0, 3 );

  smallerThanAmount = new TQLineEdit( sizeGroup, "smallerThanAmount" );
  smallerThanAmount->setEnabled( false );
  smallerThanAmount->setSizePolicy( TQSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed, smallerThanAmount->sizePolicy().hasHeightForWidth() ) );
  sizeLayout->addWidget( smallerThanAmount, 0, 4 );

  smallerThanType = new KComboBox( false, sizeGroup, "smallerThanType" );
  smallerThanType->insertItem( i18n( "Bytes" ) );
  smallerThanType->insertItem( i18n( "KB" ) );
  smallerThanType->insertItem( i18n( "MB" ) );
  smallerThanType->setEnabled( false );
  sizeLayout->addWidget( smallerThanType, 0, 5 );

  // set a tighter box around the type box

  int height = TQFontMetrics(biggerThanType->font()).height()+2;
  biggerThanType->setMaximumHeight(height);
  smallerThanType->setMaximumHeight(height);

  filterLayout->addWidget( sizeGroup, 0, 0 );

  // Options for date

  TQPixmap iconDate = krLoader->loadIcon( "date", TDEIcon::Toolbar, 16 );

  TQButtonGroup *dateGroup = new TQButtonGroup( this, "dateGroup" );
  dateGroup->setTitle( i18n( "Date" ) );
  dateGroup->setExclusive( true );
  dateGroup->setColumnLayout(0, Qt::Vertical );
  dateGroup->layout()->setSpacing( 0 );
  dateGroup->layout()->setMargin( 0 );
  TQGridLayout *dateLayout = new TQGridLayout( dateGroup->layout() );
  dateLayout->setAlignment( TQt::AlignTop );
  dateLayout->setSpacing( 6 );
  dateLayout->setMargin( 11 );

  modifiedBetweenEnabled = new TQRadioButton( dateGroup, "modifiedBetweenEnabled" );
  modifiedBetweenEnabled->setText( i18n( "&Modified between" ) );
  dateGroup->insert( modifiedBetweenEnabled, 0 );

  dateLayout->addMultiCellWidget( modifiedBetweenEnabled, 0, 0, 0, 1 );

  modifiedBetweenData1 = new TQLineEdit( dateGroup, "modifiedBetweenData1" );
  modifiedBetweenData1->setEnabled( false );
  modifiedBetweenData1->setText( "" );
  dateLayout->addMultiCellWidget( modifiedBetweenData1, 0, 0, 2, 3 );

  modifiedBetweenBtn1 = new TQToolButton( dateGroup, "modifiedBetweenBtn1" );
  modifiedBetweenBtn1->setEnabled( false );
  modifiedBetweenBtn1->setText( "" );
  modifiedBetweenBtn1->setPixmap( iconDate );
  dateLayout->addWidget( modifiedBetweenBtn1, 0, 4 );

  TQLabel *andLabel = new TQLabel( dateGroup, "andLabel" );
  andLabel->setText( i18n( "an&d" ) );
  dateLayout->addWidget( andLabel, 0, 5 );

  modifiedBetweenData2 = new TQLineEdit( dateGroup, "modifiedBetweenData2" );
  modifiedBetweenData2->setEnabled( false );
  modifiedBetweenData2->setText( "" );
  andLabel->setBuddy(modifiedBetweenData2);
  dateLayout->addWidget( modifiedBetweenData2, 0, 6 );

  modifiedBetweenBtn2 = new TQToolButton( dateGroup, "modifiedBetweenBtn2" );
  modifiedBetweenBtn2->setEnabled( false );
  modifiedBetweenBtn2->setText( "" );
  modifiedBetweenBtn2->setPixmap( iconDate );
  dateLayout->addWidget( modifiedBetweenBtn2, 0, 7 );

  notModifiedAfterEnabled = new TQRadioButton( dateGroup, "notModifiedAfterEnabled" );
  notModifiedAfterEnabled->setText( i18n( "&Not modified after" ) );
  dateGroup->insert( notModifiedAfterEnabled, 0 );
  dateLayout->addMultiCellWidget( notModifiedAfterEnabled, 1, 1, 0, 1 );

  notModifiedAfterData = new TQLineEdit( dateGroup, "notModifiedAfterData" );
  notModifiedAfterData->setEnabled( false );
  notModifiedAfterData->setText( "" );
  dateLayout->addMultiCellWidget( notModifiedAfterData, 1, 1, 2, 3 );

  notModifiedAfterBtn = new TQToolButton( dateGroup, "notModifiedAfterBtn" );
  notModifiedAfterBtn->setEnabled( false );
  notModifiedAfterBtn->setText( "" );
  notModifiedAfterBtn->setPixmap( iconDate );
  dateLayout->addWidget( notModifiedAfterBtn, 1, 4 );

  modifiedInTheLastEnabled = new TQRadioButton( dateGroup, "modifiedInTheLastEnabled" );
  modifiedInTheLastEnabled->setText( i18n("Mod&ified in the last") );
  dateGroup->insert( modifiedInTheLastEnabled, 0 );
  dateLayout->addWidget( modifiedInTheLastEnabled, 2, 0 );

  modifiedInTheLastData = new TQLineEdit( dateGroup, "modifiedInTheLastData" );
  modifiedInTheLastData->setEnabled( false );
  modifiedInTheLastData->setText( "" );
  dateLayout->addWidget( modifiedInTheLastData, 2, 2 );

  modifiedInTheLastType = new TQComboBox( false, dateGroup, "modifiedInTheLastType" );
  modifiedInTheLastType->insertItem( i18n( "days" ) );
  modifiedInTheLastType->insertItem( i18n( "weeks" ) );
  modifiedInTheLastType->insertItem( i18n( "months" ) );
  modifiedInTheLastType->insertItem( i18n( "years" ) );
  modifiedInTheLastType->setEnabled( false );
  dateLayout->addMultiCellWidget( modifiedInTheLastType, 2, 2, 3, 4 );

  notModifiedInTheLastData = new TQLineEdit( dateGroup, "notModifiedInTheLastData" );
  notModifiedInTheLastData->setEnabled( false );
  notModifiedInTheLastData->setText( "" );
  dateLayout->addWidget( notModifiedInTheLastData, 3, 2 );

  TQLabel *notModifiedInTheLastLbl = new TQLabel( dateGroup, "notModifiedInTheLastLbl" );
  notModifiedInTheLastLbl->setText( i18n( "No&t modified in the last" ) );
  notModifiedInTheLastLbl->setBuddy(notModifiedInTheLastData);
  dateLayout->addWidget( notModifiedInTheLastLbl, 3, 0 );

  notModifiedInTheLastType = new TQComboBox( false, dateGroup, "notModifiedInTheLastType" );
  notModifiedInTheLastType->insertItem( i18n( "days" ) );
  notModifiedInTheLastType->insertItem( i18n( "weeks" ) );
  notModifiedInTheLastType->insertItem( i18n( "months" ) );
  notModifiedInTheLastType->insertItem( i18n( "years" ) );
  notModifiedInTheLastType->setEnabled( false );
  dateLayout->addMultiCellWidget( notModifiedInTheLastType, 3, 3, 3, 4 );

  filterLayout->addWidget( dateGroup, 1, 0 );

  // Options for ownership

  TQGroupBox *ownershipGroup = new TQGroupBox( this, "ownershipGroup" );
  ownershipGroup->setTitle( i18n( "Ownership" ) );
  ownershipGroup->setColumnLayout(0, Qt::Vertical );
  ownershipGroup->layout()->setSpacing( 0 );
  ownershipGroup->layout()->setMargin( 0 );
  TQGridLayout *ownershipLayout = new TQGridLayout( ownershipGroup->layout() );
  ownershipLayout->setAlignment( TQt::AlignTop );
  ownershipLayout->setSpacing( 6 );
  ownershipLayout->setMargin( 11 );

  TQHBoxLayout *hboxLayout = new TQHBoxLayout();
  hboxLayout->setSpacing( 6 );
  hboxLayout->setMargin( 0 );

  belongsToUserEnabled = new TQCheckBox( ownershipGroup, "belongsToUserEnabled" );
  belongsToUserEnabled->setText( i18n( "Belongs to &user" ) );
  hboxLayout->addWidget( belongsToUserEnabled );

  belongsToUserData = new TQComboBox( false, ownershipGroup, "belongsToUserData" );
  belongsToUserData->setEnabled( false );
  belongsToUserData->setEditable( false );
  hboxLayout->addWidget( belongsToUserData );

  belongsToGroupEnabled = new TQCheckBox( ownershipGroup, "belongsToGroupEnabled" );
  belongsToGroupEnabled->setText( i18n( "Belongs to gr&oup" ) );
  hboxLayout->addWidget( belongsToGroupEnabled );

  belongsToGroupData = new TQComboBox( false, ownershipGroup, "belongsToGroupData" );
  belongsToGroupData->setEnabled( false );
  belongsToGroupData->setEditable( false );
  hboxLayout->addWidget( belongsToGroupData );

  ownershipLayout->addMultiCellLayout( hboxLayout, 0, 0, 0, 3 );

  permissionsEnabled = new TQCheckBox( ownershipGroup, "permissionsEnabled" );
  permissionsEnabled->setText( i18n( "P&ermissions" ) );
  ownershipLayout->addWidget( permissionsEnabled, 1, 0 );

  TQGroupBox *ownerGroup = new TQGroupBox( ownershipGroup, "ownerGroup" );
  ownerGroup->setTitle( i18n( "O&wner" ) );
  int width = 2*height + height / 2;

  ownerR = new TQComboBox( false, ownerGroup, "ownerR" );
  ownerR->insertItem( i18n( "?" ) );
  ownerR->insertItem( i18n( "r" ) );
  ownerR->insertItem( i18n( "-" ) );
  ownerR->setEnabled( false );
  ownerR->setGeometry( TQRect( 10, 20, width, height+6 ) );

  ownerW = new TQComboBox( false, ownerGroup, "ownerW" );
  ownerW->insertItem( i18n( "?" ) );
  ownerW->insertItem( i18n( "w" ) );
  ownerW->insertItem( i18n( "-" ) );
  ownerW->setEnabled( false );
  ownerW->setGeometry( TQRect( 10 + width, 20, width, height+6 ) );

  ownerX = new TQComboBox( false, ownerGroup, "ownerX" );
  ownerX->insertItem( i18n( "?" ) );
  ownerX->insertItem( i18n( "x" ) );
  ownerX->insertItem( i18n( "-" ) );
  ownerX->setEnabled( false );
  ownerX->setGeometry( TQRect( 10 + 2*width, 20, width, height+6 ) );

  ownershipLayout->addWidget( ownerGroup, 1, 1 );

  TQGroupBox *groupGroup = new TQGroupBox( ownershipGroup, "groupGroup" );
  groupGroup->setTitle( i18n( "Grou&p" ) );

  groupR = new TQComboBox( false, groupGroup, "groupR" );
  groupR->insertItem( i18n( "?" ) );
  groupR->insertItem( i18n( "r" ) );
  groupR->insertItem( i18n( "-" ) );
  groupR->setEnabled( false );
  groupR->setGeometry( TQRect( 10, 20, width, height+6 ) );

  groupW = new TQComboBox( false, groupGroup, "groupW" );
  groupW->insertItem( i18n( "?" ) );
  groupW->insertItem( i18n( "w" ) );
  groupW->insertItem( i18n( "-" ) );
  groupW->setEnabled( false );
  groupW->setGeometry( TQRect( 10 + width, 20, width, height+6 ) );

  groupX = new TQComboBox( false, groupGroup, "groupX" );
  groupX->insertItem( i18n( "?" ) );
  groupX->insertItem( i18n( "x" ) );
  groupX->insertItem( i18n( "-" ) );
  groupX->setEnabled( false );
  groupX->setGeometry( TQRect( 10 + 2*width, 20, width, height+6 ) );

  ownershipLayout->addWidget( groupGroup, 1, 2 );

  TQGroupBox *allGroup = new TQGroupBox( ownershipGroup, "allGroup" );
  allGroup->setTitle( i18n( "A&ll" ) );

  allR = new TQComboBox( false, allGroup, "allR" );
  allR->insertItem( i18n( "?" ) );
  allR->insertItem( i18n( "r" ) );
  allR->insertItem( i18n( "-" ) );
  allR->setEnabled( false );
  allR->setGeometry( TQRect( 10, 20, width, height+6 ) );

  allW = new TQComboBox( false, allGroup, "allW" );
  allW->insertItem( i18n( "?" ) );
  allW->insertItem( i18n( "w" ) );
  allW->insertItem( i18n( "-" ) );
  allW->setEnabled( false );
  allW->setGeometry( TQRect( 10 + width, 20, width, height+6 ) );

  allX = new TQComboBox( false, allGroup, "allX" );
  allX->insertItem( i18n( "?" ) );
  allX->insertItem( i18n( "x" ) );
  allX->insertItem( i18n( "-" ) );
  allX->setEnabled( false );
  allX->setGeometry( TQRect( 10 + 2*width, 20, width, height+6 ) );

  ownershipLayout->addWidget( allGroup, 1, 3 );

  TQLabel *infoLabel = new TQLabel( ownershipGroup, "TextLabel4" );
  TQFont infoLabel_font( infoLabel->font() );
  infoLabel_font.setFamily( "adobe-helvetica" );
  infoLabel_font.setItalic( true );
  infoLabel->setFont( infoLabel_font );
  infoLabel->setText( i18n( "Note: a '?' is a wildcard" ) );

  ownershipLayout->addMultiCellWidget( infoLabel, 2, 2, 0, 3, TQt::AlignRight );

  filterLayout->addWidget( ownershipGroup, 2, 0 );

  // Connection table

  connect( biggerThanEnabled, TQT_SIGNAL( toggled(bool) ), biggerThanAmount, TQT_SLOT( setEnabled(bool) ) );
  connect( biggerThanEnabled, TQT_SIGNAL( toggled(bool) ), biggerThanType, TQT_SLOT( setEnabled(bool) ) );
  connect( smallerThanEnabled, TQT_SIGNAL( toggled(bool) ), smallerThanAmount, TQT_SLOT( setEnabled(bool) ) );
  connect( smallerThanEnabled, TQT_SIGNAL( toggled(bool) ), smallerThanType, TQT_SLOT( setEnabled(bool) ) );
  connect( modifiedBetweenEnabled, TQT_SIGNAL( toggled(bool) ), modifiedBetweenData1, TQT_SLOT( setEnabled(bool) ) );
  connect( modifiedBetweenEnabled, TQT_SIGNAL( toggled(bool) ), modifiedBetweenBtn1, TQT_SLOT( setEnabled(bool) ) );
  connect( modifiedBetweenEnabled, TQT_SIGNAL( toggled(bool) ), modifiedBetweenData2, TQT_SLOT( setEnabled(bool) ) );
  connect( modifiedBetweenEnabled, TQT_SIGNAL( toggled(bool) ), modifiedBetweenBtn2, TQT_SLOT( setEnabled(bool) ) );
  connect( notModifiedAfterEnabled, TQT_SIGNAL( toggled(bool) ), notModifiedAfterData, TQT_SLOT( setEnabled(bool) ) );
  connect( notModifiedAfterEnabled, TQT_SIGNAL( toggled(bool) ), notModifiedAfterBtn, TQT_SLOT( setEnabled(bool) ) );
  connect( modifiedInTheLastEnabled, TQT_SIGNAL( toggled(bool) ), modifiedInTheLastData, TQT_SLOT( setEnabled(bool) ) );
  connect( modifiedInTheLastEnabled, TQT_SIGNAL( toggled(bool) ), modifiedInTheLastType, TQT_SLOT( setEnabled(bool) ) );
  connect( modifiedInTheLastEnabled, TQT_SIGNAL( toggled(bool) ), notModifiedInTheLastData, TQT_SLOT( setEnabled(bool) ) );
  connect( modifiedInTheLastEnabled, TQT_SIGNAL( toggled(bool) ), notModifiedInTheLastType, TQT_SLOT( setEnabled(bool) ) );
  connect( belongsToUserEnabled, TQT_SIGNAL( toggled(bool) ), belongsToUserData, TQT_SLOT( setEnabled(bool) ) );
  connect( belongsToGroupEnabled, TQT_SIGNAL( toggled(bool) ), belongsToGroupData, TQT_SLOT( setEnabled(bool) ) );
  connect( permissionsEnabled, TQT_SIGNAL( toggled(bool) ), ownerR, TQT_SLOT( setEnabled(bool) ) );
  connect( permissionsEnabled, TQT_SIGNAL( toggled(bool) ), ownerW, TQT_SLOT( setEnabled(bool) ) );
  connect( permissionsEnabled, TQT_SIGNAL( toggled(bool) ), ownerX, TQT_SLOT( setEnabled(bool) ) );
  connect( permissionsEnabled, TQT_SIGNAL( toggled(bool) ), groupR, TQT_SLOT( setEnabled(bool) ) );
  connect( permissionsEnabled, TQT_SIGNAL( toggled(bool) ), groupW, TQT_SLOT( setEnabled(bool) ) );
  connect( permissionsEnabled, TQT_SIGNAL( toggled(bool) ), groupX, TQT_SLOT( setEnabled(bool) ) );
  connect( permissionsEnabled, TQT_SIGNAL( toggled(bool) ), allR, TQT_SLOT( setEnabled(bool) ) );
  connect( permissionsEnabled, TQT_SIGNAL( toggled(bool) ), allW, TQT_SLOT( setEnabled(bool) ) );
  connect( permissionsEnabled, TQT_SIGNAL( toggled(bool) ), allX, TQT_SLOT( setEnabled(bool) ) );
  connect( modifiedBetweenBtn1, TQT_SIGNAL( clicked() ), this, TQT_SLOT( modifiedBetweenSetDate1() ) );
  connect( modifiedBetweenBtn2, TQT_SIGNAL( clicked() ), this, TQT_SLOT( modifiedBetweenSetDate2() ) );
  connect( notModifiedAfterBtn, TQT_SIGNAL( clicked() ), this, TQT_SLOT( notModifiedAfterSetDate() ) );

  // fill the users and groups list

  fillList(belongsToUserData, USERSFILE);
  fillList(belongsToGroupData, GROUPSFILE);

  // tab order
  setTabOrder( biggerThanEnabled, biggerThanAmount );
  setTabOrder( biggerThanAmount, smallerThanEnabled );
  setTabOrder( smallerThanEnabled, smallerThanAmount );
  setTabOrder( smallerThanAmount, modifiedBetweenEnabled );
  setTabOrder( modifiedBetweenEnabled, modifiedBetweenData1 );
  setTabOrder( modifiedBetweenData1, modifiedBetweenData2 );
  setTabOrder( modifiedBetweenData2, notModifiedAfterEnabled );
  setTabOrder( notModifiedAfterEnabled, notModifiedAfterData );
  setTabOrder( notModifiedAfterData, modifiedInTheLastEnabled );
  setTabOrder( modifiedInTheLastEnabled, modifiedInTheLastData );
  setTabOrder( modifiedInTheLastData, notModifiedInTheLastData );
  setTabOrder( notModifiedInTheLastData, belongsToUserEnabled );
  setTabOrder( belongsToUserEnabled, belongsToUserData );
  setTabOrder( belongsToUserData, belongsToGroupEnabled );
  setTabOrder( belongsToGroupEnabled, belongsToGroupData );
  setTabOrder( belongsToGroupData, permissionsEnabled );
  setTabOrder( permissionsEnabled, ownerR );
  setTabOrder( ownerR, ownerW );
  setTabOrder( ownerW, ownerX );
  setTabOrder( ownerX, groupR );
  setTabOrder( groupR, groupW );
  setTabOrder( groupW, groupX );
  setTabOrder( groupX, allR );
  setTabOrder( allR, allW );
  setTabOrder( allW, allX );
  setTabOrder( allX, biggerThanType );
  setTabOrder( biggerThanType, smallerThanType );
  setTabOrder( smallerThanType, modifiedInTheLastType );
  setTabOrder( modifiedInTheLastType, notModifiedInTheLastType );
}

void AdvancedFilter::modifiedBetweenSetDate1()
{
  changeDate(modifiedBetweenData1);
}

void AdvancedFilter::modifiedBetweenSetDate2()
{
  changeDate(modifiedBetweenData2);
}

void AdvancedFilter::notModifiedAfterSetDate()
{
  changeDate(notModifiedAfterData);
}

void AdvancedFilter::changeDate(TQLineEdit *p) {
  // check if the current date is valid
  TQDate d = TDEGlobal::locale()->readDate(p->text());
  if (!d.isValid()) d = TQDate::currentDate();

  KRGetDate *gd = new KRGetDate(d, this);
  d = gd->getDate();
  // if a user pressed ESC or closed the dialog, we'll return an invalid date
  if (d.isValid())
    p->setText(TDEGlobal::locale()->formatDate(d, true));
  delete gd;
}

// bool start: set it to true if this date is the beginning of the search,
// if it's the end, set it to false
void AdvancedFilter::qdate2time_t(time_t *dest, TQDate d, bool start) {
  struct tm t;
  t.tm_sec   = (start ? 0 : 59);
  t.tm_min   = (start ? 0 : 59);
  t.tm_hour  = (start ? 0 : 23);
  t.tm_mday  = d.day();
  t.tm_mon   = d.month() - 1;
  t.tm_year  = d.year() - 1900;
  t.tm_wday  = d.dayOfWeek() - 1; // actually ignored by mktime
  t.tm_yday  = d.dayOfYear() - 1; // actually ignored by mktime
  t.tm_isdst = -1; // daylight saving time information isn't availble

  (*dest) = mktime( &t );
}


void AdvancedFilter::fillList(TQComboBox *list, TQString filename) {
  TQFile data(filename);
  if (!data.open(IO_ReadOnly))
  {
    krOut << "Search: Unable to read " << filename << " !!!" << endl;
    return;
  }
  // and read it into the temporary array
  TQTextStream t(&data);
  while (!data.atEnd())
  {
    TQString s = t.readLine();
    TQString name = s.left(s.find(':'));
    list->insertItem(name);
  }
}

void AdvancedFilter::invalidDateMessage(TQLineEdit *p)
{
  // FIXME p->text() is empty sometimes (to reproduce, set date to "13.09.005")
  KMessageBox::detailedError(this, i18n("Invalid date entered."),
                             i18n("The date %1 is not valid according to your locale. Please re-enter a valid date (use the date button for easy access).").arg(p->text()) );
  p->setFocus();
}

bool AdvancedFilter::fillQuery( KRQuery *query )
{
  TDEIO::filesize_t minSize = 0, maxSize = 0;

  // size calculations ////////////////////////////////////////////////
  if ( biggerThanEnabled->isChecked() &&
      !(biggerThanAmount->text().simplifyWhiteSpace()).isEmpty() ) {
    minSize = biggerThanAmount->text().toULong();
    switch ( biggerThanType->currentItem() ) {
      case 1 : minSize *= 1024;
               break;
      case 2 : minSize *= (1024*1024);
               break;
    }
    query->setMinimumFileSize( minSize );
  }
  if ( smallerThanEnabled->isChecked() &&
      !(smallerThanAmount->text().simplifyWhiteSpace()).isEmpty()) {
    maxSize = smallerThanAmount->text().toULong();
    switch (smallerThanType->currentItem()) {
      case 1 : maxSize *= 1024;
               break;
      case 2 : maxSize *= (1024*1024);
               break;
    }
    query->setMaximumFileSize( maxSize );
  }
  // check that minSize is smaller than maxSize
  if ( ( minSize > 0) && ( maxSize > 0) && ( maxSize < minSize)) {
    KMessageBox::detailedError(this, i18n("Specified sizes are inconsistent!"),
      i18n("Please re-enter the values, so that the left side size will be smaller than (or equal to) the right side size."));
    biggerThanAmount->setFocus();
    return false;
  }

  // date calculations ////////////////////////////////////////////////////
  if (modifiedBetweenEnabled->isChecked()) {
    // first, if both dates are empty, than don't use them
    if ( !(modifiedBetweenData1->text().simplifyWhiteSpace().isEmpty() &&
          modifiedBetweenData2->text().simplifyWhiteSpace().isEmpty()) ) {
      // check if date is valid
      TQDate d1 = TDEGlobal::locale()->readDate(modifiedBetweenData1->text());
      if (!d1.isValid()) { invalidDateMessage(modifiedBetweenData1); return false; }
      TQDate d2 = TDEGlobal::locale()->readDate(modifiedBetweenData2->text());
      if (!d2.isValid()) { invalidDateMessage(modifiedBetweenData2); return false; }

      if (d1 > d2) {
        KMessageBox::detailedError(this, i18n("Dates are inconsistent!"),
          i18n("The date on the left is later than the date on the right. Please re-enter the dates, so that the left side date will be earlier than the right side date."));
        modifiedBetweenData1->setFocus();
        return false;
      }
      // all seems to be ok, create time_t

      time_t newerTime, olderTime;
      qdate2time_t(&newerTime, d1, true);
      qdate2time_t(&olderTime, d2, false);
      query->setNewerThan( newerTime );
      query->setOlderThan( olderTime );
    }
  } else if (notModifiedAfterEnabled->isChecked()) {
    if ( !notModifiedAfterData->text().simplifyWhiteSpace().isEmpty() ) {
      TQDate d = TDEGlobal::locale()->readDate(notModifiedAfterData->text());
      if (!d.isValid()) { invalidDateMessage(notModifiedAfterData); return false; }
      time_t olderTime;
      qdate2time_t(&olderTime, d, false);
      query->setOlderThan( olderTime );
    }
  } else if (modifiedInTheLastEnabled->isChecked()) {
    if ( !(modifiedInTheLastData->text().simplifyWhiteSpace().isEmpty() &&
          notModifiedInTheLastData->text().simplifyWhiteSpace().isEmpty()) ) {
      TQDate d1 = TQDate::currentDate(); TQDate d2 = TQDate::currentDate();
      if (!modifiedInTheLastData->text().simplifyWhiteSpace().isEmpty()) {
        int tmp1 = modifiedInTheLastData->text().simplifyWhiteSpace().toInt();
        switch (modifiedInTheLastType->currentItem()) {
          case 1 : tmp1 *= 7;
                   break;
          case 2 : tmp1 *= 30;
                   break;
          case 3 : tmp1 *= 365;
                   break;
        }
        d1 = d1.addDays((-1) * tmp1);
        time_t newerTime;
        qdate2time_t(&newerTime, d1, true);
        query->setNewerThan( newerTime );
      }
      if (!notModifiedInTheLastData->text().simplifyWhiteSpace().isEmpty()) {
        int tmp2 = notModifiedInTheLastData->text().simplifyWhiteSpace().toInt();
        switch (notModifiedInTheLastType->currentItem()) {
          case 1 : tmp2 *= 7;
                   break;
          case 2 : tmp2 *= 30;
                   break;
          case 3 : tmp2 *= 365;
                   break;
        }
        d2 = d2.addDays((-1) * tmp2);
        time_t olderTime;
        qdate2time_t(&olderTime, d2, true);
        query->setOlderThan( olderTime );
      }
      if ( !modifiedInTheLastData->text().simplifyWhiteSpace().isEmpty() &&
           !notModifiedInTheLastData->text().simplifyWhiteSpace().isEmpty() ) {
        if (d1 > d2) {
          KMessageBox::detailedError(this, i18n("Dates are inconsistent!"),
            i18n("The date on top is later than the date on the bottom. Please re-enter the dates, so that the top date will be earlier than the bottom date."));
          modifiedInTheLastData->setFocus();
          return false;
        }
      }
    }
  }
    // permissions and ownership /////////////////////////////////////
  if (permissionsEnabled->isChecked()) {
    TQString perm = ownerR->currentText() + ownerW->currentText() + ownerX->currentText() +
                   groupR->currentText() + groupW->currentText() + groupX->currentText() +
                   allR->currentText()   + allW->currentText()   + allX->currentText();
    query->setPermissions( perm );
  }
  if (belongsToUserEnabled->isChecked())
    query->setOwner( belongsToUserData->currentText() );
  if (belongsToGroupEnabled->isChecked())
    query->setGroup( belongsToGroupData->currentText() );

  return true;
}

void AdvancedFilter::loadFromProfile( TQString name )
{
  krConfig->setGroup( name );

  smallerThanEnabled->setChecked( krConfig->readBoolEntry( "Smaller Than Enabled", false ) );
  smallerThanAmount->setText( krConfig->readEntry( "Smaller Than Amount", "" ) );
  smallerThanType->setCurrentItem( krConfig->readNumEntry( "Smaller Than Type", 0 ) );

  biggerThanEnabled->setChecked( krConfig->readBoolEntry( "Bigger Than Enabled", false ) );
  biggerThanAmount->setText( krConfig->readEntry( "Bigger Than Amount", "" ) );
  biggerThanType->setCurrentItem( krConfig->readNumEntry( "Bigger Than Type", 0 ) );

  modifiedBetweenEnabled->setChecked( krConfig->readBoolEntry( "Modified Between Enabled", false ) );
  notModifiedAfterEnabled->setChecked( krConfig->readBoolEntry( "Not Modified After Enabled", false ) );
  modifiedInTheLastEnabled->setChecked( krConfig->readBoolEntry( "Modified In The Last Enabled", false ) );

  modifiedBetweenData1->setText( krConfig->readEntry( "Modified Between 1", "" ) );
  modifiedBetweenData2->setText( krConfig->readEntry( "Modified Between 2", "" ) );

  notModifiedAfterData->setText( krConfig->readEntry( "Not Modified After", "" ) );
  modifiedInTheLastData->setText( krConfig->readEntry( "Modified In The Last", "" ) );
  notModifiedInTheLastData->setText( krConfig->readEntry( "Not Modified In The Last", "" ) );

  modifiedInTheLastType->setCurrentItem( krConfig->readNumEntry( "Modified In The Last Type", 0 ) );
  notModifiedInTheLastType->setCurrentItem( krConfig->readNumEntry( "Not Modified In The Last Type", 0 ) );

  belongsToUserEnabled->setChecked( krConfig->readBoolEntry( "Belongs To User Enabled", false ) );
  belongsToGroupEnabled->setChecked( krConfig->readBoolEntry( "Belongs To Group Enabled", false ) );

  TQString user = krConfig->readEntry( "Belongs To User", "" );
  for( int i = belongsToUserData->count(); i >= 0; i-- )
  {
    belongsToUserData->setCurrentItem( i );
    if( belongsToUserData->currentText() == user )
      break;
  }

  TQString group = krConfig->readEntry( "Belongs To Group", "" );
  for( int i = belongsToGroupData->count(); i >= 0; i-- )
  {
    belongsToGroupData->setCurrentItem( i );
    if( belongsToGroupData->currentText() == group )
      break;
  }

  permissionsEnabled->setChecked( krConfig->readBoolEntry( "Permissions Enabled", false ) );

  ownerW->setCurrentItem( krConfig->readNumEntry( "Owner Write", 0 ) );
  ownerR->setCurrentItem( krConfig->readNumEntry( "Owner Read", 0 ) );
  ownerX->setCurrentItem( krConfig->readNumEntry( "Owner Execute", 0 ) );
  groupW->setCurrentItem( krConfig->readNumEntry( "Group Write", 0 ) );
  groupR->setCurrentItem( krConfig->readNumEntry( "Group Read", 0 ) );
  groupX->setCurrentItem( krConfig->readNumEntry( "Group Execute", 0 ) );
  allW->setCurrentItem( krConfig->readNumEntry( "All Write", 0 ) );
  allR->setCurrentItem( krConfig->readNumEntry( "All Read", 0 ) );
  allX->setCurrentItem( krConfig->readNumEntry( "All Execute", 0 ) );
}

void AdvancedFilter::saveToProfile( TQString name )
{
  krConfig->setGroup( name );

  krConfig->writeEntry( "Smaller Than Enabled", smallerThanEnabled->isChecked() );
  krConfig->writeEntry( "Smaller Than Amount", smallerThanAmount->text() );
  krConfig->writeEntry( "Smaller Than Type", smallerThanType->currentItem() );

  krConfig->writeEntry( "Bigger Than Enabled", biggerThanEnabled->isChecked() );
  krConfig->writeEntry( "Bigger Than Amount", biggerThanAmount->text() );
  krConfig->writeEntry( "Bigger Than Type", biggerThanType->currentItem() );

  krConfig->writeEntry( "Modified Between Enabled", modifiedBetweenEnabled->isChecked() );
  krConfig->writeEntry( "Not Modified After Enabled", notModifiedAfterEnabled->isChecked() );
  krConfig->writeEntry( "Modified In The Last Enabled", modifiedInTheLastEnabled->isChecked() );

  krConfig->writeEntry( "Modified Between 1", modifiedBetweenData1->text() );
  krConfig->writeEntry( "Modified Between 2", modifiedBetweenData2->text() );

  krConfig->writeEntry( "Not Modified After", notModifiedAfterData->text() );
  krConfig->writeEntry( "Modified In The Last", modifiedInTheLastData->text() );
  krConfig->writeEntry( "Not Modified In The Last", notModifiedInTheLastData->text() );

  krConfig->writeEntry( "Modified In The Last Type", modifiedInTheLastType->currentItem() );
  krConfig->writeEntry( "Not Modified In The Last Type", notModifiedInTheLastType->currentItem() );

  krConfig->writeEntry( "Belongs To User Enabled", belongsToUserEnabled->isChecked() );
  krConfig->writeEntry( "Belongs To Group Enabled", belongsToGroupEnabled->isChecked() );

  krConfig->writeEntry( "Belongs To User", belongsToUserData->currentText() );
  krConfig->writeEntry( "Belongs To Group", belongsToGroupData->currentText() );

  krConfig->writeEntry( "Permissions Enabled", permissionsEnabled->isChecked() );

  krConfig->writeEntry( "Owner Write", ownerW->currentItem() );
  krConfig->writeEntry( "Owner Read", ownerR->currentItem() );
  krConfig->writeEntry( "Owner Execute", ownerX->currentItem() );
  krConfig->writeEntry( "Group Write", groupW->currentItem() );
  krConfig->writeEntry( "Group Read", groupR->currentItem() );
  krConfig->writeEntry( "Group Execute", groupX->currentItem() );
  krConfig->writeEntry( "All Write", allW->currentItem() );
  krConfig->writeEntry( "All Read", allR->currentItem() );
  krConfig->writeEntry( "All Execute", allX->currentItem() );
}


#include "advancedfilter.moc"
