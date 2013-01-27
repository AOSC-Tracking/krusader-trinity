/***************************************************************************
                                 packguibase.cpp
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
#include "packguibase.h"

#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqlabel.h>
#include <tqlineedit.h>
#include <tqpushbutton.h>
#include <tqtoolbutton.h>
#include <tqlayout.h>
#include <tqvariant.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>
#include <tqimage.h>
#include <tqpixmap.h>
#include <tqspinbox.h>
#include <tqslider.h>
#include <tqhbox.h>
#include <tqvbox.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kcombobox.h>
#include <kmessagebox.h>
#include <tdeio/global.h>
#include "../krusader.h"

/* 
 *  Constructs a PackGUIBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
PackGUIBase::PackGUIBase( TQWidget* parent,  const char* name, bool modal, WFlags fl )
    : TQDialog( parent, name, modal, fl ), expanded( false )
{
    if ( !name )
	setName( "PackGUIBase" );
    resize( 430, 140 );
    setCaption( i18n( "Pack" ) );
    grid = new TQGridLayout( this );
    grid->setSpacing( 6 );
    grid->setMargin( 11 );

    hbox = new TQHBoxLayout;
    hbox->setSpacing( 6 );
    hbox->setMargin( 0 );

    TextLabel3 = new TQLabel( this, "TextLabel3" );
    TextLabel3->setText( i18n( "To archive"  ) );
    hbox->addWidget( TextLabel3 );

    nameData = new TQLineEdit( this, "nameData" );
    hbox->addWidget( nameData );

    typeData = new TQComboBox( FALSE, this, "typeData" );
    typeData->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)0 ) );
    connect( typeData, TQT_SIGNAL( activated( const TQString & ) ), this,  TQT_SLOT( checkConsistency() ) );
    connect( typeData, TQT_SIGNAL( highlighted( const TQString & ) ), this,  TQT_SLOT( checkConsistency() ) );
    hbox->addWidget( typeData );

    grid->addLayout( hbox, 1, 0 );

    hbox_2 = new TQHBoxLayout;
    hbox_2->setSpacing( 6 );
    hbox_2->setMargin( 0 );

    TextLabel5 = new TQLabel( this, "TextLabel5" );
    TextLabel5->setText( i18n( "In directory"  ) );
    hbox_2->addWidget( TextLabel5 );

    dirData = new TQLineEdit( this, "dirData" );
    hbox_2->addWidget( dirData );

    browseButton = new TQToolButton( this, "browseButton" );
    browseButton->setIconSet( SmallIcon( "fileopen" ) );
    hbox_2->addWidget( browseButton );
    TQSpacerItem* spacer = new TQSpacerItem( 48, 20, TQSizePolicy::Fixed, TQSizePolicy::Fixed );
    hbox_2->addItem( spacer );

    grid->addLayout( hbox_2, 2, 0 );

    hbox_3 = new TQHBoxLayout;
    hbox_3->setSpacing( 6 );
    hbox_3->setMargin( 0 );

    PixmapLabel1 = new TQLabel( this, "PixmapLabel1" );
    PixmapLabel1->setPixmap( krLoader->loadIcon("package", KIcon::Desktop, 32) );
    PixmapLabel1->setScaledContents( TRUE );
    PixmapLabel1->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)0, (TQSizePolicy::SizeType)0 ) );
    hbox_3->addWidget( PixmapLabel1 );

    TextLabel1 = new TQLabel( this, "TextLabel1" );
    TextLabel1->setText( i18n( "Pack"  ) );
    hbox_3->addWidget( TextLabel1 );

    grid->addLayout( hbox_3, 0, 0 );


    hbox_4 = new TQHBoxLayout;
    hbox_4->setSpacing( 6 );
    hbox_4->setMargin( 0 );

    TQSpacerItem* spacer_3 = new TQSpacerItem( 20, 26, TQSizePolicy::Fixed, TQSizePolicy::Expanding );
    hbox_4->addItem( spacer_3 );
    grid->addLayout( hbox_4, 3, 0 );

    advancedWidget = new TQWidget( this, "advancedWidget" );

    hbox_5 = new TQGridLayout( advancedWidget );
    hbox_5->setSpacing( 6 );
    hbox_5->setMargin( 0 );


    TQVBoxLayout *compressLayout = new TQVBoxLayout;
    compressLayout->setSpacing( 6 );
    compressLayout->setMargin( 0 );

    multipleVolume = new TQCheckBox( i18n( "Multiple volume archive" ), advancedWidget, "multipleVolume" );
    connect( multipleVolume, TQT_SIGNAL( toggled( bool ) ), this, TQT_SLOT( checkConsistency() ) );
    compressLayout->addWidget( multipleVolume, 0, 0 );

    TQHBoxLayout * volumeHbox = new TQHBoxLayout;

    TQSpacerItem* spacer_5 = new TQSpacerItem( 20, 26, TQSizePolicy::Fixed, TQSizePolicy::Fixed );
    volumeHbox->addItem( spacer_5 );

    TextLabel7 = new TQLabel( i18n("Size:" ), advancedWidget, "TextLabel7" );
    volumeHbox->addWidget( TextLabel7 );
    
    volumeSpinBox = new TQSpinBox( advancedWidget, "volumeSpinBox" );
    volumeSpinBox->setMinValue( 1 );
    volumeSpinBox->setMaxValue( 9999 );
    volumeSpinBox->setValue( 1440 );
    volumeHbox->addWidget( volumeSpinBox );

    volumeUnitCombo = new TQComboBox( FALSE, advancedWidget, "volumeUnitCombo" );
    volumeUnitCombo->insertItem( "B" );
    volumeUnitCombo->insertItem( "KB" );
    volumeUnitCombo->insertItem( "MB" );
    volumeUnitCombo->setCurrentItem( 1 );
    volumeHbox->addWidget( volumeUnitCombo );

    compressLayout->addLayout ( volumeHbox );

    setCompressionLevel = new TQCheckBox( i18n( "Set compression level" ), advancedWidget, "multipleVolume" );
    connect( setCompressionLevel, TQT_SIGNAL( toggled( bool ) ), this, TQT_SLOT( checkConsistency() ) );
    compressLayout->addWidget( setCompressionLevel, 0, 0 );

    TQHBoxLayout * sliderHbox = new TQHBoxLayout;

    TQSpacerItem* spacer_6 = new TQSpacerItem( 20, 26, TQSizePolicy::Fixed, TQSizePolicy::Fixed );
    sliderHbox->addItem( spacer_6 );

    TQVBox * sliderVBox = new TQVBox( advancedWidget );

    compressionSlider = new TQSlider( 1, 9, 1, 5, Qt::Horizontal, sliderVBox, "compressionSlider" );
    compressionSlider->setTickmarks( TQSlider::Below );

    TQHBox * minmaxHBox = new TQHBox( sliderVBox );
    minLabel = new TQLabel( i18n("MIN"), minmaxHBox );
    maxLabel = new TQLabel( i18n("MAX"), minmaxHBox );
    maxLabel->setSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed );

    sliderHbox->addWidget( sliderVBox );

    compressLayout->addLayout( sliderHbox );

    compressLayout->addStretch( 0 );
    hbox_5->addLayout( compressLayout, 0, 0 );

    TQFrame *vline = new TQFrame( advancedWidget, "vline" );
    vline->setFrameStyle( TQFrame::VLine | TQFrame::Sunken );
    vline->setMinimumWidth( 20 );
    hbox_5->addWidget( vline, 0, 1 );


    TQGridLayout * passwordGrid = new TQGridLayout;
    passwordGrid->setSpacing( 6 );
    passwordGrid->setMargin( 0 );

    TextLabel4 = new TQLabel( advancedWidget, "TextLabel4" );
    TextLabel4->setText( i18n( "Password"  ) );
    passwordGrid->addWidget( TextLabel4, 0, 0 );

    password = new TQLineEdit( advancedWidget, "password" );
    password->setEchoMode( TQLineEdit::Password );
    connect( password, TQT_SIGNAL( textChanged ( const TQString & ) ), this, TQT_SLOT( checkConsistency() ) );

    passwordGrid->addWidget( password, 0, 1 );

    TextLabel6 = new TQLabel( advancedWidget, "TextLabel6" );
    TextLabel6->setText( i18n( "Again"  ) );
    passwordGrid->addWidget( TextLabel6, 1, 0 );

    passwordAgain = new TQLineEdit( advancedWidget, "password" );
    passwordAgain->setEchoMode( TQLineEdit::Password );
    connect( passwordAgain, TQT_SIGNAL( textChanged ( const TQString & ) ), this, TQT_SLOT( checkConsistency() ) );

    passwordGrid->addWidget( passwordAgain, 1, 1 );

    TQHBoxLayout *consistencyHbox = new TQHBoxLayout;

    TQSpacerItem* spacer_cons = new TQSpacerItem( 48, 20, TQSizePolicy::Expanding, TQSizePolicy::Fixed );
    consistencyHbox->addItem( spacer_cons );

    passwordConsistencyLabel = new TQLabel( advancedWidget, "passwordConsistencyLabel" );
    consistencyHbox->addWidget( passwordConsistencyLabel );
    passwordGrid->addMultiCellLayout ( consistencyHbox, 2, 2, 0, 1 );

    encryptHeaders = new TQCheckBox( i18n( "Encrypt headers" ), advancedWidget, "encryptHeaders" );
    passwordGrid->addMultiCellWidget ( encryptHeaders, 3, 3, 0, 1 );

    TQSpacerItem* spacer_psw = new TQSpacerItem( 20, 20, TQSizePolicy::Fixed, TQSizePolicy::Expanding );
    passwordGrid->addItem( spacer_psw, 4, 0 );

    hbox_5->addLayout( passwordGrid, 0, 2 );

    hbox_7 = new TQHBoxLayout;
    hbox_7->setSpacing( 6 );
    hbox_7->setMargin( 0 );

    TextLabel8 = new TQLabel( i18n( "Command line switches:" ), advancedWidget, "TextLabel8" );
    TextLabel8->setSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed );
    hbox_7->addWidget( TextLabel8 );

    commandLineSwitches = new KHistoryCombo( advancedWidget, "commandLineSwitches" );
    commandLineSwitches->setMaxCount(25);  // remember 25 items
    commandLineSwitches->setDuplicatesEnabled(false);
    krConfig->setGroup("Archives");
    TQStringList list = krConfig->readListEntry("Command Line Switches");
    commandLineSwitches->setHistoryItems(list);

    hbox_7->addWidget( commandLineSwitches );
    
    hbox_5->addMultiCellLayout( hbox_7, 1, 1, 0, 2 );


    advancedWidget->hide();
    checkConsistency();

    grid->addWidget( advancedWidget, 4, 0 );
    
    hbox_6 = new TQHBoxLayout;
    hbox_6->setSpacing( 6 );
    hbox_6->setMargin( 0 );

    advancedButton = new TQPushButton( this, "advancedButton" );
    advancedButton->setText( i18n( "&Advanced" ) + " >>" );
    hbox_6->addWidget( advancedButton );

    TQSpacerItem* spacer_2 = new TQSpacerItem( 140, 20, TQSizePolicy::Expanding, TQSizePolicy::Fixed );
    hbox_6->addItem( spacer_2 );

    okButton = new TQPushButton( this, "okButton" );
    okButton->setText( i18n( "Ok"  ) );
    okButton->setDefault( true );
    hbox_6->addWidget( okButton );

    cancelButton = new TQPushButton( this, "cancelButton" );
    cancelButton->setText( i18n( "Cancel"  ) );
    hbox_6->addWidget( cancelButton );

    grid->addLayout( hbox_6, 6, 0 );

    // signals and slots connections
    connect( okButton, TQT_SIGNAL( clicked() ), this, TQT_SLOT( accept() ) );
    connect( advancedButton, TQT_SIGNAL( clicked() ), this, TQT_SLOT( expand() ) );
    connect( cancelButton, TQT_SIGNAL( clicked() ), this, TQT_SLOT( reject() ) );
    connect( browseButton, TQT_SIGNAL( clicked() ), this, TQT_SLOT( browse() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
PackGUIBase::~PackGUIBase()
{
    // no need to delete child widgets, TQt does it all for us
}

void PackGUIBase::browse()
{
    tqWarning( "PackGUIBase::browse(): Not implemented yet!" );
}

void PackGUIBase::expand() {
    expanded = !expanded;
    
    advancedButton->setText( i18n( "&Advanced" ) + ( expanded ?  " <<" : " >>" ) );

    if( expanded )
      advancedWidget->show();
    else {
      advancedWidget->hide();
      layout()->activate();
      TQSize minSize = minimumSize();
      resize( width(), minSize.height() );
    }
    show();
}

void PackGUIBase::checkConsistency() {
    if( password->text().isEmpty() && passwordAgain->text().isEmpty()) {
      passwordConsistencyLabel->setPaletteForegroundColor( TDEGlobalSettings::textColor() );
      passwordConsistencyLabel->setText( i18n( "No password specified" ) );
    }
    else
    if( password->text() == passwordAgain->text() ) {
      passwordConsistencyLabel->setPaletteForegroundColor( TDEGlobalSettings::textColor() );
      passwordConsistencyLabel->setText( i18n( "The passwords are equal" ) );
    }
    else {
      passwordConsistencyLabel->setPaletteForegroundColor( TQt::red );
      passwordConsistencyLabel->setText( i18n( "The passwords are different" ) );
    }

    TQString packer = typeData->currentText();

    bool passworded = false;
    if( packer == "7z" || packer == "rar" || packer == "zip" || packer == "arj" )
      passworded = true;

    passwordConsistencyLabel->setEnabled( passworded );
    password->setEnabled( passworded );
    passwordAgain->setEnabled( passworded );
    TextLabel4->setEnabled( passworded );
    TextLabel6->setEnabled( passworded );

    encryptHeaders->setEnabled( packer == "rar" );

    multipleVolume->setEnabled( packer == "rar" || packer == "arj" );
    bool volumeEnabled = multipleVolume->isEnabled() && multipleVolume->isChecked();
    volumeSpinBox->setEnabled( volumeEnabled );
    volumeUnitCombo->setEnabled( volumeEnabled );
    TextLabel7->setEnabled( volumeEnabled );

    /* TODO */
    setCompressionLevel->setEnabled( packer == "rar" || packer == "arj" || packer == "zip" ||
                                     packer == "7z" );
    bool sliderEnabled = setCompressionLevel->isEnabled() && setCompressionLevel->isChecked();
    compressionSlider->setEnabled( sliderEnabled );
    minLabel->setEnabled( sliderEnabled );
    maxLabel->setEnabled( sliderEnabled );
}

bool PackGUIBase::extraProperties( TQMap<TQString,TQString> & inMap ) {
    inMap.clear();

    if( password->isEnabled() && passwordAgain->isEnabled() ) {
      if( password->text() != passwordAgain->text() ) {
        KMessageBox::error( this, i18n( "Cannot pack! The passwords are different!" ) );
        return false;
      }

      if( !password->text().isEmpty() ) {
        inMap[ "Password" ] = password->text();

        if( encryptHeaders->isEnabled() && encryptHeaders->isChecked() )
          inMap[ "EncryptHeaders" ] = "1";
      }
    }

    if( multipleVolume->isEnabled() && multipleVolume->isChecked() ) {
      TDEIO::filesize_t size = volumeSpinBox->value();

      switch( volumeUnitCombo->currentItem() ) {
      case 2:
        size *= 1000;
      case 1:
        size *= 1000;
      default:
        break;
      }

      if( size < 10000 ) {
        KMessageBox::error( this, i18n( "Invalid volume size!" ) );
        return false;
      }

      TQString sbuffer;
      sbuffer.sprintf("%llu",size);

      inMap[ "VolumeSize" ] = sbuffer;
    }

    if( setCompressionLevel->isEnabled() && setCompressionLevel->isChecked() ) {
      inMap[ "CompressionLevel" ] = TQString("%1").arg( compressionSlider->value() );
    }

    TQString cmdArgs = commandLineSwitches->currentText().stripWhiteSpace();
    if( !cmdArgs.isEmpty() ) {
      bool firstChar = true;
      TQChar quote = '\0';
      
      for( unsigned i=0; i < cmdArgs.length(); i++ ) {
         TQChar ch( cmdArgs[ i ] );
         if( ch.isSpace() )
           continue;

         if( ch == quote ) {
           quote = '\0';
           continue;
         }
         
         if( firstChar && ch != '-' ) {
           KMessageBox::error( this, i18n( "Invalid command line switch!\nSwitch must start with '-'!" ) );
           return false;
         }
         
         firstChar = false;

         if( quote == '"' )
           continue;
         if( quote == '\0' && ( ch == '\'' || ch == '"' ) )
           quote = ch;
         if( ch == '\\' ) {
           if( i == cmdArgs.length() - 1 ) {
             KMessageBox::error( this, i18n( "Invalid command line switch!\nBackslash cannot be the last character" ) );
             return false;
           }
           i++;
         }
      }

      if( quote != '\0' ) {
             KMessageBox::error( this, i18n( "Invalid command line switch!\nUnclosed quotation mark!" ) );
             return false;
      }

      commandLineSwitches->addToHistory( cmdArgs );
      TQStringList list = commandLineSwitches->historyItems();
      krConfig->setGroup("Archives");
      krConfig->writeEntry("Command Line Switches", list);

      inMap[ "CommandLineSwitches" ] = cmdArgs;      
    }
    return true;
}

#include "packguibase.moc"
