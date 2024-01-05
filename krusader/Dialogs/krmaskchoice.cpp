/***************************************************************************
                                 krmaskchoice.cpp
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
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
#include "krmaskchoice.h"

#include <tqcombobox.h>
#include <tqgroupbox.h>
#include <tqlabel.h>
#include <tqlistbox.h>
#include <tqpushbutton.h>
#include <tqlayout.h>
#include <tqvariant.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>
#include <tqmessagebox.h>
#include <tdelocale.h>
#include <tqlineedit.h>

/*
 *  Constructs a KRMaskChoice which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
KRMaskChoice::KRMaskChoice( TQWidget* parent,  const char* name, bool modal, WFlags fl )
    : TQDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "KRMaskChoice" );
    resize( 401, 314 );
    setCaption( i18n( "Choose Files"  ) );
    setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)5, (TQSizePolicy::SizeType)5 ) );

    selection = new TQComboBox( FALSE, this, "selection" );
    int height = TQFontMetrics( selection->font() ).height();
    height =  height + 5*(height > 14) + 6;
    selection->setGeometry( TQRect( 12, 48, 377, height) );
    selection->setEditable( TRUE );
    selection->setInsertionPolicy( TQComboBox::AtTop );
    selection->setAutoCompletion( TRUE );

    TQWidget* Layout7 = new TQWidget( this, "Layout7" );
    Layout7->setGeometry( TQRect( 10, 10, 380, 30 ) ); 
    hbox = new TQHBoxLayout( Layout7 );
    hbox->setSpacing( 6 );
    hbox->setMargin( 0 );

    PixmapLabel1 = new TQLabel( Layout7, "PixmapLabel1" );
    PixmapLabel1->setScaledContents( TRUE );
    PixmapLabel1->setMaximumSize( TQSize( 31, 31 ) );
	// now, add space for the pixmap    
    hbox->addWidget( PixmapLabel1 );

    label = new TQLabel( Layout7, "label" );
    label->setText( i18n( "Select the following files:"  ) );
    hbox->addWidget( label );

    GroupBox1 = new TQGroupBox( this, "GroupBox1" );
    GroupBox1->setGeometry( TQRect( 11, 77, 379, 190 ) );
    GroupBox1->setTitle( i18n( "Predefined Selections"  ) );

    TQWidget* Layout6 = new TQWidget( GroupBox1, "Layout6" );
    Layout6->setGeometry( TQRect( 10, 20, 360, 160 ) ); 
    hbox_2 = new TQHBoxLayout( Layout6 ); 
    hbox_2->setSpacing( 6 );
    hbox_2->setMargin( 0 );

    preSelections = new TQListBox( Layout6, "preSelections" );
    preSelections->setVScrollBarMode( TQListBox::AlwaysOn );
    TQWhatsThis::add(  preSelections, i18n( "A predefined selection is a file-mask which you use often.\nSome examples are: \"*.c, *.h\", \"*.c, *.o\", etc.\nYou can add these masks to the list by typing them and pressing the Add button.\nDelete removes a predefined selection and Clear removes all of them.\nNotice that the line in which you edit the mask has it's own history, you can scroll it, if needed." ) );
    hbox_2->addWidget( preSelections );

    vbox = new TQVBoxLayout; 
    vbox->setSpacing( 6 );
    vbox->setMargin( 0 );

    PushButton7 = new TQPushButton( Layout6, "PushButton7" );
    PushButton7->setText( i18n( "Add"  ) );
    TQToolTip::add(  PushButton7, i18n( "Adds the selection in the line-edit to the list" ) );
    vbox->addWidget( PushButton7 );

    PushButton7_2 = new TQPushButton( Layout6, "PushButton7_2" );
    PushButton7_2->setText( i18n( "Delete"  ) );
    TQToolTip::add(  PushButton7_2, i18n( "Delete the marked selection from the list" ) );
    vbox->addWidget( PushButton7_2 );

    PushButton7_3 = new TQPushButton( Layout6, "PushButton7_3" );
    PushButton7_3->setText( i18n( "Clear"  ) );
    TQToolTip::add(  PushButton7_3, i18n( "Clears the entire list of selections" ) );
    vbox->addWidget( PushButton7_3 );
    TQSpacerItem* spacer = new TQSpacerItem( 20, 54, TQSizePolicy::Fixed, TQSizePolicy::Expanding );
    vbox->addItem( spacer );
    hbox_2->addLayout( vbox );

    TQWidget* Layout18 = new TQWidget( this, "Layout18" );
    Layout18->setGeometry( TQRect( 10, 280, 379, 30 ) ); 
    hbox_3 = new TQHBoxLayout( Layout18 ); 
    hbox_3->setSpacing( 6 );
    hbox_3->setMargin( 0 );
    TQSpacerItem* spacer_2 = new TQSpacerItem( 205, 20, TQSizePolicy::Expanding, TQSizePolicy::Fixed );
    hbox_3->addItem( spacer_2 );

    PushButton3 = new TQPushButton( Layout18, "PushButton3" );
    PushButton3->setText( i18n( "OK"  ) );
    hbox_3->addWidget( PushButton3 );

    PushButton3_2 = new TQPushButton( Layout18, "PushButton3_2" );
    PushButton3_2->setText( i18n( "Cancel"  ) );
    hbox_3->addWidget( PushButton3_2 );

    // signals and slots connections
    connect( PushButton3_2, TQ_SIGNAL( clicked() ), this, TQ_SLOT( reject() ) );
    connect( PushButton3, TQ_SIGNAL( clicked() ), this, TQ_SLOT( accept() ) );
    connect( PushButton7, TQ_SIGNAL( clicked() ), this, TQ_SLOT( addSelection() ) );
    connect( PushButton7_2, TQ_SIGNAL( clicked() ), this, TQ_SLOT( deleteSelection() ) );
    connect( PushButton7_3, TQ_SIGNAL( clicked() ), this, TQ_SLOT( clearSelections() ) );
    connect( selection, TQ_SIGNAL( activated(const TQString&) ), selection, TQ_SLOT( setEditText(const TQString &) ) );
    connect( selection->lineEdit(), TQ_SIGNAL( returnPressed() ), this, TQ_SLOT( accept() ));
    connect( preSelections, TQ_SIGNAL( doubleClicked(TQListBoxItem*) ), this, TQ_SLOT( acceptFromList(TQListBoxItem *) ) );
    connect( preSelections, TQ_SIGNAL( highlighted(const TQString&) ), selection, TQ_SLOT( setEditText(const TQString &) ) );
    connect( preSelections, TQ_SIGNAL( returnPressed(TQListBoxItem*) ), this, TQ_SLOT( acceptFromList(TQListBoxItem *) ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
KRMaskChoice::~KRMaskChoice()
{
    // no need to delete child widgets, TQt does it all for us
}

void KRMaskChoice::addSelection()
{
    tqWarning( "KRMaskChoice::addSelection(): Not implemented yet!" );
}

void KRMaskChoice::clearSelections()
{
    tqWarning( "KRMaskChoice::clearSelections(): Not implemented yet!" );
}

void KRMaskChoice::deleteSelection()
{
    tqWarning( "KRMaskChoice::deleteSelection(): Not implemented yet!" );
}

void KRMaskChoice::acceptFromList(TQListBoxItem *)
{
    tqWarning( "KRMaskChoice::acceptFromList(TQListBoxItem *): Not implemented yet!" );
}

#include "krmaskchoice.moc"
