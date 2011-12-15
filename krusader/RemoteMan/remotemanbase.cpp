/****************************************************************************
** Form implementation generated from reading ui file 'remotemanbase.ui'
**
** Created: Thu Jun 7 16:24:11 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "remotemanbase.h"

#include <klocale.h>
#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqheader.h>
#include <tqlabel.h>
#include <tqlineedit.h>
#include <tqlistview.h>
#include <tqmultilineedit.h>
#include <tqpushbutton.h>
#include <tqspinbox.h>
#include <layout.h>
#include <tqvariant.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>
#include <kprotocolinfo.h>


/* 
 *  Constructs a remoteManBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
remoteManBase::remoteManBase( TQWidget* parent,  const char* name, bool modal, WFlags fl )
    : TQDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "remoteManBase" );
    resize( 670, 502 ); 
    setCaption( i18n( "RemoteMan: Connection Manager" ) );
    setSizeGripEnabled( TRUE );
    remoteManBaseLayout = new TQGridLayout( this );
    remoteManBaseLayout->setSpacing( 6 );
    remoteManBaseLayout->setMargin( 11 );

    Layout23 = new TQVBoxLayout;
    Layout23->setSpacing( 6 );
    Layout23->setMargin( 0 );

    TextLabel1 = new TQLabel( this, "TextLabel1" );
    TextLabel1->setText( i18n( "Session name:" ) );
    Layout23->addWidget( TextLabel1 );

    sessionName = new TQLineEdit( this, "sessionName" );
    Layout23->addWidget( sessionName );

    remoteManBaseLayout->addLayout( Layout23, 0, 1 );

    Layout12 = new TQHBoxLayout;
    Layout12->setSpacing( 6 );
    Layout12->setMargin( 0 );
    TQSpacerItem* spacer = new TQSpacerItem( 20, 20, TQSizePolicy::Expanding, TQSizePolicy::Minimum );
    Layout12->addItem( spacer );

    moreBtn = new TQPushButton( this, "moreBtn" );
    moreBtn->setText( i18n( "&More" ) );
    moreBtn->setAutoDefault( FALSE );
    moreBtn->setDefault( FALSE );
    Layout12->addWidget( moreBtn );

    closeBtn = new TQPushButton( this, "closeBtn" );
    closeBtn->setText( i18n( "&Close" ) );
    closeBtn->setAutoDefault( FALSE );
    closeBtn->setDefault( FALSE );
    Layout12->addWidget( closeBtn );

    remoteManBaseLayout->addMultiCellLayout( Layout12, 7, 7, 0, 1 );

    sessions = new TQListView( this, "sessions" );
    sessions->addColumn( i18n( "Sessions" ) );
    sessions->header()->setClickEnabled( FALSE, sessions->header()->count() - 1 );
    sessions->header()->setResizeEnabled( FALSE, sessions->header()->count() - 1 );
    sessions->setMinimumSize( TQSize( 300, 400 ) );
    sessions->setVScrollBarMode( TQListView::AlwaysOn );
    sessions->setHScrollBarMode( TQListView::Auto );
    sessions->setRootIsDecorated( TRUE );

    remoteManBaseLayout->addMultiCellWidget( sessions, 0, 6, 0, 0 );

    Layout9 = new TQVBoxLayout;
    Layout9->setSpacing( 6 );
    Layout9->setMargin( 0 );

    Layout10 = new TQGridLayout;
    Layout10->setSpacing( 6 );
    Layout10->setMargin( 0 );

    TextLabel1_3_3 = new TQLabel( this, "TextLabel1_3_3" );
    TextLabel1_3_3->setText( i18n( "Password:" ) );

    Layout10->addWidget( TextLabel1_3_3, 0, 1 );

    password = new TQLineEdit( this, "password" );

    Layout10->addWidget( password, 1, 1 );

    TextLabel1_3 = new TQLabel( this, "TextLabel1_3" );
    TextLabel1_3->setText( i18n( "User name:" ) );

    Layout10->addWidget( TextLabel1_3, 0, 0 );

    userName = new TQLineEdit( this, "userName" );

    Layout10->addWidget( userName, 1, 0 );
    Layout9->addLayout( Layout10 );

    anonymous = new TQCheckBox( this, "anonymous" );
    anonymous->setText( i18n( "Anonymous" ) );
    Layout9->addWidget( anonymous );

    remoteManBaseLayout->addLayout( Layout9, 2, 1 );

    Layout26 = new TQVBoxLayout;
    Layout26->setSpacing( 6 );
    Layout26->setMargin( 0 );

    TextLabel1_3_2 = new TQLabel( this, "TextLabel1_3_2" );
    TextLabel1_3_2->setText( i18n( "Remote directory:" ) );
    Layout26->addWidget( TextLabel1_3_2 );

    remoteDir = new TQLineEdit( this, "remoteDir" );
    Layout26->addWidget( remoteDir );

    remoteManBaseLayout->addLayout( Layout26, 4, 1 );

    Layout27 = new TQVBoxLayout;
    Layout27->setSpacing( 6 );
    Layout27->setMargin( 0 );

    TextLabel1_3_2_2 = new TQLabel( this, "TextLabel1_3_2_2" );
    TextLabel1_3_2_2->setText( i18n( "Description:" ) );
    Layout27->addWidget( TextLabel1_3_2_2 );

    description = new TQMultiLineEdit( this, "description" );
    Layout27->addWidget( description );

    remoteManBaseLayout->addLayout( Layout27, 5, 1 );

    layout = new TQGridLayout;
    layout->setSpacing( 6 );
    layout->setMargin( 0 );

    removeBtn = new TQPushButton( this, "removeBtn" );
    removeBtn->setText( i18n( "&Remove" ) );
    removeBtn->setAutoDefault( FALSE );
    removeBtn->setDefault( FALSE );

    layout->addWidget( removeBtn, 0, 2 );

    connectBtn = new TQPushButton( this, "connectBtn" );
    connectBtn->setText( i18n( "Co&nnect" ) );
    connectBtn->setAutoDefault( FALSE );
    connectBtn->setDefault( FALSE );

    layout->addMultiCellWidget( connectBtn, 1, 1, 0, 2 );

    newGroupBtn = new TQPushButton( this, "newGroupBtn" );
    newGroupBtn->setEnabled( TRUE );
    newGroupBtn->setText( i18n( "New &Group" ) );
    newGroupBtn->setAutoDefault( FALSE );
    newGroupBtn->setDefault( FALSE );
    newGroupBtn->setFlat( FALSE );

    layout->addWidget( newGroupBtn, 0, 0 );

    addBtn = new TQPushButton( this, "addBtn" );
    addBtn->setEnabled( TRUE );
    addBtn->setText( i18n( "New Connec&tion" ) );
    addBtn->setAutoDefault( FALSE );
    addBtn->setDefault( FALSE );
    addBtn->setFlat( FALSE );

    layout->addWidget( addBtn, 0, 1 );

    remoteManBaseLayout->addLayout( layout, 6, 1 );

    Layout11 = new TQGridLayout;
    Layout11->setSpacing( 6 );
    Layout11->setMargin( 0 );

    TextLabel1_2 = new TQLabel( this, "TextLabel1_2" );
    TextLabel1_2->setText( i18n( "Host:" ) );

    Layout11->addMultiCellWidget( TextLabel1_2, 0, 0, 0, 1 );

    TQStringList protocols = KProtocolInfo::protocols();

    protocol = new TQComboBox( FALSE, this, "protocol" );
    if( protocols.contains("ftp") )
      protocol->insertItem( i18n( "ftp://" ) );
    if( protocols.contains("smb") )
      protocol->insertItem( i18n( "smb://" ) );
    if( protocols.contains("fish") )
      protocol->insertItem( i18n( "fish://" ));
    if( protocols.contains("sftp") )
      protocol->insertItem( i18n( "sftp://" ));
    protocol->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)0, protocol->sizePolicy().hasHeightForWidth() ) );

    Layout11->addWidget( protocol, 1, 0 );

    hostName = new TQLineEdit( this, "hostName" );
    hostName->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)7, (TQSizePolicy::SizeType)0, hostName->sizePolicy().hasHeightForWidth() ) );
    hostName->setMinimumSize( TQSize( 0, 0 ) );

    Layout11->addWidget( hostName, 1, 1 );

    portNum = new TQSpinBox( this, "portNum" );
    portNum->setMaxValue( 99999 );
    portNum->setValue( 21 );

    Layout11->addWidget( portNum, 1, 2 );

    TextLabel1_2_2 = new TQLabel( this, "TextLabel1_2_2" );
    TextLabel1_2_2->setText( i18n( "Port:   " ) );

    Layout11->addWidget( TextLabel1_2_2, 0, 2 );

    remoteManBaseLayout->addLayout( Layout11, 1, 1 );

    TextLabel1_4 = new TQLabel( this, "TextLabel1_4" );
    TQFont TextLabel1_4_font(  TextLabel1_4->font() );
    TextLabel1_4_font.setPointSize( 10 );
    TextLabel1_4->setFont( TextLabel1_4_font );
    TextLabel1_4->setText( i18n( "* Warning: Storing your password is not secure !!!" ) );
    TextLabel1_4->setAlignment( int( TQLabel::AlignVCenter | TQLabel::AlignRight ) );

    remoteManBaseLayout->addWidget( TextLabel1_4, 3, 1 );

    // signals and slots connections
    connect( closeBtn, TQT_SIGNAL( clicked() ), this, TQT_SLOT( accept() ) );
    connect( addBtn, TQT_SIGNAL( clicked() ), this, TQT_SLOT( addSession() ) );
    connect( removeBtn, TQT_SIGNAL( clicked() ), this, TQT_SLOT( removeSession() ) );
    connect( sessions, TQT_SIGNAL( selectionChanged(TQListViewItem*) ), this, TQT_SLOT( refreshData() ) );
    connect( sessions, TQT_SIGNAL( selectionChanged() ), this, TQT_SLOT( refreshData() ) );
    connect( sessions, TQT_SIGNAL( expanded(TQListViewItem*) ), this, TQT_SLOT( refreshData() ) );
    connect( sessions, TQT_SIGNAL( currentChanged(TQListViewItem*) ), this, TQT_SLOT( refreshData() ) );
    connect( sessions, TQT_SIGNAL( mouseButtonClicked(int,TQListViewItem*,const 
TQPoint&,int) ), this, TQT_SLOT( refreshData() ) );
    connect( sessions, TQT_SIGNAL( collapsed(TQListViewItem*) ), this, TQT_SLOT( refreshData() ) );
    connect( connectBtn, TQT_SIGNAL( clicked() ), this, TQT_SLOT( connection() ) );
    connect( sessionName, TQT_SIGNAL( textChanged(const TQString&) ), this, TQT_SLOT( updateName(const TQString&) ) );
    connect( newGroupBtn, TQT_SIGNAL( clicked() ), this, TQT_SLOT( addGroup() ) );
    connect( anonymous, TQT_SIGNAL( clicked() ), this, TQT_SLOT( refreshData() ) );
    connect( protocol, TQT_SIGNAL(activated(int)), this, TQT_SLOT(refreshData()));

    // tab order
    setTabOrder( sessionName, hostName );
    setTabOrder( hostName, userName );
    setTabOrder( userName, password );
    setTabOrder( password, remoteDir );
    setTabOrder( remoteDir, description );
    setTabOrder( description, connectBtn );
    setTabOrder( connectBtn, addBtn );
    setTabOrder( addBtn, newGroupBtn );
    setTabOrder( newGroupBtn, removeBtn );
    setTabOrder( removeBtn, moreBtn );
    setTabOrder( moreBtn, closeBtn );
    setTabOrder( closeBtn, sessions );
    setTabOrder( sessions, portNum );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
remoteManBase::~remoteManBase()
{
    // no need to delete child widgets, TQt does it all for us
}

/*  
 *  Main event handler. Reimplemented to handle application
 *  font changes
 */
bool remoteManBase::event( TQEvent* ev )
{
    bool ret = TQDialog::event( ev ); 
    if ( ev->type() == TQEvent::ApplicationFontChange ) {
	TQFont TextLabel1_4_font(  TextLabel1_4->font() );
	TextLabel1_4_font.setPointSize( 10 );
	TextLabel1_4->setFont( TextLabel1_4_font ); 
    }
    return ret;
}

void remoteManBase::addSession()
{
    qWarning( "remoteManBase::addSession(): Not implemented yet!" );
}

void remoteManBase::connection()
{
    qWarning( "remoteManBase::connection(): Not implemented yet!" );
}

void remoteManBase::moreInfo()
{
    qWarning( "remoteManBase::moreInfo(): Not implemented yet!" );
}

void remoteManBase::addGroup()
{
    qWarning( "remoteManBase::addGroup(): Not implemented yet!" );
}

void remoteManBase::refreshData()
{
    qWarning( "remoteManBase::refreshData(): Not implemented yet!" );
}

void remoteManBase::removeSession()
{
    qWarning( "remoteManBase::removeSession(): Not implemented yet!" );
}

void remoteManBase::updateName(const TQString&)
{
    qWarning( "remoteManBase::updateName(const TQString&): Not implemented yet!" );
}

#include "remotemanbase.moc"
