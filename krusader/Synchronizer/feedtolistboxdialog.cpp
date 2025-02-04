/***************************************************************************
                     feedtolistboxdialog.cpp  -  description
                             -------------------
    copyright            : (C) 2006 + by Csaba Karai
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

#include "feedtolistboxdialog.h"
#include "synchronizer.h"
#include "synchronizergui.h"
#include "../VFS/vfs.h"
#include "../VFS/virt_vfs.h"
#include "../krusader.h"
#include "../krusaderview.h"
#include "../panelmanager.h"
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <tqcheckbox.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqlabel.h>
#include <tqhbox.h>
#include <tqcombobox.h>
#include <tqframe.h>

#define  S_LEFT        0
#define  S_RIGHT       1
#define  S_BOTH        2

FeedToListBoxDialog::FeedToListBoxDialog(TQWidget *parent,  const char *name, Synchronizer *sync,
    TQListView *syncL, bool equOK) : KDialogBase( parent, name, true, i18n( "Krusader::Feed to listbox" ),
    KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::User1, Ok, true, KStdGuiItem::clear() ),
    synchronizer( sync ), syncList( syncL ), equalAllowed( equOK ), accepted( false ) {
  
  // autodetecting the parameters

  int selectedNum = 0;
  int itemNum = 0;
  int leftExistingNum = 0;
  int rightExistingNum = 0;

  TQListViewItemIterator it( syncList );
  while( it.current() ) {
    SynchronizerGUI::SyncViewItem *item = (SynchronizerGUI::SyncViewItem *) it.current();
    SynchronizerFileItem *syncItem = item->synchronizerItemRef();

    if( syncItem && syncItem->isMarked() ) {
      if( item->isSelected() || syncItem->task() != TT_EQUALS || equalAllowed ) {
        itemNum++;
        if( item->isSelected() )
          selectedNum++;

        if( syncItem->existsInLeft() )
          leftExistingNum++;
        if( syncItem->existsInRight() )
          rightExistingNum++;
      }
    }
    it++;
  }

  if( itemNum == 0 ) {
    hide();
    KMessageBox::error( parent, i18n( "No elements to feed!" ) );
    return;
  }

  // guessing the collection name

  virt_vfs v(0,true);
  if( !v.vfs_refresh( KURL( "virt:/" ) ) )
    return;

  krConfig->setGroup( "Synchronize" );
  int listBoxNum = krConfig->readNumEntry( "Feed To Listbox Counter", 1 );
  TQString queryName;
  do {
    queryName = i18n("Synchronize results")+TQString( " %1" ).arg( listBoxNum++ );
  }while( v.vfs_search( queryName ) != 0 );
  krConfig->writeEntry( "Feed To Listbox Counter", listBoxNum );

  // creating the widget

  TQWidget *widget=new TQWidget(this, "feedToListBoxMainWidget");
  TQVBoxLayout *layout = new TQVBoxLayout( widget, 0, 10, "FeedToListBoxDialogLayout" );
    
  TQLabel *label = new TQLabel( i18n("Here you can name the file collection"), widget, "fbLabel" );
  layout->addWidget( label );    
  
  lineEdit = new TQLineEdit( widget, "fbLineEdit" );
  lineEdit->setText( queryName );
  lineEdit->selectAll();
  layout->addWidget( lineEdit );  
  
  TQHBox *hbox = new TQHBox( widget, "fbHBox" );
  
  TQLabel *label2 = new TQLabel( i18n( "Side to feed:" ), hbox, "fbSideLabel" );  
  label2->setSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed );
  
  sideCombo = new TQComboBox( hbox, "fbSideCombo" );
  sideCombo->setSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed );
  sideCombo->insertItem( i18n( "Left" ) );
  sideCombo->insertItem( i18n( "Right" ) );
  sideCombo->insertItem( i18n( "Both" ) );
  if( leftExistingNum == 0 ) {
    sideCombo->setCurrentItem( 1 );
    sideCombo->setEnabled( false );
  } else if( rightExistingNum == 0 ) {
    sideCombo->setCurrentItem( 0 );
    sideCombo->setEnabled( false );
  } else
    sideCombo->setCurrentItem( 2 );
  
  TQFrame *line = new TQFrame( hbox, "fbVLine" );
  line->setFrameStyle( TQFrame::VLine | TQFrame::Sunken );
  line->setSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Expanding );
  
  cbSelected = new TQCheckBox( i18n( "Selected files only" ), hbox, "cbSelected" );
  cbSelected->setSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed );
  cbSelected->setEnabled( selectedNum != 0 );
  cbSelected->setChecked( selectedNum != 0 );
  
  layout->addWidget( hbox );      
    
  setMainWidget(widget);
  
  exec();
}

void FeedToListBoxDialog::slotUser1() {
  lineEdit->clear();
}

void FeedToListBoxDialog::slotOk() {
  int side = sideCombo->currentItem();
  bool selected = cbSelected->isChecked();
  TQString name = lineEdit->text();
  KURL::List urlList;

  TQListViewItemIterator it( syncList );
  for( ;it.current(); it++ ) {
    SynchronizerGUI::SyncViewItem *item = (SynchronizerGUI::SyncViewItem *) it.current();
    SynchronizerFileItem *syncItem = item->synchronizerItemRef();

    if( !syncItem || !syncItem->isMarked() )
      continue;
    if( selected && !item->isSelected() )
      continue;
    if( !equalAllowed && syncItem->task() == TT_EQUALS && (!selected || !item->isSelected() ) )
      continue;

    if( ( side == S_BOTH || side == S_LEFT ) && syncItem->existsInLeft() ) {
      TQString leftDirName = syncItem->leftDirectory().isEmpty() ? "" : syncItem->leftDirectory() + "/";
      KURL leftURL = vfs::fromPathOrURL( synchronizer->leftBaseDirectory() + leftDirName + syncItem->leftName() );
      urlList.push_back( leftURL );
    }

    if( ( side == S_BOTH || side == S_RIGHT ) && syncItem->existsInRight() ) {
      TQString rightDirName = syncItem->rightDirectory().isEmpty() ? "" : syncItem->rightDirectory() + "/";
      KURL leftURL = vfs::fromPathOrURL( synchronizer->rightBaseDirectory() + rightDirName + syncItem->rightName() );
      urlList.push_back( leftURL );
    }
  }

  KURL url = KURL::fromPathOrURL(TQString("virt:/")+ name);
  virt_vfs v(0,true);
  if( !v.vfs_refresh( url ) ) {
    KMessageBox::error( parentWidget(), i18n( "Cannot open %1!" ).arg( url.prettyURL() ) );
    return;
  }
  v.vfs_addFiles( &urlList, TDEIO::CopyJob::Copy, 0 );
  ACTIVE_MNG->slotNewTab(url.prettyURL());
  accepted = true;
  accept();
}

#include "feedtolistboxdialog.moc"
