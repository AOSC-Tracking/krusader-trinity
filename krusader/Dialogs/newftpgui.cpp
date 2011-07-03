/****************************************************************************
** Form implementation generated from reading ui file 'newftpgui.ui'
**
** Created: Fri Oct 27 23:47:10 2000
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "newftpgui.h"

#include <tqlabel.h>
#include <tqlineedit.h>
#include <tqpushbutton.h>
#include <tqspinbox.h>
#include <tqlayout.h>
#include <tqhbox.h>
#include <tqgrid.h>
#include <tqvariant.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>
#include <tqimage.h>
#include <tqpixmap.h>
#include <klocale.h>
#include <kprotocolinfo.h>
#include <kcombobox.h>
#include <kiconloader.h>
#include "../krusader.h"


/* 
 *  Constructs a newFTPGUI which is a child of 'tqparent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
 
 #define SIZE_MINIMUM	TQSizePolicy( (TQSizePolicy::SizeType)0, (TQSizePolicy::SizeType)0 )
 
newFTPGUI::newFTPGUI( TQWidget* tqparent,  const char* name, bool modal, WFlags fl )
    : TQDialog( tqparent, name, modal, fl ){
    
    TQVBoxLayout * tqlayout = new TQVBoxLayout( this, 11, 6, "newFTPGUI_layout" );
    tqlayout->setAutoAdd(true);
    
    if ( !name )
    setName( "newFTPGUI" );
    resize( 342, 261 );
    setCaption( i18n( "New Network Connection"  ) );
//     setSizeGripEnabled( TRUE );
    tqsetSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)5, (TQSizePolicy::SizeType)5, sizePolicy().hasHeightForWidth() ) );
    setMinimumSize( TQSize( 342, 261 ) );

    
    TQHBox* hbox_image = new TQHBox( this, "hbox_image" );
    hbox_image->setSpacing( 6 );
    
    PixmapLabel1 = new TQLabel( hbox_image, "PixmapLabel1" );
    PixmapLabel1->setPixmap( krLoader->loadIcon("network", KIcon::Desktop, 32) );
    PixmapLabel1->tqsetSizePolicy( SIZE_MINIMUM );

    TextLabel3 = new TQLabel( i18n( "About to connect to..."  ), hbox_image, "TextLabel3" );
    TQFont TextLabel3_font(  TextLabel3->font() );
    TextLabel3_font.setBold( TRUE );
    TextLabel3->setFont( TextLabel3_font );

    
    TQGrid* grid_host = new TQGrid( 3, this, "grid_host" );
    
    TextLabel1 = new TQLabel( i18n( "Protocol:"  ), grid_host, "TextLabel1" );
    TextLabel1_22 = new TQLabel( i18n( "Host:"), grid_host, "TextLabel_2" );
    TextLabel1_3 = new TQLabel( i18n( "Port:"  ), grid_host, "TextLabel1_3" );

    TQStringList protocols = KProtocolInfo::protocols();

    prefix = new KComboBox( FALSE, grid_host, "protocol" );
    if( protocols.tqcontains("ftp") )
      prefix->insertItem( i18n( "ftp://" ) );
    if( protocols.tqcontains("smb") )
      prefix->insertItem( i18n( "smb://" ) );
    if( protocols.tqcontains("fish") )
      prefix->insertItem( i18n( "fish://" ));
    if( protocols.tqcontains("sftp") )
      prefix->insertItem( i18n( "sftp://" ));
    prefix->setAcceptDrops( FALSE );
    prefix->setEnabled( TRUE );
    prefix->tqsetSizePolicy( SIZE_MINIMUM );
    connect( prefix,TQT_SIGNAL(activated(const TQString& )),
               this,TQT_SLOT(slotTextChanged(const TQString& )));

    url = new KHistoryCombo( grid_host, "url" );
    url->setMaximumHeight( 20 );
    url->setMaxCount( 25 );
    url->setDuplicatesEnabled( false );
    connect( url, TQT_SIGNAL( activated( const TQString& )),
             url, TQT_SLOT( addToHistory( const TQString& )));
    // load the history and completion list after creating the history combo
    krConfig->setGroup("Private");
    TQStringList list = krConfig->readListEntry( "newFTP Completion list" );
    url->completionObject()->setItems( list );
    list = krConfig->readListEntry( "newFTP History list" );
    url->setHistoryItems( list );

    port = new TQSpinBox( grid_host, "port" );
    port->setMaxValue( 65535 );
    port->setValue( 21 );
    port->tqsetSizePolicy( SIZE_MINIMUM );


    TextLabel1_2 = new TQLabel( i18n( "Username:"  ), this, "TextLabel1_2" );
    username = new TQLineEdit( this, "username" );
    TextLabel1_2_2 = new TQLabel( i18n( "Password:"  ), this, "TextLabel1_2_2" );
    password = new TQLineEdit( this, "password" );
    password->setEchoMode( TQLineEdit::Password );

    
    TQWidget* Layout6 = new TQWidget( this, "Layout6" );
    hbox = new TQHBoxLayout( Layout6 );
    hbox->setSpacing( 6 );
    hbox->setMargin( 0 );

	 hbox->addItem(new TQSpacerItem(1,1,TQSizePolicy::Expanding));
	 
    connectBtn = new TQPushButton( i18n( "&Connect"  ), Layout6, "connectBtn" );
    connectBtn->setAutoDefault( TRUE );
    connectBtn->setDefault( TRUE );
    hbox->addWidget( connectBtn );

    //saveBtn = new TQPushButton( i18n( "&Save"  ), Layout6, "saveBtn" );
    //saveBtn->setAutoDefault( TRUE );
    //hbox->addWidget( saveBtn );

    cancelBtn = new TQPushButton( i18n( "&Cancel"  ), Layout6, "cancelBtn" );
    cancelBtn->setAutoDefault( TRUE );
    hbox->addWidget( cancelBtn );

    // signals and slots connections
    connect( connectBtn, TQT_SIGNAL( clicked() ), this, TQT_SLOT( accept() ) );
    connect( cancelBtn, TQT_SIGNAL( clicked() ), this, TQT_SLOT( reject() ) );

    // tab order
    setTabOrder( url, username );
    setTabOrder( username, password );
    setTabOrder( password, connectBtn );
    setTabOrder( connectBtn, cancelBtn );
    setTabOrder( cancelBtn, prefix );
    setTabOrder( prefix, url );
}

/*
 *  Destroys the object and frees any allocated resources
 */
newFTPGUI::~newFTPGUI(){
    // no need to delete child widgets, TQt does it all for us
}

void newFTPGUI::slotTextChanged(const TQString& string){
   if( string.startsWith("ftp") || string.startsWith("sftp") || string.startsWith("fish") )
   {
     if( port->value() == 21 || port->value() == 22 )
       port->setValue( string.startsWith("ftp") ? 21 : 22 );
     port->setEnabled(true);
   }
   else
     port->setEnabled(false);
}

/*
 *  Main event handler. Reimplemented to handle application
 *  font changes
 */
bool newFTPGUI::event( TQEvent* ev ) {
    bool ret = TQDialog::event( ev ); 
    if ( ev->type() == TQEvent::ApplicationFontChange ) {
	TQFont TextLabel3_font(  TextLabel3->font() );
	TextLabel3_font.setBold( TRUE );
	TextLabel3->setFont( TextLabel3_font ); 
    }
    return ret;
}

#include "newftpgui.moc"
