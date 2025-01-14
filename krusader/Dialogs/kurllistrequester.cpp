/***************************************************************************
                    kurllistrequester.cpp  -  description
                             -------------------
    copyright            : (C) 2005 by Csaba Karai
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

#include "kurllistrequester.h"
#include "../VFS/vfs.h"
#include <tqpixmap.h>
#include <tqcursor.h>
#include <tqlayout.h>
#include <tdefiledialog.h>
#include <tdepopupmenu.h>
#include <kiconloader.h>
#include <tdelocale.h>
#include <tdemessagebox.h>

#define DELETE_ITEM_ID    100

KURLListRequester::KURLListRequester( TQWidget *parent, const char * name ) : TQWidget( parent, name )
{
  TDEIconLoader *iconLoader = new TDEIconLoader();
  TQPixmap imageAdd = iconLoader->loadIcon( "1downarrow", TDEIcon::Panel, 16 );
  TQPixmap imageFolder = iconLoader->loadIcon( "folder", TDEIcon::Panel, 16 );
    
  // Creating the widget
  
  TQGridLayout *urlListRequesterGrid = new TQGridLayout( this );
  urlListRequesterGrid->setSpacing( 0 );
  urlListRequesterGrid->setMargin( 0 );
    
  urlLineEdit = new KLineEdit( this, "urlLineEdit" );
  urlListRequesterGrid->addWidget( urlLineEdit, 0, 0 );
      
  urlListBox = new TQListBox( this, "urlListBox" );
  urlListBox->setSelectionMode( TQListBox::Extended );
  urlListRequesterGrid->addMultiCellWidget( urlListBox, 1, 1, 0, 2 );

  urlAddBtn = new TQToolButton( this, "urlAddBtn" );
  urlAddBtn->setText( "" );
  urlAddBtn->setPixmap( imageAdd );
  urlListRequesterGrid->addWidget( urlAddBtn, 0, 1 );
    
  urlBrowseBtn = new TQToolButton( this, "urlBrowseBtn" );
  urlBrowseBtn->setText( "" );
  urlBrowseBtn->setPixmap( imageFolder );
  urlListRequesterGrid->addWidget( urlBrowseBtn, 0, 2 );

  // add shell completion
    
  completion.setMode( KURLCompletion::FileCompletion );
  urlLineEdit->setCompletionObject( &completion );
  
  // connection table
  
  connect( urlAddBtn, TQ_SIGNAL( clicked() ), this, TQ_SLOT( slotAdd() ) );
  connect( urlBrowseBtn, TQ_SIGNAL( clicked() ), this, TQ_SLOT( slotBrowse() ) );
  connect( urlLineEdit, TQ_SIGNAL( returnPressed(const TQString&) ), this, TQ_SLOT( slotAdd() ) );
  connect( urlListBox, TQ_SIGNAL( rightButtonClicked ( TQListBoxItem *, const TQPoint & ) ), this,
                       TQ_SLOT( slotRightClicked( TQListBoxItem * ) ) );
}

void KURLListRequester::slotAdd()
{
  TQString text = urlLineEdit->text().simplifyWhiteSpace();
  if( text.length() )
  {  
    TQString error = TQString();    
    emit checkValidity( text, error );
    
    if( !error.isNull() )
      KMessageBox::error( this, error );
    else
    {  
      urlListBox->insertItem( text );
      urlLineEdit->clear();
    }
  }
}

void KURLListRequester::slotBrowse()
{
  KURL url = KFileDialog::getExistingURL( TQString(), this );
  if( !url.isEmpty())
    urlLineEdit->setText( vfs::pathOrURL( url ) );
  urlLineEdit->setFocus();
}

void KURLListRequester::keyPressEvent(TQKeyEvent *e)
{
  if( e->key() == Key_Delete )
  {
    if( urlListBox->hasFocus() )
    {
      deleteSelectedItems();
      return;
    }
  }

  TQWidget::keyPressEvent( e );
}

void KURLListRequester::deleteSelectedItems()
{
  int i=0;
  TQListBoxItem *item;

  while( (item = urlListBox->item(i)) )
  {
    if( item->isSelected() )
    {
      urlListBox->removeItem( i );
      continue;
    }
    i++;
  }
}

void KURLListRequester::slotRightClicked( TQListBoxItem *item )
{
  if( item == 0 )
    return;
    
  TDEPopupMenu popupMenu( this );
  popupMenu.insertItem( i18n( "Delete" ), DELETE_ITEM_ID );
  
  switch( popupMenu.exec( TQCursor::pos() ) )
  {
  case DELETE_ITEM_ID:
    if( item->isSelected() )
      deleteSelectedItems();
    else
      urlListBox->removeItem( urlListBox->index( item ) );
    break;
  }
}

KURL::List KURLListRequester::urlList()
{
  KURL::List urls;
  
  TQString text = urlLineEdit->text().simplifyWhiteSpace();
  if (!text.isEmpty())
  {
    TQString error = TQString();
    emit checkValidity( text, error );
    if( error.isNull() )
      urls.append( vfs::fromPathOrURL( text ) );
  }
    
  TQListBoxItem *item = urlListBox->firstItem();
  while ( item )
  {    
    TQString text = item->text().simplifyWhiteSpace();
    
    TQString error = TQString();
    emit checkValidity( text, error );    
    if( error.isNull() )
      urls.append( vfs::fromPathOrURL( text ) );
      
    item = item->next();
  }
    
  return urls;
}

void KURLListRequester::setUrlList( KURL::List urlList )
{
  urlLineEdit->clear();
  urlListBox->clear();

  KURL::List::iterator it;
    
  for ( it = urlList.begin(); it != urlList.end(); ++it )
    urlListBox->insertItem( vfs::pathOrURL(*it) );
}

#include "kurllistrequester.moc"
