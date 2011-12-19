/***************************************************************************
                                 krsearchdialog.cpp
                             -------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
    web site		 : http://krusader.sourceforge.net
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
#include "../defaults.h"
#include "../panelmanager.h"
#include "../VFS/vfs.h"
#include "../krusaderview.h"
#include "../Panel/listpanel.h"
#include "../Panel/panelfunc.h"
#include "../Dialogs/krdialogs.h"
#include "../VFS/virt_vfs.h"
#include "../KViewer/krviewer.h"
#include "krsearchmod.h"
#include "krsearchdialog.h"

#include <kinputdialog.h>
#include <tqregexp.h>
#include <tqfontmetrics.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <tqcursor.h>
#include <tqclipboard.h>
#include <tqheader.h>
#include <kurldrag.h>
#include <../kicons.h>

class SearchListView : public TQListView
{
public:
  SearchListView( TQWidget * parent, const char * name ) : TQListView( parent, name )
  {
  }

  void startDrag() 
  {
    KURL::List urls;

    TQListViewItem * item = firstChild();
    while( item )
    {
      if( item->isSelected() )
      {
         TQString name = item->text(1);
         name += (name.endsWith( "/" ) ? item->text(0) : "/" + item->text(0) );
         urls.push_back( vfs::fromPathOrURL( name ) );
      }
      item = item->nextSibling();
    }

    if( urls.count() == 0 )
      return;

    KURLDrag *d = new KURLDrag(urls, this);
    d->setPixmap( FL_LOADICON( "file" ), TQPoint( -7, 0 ) );
    d->dragCopy();
  }
};



KrSearchDialog *KrSearchDialog::SearchDialog = 0;

TQString KrSearchDialog::lastSearchText = "*";
int KrSearchDialog::lastSearchType = 0;
bool KrSearchDialog::lastSearchForCase = false;
bool KrSearchDialog::lastRemoteContentSearch = false;
bool KrSearchDialog::lastContainsWholeWord = false;
bool KrSearchDialog::lastContainsWithCase = true;
bool KrSearchDialog::lastSearchInSubDirs = true;
bool KrSearchDialog::lastSearchInArchives = false;
bool KrSearchDialog::lastFollowSymLinks = false;

// class starts here /////////////////////////////////////////
KrSearchDialog::KrSearchDialog( TQString profile, TQWidget* parent,  const char* name, bool modal, WFlags fl )
                : TQDialog( parent, name, modal, fl ), query(0), searcher(0)
{
  setCaption( i18n( "Krusader::Search" ) );

  TQGridLayout* searchBaseLayout = new TQGridLayout( this );
  searchBaseLayout->setSpacing( 6 );
  searchBaseLayout->setMargin( 11 );

  // creating the dialog buttons ( Search, Stop, Close )

  TQHBoxLayout* buttonsLayout = new TQHBoxLayout();
  buttonsLayout->setSpacing( 6 );
  buttonsLayout->setMargin( 0 );

  profileManager = new ProfileManager( "SearcherProfile", this, "profileManager" );
  buttonsLayout->addWidget( profileManager );

  TQSpacerItem* spacer = new TQSpacerItem( 20, 20, TQSizePolicy::Expanding, TQSizePolicy::Minimum );
  buttonsLayout->addItem( spacer );

  mainFeedToListBoxBtn = new TQPushButton( this, "mainFeedToListBoxBtn" );
  mainFeedToListBoxBtn->setText( i18n( "Feed to listbox" ) );
  mainFeedToListBoxBtn->setEnabled(false);
  buttonsLayout->addWidget( mainFeedToListBoxBtn );

  mainSearchBtn = new TQPushButton( this, "mainSearchBtn" );
  mainSearchBtn->setText( i18n( "Search" ) );
  mainSearchBtn->setDefault(true);
  buttonsLayout->addWidget( mainSearchBtn );

  mainStopBtn = new TQPushButton( this, "mainStopBtn" );
  mainStopBtn->setEnabled( false );
  mainStopBtn->setText( i18n( "Stop" ) );
  buttonsLayout->addWidget( mainStopBtn );

  mainCloseBtn = new TQPushButton( this, "mainCloseBtn" );
  mainCloseBtn->setText( i18n( "Close" ) );
  buttonsLayout->addWidget( mainCloseBtn );

  searchBaseLayout->addLayout( buttonsLayout, 1, 0 );

  // creating the searcher tabs

  searcherTabs = new TQTabWidget( this, "searcherTabs" );

  filterTabs = FilterTabs::addTo( searcherTabs, FilterTabs::Default | FilterTabs::HasRemoteContentSearch );
  generalFilter = (GeneralFilter *)filterTabs->get( "GeneralFilter" );

  resultTab = new TQWidget( searcherTabs, "resultTab" );
  resultLayout = new TQGridLayout( resultTab );
  resultLayout->setSpacing( 6 );
  resultLayout->setMargin( 11 );

  // creating the result tab

  TQHBoxLayout* resultLabelLayout = new TQHBoxLayout();
  resultLabelLayout->setSpacing( 6 );
  resultLabelLayout->setMargin( 0 );

  foundLabel = new TQLabel( resultTab, "foundLabel" );
  foundLabel->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)1, foundLabel->sizePolicy().hasHeightForWidth() ) );
  foundLabel->setFrameShape( TQLabel::StyledPanel );
  foundLabel->setFrameShadow( TQLabel::Sunken );
  foundLabel->setText( i18n( "Found 0 matches." ) );
  resultLabelLayout->addWidget( foundLabel );

  searchingLabel = new KSqueezedTextLabel( resultTab, "searchingLabel" );
  searchingLabel->setFrameShape( TQLabel::StyledPanel );
  searchingLabel->setFrameShadow( TQLabel::Sunken );
  searchingLabel->setText( "" );
  resultLabelLayout->addWidget( searchingLabel );

  resultLayout->addLayout( resultLabelLayout, 2, 0 );

  // creating the result list view

  resultsList = new SearchListView( resultTab, "resultsList" );
  resultsList->addColumn( i18n( "Name" ) );
  resultsList->addColumn( i18n( "Location" ) );
  resultsList->addColumn( i18n( "Size" ) );
  resultsList->addColumn( i18n( "Date" ) );
  resultsList->addColumn( i18n( "Permissions" ) );

  resultsList->setSorting(1); // sort by location
  resultsList->setSelectionMode( TQListView::Extended );

  // fix the results list
  // => make the results font smaller
  TQFont resultsFont(  resultsList->font() );
  resultsFont.setPointSize(resultsFont.pointSize()-1);
  resultsList->setFont(resultsFont);

  resultsList->setAllColumnsShowFocus(true);
  for (int i=0; i<5; ++i) // don't let it resize automatically
    resultsList->setColumnWidthMode(i, TQListView::Manual);

  int i=TQFontMetrics(resultsList->font()).width("W");
  int j=TQFontMetrics(resultsList->font()).width("0");
  j=(i>j ? i : j);

  resultsList->setColumnWidth(0, krConfig->readNumEntry("Name Width", j*14) );
  resultsList->setColumnWidth(1, krConfig->readNumEntry("Path Width", j*25) );
  resultsList->setColumnWidth(2, krConfig->readNumEntry("Size Width", j*6) );
  resultsList->setColumnWidth(3, krConfig->readNumEntry("Date Width", j*7) );
  resultsList->setColumnWidth(4, krConfig->readNumEntry("Perm Width", j*7) );
  resultsList->setColumnAlignment( 2, AlignRight );

  resultsList->header()->setStretchEnabled( true, 1 );

  resultLayout->addWidget( resultsList, 0, 0 );

  TQHBoxLayout* foundTextLayout = new TQHBoxLayout();
  foundTextLayout->setSpacing( 6 );
  foundTextLayout->setMargin( 0 );
  
  TQLabel *l1 = new TQLabel(resultTab);
  l1->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)1, l1->sizePolicy().hasHeightForWidth() ) );
  l1->setFrameShape( TQLabel::StyledPanel );
  l1->setFrameShadow( TQLabel::Sunken );
  l1->setText(i18n("Text found:"));
  foundTextLayout->addWidget( l1 );

  foundTextLabel = new KrSqueezedTextLabel(resultTab);
  foundTextLabel->setFrameShape( TQLabel::StyledPanel );
  foundTextLabel->setFrameShadow( TQLabel::Sunken );
  foundTextLabel->setText("");
  foundTextLayout->addWidget( foundTextLabel );
  resultLayout->addLayout(foundTextLayout, 1, 0);
  
  searcherTabs->insertTab( resultTab, i18n( "&Results" ) );

  searchBaseLayout->addWidget( searcherTabs, 0, 0 );

  // signals and slots connections

  connect( mainSearchBtn, TQT_SIGNAL( clicked() ), this, TQT_SLOT( startSearch() ) );
  connect( mainStopBtn, TQT_SIGNAL( clicked() ), this, TQT_SLOT( stopSearch() ) );
  connect( resultsList, TQT_SIGNAL( returnPressed(TQListViewItem*) ), this,
  	TQT_SLOT( resultDoubleClicked(TQListViewItem*) ) );
  connect( resultsList, TQT_SIGNAL( doubleClicked(TQListViewItem*) ), this,
  	TQT_SLOT( resultDoubleClicked(TQListViewItem*) ) );
  connect( resultsList, TQT_SIGNAL( currentChanged(TQListViewItem*) ), this,
  		TQT_SLOT( resultClicked(TQListViewItem*) ) );
  connect( resultsList, TQT_SIGNAL( clicked(TQListViewItem*) ), this,
  		TQT_SLOT( resultClicked(TQListViewItem*) ) );
  connect( resultsList, TQT_SIGNAL( rightButtonClicked(TQListViewItem*,const TQPoint&,int) ), this, TQT_SLOT( rightClickMenu(TQListViewItem*, const TQPoint&, int) ) );
  connect( mainCloseBtn, TQT_SIGNAL( clicked() ), this, TQT_SLOT( closeDialog() ) );
  connect( mainFeedToListBoxBtn, TQT_SIGNAL( clicked() ), this, TQT_SLOT( feedToListBox() ) );

  connect( profileManager, TQT_SIGNAL( loadFromProfile( TQString ) ), filterTabs, TQT_SLOT( loadFromProfile( TQString ) ) );
  connect( profileManager, TQT_SIGNAL( saveToProfile( TQString ) ), filterTabs, TQT_SLOT( saveToProfile( TQString ) ) );

  // tab order

  setTabOrder( mainSearchBtn, mainCloseBtn );
  setTabOrder( mainCloseBtn, mainStopBtn );
  setTabOrder( mainStopBtn, searcherTabs );
  setTabOrder( searcherTabs, resultsList );

  krConfig->setGroup( "Search" );
  int sx = krConfig->readNumEntry( "Window Width",  -1 );
  int sy = krConfig->readNumEntry( "Window Height",  -1 );

  if( sx != -1 && sy != -1 )
    resize( sx, sy );

  if( krConfig->readBoolEntry( "Window Maximized",  false ) )
      showMaximized();
  else
      show();

  generalFilter->searchFor->setFocus();

  isSearching = closed = false;

  // finaly, load a profile of apply defaults:

  if ( profile.isEmpty() ) {
    // load the last used values
    generalFilter->searchFor->setEditText( lastSearchText );
    generalFilter->searchFor->lineEdit()->selectAll();
    generalFilter->ofType->setCurrentItem( lastSearchType );
    generalFilter->searchForCase->setChecked( lastSearchForCase );
    generalFilter->remoteContentSearch->setChecked( lastRemoteContentSearch );
    generalFilter->containsWholeWord->setChecked( lastContainsWholeWord );
    generalFilter->containsTextCase->setChecked( lastContainsWithCase );
    generalFilter->searchInDirs->setChecked( lastSearchInSubDirs );
    generalFilter->searchInArchives->setChecked( lastSearchInArchives );
    generalFilter->followLinks->setChecked( lastFollowSymLinks );
    // the path in the active panel should be the default search location
    generalFilter->searchIn->lineEdit()->setText( ACTIVE_PANEL->virtualPath().prettyURL() );
  }
  else
    profileManager->loadProfile( profile ); // important: call this _after_ you've connected profileManager ot the loadFromProfile!!
}

void KrSearchDialog::closeDialog( bool isAccept )
{
  // stop the search if it's on-going
  if (searcher!=0) {
    delete searcher;
    searcher = 0;
  }

  // saving the searcher state

  krConfig->setGroup( "Search" );

  krConfig->writeEntry("Window Width", sizeX );
  krConfig->writeEntry("Window Height", sizeY );
  krConfig->writeEntry("Window Maximized", isMaximized() );

  krConfig->writeEntry("Name Width",  resultsList->columnWidth( 0 ) );
  krConfig->writeEntry("Path Width",  resultsList->columnWidth( 1 ) );
  krConfig->writeEntry("Size Width",  resultsList->columnWidth( 2 ) );
  krConfig->writeEntry("Date Width",  resultsList->columnWidth( 3 ) );
  krConfig->writeEntry("Perm Width",  resultsList->columnWidth( 4 ) );

  lastSearchText = generalFilter->searchFor->currentText();
  lastSearchType = generalFilter->ofType->currentItem();
  lastSearchForCase = generalFilter->searchForCase->isChecked();
  lastRemoteContentSearch = generalFilter->remoteContentSearch->isChecked();
  lastContainsWholeWord = generalFilter->containsWholeWord->isChecked();
  lastContainsWithCase = generalFilter->containsTextCase->isChecked();
  lastSearchInSubDirs = generalFilter->searchInDirs->isChecked();
  lastSearchInArchives = generalFilter->searchInArchives->isChecked();
  lastFollowSymLinks = generalFilter->followLinks->isChecked();
  hide();
  
  SearchDialog = 0;
  if( isAccept )
    TQDialog::accept();
  else
    TQDialog::reject();

  this->deleteLater();
}

void KrSearchDialog::reject() {
  closeDialog( false );
}

void KrSearchDialog::resizeEvent( TQResizeEvent *e )
{
  if( !isMaximized() )
  {
    sizeX = e->size().width();
    sizeY = e->size().height();
  }
}

void KrSearchDialog::found(TQString what, TQString where, KIO::filesize_t size, time_t mtime, TQString perm, TQString foundText){
  // convert the time_t to struct tm
  struct tm* t=localtime((time_t *)&mtime);
  TQDateTime tmp(TQDate(t->tm_year+1900, t->tm_mon+1, t->tm_mday), TQTime(t->tm_hour, t->tm_min));
  ResultListViewItem *it =new ResultListViewItem(resultsList, what,
  	 where.replace(TQRegExp("\\\\"),"#"), size, tmp, perm);
  TQString totals = TQString(i18n("Found %1 matches.")).tqarg(resultsList->childCount());
  foundLabel->setText(totals);

  if (!foundText.isEmpty()) it->setFoundText(foundText);
}

bool KrSearchDialog::gui2query() {
  // prepare the query ...
  /////////////////// names, locations and greps
  if (query!=0) { delete query; query = 0; }
  query = new KRQuery();

  return filterTabs->fillQuery( query );
}

void KrSearchDialog::startSearch() {

  // prepare the query /////////////////////////////////////////////
  if (!gui2query()) return;

  // first, informative messages
  if ( query->searchInArchives() ) {
    KMessageBox::information(this, i18n("Since you chose to also search in archives, "
                                        "note the following limitations:\n"
                                        "You cannot search for text (grep) while doing"
                                        " a search that includes archives."), 0, "searchInArchives");
  }

  // prepare the gui ///////////////////////////////////////////////
  mainSearchBtn->setEnabled(false);
  mainCloseBtn->setEnabled(false);
  mainStopBtn->setEnabled(true);
  mainFeedToListBoxBtn->setEnabled(false);
  resultsList->clear();
  searchingLabel->setText("");
  foundLabel->setText(i18n("Found 0 matches."));
  searcherTabs->setCurrentPage(2); // show the results page
  foundTextLabel->setText("");
  tqApp->processEvents();

  // start the search.
  if (searcher != 0) {
    delete searcher;
    searcher = 0;
  }
  searcher  = new KRSearchMod(query);
  connect(searcher, TQT_SIGNAL(searching(const TQString&)),
          searchingLabel, TQT_SLOT(setText(const TQString&)));
  connect(searcher, TQT_SIGNAL(found(TQString,TQString,KIO::filesize_t,time_t,TQString,TQString)),
                this, TQT_SLOT(found(TQString,TQString,KIO::filesize_t,time_t,TQString,TQString)));
  connect(searcher, TQT_SIGNAL(finished()), this, TQT_SLOT(stopSearch()));

  isSearching = true;
  searcher->start();
  isSearching = false;
  if( closed )
    emit closeDialog();
}

void KrSearchDialog::stopSearch() {
  if (searcher!=0) {
    searcher->stop();
    disconnect(searcher,0,0,0);
    delete query;
    query = 0;
  }

  // gui stuff
  mainSearchBtn->setEnabled(true);
  mainCloseBtn->setEnabled(true);
  mainStopBtn->setEnabled(false);
  if( resultsList->childCount() )
    mainFeedToListBoxBtn->setEnabled( true );
  searchingLabel->setText(i18n("Finished searching."));
}

void KrSearchDialog::resultDoubleClicked(TQListViewItem* i) {
  ACTIVE_FUNC->openUrl(vfs::fromPathOrURL(i->text(1)),i->text(0));
  showMinimized();
}

void KrSearchDialog::resultClicked(TQListViewItem* i) {
	ResultListViewItem *it = dynamic_cast<ResultListViewItem*>(i);
	if( it == 0 )
		return;                
	if (!it->foundText().isEmpty()) {
		// ugly hack: find the <b> and </b> in the text, then
		// use it to set the are which we don't want squeezed
		int idx = it->foundText().find("<b>")-4; // take "<qt>" into account
		int length = it->foundText().find("</b>")-idx+4;
		foundTextLabel->setText(it->foundText(), idx, length);
	}
}

void KrSearchDialog::closeEvent(TQCloseEvent *e)
{                     /* if searching is in progress we must not close the window */
  if( isSearching )   /* because tqApp->processEvents() is called by the searcher and */
  {                   /* at window desruction, the searcher object will be deleted */
    stopSearch();         /* instead we stop searching */
    closed = true;        /* and after stopping: startSearch can close the window */
    e->ignore();          /* ignoring the close event */
  }
  else
    TQDialog::closeEvent( e );   /* if no searching, let TQDialog handle the event */
}

void KrSearchDialog::keyPressEvent(TQKeyEvent *e)
{
  KKey pressedKey( e );

  if( isSearching && e->key() == Key_Escape ) /* at searching we must not close the window */
  {
    stopSearch();         /* so we simply stop searching */
    return;
  }
  if( resultsList->hasFocus() )
  {
    if( e->key() == Key_F4 )
    {
      if (!generalFilter->containsText->currentText().isEmpty() && TQApplication::tqclipboard()->text() != generalFilter->containsText->currentText())
        TQApplication::tqclipboard()->setText(generalFilter->containsText->currentText());
      editCurrent();
      return;
    }
    else if( e->key() == Key_F3 )
    {
      if (!generalFilter->containsText->currentText().isEmpty() && TQApplication::tqclipboard()->text() != generalFilter->containsText->currentText())
        TQApplication::tqclipboard()->setText(generalFilter->containsText->currentText());
      viewCurrent();
      return;
    }
    else if( Krusader::actCopy->shortcut().contains( pressedKey ) )
    {
      copyToClipBoard();
      return;
    }
  }

  TQDialog::keyPressEvent( e );
}

void KrSearchDialog::editCurrent()
{
  TQListViewItem *current = resultsList->currentItem();
  if( current )
  {
    TQString name = current->text(1);
    name += (name.endsWith( "/" ) ? current->text(0) : "/" + current->text(0) );
    KURL url = vfs::fromPathOrURL( name );
    KrViewer::edit( url, this );
  }
}

void KrSearchDialog::viewCurrent()
{
  TQListViewItem *current = resultsList->currentItem();
  if( current )
  {
    TQString name = current->text(1);
    name += (name.endsWith( "/" ) ? current->text(0) : "/" + current->text(0) );
    KURL url = vfs::fromPathOrURL( name );
    KrViewer::view( url, this );
  }
}

void KrSearchDialog::rightClickMenu(TQListViewItem *item, const TQPoint&, int)
{
  // these are the values that will exist in the menu
  #define EDIT_FILE_ID                110
  #define VIEW_FILE_ID                111
  #define COPY_SELECTED_TO_CLIPBOARD  112
  //////////////////////////////////////////////////////////
  if (!item)
    return;

  // create the menu
  KPopupMenu popup;
  popup.insertTitle(i18n("Krusader Search"));

  popup.insertItem(i18n("View File (F3)"),            VIEW_FILE_ID);
  popup.insertItem(i18n("Edit File (F4)"),            EDIT_FILE_ID);
  popup.insertItem(i18n("Copy selected to clipboard"),COPY_SELECTED_TO_CLIPBOARD);

  int result=popup.exec(TQCursor::pos());

  // check out the user's option
  switch (result)
  {
    case VIEW_FILE_ID:
      viewCurrent();
      break;
    case EDIT_FILE_ID:
      editCurrent();
      break;
    case COPY_SELECTED_TO_CLIPBOARD:
      copyToClipBoard();
      break;
    default:    // the user clicked outside of the menu
      break;
  }
}

void KrSearchDialog::feedToListBox()
{
  virt_vfs v(0,true);
  v.vfs_refresh( KURL( "/" ) );

  krConfig->setGroup( "Search" );
  int listBoxNum = krConfig->readNumEntry( "Feed To Listbox Counter", 1 );
  TQString queryName;
  do {
    queryName = i18n("Search results")+TQString( " %1" ).tqarg( listBoxNum++ );
  }while( v.vfs_search( queryName ) != 0 );
  krConfig->writeEntry( "Feed To Listbox Counter", listBoxNum );

  krConfig->setGroup( "Advanced" );
  if ( krConfig->readBoolEntry( "Confirm Feed to Listbox",  _ConfirmFeedToListbox ) ) {
    bool ok;
    queryName = KInputDialog::getText(
                i18n("Query name"),		// Caption
                i18n("Here you can name the file collection"),	// Questiontext
                queryName,	// Default
                &ok );
     if ( ! ok)
       return;
  }

  KURL::List urlList;
  TQListViewItem * item = resultsList->firstChild();
  while( item )
  {
    TQString name = item->text(1);
    name += (name.endsWith( "/" ) ? item->text(0) : "/" + item->text(0) );
    urlList.push_back( vfs::fromPathOrURL( name ) );
    item = item->nextSibling();
  }
  KURL url = KURL::fromPathOrURL( TQString("virt:/") + queryName );
  v.vfs_refresh( url );
  v.vfs_addFiles( &urlList, KIO::CopyJob::Copy, 0 );
  //ACTIVE_FUNC->openUrl(url);
  ACTIVE_MNG->slotNewTab(url.prettyURL());
  closeDialog();
}

void KrSearchDialog::copyToClipBoard()
{
  KURL::List urls;

  TQListViewItem * item = resultsList->firstChild();
  while( item )
  {
    if( item->isSelected() )
    {
       TQString name = item->text(1);
       name += (name.endsWith( "/" ) ? item->text(0) : "/" + item->text(0) );
       urls.push_back( vfs::fromPathOrURL( name ) );
    }
    item = item->nextSibling();
  }

  if( urls.count() == 0 )
    return;

  KURLDrag *d = new KURLDrag(urls, this);
  d->setPixmap( FL_LOADICON( "file" ), TQPoint( -7, 0 ) );
  TQApplication::tqclipboard()->setData( d );
}

#include "krsearchdialog.moc"
