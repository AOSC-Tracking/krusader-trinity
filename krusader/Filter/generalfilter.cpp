/***************************************************************************
                      generalfilter.cpp  -  description
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

#include "generalfilter.h"
#include "filtertabs.h"
#include "../krusader.h"
#include "../VFS/vfs.h"

#include <tdelocale.h>
#include <tdefiledialog.h>
#include <tdemessagebox.h>
#include <kiconloader.h>
#include <tqwhatsthis.h>
#include <tqpushbutton.h>

GeneralFilter::GeneralFilter( FilterTabs *tabs, int properties, TQWidget *parent, const char *name ) : TQWidget( parent, name ),
    profileManager( 0 ), fltTabs( tabs )
{
  TQGridLayout *filterLayout = new TQGridLayout( this );
  filterLayout->setSpacing( 6 );
  filterLayout->setMargin( 11 );

  this->properties = properties;

  // Options for name filtering

  TQGroupBox *nameGroup = new TQGroupBox( this, "nameGroup" );
  nameGroup->setTitle( i18n( "File name" ) );
  nameGroup->setColumnLayout(0, TQt::Vertical );
  nameGroup->layout()->setSpacing( 0 );
  nameGroup->layout()->setMargin( 0 );
  TQGridLayout *nameGroupLayout = new TQGridLayout( nameGroup->layout() );
  nameGroupLayout->setAlignment( TQt::AlignTop );
  nameGroupLayout->setSpacing( 6 );
  nameGroupLayout->setMargin( 11 );

  searchForCase = new TQCheckBox( nameGroup, "searchForCase" );
  searchForCase->setText( i18n( "&Case sensitive" ) );
  searchForCase->setChecked( false );
  nameGroupLayout->addWidget( searchForCase, 1, 2 );

  TQLabel *searchForLabel = new TQLabel( nameGroup, "searchForLabel" );
  searchForLabel->setText( i18n( "Search &for:" ) );
  nameGroupLayout->addWidget( searchForLabel, 0, 0 );

  searchFor = new KHistoryCombo( false, nameGroup, "searchFor" );
  searchFor->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)7, (TQSizePolicy::SizeType)0, searchFor->sizePolicy().hasHeightForWidth() ) );
  searchFor->setEditable( true );
  searchFor->setDuplicatesEnabled( false );
  searchFor->setMaxCount( 25 );
  searchForLabel->setBuddy( searchFor );
  nameGroupLayout->addMultiCellWidget( searchFor, 0, 0, 1, 2 );

  TQString s = "<p><img src='toolbar|find'></p>" + i18n("<p>The filename filtering criteria is defined here.</p><p>You can make use of wildcards. Multiple patterns are separated by space (means logical OR) and patterns are excluded from the search using the pipe symbol.</p><p>If the pattern is ended with a slash (<code>*pattern*/</code>), that means that pattern relates to recursive search of directories.<ul><li><code>pattern</code> - means to search those files/directories that name is <code>pattern</code>, recursive search goes through all subdirectories independently of the value of <code>pattern</code></li><li><code>pattern/</code> - means to search all files/directories, but recursive search goes through/excludes the directories that name is <code>pattern</code></li></ul><p></p><p>It's allowed to use quotation marks for names that contain space. Filter <code>\"Program&nbsp;Files\"</code> searches out those files/directories that name is <code>Program&nbsp;Files</code>.</p><p>Examples:<ul><code><li>*.o</li><li>*.h *.c\?\?</li><li>*.cpp *.h | *.moc.cpp</li><li>* | CVS/ .svn/</li></code></ul><b>Note</b>: the search term '<code>text</code>' is equivalent to '<code>*text*</code>'.</p>");
  TQWhatsThis::add(searchFor, s);
  TQWhatsThis::add(searchForLabel, s);

  TQLabel *searchType = new TQLabel( nameGroup, "searchType" );
  searchType->setText( i18n( "&Of type:" ) );
  nameGroupLayout->addWidget( searchType, 1, 0 );

  ofType = new KComboBox( false, nameGroup, "ofType" );
  ofType->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)7, (TQSizePolicy::SizeType)0, ofType->sizePolicy().hasHeightForWidth() ) );
  ofType->setEditable( false );
  searchType->setBuddy(ofType);
  ofType->insertItem(i18n("All Files"));
  ofType->insertItem(i18n("Archives"));
  ofType->insertItem(i18n("Directories"));
  ofType->insertItem(i18n("Image Files"));
  ofType->insertItem(i18n("Text Files"));
  ofType->insertItem(i18n("Video Files"));
  ofType->insertItem(i18n("Audio Files"));

  nameGroupLayout->addWidget( ofType, 1, 1 );
  filterLayout->addWidget( nameGroup, 0, 0 );

  middleLayout = new TQHBoxLayout();
  middleLayout->setSpacing( 6 );
  middleLayout->setMargin( 0 );
  TQSpacerItem* middleSpacer = new TQSpacerItem( 1, 1, TQSizePolicy::Fixed, TQSizePolicy::Fixed );
  middleLayout->addItem( middleSpacer );

  if( properties & FilterTabs::HasProfileHandler )
  {
    // The profile handler

    TQGroupBox *profileHandler = new TQGroupBox( this, "profileHandler" );
    profileHandler->setTitle( i18n( "&Profile handler" ) );
    profileHandler->setColumnLayout(0, TQt::Vertical );
    profileHandler->layout()->setSpacing( 0 );
    profileHandler->layout()->setMargin( 0 );
    TQGridLayout *profileLayout = new TQGridLayout( profileHandler->layout() );
    profileLayout->setAlignment( TQt::AlignTop );
    profileLayout->setSpacing( 6 );
    profileLayout->setMargin( 11 );

    profileListBox = new TQListBox( profileHandler, "profileListBox" );
    profileLayout->addMultiCellWidget( profileListBox, 0, 3, 0, 0 );

    profileAddBtn = new TQPushButton( i18n( "&Add" ), profileHandler, "profileAddBtn" );
    profileLayout->addWidget( profileAddBtn, 0, 1 );

    profileLoadBtn = new TQPushButton( i18n( "&Load" ), profileHandler, "profileLoadBtn" );
    profileLoadBtn->setEnabled( false );
    profileLayout->addWidget( profileLoadBtn, 1, 1 );

    profileOverwriteBtn = new TQPushButton( i18n( "&Overwrite" ), profileHandler, "profileOverwriteBtn" );
    profileOverwriteBtn->setEnabled( false );
    profileLayout->addWidget( profileOverwriteBtn, 2, 1 );

    profileRemoveBtn = new TQPushButton( i18n( "&Remove" ), profileHandler, "profileRemoveBtn" );
    profileRemoveBtn->setEnabled( false );
    profileLayout->addWidget( profileRemoveBtn, 3, 1 );

    profileManager = new ProfileManager( "SelectionProfile", this, "selectionProfileManager" );
    profileManager->hide();

    middleLayout->addWidget( profileHandler );

    refreshProfileListBox();
  }

  if( properties & FilterTabs::HasSearchIn )
  {
    // Options for search in

    TQGroupBox *searchInGroup = new TQGroupBox( this, "searchInGroup" );
    searchInGroup->setTitle( i18n( "&Search in" ) );
    searchInGroup->setColumnLayout(0, TQt::Vertical );
    searchInGroup->layout()->setSpacing( 0 );
    searchInGroup->layout()->setMargin( 0 );
    TQGridLayout *searchInLayout = new TQGridLayout( searchInGroup->layout() );
    searchInLayout->setAlignment( TQt::AlignTop );
    searchInLayout->setSpacing( 6 );
    searchInLayout->setMargin( 11 );

    searchIn = new KURLListRequester( searchInGroup, "searchIn" );
    searchInLayout->addWidget( searchIn, 0, 0 );

    middleLayout->addWidget( searchInGroup );
  }

  if( properties & FilterTabs::HasDontSearchIn )
  {
    // Options for don't search in

    TQGroupBox *dontSearchInGroup = new TQGroupBox( this, "dontSearchInGroup" );
    dontSearchInGroup->setTitle( i18n( "&Don't search in" ) );
    dontSearchInGroup->setColumnLayout(0, TQt::Vertical );
    dontSearchInGroup->layout()->setSpacing( 0 );
    dontSearchInGroup->layout()->setMargin( 0 );
    TQGridLayout *dontSearchInLayout = new TQGridLayout( dontSearchInGroup->layout() );
    dontSearchInLayout->setAlignment( TQt::AlignTop );
    dontSearchInLayout->setSpacing( 6 );
    dontSearchInLayout->setMargin( 11 );

    dontSearchIn = new KURLListRequester( dontSearchInGroup, "dontSearchIn" );
    dontSearchInLayout->addWidget( dontSearchIn, 0, 0 );

    middleLayout->addWidget( dontSearchInGroup );
  }

  filterLayout->addLayout( middleLayout, 1, 0 );

  // Options for containing text

  TQGroupBox *containsGroup = new TQGroupBox( this, "containsGroup" );
  containsGroup->setTitle( i18n( "Containing text" ) );
  containsGroup->setColumnLayout(0, TQt::Vertical );
  containsGroup->layout()->setSpacing( 0 );
  containsGroup->layout()->setMargin( 0 );
  TQGridLayout *containsLayout = new TQGridLayout( containsGroup->layout() );
  containsLayout->setAlignment( TQt::AlignTop );
  containsLayout->setSpacing( 6 );
  containsLayout->setMargin( 11 );

  TQHBoxLayout *containsTextLayout = new TQHBoxLayout();
  containsTextLayout->setSpacing( 6 );
  containsTextLayout->setMargin( 0 );

  TQLabel *containsLabel = new TQLabel( containsGroup, "containsLabel" );
  containsLabel->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)0, (TQSizePolicy::SizeType)1, containsLabel->sizePolicy().hasHeightForWidth() ) );
  containsLabel->setText( i18n( "&Text:" ) );
  containsTextLayout->addWidget( containsLabel );

  containsText = new KHistoryCombo( false, containsGroup, "containsText" );
  containsText->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)7, (TQSizePolicy::SizeType)0, containsText->sizePolicy().hasHeightForWidth() ) );
  containsText->setDuplicatesEnabled( false );
  containsText->setMaxCount( 25 );
  containsTextLayout->addWidget( containsText );
  containsLabel->setBuddy(containsText);

  containsLayout->addLayout( containsTextLayout, 0, 0 );

  TQHBoxLayout *containsCbsLayout = new TQHBoxLayout();
  containsCbsLayout->setSpacing( 6 );
  containsCbsLayout->setMargin( 0 );
  TQSpacerItem* cbSpacer = new TQSpacerItem( 20, 20, TQSizePolicy::Expanding, TQSizePolicy::Minimum );
  containsCbsLayout->addItem( cbSpacer );

  remoteContentSearch = new TQCheckBox( containsGroup, "remoteContentSearch" );
  remoteContentSearch->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)5, (TQSizePolicy::SizeType)0, remoteContentSearch->sizePolicy().hasHeightForWidth() ) );
  remoteContentSearch->setText( i18n( "&Remote content search" ) );
  remoteContentSearch->setChecked( false );
  containsCbsLayout->addWidget( remoteContentSearch );
  if( !( properties & FilterTabs::HasRemoteContentSearch ) )
    remoteContentSearch->hide();

  containsWholeWord = new TQCheckBox( containsGroup, "containsWholeWord" );
  containsWholeWord->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)5, (TQSizePolicy::SizeType)0, containsWholeWord->sizePolicy().hasHeightForWidth() ) );
  containsWholeWord->setText( i18n( "&Match whole word only" ) );
  containsWholeWord->setChecked( false );
  containsCbsLayout->addWidget( containsWholeWord );

  containsTextCase = new TQCheckBox( containsGroup, "containsTextCase" );
  containsTextCase->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)5, (TQSizePolicy::SizeType)0, containsTextCase->sizePolicy().hasHeightForWidth() ) );
  containsTextCase->setText( i18n( "Cas&e sensitive" ) );
  containsTextCase->setChecked( true );
  containsCbsLayout->addWidget( containsTextCase );

  containsLayout->addLayout( containsCbsLayout, 1, 0 );

  filterLayout->addWidget( containsGroup, 2, 0 );

  if( properties & FilterTabs::HasRecurseOptions )
  {
    // Options for recursive searching

    TQHBoxLayout *recurseLayout = new TQHBoxLayout();
    recurseLayout->setSpacing( 6 );
    recurseLayout->setMargin( 0 );
    TQSpacerItem* recurseSpacer = new TQSpacerItem( 20, 20, TQSizePolicy::Expanding, TQSizePolicy::Minimum );
    recurseLayout->addItem( recurseSpacer );

    searchInDirs = new TQCheckBox( this, "searchInDirs" );
    searchInDirs->setText( i18n( "Search in s&ubdirectories" ) );
    searchInDirs->setChecked( true );
    recurseLayout->addWidget( searchInDirs );

    searchInArchives = new TQCheckBox( this, "searchInArchives" );
    searchInArchives->setText( i18n( "Search in arch&ives" ) );
    recurseLayout->addWidget( searchInArchives );

    followLinks = new TQCheckBox( this, "followLinks" );
    followLinks->setText( i18n( "Follow &links" ) );
    recurseLayout->addWidget( followLinks );

    filterLayout->addLayout( recurseLayout, 3, 0 );
  }

  // Connection table

  if( properties & FilterTabs::HasProfileHandler )
  {
    connect( profileAddBtn,       TQ_SIGNAL(clicked()) , this, TQ_SLOT( slotAddBtnClicked() ) );
    connect( profileLoadBtn,      TQ_SIGNAL(clicked()) , this, TQ_SLOT( slotLoadBtnClicked() ) );
    connect( profileOverwriteBtn, TQ_SIGNAL(clicked()) , this, TQ_SLOT( slotOverwriteBtnClicked() ) );
    connect( profileRemoveBtn,    TQ_SIGNAL(clicked()) , this, TQ_SLOT( slotRemoveBtnClicked() ) );
    connect( profileListBox,      TQ_SIGNAL(doubleClicked(TQListBoxItem *)) , this, TQ_SLOT( slotProfileDoubleClicked(TQListBoxItem *) ) );
    connect( profileManager,      TQ_SIGNAL(loadFromProfile(TQString )), fltTabs, TQ_SLOT( loadFromProfile(TQString ) ) );
    connect( profileManager,      TQ_SIGNAL(saveToProfile(TQString )), fltTabs, TQ_SLOT( saveToProfile(TQString ) ) );
  }

  connect( searchFor, TQ_SIGNAL(activated(const TQString&)), searchFor, TQ_SLOT(addToHistory(const TQString&)));
  connect( containsText, TQ_SIGNAL(activated(const TQString&)), containsText, TQ_SLOT(addToHistory(const TQString&)));

  // load the completion and history lists
  // ==> search for
  krConfig->setGroup("Search");
  TQStringList list = krConfig->readListEntry("SearchFor Completion");
  searchFor->completionObject()->setItems(list);
  list = krConfig->readListEntry("SearchFor History");
  searchFor->setHistoryItems(list);
  // ==> grep
  krConfig->setGroup("Search");
  list = krConfig->readListEntry("ContainsText Completion");
  containsText->completionObject()->setItems(list);
  list = krConfig->readListEntry("ContainsText History");
  containsText->setHistoryItems(list);

  setTabOrder( searchFor, containsText );  // search for -> content 
  setTabOrder( containsText, searchType ); // content -> search type
}

GeneralFilter::~GeneralFilter()
{
  // save the history combos
  // ==> search for
  TQStringList list = searchFor->completionObject()->items();
  krConfig->setGroup("Search");
  krConfig->writeEntry("SearchFor Completion", list);
  list = searchFor->historyItems();
  krConfig->writeEntry("SearchFor History", list);
  // ==> grep text
  list = containsText->completionObject()->items();
  krConfig->setGroup("Search");
  krConfig->writeEntry("ContainsText Completion", list);
  list = containsText->historyItems();
  krConfig->writeEntry("ContainsText History", list);

  krConfig->sync();
}

bool GeneralFilter::fillQuery( KRQuery *query )
{
  // check that we have (at least) what to search, and where to search in
  if (searchFor->currentText().simplifyWhiteSpace().isEmpty()) {
    KMessageBox::error(this ,i18n("No search criteria entered!"));
    searchFor->setFocus();
    return false;
  }

  // now fill the query

  query->setNameFilter( searchFor->currentText().stripWhiteSpace(), searchForCase->isChecked() );

  bool remoteContent = (properties & FilterTabs::HasRemoteContentSearch ) ? 
                       remoteContentSearch->isChecked() : false;

  query->setContent( containsText->currentText(),
                     containsTextCase->isChecked(),
                     containsWholeWord->isChecked(),
                     remoteContent );

  if (ofType->currentText()!=i18n("All Files"))
    query->setMimeType( ofType->currentText() );
  else query->setMimeType( TQString() );

  if ( properties & FilterTabs::HasRecurseOptions )
  {
    query->setSearchInArchives( searchInArchives->isChecked() );
    query->setRecursive( searchInDirs->isChecked() );
    query->setFollowLinks( followLinks->isChecked() );

    // create the lists
  }
  if( properties & FilterTabs::HasSearchIn )
  {
    query->setSearchInDirs( searchIn->urlList() );

    // checking the lists

    if (query->searchInDirs().isEmpty() ) { // we need a place to search in
      KMessageBox::error(this ,i18n("Please specify a location to search in."));
      searchIn->lineEdit()->setFocus();
      return false;
    }
  }

  if( properties & FilterTabs::HasDontSearchIn )
    query->setDontSearchInDirs( dontSearchIn->urlList() );

  return true;
}

void GeneralFilter::queryAccepted()
{
  searchFor->addToHistory(searchFor->currentText());
  containsText->addToHistory(containsText->currentText());
}

void GeneralFilter::loadFromProfile( TQString name )
{
  krConfig->setGroup( name );

  searchForCase->setChecked( krConfig->readBoolEntry( "Case Sensitive Search", false ) );
  containsTextCase->setChecked( krConfig->readBoolEntry( "Case Sensitive Content", false ) );
  remoteContentSearch->setChecked( krConfig->readBoolEntry( "Remote Content Search", false ) );
  containsWholeWord->setChecked( krConfig->readBoolEntry( "Match Whole Word Only", false ) );
  containsText->setEditText( krConfig->readEntry( "Contains Text", "" ) );
  searchFor->setEditText( krConfig->readEntry( "Search For", "" ) );

  TQString mime = krConfig->readEntry( "Mime Type", "" );
  for( int i = ofType->count(); i >= 0; i-- )
  {
    ofType->setCurrentItem( i );
    if( ofType->currentText() == mime )
      break;
  }

  if( properties & FilterTabs::HasRecurseOptions )
  {
    searchInDirs->setChecked( krConfig->readBoolEntry( "Search In Subdirectories", true ) );
    searchInArchives->setChecked( krConfig->readBoolEntry( "Search In Archives", false ) );
    followLinks->setChecked( krConfig->readBoolEntry( "Follow Symlinks", false ) );
  }

  if( properties & FilterTabs::HasSearchIn )
  {
    searchIn->lineEdit()->setText( krConfig->readEntry( "Search In Edit", "" ) );

    searchIn->listBox()->clear();
    TQStringList searchInList = krConfig->readListEntry( "Search In List" );
    if( !searchInList.isEmpty() )
      searchIn->listBox()->insertStringList( searchInList );
  }

  if( properties & FilterTabs::HasDontSearchIn )
  {
    dontSearchIn->lineEdit()->setText( krConfig->readEntry( "Dont Search In Edit", "" ) );

    dontSearchIn->listBox()->clear();
    TQStringList dontSearchInList = krConfig->readListEntry( "Dont Search In List" );
    if( !dontSearchInList.isEmpty() )
      dontSearchIn->listBox()->insertStringList( dontSearchInList );
  }
}

void GeneralFilter::saveToProfile( TQString name )
{
  krConfig->setGroup( name );

  krConfig->writeEntry( "Case Sensitive Search", searchForCase->isChecked() );
  krConfig->writeEntry( "Case Sensitive Content", containsTextCase->isChecked() );
  krConfig->writeEntry( "Remote Content Search", remoteContentSearch->isChecked() );
  krConfig->writeEntry( "Match Whole Word Only", containsWholeWord->isChecked() );
  krConfig->writeEntry( "Contains Text", containsText->currentText() );
  krConfig->writeEntry( "Search For", searchFor->currentText() );

  krConfig->writeEntry( "Mime Type", ofType->currentText() );

  if( properties & FilterTabs::HasRecurseOptions )
  {
    krConfig->writeEntry( "Search In Subdirectories", searchInDirs->isChecked() );
    krConfig->writeEntry( "Search In Archives", searchInArchives->isChecked() );
    krConfig->writeEntry( "Follow Symlinks", followLinks->isChecked() );
  }

  if( properties & FilterTabs::HasSearchIn )
  {
    krConfig->writeEntry( "Search In Edit", searchIn->lineEdit()->text() );

    TQStringList searchInList;
    for ( TQListBoxItem *item = searchIn->listBox()->firstItem(); item != 0; item = item->next() )
      searchInList.append( item->text().simplifyWhiteSpace() );
    krConfig->writeEntry( "Search In List", searchInList );
  }

  if( properties & FilterTabs::HasDontSearchIn )
  {
    krConfig->writeEntry( "Dont Search In Edit", dontSearchIn->lineEdit()->text() );

    TQStringList dontSearchInList;
    for ( TQListBoxItem *item = dontSearchIn->listBox()->firstItem(); item != 0; item = item->next() )
      dontSearchInList.append( item->text().simplifyWhiteSpace() );
    krConfig->writeEntry( "Dont Search In List", dontSearchInList );
  }
}

void GeneralFilter::refreshProfileListBox()
{
  profileListBox->clear();
  profileListBox->insertStringList( ProfileManager::availableProfiles( "SelectionProfile" ) );

  if( profileListBox->count() != 0 )
  {
    profileLoadBtn->setEnabled( true );
    profileOverwriteBtn->setEnabled( true );
    profileRemoveBtn->setEnabled( true );
  }
  else
  {
    profileLoadBtn->setEnabled( false );
    profileOverwriteBtn->setEnabled( false );
    profileRemoveBtn->setEnabled( false );
  }
}

void GeneralFilter::slotAddBtnClicked()
{
  profileManager->newProfile( searchFor->currentText().simplifyWhiteSpace() );
  refreshProfileListBox();
}

void GeneralFilter::slotOverwriteBtnClicked()
{
  TQListBoxItem *item = profileListBox->item( profileListBox->currentItem() );
  if( item != 0 )
    profileManager->overwriteProfile( item->text() );
}

void GeneralFilter::slotRemoveBtnClicked()
{
  TQListBoxItem *item = profileListBox->item( profileListBox->currentItem() );
  if( item != 0 )
  {
    profileManager->deleteProfile( item->text() );
    refreshProfileListBox();
  }
}

void GeneralFilter::slotProfileDoubleClicked( TQListBoxItem *item )
{
  if( item != 0 )
  {
    TQString profileName = item->text();
    profileManager->loadProfile( profileName );
    fltTabs->close( true );
  }
}

void GeneralFilter::slotLoadBtnClicked()
{
  TQListBoxItem *item = profileListBox->item( profileListBox->currentItem() );
  if( item != 0 )
    profileManager->loadProfile( item->text() );
}

#include "generalfilter.moc"
