/***************************************************************************
                         locate.cpp  -  description
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

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "locate.h"
#include "../krusader.h"
#include "../krusaderview.h"
#include "../Panel/listpanel.h"
#include "../Panel/panelfunc.h"
#include "../defaults.h"
#include "../krservices.h"
#include "../VFS/vfs.h"
#include "../VFS/virt_vfs.h"
#include "../KViewer/krviewer.h"
#include "../panelmanager.h"
#include <klocale.h>
#include <tqhbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqfontmetrics.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <tqcursor.h>
#include <tqeventloop.h>
#include <kfinddialog.h>
#include <kinputdialog.h>
#include <tqregexp.h>
#include <tqdir.h>
#include <tqclipboard.h>
#include <kurldrag.h>
#include <../kicons.h>

// these are the values that will exist in the menu
#define VIEW_ID                     90
#define EDIT_ID                     91
#define FIND_ID                     92
#define FIND_NEXT_ID                93
#define FIND_PREV_ID                94
#define COPY_SELECTED_TO_CLIPBOARD  95
//////////////////////////////////////////////////////////

class LocateListView : public KListView
{
public:
  LocateListView( TQWidget * tqparent, const char * name = 0 ) : KListView( tqparent, name )
  {
  }

  void startDrag()
  {
    KURL::List urls;

    TQListViewItem * item = firstChild();
    while( item )
    {
      if( item->isSelected() )
         urls.push_back( vfs::fromPathOrURL( item->text( 0 ) ) );

      item = item->nextSibling();
    }

    if( urls.count() == 0 )
      return;

    KURLDrag *d = new KURLDrag(urls, this);
    d->setPixmap( FL_LOADICON( "file" ), TQPoint( -7, 0 ) );
    d->dragCopy();
  }
};

KProcess *  LocateDlg::updateProcess = 0;
LocateDlg * LocateDlg::LocateDialog = 0;

LocateDlg::LocateDlg() : KDialogBase(0,0,false,"Locate", KDialogBase::User1 | KDialogBase::User2 | KDialogBase::User3 | KDialogBase::Close,
      KDialogBase::User3, false, i18n("Stop"), i18n("Update DB"), i18n("Locate") ), isFeedToListBox( false )
{
  TQWidget *widget=new TQWidget(this, "locateMainWidget");
  TQGridLayout *grid = new TQGridLayout( widget );
  grid->setSpacing( 6 );
  grid->setMargin( 11 );

  setPlainCaption( i18n( "Krusader::Locate" ) );
  
  TQHBox *hbox = new TQHBox( widget, "locateHBox" );
  TQLabel *label = new TQLabel( i18n( "Search for:" ), hbox, "locateLabel" );
  locateSearchFor = new KHistoryCombo( false, hbox, "locateSearchFor" );
  label->setBuddy( locateSearchFor );
  krConfig->setGroup("Locate");
  TQStringList list = krConfig->readListEntry("Search For");
  locateSearchFor->setMaxCount(25);  // remember 25 items
  locateSearchFor->setHistoryItems(list);
  locateSearchFor->setEditable( true );
  locateSearchFor->setDuplicatesEnabled( false );
  locateSearchFor->tqsetSizePolicy(TQSizePolicy::Expanding,TQSizePolicy::Fixed);
  locateSearchFor->lineEdit()->setFocus();

  grid->addWidget( hbox, 0, 0 );

  TQHBox *hbox2 = new TQHBox( widget, "locateHBox" );
  TQSpacerItem* spacer = new TQSpacerItem( 40, 20, TQSizePolicy::Expanding, TQSizePolicy::Minimum );
  hbox2->tqlayout()->addItem( spacer );
  dontSearchInPath = new TQCheckBox( i18n( "Don't search in path" ), hbox2, "dontSearchInPath" );
  dontSearchInPath->setChecked( krConfig->readBoolEntry("Dont Search In Path") );
  existingFiles = new TQCheckBox( i18n( "Show only the existing files" ), hbox2, "existingFiles" );
  existingFiles->setChecked( krConfig->readBoolEntry("Existing Files") );
  caseSensitive = new TQCheckBox( i18n( "Case Sensitive" ), hbox2, "caseSensitive" );
  caseSensitive->setChecked( krConfig->readBoolEntry("Case Sensitive") );
  grid->addWidget( hbox2, 1, 0 );

  TQFrame *line1 = new TQFrame( widget, "locateLine1" );
  line1->setFrameStyle( TQFrame::HLine | TQFrame::Sunken );
  grid->addWidget( line1, 2, 0 );

  resultList=new LocateListView( widget );  // create the main container

  krConfig->setGroup("Look&Feel");
  resultList->setFont(krConfig->readFontEntry("Filelist Font",_FilelistFont));

  resultList->setAllColumnsShowFocus(true);
  resultList->setVScrollBarMode(TQScrollView::Auto);
  resultList->setHScrollBarMode(TQScrollView::Auto);
  resultList->setShowSortIndicator(false);
  resultList->setSorting(-1);
  resultList->setSelectionMode( TQListView::Extended );

  resultList->addColumn( i18n("Results"), TQFontMetrics(resultList->font()).width("W") * 60 );
  resultList->setColumnWidthMode(0,TQListView::Maximum);

  connect( resultList,TQT_SIGNAL(rightButtonPressed(TQListViewItem *, const TQPoint &, int)),
           this, TQT_SLOT(slotRightClick(TQListViewItem *)));
  connect( resultList,TQT_SIGNAL(doubleClicked(TQListViewItem *)),
           this, TQT_SLOT(slotDoubleClick(TQListViewItem *)));
  connect( resultList,TQT_SIGNAL(returnPressed(TQListViewItem *)),
           this, TQT_SLOT(slotDoubleClick(TQListViewItem *)));
           
  grid->addWidget( resultList, 3, 0 );

  TQFrame *line2 = new TQFrame( widget, "locateLine2" );
  line2->setFrameStyle( TQFrame::HLine | TQFrame::Sunken );
  grid->addWidget( line2, 4, 0 );

  enableButton( KDialogBase::User1, false );  /* disable the stop button */

  if( updateProcess )
  {
    if( updateProcess->isRunning() )
    {
      connect( updateProcess, TQT_SIGNAL(processExited(KProcess *)), this, TQT_SLOT(updateFinished()));
      enableButton( KDialogBase::User2, false );
    }
    else
      updateFinished();
  }

  setMainWidget(widget);
  show();

  LocateDialog = this;
}

void LocateDlg::slotUser1()   /* The stop / feed to listbox button */
{
  if( isFeedToListBox )
    feedToListBox();
  else
    stopping = true;
}

void LocateDlg::slotUser2()   /* The Update DB button */
{
  if( !updateProcess )
  {
    krConfig->setGroup("Locate");

    updateProcess = new KProcess();
    *updateProcess << KrServices::fullPathName( "updatedb" );
    *updateProcess << KrServices::separateArgs( krConfig->readEntry( "UpdateDB Arguments", "" ) );
    
    connect( updateProcess, TQT_SIGNAL(processExited(KProcess *)), this, TQT_SLOT(updateFinished()));
    updateProcess->start(KProcess::NotifyOnExit);
    enableButton( KDialogBase::User2, false );
  }
}

void LocateDlg::updateFinished()
{
  delete updateProcess;
  updateProcess = 0;
  enableButton( KDialogBase::User2, true );
}

void LocateDlg::slotUser3()   /* The locate button */
{
  locateSearchFor->addToHistory(locateSearchFor->currentText());
  TQStringList list = locateSearchFor->historyItems();
  krConfig->setGroup("Locate");
  krConfig->writeEntry("Search For", list);
  krConfig->writeEntry("Dont Search In Path", dontSearchPath = dontSearchInPath->isChecked() );
  krConfig->writeEntry("Existing Files", onlyExist = existingFiles->isChecked() );
  krConfig->writeEntry("Case Sensitive", isCs = caseSensitive->isChecked() );

  if( !KrServices::cmdExist( "locate" ) )
  {
    KMessageBox::error(0,
      i18n("Can't start 'locate'! Check the 'Dependencies' page in konfigurator."));
    return;
  }
  
  resultList->clear();
  lastItem = 0;
  remaining = "";

  enableButton( KDialogBase::User3, false );  /* disable the locate button */
  enableButton( KDialogBase::User1, true );   /* enable the stop button */
  setButtonText( KDialogBase::User1, i18n( "Stop" ) ); /* the button behaves as stop */
  isFeedToListBox = false;
  resultList->setFocus();

  tqApp->processEvents();

  stopping = false;
  
  KProcess locateProc;
  connect( &locateProc, TQT_SIGNAL( receivedStdout(KProcess *, char *, int) ),
            this, TQT_SLOT( processStdout(KProcess *, char *, int) ) );
  connect( &locateProc, TQT_SIGNAL( receivedStderr(KProcess *, char *, int) ),
            this, TQT_SLOT( processStderr(KProcess *, char *, int) ) );

  locateProc << KrServices::fullPathName( "locate" );
  if( !isCs )
    locateProc << "-i";
  locateProc << (pattern = locateSearchFor->currentText());
  
  if( !pattern.startsWith( "*" ) )
    pattern = "*" + pattern;
  if( !pattern.endsWith( "*" ) )
    pattern = pattern + "*";
  
  collectedErr = "";
  bool result = !locateProc.start( KProcess::Block, KProcess::AllOutput );
  if( !collectedErr.isEmpty() && ( !locateProc.normalExit() || locateProc.exitStatus() ) )
  {
     KMessageBox::error( krApp, i18n( "Locate produced the following error message:\n\n" ) + collectedErr );
  }else if ( result )
  {
     KMessageBox::error( krApp, i18n( "Error during the start of 'locate' process!" ) );
  }
  enableButton( KDialogBase::User3, true );  /* enable the locate button */
  
  if( resultList->childCount() == 0 )
  {
    locateSearchFor->setFocus();
    enableButton( KDialogBase::User1, false ); /* disable the stop button */
    isFeedToListBox = false;
  }else{
    setButtonText( KDialogBase::User1, i18n("Feed to listbox") ); /* feed to listbox */
    isFeedToListBox = true;
  }
}

void LocateDlg::processStdout(KProcess *proc, char *buffer, int length)
{
  char *buf = new char[ length+1 ];
  memcpy( buf, buffer, length );
  buf[ length ] = 0;

  remaining += TQString::fromLocal8Bit( buf );
  delete []buf;

  TQStringList list = TQStringList::split("\n", remaining );
  int items = list.size();

  for ( TQStringList::Iterator it = list.begin(); it != list.end(); ++it )
  {
    if( --items == 0 && buffer[length-1] != '\n' )
      remaining = *it;
    else
    {
      if( dontSearchPath )
      {
        TQRegExp regExp( pattern, isCs, true );
        TQString fileName = (*it).stripWhiteSpace();
        if( fileName.endsWith( "/" ) && fileName != "/" )
          fileName.truncate( fileName.length() -1 );
        fileName = fileName.mid( fileName.tqfindRev( '/' ) + 1 );
        
        if( !regExp.exactMatch( fileName ) )
          continue;        
      }
      if( onlyExist )
      {
        KFileItem file(KFileItem::Unknown, KFileItem::Unknown, (*it).stripWhiteSpace() );
        if( !file.isReadable() )
          continue;
      }
      
      if( lastItem )    
        lastItem = new KListViewItem( resultList, lastItem, *it );
      else
        lastItem = new KListViewItem( resultList, *it );

      lastItem->setDragEnabled( true );
    }
  }

  if( stopping )
    proc->kill( SIGKILL );
  
  tqApp->processEvents();
}

void LocateDlg::processStderr(KProcess *, char *buffer, int length)
{
  char *buf = new char[ length+1 ];
  memcpy( buf, buffer, length );
  buf[ length ] = 0;

  collectedErr += TQString::fromLocal8Bit( buf );
  delete []buf;  
}

void LocateDlg::slotRightClick(TQListViewItem *item)
{
  if ( !item )
    return;

  // create the menu
  KPopupMenu popup;
  popup.insertTitle(i18n("Locate"));

  popup.insertItem(i18n("View (F3)"), VIEW_ID);
  popup.insertItem(i18n("Edit (F4)"), EDIT_ID);

  popup.insertSeparator();

  popup.insertItem(i18n("Find (Ctrl+F)"), FIND_ID);
  popup.insertItem(i18n("Find next (Ctrl+N)"), FIND_NEXT_ID);
  popup.insertItem(i18n("Find previous (Ctrl+P)"), FIND_PREV_ID);

  popup.insertSeparator();

  popup.insertItem(i18n("Copy selected to clipboard"), COPY_SELECTED_TO_CLIPBOARD);


  int result=popup.exec(TQCursor::pos());

  // check out the user's option
  switch (result)
  {
  case VIEW_ID:
  case EDIT_ID:
  case FIND_ID:
  case FIND_NEXT_ID:
  case FIND_PREV_ID:
  case COPY_SELECTED_TO_CLIPBOARD:
    operate( item, result );
    break;
  }
}

void LocateDlg::slotDoubleClick(TQListViewItem *item)
{
  if ( !item )
    return;

  TQString dirName = item->text(0);
  TQString fileName;

  if( !TQDir( dirName ).exists() )
  {
    fileName = dirName.mid( dirName.tqfindRev( '/' ) + 1 );
    dirName.truncate( dirName.tqfindRev( '/' ) );
  }
    
  ACTIVE_FUNC->openUrl(vfs::fromPathOrURL( dirName ), fileName );
  KDialogBase::accept();
}

void LocateDlg::keyPressEvent( TQKeyEvent *e )
{
  if( Krusader::actCopy->shortcut().contains( KKey( e ) ) )
  {
    operate( 0, COPY_SELECTED_TO_CLIPBOARD );
    e->accept();
    return;
  }

  switch ( e->key() )
  {
  case Key_M :
    if( e->state() == ControlButton )
    {
      resultList->setFocus();
      e->accept();
    }
    break;
  case Key_F3 :
    if( resultList->currentItem() )
      operate( resultList->currentItem(), VIEW_ID );
    break;
  case Key_F4 :
    if( resultList->currentItem() )
      operate( resultList->currentItem(), EDIT_ID );
    break;
  case Key_N :
    if ( e->state() == ControlButton )
      operate( resultList->currentItem(), FIND_NEXT_ID );
    break;
  case Key_P :
    if ( e->state() == ControlButton )
      operate( resultList->currentItem(), FIND_PREV_ID );
    break;
  case Key_F :
    if ( e->state() == ControlButton )
      operate( resultList->currentItem(), FIND_ID );
    break;
  }

  TQDialog::keyPressEvent( e );
}

void LocateDlg::operate( TQListViewItem *item, int task )
{
  KURL name;
  if( item != 0 )
    name = vfs::fromPathOrURL( item->text( 0 ) );
  
  switch ( task )
  {
  case VIEW_ID:
    KrViewer::view( name, this ); // view the file
    break;
  case EDIT_ID:
    KrViewer::edit( name, this ); // view the file
    break;
  case FIND_ID:
    {
      krConfig->setGroup("Locate");
      long options = krConfig->readNumEntry("Find Options", 0);
      TQStringList list = krConfig->readListEntry("Find Patterns");
      
      KFindDialog dlg( this, "locateFindDialog", options, list );
      if ( dlg.exec() != TQDialog::Accepted )
        return;

      if( list.first() != ( findPattern = dlg.pattern() ) )
        list.push_front( dlg.pattern() );
        
      krConfig->writeEntry( "Find Options", findOptions = dlg.options() );
      krConfig->writeEntry( "Find Patterns", list );

      if( !( findOptions & KFindDialog::FromCursor ) )
        resultList->setCurrentItem( ( findOptions & KFindDialog::FindBackwards ) ?
                                    resultList->lastItem() : resultList->firstChild() );

      findCurrentItem = (KListViewItem *)resultList->currentItem();
      
      if( tqfind() && findCurrentItem )
        resultList->setCurrentItem( findCurrentItem );
      else
        KMessageBox::information( 0, i18n( "Search string not found!" ) );
        
      resultList->ensureItemVisible( resultList->currentItem() );
    }
    break;      
  case FIND_NEXT_ID:
  case FIND_PREV_ID:
    {
      if( task == FIND_PREV_ID )
        findOptions ^= KFindDialog::FindBackwards;
      
      findCurrentItem = (KListViewItem *)resultList->currentItem();
      nextLine();

      if( tqfind() && findCurrentItem )
        resultList->setCurrentItem( findCurrentItem );
      else
        KMessageBox::information( 0, i18n( "Search string not found!" ) );

      resultList->ensureItemVisible( resultList->currentItem() );

      if( task == FIND_PREV_ID )
        findOptions ^= KFindDialog::FindBackwards;
    }
    break;
  case COPY_SELECTED_TO_CLIPBOARD:
    {
      KURL::List urls;

      TQListViewItem * item = resultList->firstChild();
      while( item )
      {
        if( item->isSelected() )
           urls.push_back( vfs::fromPathOrURL( item->text( 0 ) ) );

        item = item->nextSibling();
      }

      if( urls.count() == 0 )
        return;

      KURLDrag *d = new KURLDrag(urls, this);
      d->setPixmap( FL_LOADICON( "file" ), TQPoint( -7, 0 ) );
      TQApplication::tqclipboard()->setData( d );
    }
    break;
  }
}

void LocateDlg::nextLine()
{
  if( findOptions & KFindDialog::FindBackwards )
    findCurrentItem = (KListViewItem *)findCurrentItem->itemAbove();
  else
    findCurrentItem = (KListViewItem *)findCurrentItem->itemBelow();
}

bool LocateDlg::tqfind()
{
  while( findCurrentItem )
  {
    TQString item = findCurrentItem->text( 0 );

    if( findOptions & KFindDialog::RegularExpression )
    {
      if( item.tqcontains( TQRegExp( findPattern, findOptions & KFindDialog::CaseSensitive ) ) )
        return true;
    }
    else
    {
      if( item.tqcontains( findPattern, findOptions & KFindDialog::CaseSensitive ) )
        return true;
    }
    
    nextLine();
  }
  
  return false;
}

void LocateDlg::feedToListBox()
{
  virt_vfs v(0,true);
  v.vfs_refresh( KURL( "/" ) );
  
  krConfig->setGroup( "Locate" );  
  int listBoxNum = krConfig->readNumEntry( "Feed To Listbox Counter", 1 );  
  TQString queryName;
  do {
    queryName = i18n("Locate results")+TQString( " %1" ).tqarg( listBoxNum++ );
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
  TQListViewItem * item = resultList->firstChild();
  while( item )
  {
    urlList.push_back( vfs::fromPathOrURL( item->text( 0 ) ) );
    item = item->nextSibling();
  }
  KURL url = KURL::fromPathOrURL(TQString("virt:/")+ queryName);
  v.vfs_refresh( url );
  v.vfs_addFiles( &urlList, KIO::CopyJob::Copy, 0 );
  //ACTIVE_FUNC->openUrl(url);  
  ACTIVE_MNG->slotNewTab(url.prettyURL());
  accept();
}

void LocateDlg::reset()
{
  locateSearchFor->lineEdit()->setFocus();
  locateSearchFor->lineEdit()->selectAll();
}

#include "locate.moc"
