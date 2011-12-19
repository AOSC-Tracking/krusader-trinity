#include <kdialog.h>
#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './actionpropertybase.ui'
**
** Created: Sat Mar 15 11:41:43 2008
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.8   edited Jan 11 14:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "actionpropertybase.h"

#include <tqvariant.h>
#include <tqpushbutton.h>
#include <tqtabwidget.h>
#include <tqtoolbutton.h>
#include <tqlabel.h>
#include <tqbuttongroup.h>
#include <tqradiobutton.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kicondialog.h>
#include <ktextedit.h>
#include <kkeybutton.h>
#include <tqcheckbox.h>
#include <tqgroupbox.h>
#include <klistbox.h>
#include <tqlayout.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>
#include "klineedit.h"
#include "kcombobox.h"
#include "kicondialog.h"
#include "ktextedit.h"
#include "kkeybutton.h"
#include "klistbox.h"

/*
 *  Constructs a ActionPropertyBase as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
ActionPropertyBase::ActionPropertyBase( TQWidget* parent, const char* name, WFlags fl )
    : TQWidget( parent, name, fl )
{
    if ( !name )
	setName( "ActionPropertyBase" );
    ActionPropertyBaseLayout = new TQGridLayout( this, 1, 1, 0, 0, "ActionPropertyBaseLayout"); 

    tabWidget3 = new TQTabWidget( this, "tabWidget3" );

    tab = new TQWidget( tabWidget3, "tab" );
    tabLayout = new TQGridLayout( tab, 1, 1, 11, 6, "tabLayout"); 

    ButtonAddStartpath = new TQToolButton( tab, "ButtonAddStartpath" );

    tabLayout->addWidget( ButtonAddStartpath, 8, 3 );

    LabelDescription = new TQLabel( tab, "LabelDescription" );
    LabelDescription->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)1, 0, 0, LabelDescription->sizePolicy().hasHeightForWidth() ) );

    tabLayout->addWidget( LabelDescription, 4, 0 );

    bgAccept = new TQButtonGroup( tab, "bgAccept" );
    bgAccept->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)5, (TQSizePolicy::SizeType)1, 0, 0, bgAccept->sizePolicy().hasHeightForWidth() ) );
    bgAccept->setColumnLayout(0, Qt::Vertical );
    bgAccept->tqlayout()->setSpacing( 6 );
    bgAccept->tqlayout()->setMargin( 11 );
    bgAcceptLayout = new TQGridLayout( bgAccept->tqlayout() );
    bgAcceptLayout->setAlignment( TQt::AlignTop );

    radioLocal = new TQRadioButton( bgAccept, "radioLocal" );
    radioLocal->setChecked( TRUE );

    bgAcceptLayout->addWidget( radioLocal, 0, 0 );

    radioUrl = new TQRadioButton( bgAccept, "radioUrl" );

    bgAcceptLayout->addWidget( radioUrl, 1, 0 );

    tabLayout->addMultiCellWidget( bgAccept, 9, 9, 2, 3 );

    leTitle = new KLineEdit( tab, "leTitle" );

    tabLayout->addMultiCellWidget( leTitle, 2, 2, 1, 3 );

    LabelTitle = new TQLabel( tab, "LabelTitle" );
    LabelTitle->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)0, 0, 0, LabelTitle->sizePolicy().hasHeightForWidth() ) );

    tabLayout->addWidget( LabelTitle, 2, 0 );

    tqlayout3 = new TQHBoxLayout( 0, 0, 6, "tqlayout3"); 

    tqlayout2 = new TQVBoxLayout( 0, 0, 6, "tqlayout2"); 

    leDistinctName = new KLineEdit( tab, "leDistinctName" );
    tqlayout2->addWidget( leDistinctName );

    cbCategory = new KComboBox( FALSE, tab, "cbCategory" );
    cbCategory->setEditable( TRUE );
    tqlayout2->addWidget( cbCategory );
    tqlayout3->addLayout( tqlayout2 );

    ButtonIcon = new KIconButton( tab, "ButtonIcon" );
    ButtonIcon->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)0, (TQSizePolicy::SizeType)0, 0, 0, ButtonIcon->sizePolicy().hasHeightForWidth() ) );
    ButtonIcon->setMinimumSize( TQSize( 50, 50 ) );
    ButtonIcon->setMaximumSize( TQSize( 50, 50 ) );
    tqlayout3->addWidget( ButtonIcon );

    tabLayout->addMultiCellLayout( tqlayout3, 0, 1, 1, 3 );

    LabelDistinctName = new TQLabel( tab, "LabelDistinctName" );
    LabelDistinctName->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)0, 0, 0, LabelDistinctName->sizePolicy().hasHeightForWidth() ) );

    tabLayout->addWidget( LabelDistinctName, 0, 0 );

    LabelCommandline = new TQLabel( tab, "LabelCommandline" );
    LabelCommandline->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)0, (TQSizePolicy::SizeType)0, 0, 0, LabelCommandline->sizePolicy().hasHeightForWidth() ) );

    tabLayout->addWidget( LabelCommandline, 7, 0 );

    leTooltip = new KLineEdit( tab, "leTooltip" );

    tabLayout->addMultiCellWidget( leTooltip, 3, 3, 1, 3 );

    leStartpath = new KLineEdit( tab, "leStartpath" );

    tabLayout->addMultiCellWidget( leStartpath, 8, 8, 1, 2 );

    LabelTooltip = new TQLabel( tab, "LabelTooltip" );
    LabelTooltip->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)0, 0, 0, LabelTooltip->sizePolicy().hasHeightForWidth() ) );

    tabLayout->addWidget( LabelTooltip, 3, 0 );

    leCommandline = new KLineEdit( tab, "leCommandline" );

    tabLayout->addMultiCellWidget( leCommandline, 7, 7, 1, 2 );

    LabelCategory = new TQLabel( tab, "LabelCategory" );
    LabelCategory->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)0, 0, 0, LabelCategory->sizePolicy().hasHeightForWidth() ) );

    tabLayout->addWidget( LabelCategory, 1, 0 );

    ButtonAddPlaceholder = new TQToolButton( tab, "ButtonAddPlaceholder" );
    ButtonAddPlaceholder->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)0, (TQSizePolicy::SizeType)0, 0, 0, ButtonAddPlaceholder->sizePolicy().hasHeightForWidth() ) );
    ButtonAddPlaceholder->setMinimumSize( TQSize( 0, 0 ) );

    tabLayout->addWidget( ButtonAddPlaceholder, 7, 3 );

    textDescription = new KTextEdit( tab, "textDescription" );
    textDescription->setWordWrap( KTextEdit::WidgetWidth );

    tabLayout->addMultiCellWidget( textDescription, 4, 6, 1, 3 );

    LabelStartpath = new TQLabel( tab, "LabelStartpath" );
    LabelStartpath->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)0, (TQSizePolicy::SizeType)0, 0, 0, LabelStartpath->sizePolicy().hasHeightForWidth() ) );

    tabLayout->addWidget( LabelStartpath, 8, 0 );
    spacer = new TQSpacerItem( 80, 19, TQSizePolicy::Minimum, TQSizePolicy::Expanding );
    tabLayout->addItem( spacer, 6, 0 );

    tqlayout4 = new TQHBoxLayout( 0, 0, 6, "tqlayout4"); 

    LabelShortcut = new TQLabel( tab, "LabelShortcut" );
    tqlayout4->addWidget( LabelShortcut );
    spacer6_2 = new TQSpacerItem( 161, 21, TQSizePolicy::Expanding, TQSizePolicy::Minimum );
    tqlayout4->addItem( spacer6_2 );

    KeyButtonShortcut = new KKeyButton( tab, "KeyButtonShortcut" );
    tqlayout4->addWidget( KeyButtonShortcut );

    tabLayout->addMultiCellLayout( tqlayout4, 10, 10, 2, 3 );

    bgExecType = new TQButtonGroup( tab, "bgExecType" );
    bgExecType->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)5, (TQSizePolicy::SizeType)1, 0, 0, bgExecType->sizePolicy().hasHeightForWidth() ) );
    bgExecType->setColumnLayout(0, Qt::Vertical );
    bgExecType->tqlayout()->setSpacing( 6 );
    bgExecType->tqlayout()->setMargin( 11 );
    bgExecTypeLayout = new TQGridLayout( bgExecType->tqlayout() );
    bgExecTypeLayout->setAlignment( TQt::AlignTop );

    radioCollectOutput = new TQRadioButton( bgExecType, "radioCollectOutput" );

    bgExecTypeLayout->addWidget( radioCollectOutput, 2, 0 );

    chkSeparateStdError = new TQCheckBox( bgExecType, "chkSeparateStdError" );
    chkSeparateStdError->setEnabled( FALSE );

    bgExecTypeLayout->addWidget( chkSeparateStdError, 3, 0 );

    radioNormal = new TQRadioButton( bgExecType, "radioNormal" );
    radioNormal->setChecked( TRUE );

    bgExecTypeLayout->addWidget( radioNormal, 0, 0 );

    radioTerminal = new TQRadioButton( bgExecType, "radioTerminal" );

    bgExecTypeLayout->addWidget( radioTerminal, 1, 0 );

    tabLayout->addMultiCellWidget( bgExecType, 9, 10, 0, 1 );
    tabWidget3->insertTab( tab, TQString::fromLatin1("") );

    tab_2 = new TQWidget( tabWidget3, "tab_2" );
    tabLayout_2 = new TQGridLayout( tab_2, 1, 1, 11, 6, "tabLayout_2"); 

    gbShowonly = new TQGroupBox( tab_2, "gbShowonly" );
    gbShowonly->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)7, (TQSizePolicy::SizeType)7, 0, 0, gbShowonly->sizePolicy().hasHeightForWidth() ) );
    gbShowonly->setColumnLayout(0, Qt::Vertical );
    gbShowonly->tqlayout()->setSpacing( 6 );
    gbShowonly->tqlayout()->setMargin( 11 );
    gbShowonlyLayout = new TQGridLayout( gbShowonly->tqlayout() );
    gbShowonlyLayout->setAlignment( TQt::AlignTop );

    tabShowonly = new TQTabWidget( gbShowonly, "tabShowonly" );
    tabShowonly->setTabPosition( TQTabWidget::Top );
    tabShowonly->setTabShape( TQTabWidget::Triangular );

    TabPage = new TQWidget( tabShowonly, "TabPage" );
    TabPageLayout = new TQGridLayout( TabPage, 1, 1, 11, 6, "TabPageLayout"); 

    ButtonNewProtocol = new TQToolButton( TabPage, "ButtonNewProtocol" );
    ButtonNewProtocol->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)0, 0, 0, ButtonNewProtocol->sizePolicy().hasHeightForWidth() ) );
    ButtonNewProtocol->setMinimumSize( TQSize( 0, 0 ) );
    ButtonNewProtocol->setMaximumSize( TQSize( 32767, 32767 ) );

    TabPageLayout->addWidget( ButtonNewProtocol, 0, 1 );

    ButtonEditProtocol = new TQToolButton( TabPage, "ButtonEditProtocol" );
    ButtonEditProtocol->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)0, 0, 0, ButtonEditProtocol->sizePolicy().hasHeightForWidth() ) );
    ButtonEditProtocol->setMinimumSize( TQSize( 0, 0 ) );
    ButtonEditProtocol->setMaximumSize( TQSize( 32767, 32767 ) );

    TabPageLayout->addWidget( ButtonEditProtocol, 1, 1 );
    spacer6_3 = new TQSpacerItem( 21, 58, TQSizePolicy::Minimum, TQSizePolicy::Expanding );
    TabPageLayout->addItem( spacer6_3, 3, 1 );

    ButtonRemoveProtocol = new TQToolButton( TabPage, "ButtonRemoveProtocol" );
    ButtonRemoveProtocol->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)0, 0, 0, ButtonRemoveProtocol->sizePolicy().hasHeightForWidth() ) );
    ButtonRemoveProtocol->setMinimumSize( TQSize( 0, 0 ) );
    ButtonRemoveProtocol->setMaximumSize( TQSize( 32767, 32767 ) );

    TabPageLayout->addWidget( ButtonRemoveProtocol, 2, 1 );

    lbShowonlyProtocol = new KListBox( TabPage, "lbShowonlyProtocol" );

    TabPageLayout->addMultiCellWidget( lbShowonlyProtocol, 0, 3, 0, 0 );
    tabShowonly->insertTab( TabPage, TQString::fromLatin1("") );

    tab_3 = new TQWidget( tabShowonly, "tab_3" );
    tabLayout_3 = new TQGridLayout( tab_3, 1, 1, 11, 6, "tabLayout_3"); 

    lbShowonlyPath = new KListBox( tab_3, "lbShowonlyPath" );

    tabLayout_3->addMultiCellWidget( lbShowonlyPath, 0, 3, 0, 0 );

    ButtonAddPath = new TQToolButton( tab_3, "ButtonAddPath" );
    ButtonAddPath->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)0, 0, 0, ButtonAddPath->sizePolicy().hasHeightForWidth() ) );
    ButtonAddPath->setMinimumSize( TQSize( 0, 0 ) );
    ButtonAddPath->setMaximumSize( TQSize( 32767, 32767 ) );

    tabLayout_3->addWidget( ButtonAddPath, 0, 1 );

    ButtonEditPath = new TQToolButton( tab_3, "ButtonEditPath" );
    ButtonEditPath->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)0, 0, 0, ButtonEditPath->sizePolicy().hasHeightForWidth() ) );
    ButtonEditPath->setMinimumSize( TQSize( 0, 0 ) );
    ButtonEditPath->setMaximumSize( TQSize( 32767, 32767 ) );

    tabLayout_3->addWidget( ButtonEditPath, 1, 1 );

    ButtonRemovePath = new TQToolButton( tab_3, "ButtonRemovePath" );
    ButtonRemovePath->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)0, 0, 0, ButtonRemovePath->sizePolicy().hasHeightForWidth() ) );
    ButtonRemovePath->setMinimumSize( TQSize( 0, 0 ) );
    ButtonRemovePath->setMaximumSize( TQSize( 32767, 32767 ) );

    tabLayout_3->addWidget( ButtonRemovePath, 2, 1 );
    spacer4 = new TQSpacerItem( 21, 61, TQSizePolicy::Minimum, TQSizePolicy::Expanding );
    tabLayout_3->addItem( spacer4, 3, 1 );
    tabShowonly->insertTab( tab_3, TQString::fromLatin1("") );

    tab_4 = new TQWidget( tabShowonly, "tab_4" );
    tabLayout_4 = new TQGridLayout( tab_4, 1, 1, 11, 6, "tabLayout_4"); 

    lbShowonlyMime = new KListBox( tab_4, "lbShowonlyMime" );

    tabLayout_4->addMultiCellWidget( lbShowonlyMime, 0, 3, 0, 0 );

    ButtonAddMime = new TQToolButton( tab_4, "ButtonAddMime" );
    ButtonAddMime->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)0, 0, 0, ButtonAddMime->sizePolicy().hasHeightForWidth() ) );
    ButtonAddMime->setMinimumSize( TQSize( 0, 0 ) );
    ButtonAddMime->setMaximumSize( TQSize( 32767, 32767 ) );

    tabLayout_4->addWidget( ButtonAddMime, 0, 1 );

    ButtonEditMime = new TQToolButton( tab_4, "ButtonEditMime" );
    ButtonEditMime->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)0, 0, 0, ButtonEditMime->sizePolicy().hasHeightForWidth() ) );
    ButtonEditMime->setMinimumSize( TQSize( 0, 0 ) );
    ButtonEditMime->setMaximumSize( TQSize( 32767, 32767 ) );

    tabLayout_4->addWidget( ButtonEditMime, 1, 1 );

    ButtonRemoveMime = new TQToolButton( tab_4, "ButtonRemoveMime" );
    ButtonRemoveMime->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)0, 0, 0, ButtonRemoveMime->sizePolicy().hasHeightForWidth() ) );
    ButtonRemoveMime->setMinimumSize( TQSize( 0, 0 ) );
    ButtonRemoveMime->setMaximumSize( TQSize( 32767, 32767 ) );

    tabLayout_4->addWidget( ButtonRemoveMime, 2, 1 );
    spacer5 = new TQSpacerItem( 21, 41, TQSizePolicy::Minimum, TQSizePolicy::Expanding );
    tabLayout_4->addItem( spacer5, 3, 1 );
    tabShowonly->insertTab( tab_4, TQString::fromLatin1("") );

    TabPage_2 = new TQWidget( tabShowonly, "TabPage_2" );
    TabPageLayout_2 = new TQGridLayout( TabPage_2, 1, 1, 11, 6, "TabPageLayout_2"); 

    lbShowonlyFile = new KListBox( TabPage_2, "lbShowonlyFile" );

    TabPageLayout_2->addMultiCellWidget( lbShowonlyFile, 0, 3, 0, 0 );

    ButtonNewFile = new TQToolButton( TabPage_2, "ButtonNewFile" );
    ButtonNewFile->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)0, 0, 0, ButtonNewFile->sizePolicy().hasHeightForWidth() ) );
    ButtonNewFile->setMinimumSize( TQSize( 0, 0 ) );
    ButtonNewFile->setMaximumSize( TQSize( 32767, 32767 ) );

    TabPageLayout_2->addWidget( ButtonNewFile, 0, 1 );

    ButtonEditFile = new TQToolButton( TabPage_2, "ButtonEditFile" );
    ButtonEditFile->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)0, 0, 0, ButtonEditFile->sizePolicy().hasHeightForWidth() ) );
    ButtonEditFile->setMinimumSize( TQSize( 0, 0 ) );
    ButtonEditFile->setMaximumSize( TQSize( 32767, 32767 ) );

    TabPageLayout_2->addWidget( ButtonEditFile, 1, 1 );

    ButtonRemoveFile = new TQToolButton( TabPage_2, "ButtonRemoveFile" );
    ButtonRemoveFile->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)0, 0, 0, ButtonRemoveFile->sizePolicy().hasHeightForWidth() ) );
    ButtonRemoveFile->setMinimumSize( TQSize( 0, 0 ) );
    ButtonRemoveFile->setMaximumSize( TQSize( 32767, 32767 ) );

    TabPageLayout_2->addWidget( ButtonRemoveFile, 2, 1 );
    spacer6 = new TQSpacerItem( 21, 41, TQSizePolicy::Minimum, TQSizePolicy::Expanding );
    TabPageLayout_2->addItem( spacer6, 3, 1 );
    tabShowonly->insertTab( TabPage_2, TQString::fromLatin1("") );

    gbShowonlyLayout->addWidget( tabShowonly, 0, 0 );

    tabLayout_2->addMultiCellWidget( gbShowonly, 0, 0, 0, 1 );

    chkConfirmExecution = new TQCheckBox( tab_2, "chkConfirmExecution" );

    tabLayout_2->addMultiCellWidget( chkConfirmExecution, 1, 1, 0, 1 );

    chkDifferentUser = new TQCheckBox( tab_2, "chkDifferentUser" );

    tabLayout_2->addWidget( chkDifferentUser, 2, 0 );

    leDifferentUser = new KLineEdit( tab_2, "leDifferentUser" );
    leDifferentUser->setEnabled( FALSE );

    tabLayout_2->addWidget( leDifferentUser, 2, 1 );
    spacer3 = new TQSpacerItem( 161, 102, TQSizePolicy::Minimum, TQSizePolicy::Expanding );
    tabLayout_2->addMultiCell( spacer3, 3, 3, 0, 1 );
    tabWidget3->insertTab( tab_2, TQString::fromLatin1("") );

    ActionPropertyBaseLayout->addWidget( tabWidget3, 0, 0 );
    languageChange();
    resize( TQSize(485, 470).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( radioCollectOutput, TQT_SIGNAL( toggled(bool) ), chkSeparateStdError, TQT_SLOT( setEnabled(bool) ) );
    connect( chkDifferentUser, TQT_SIGNAL( toggled(bool) ), leDifferentUser, TQT_SLOT( setEnabled(bool) ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
ActionPropertyBase::~ActionPropertyBase()
{
    // no need to delete child widgets, TQt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void ActionPropertyBase::languageChange()
{
    setCaption( tr2i18n( "Action Property" ) );
    ButtonAddStartpath->setText( tr2i18n( "..." ) );
    LabelDescription->setText( tr2i18n( "Description:" ) );
    TQWhatsThis::add( LabelDescription, tr2i18n( "A detailed description of the <b>Useraction</b>. It is only displayed in the <i>Konfigurator</i> and via <code>Shift-F1</code>." ) );
    bgAccept->setTitle( tr2i18n( "Command accepts" ) );
    radioLocal->setText( tr2i18n( "Local files only (no URL's)" ) );
    TQWhatsThis::add( radioLocal, tr2i18n( "Substitute the <b>Placeholders</b> with local filenames." ) );
    radioUrl->setText( tr2i18n( "URL's (remote and local)" ) );
    TQWhatsThis::add( radioUrl, tr2i18n( "Substitute the <b>Placeholders</b> with valid URL's." ) );
    TQWhatsThis::add( leTitle, tr2i18n( "The title displayed in the <b>Usermenu</b>." ) );
    LabelTitle->setText( tr2i18n( "Title:" ) );
    TQWhatsThis::add( LabelTitle, tr2i18n( "The title displayed in the <b>Usermenu</b>." ) );
    TQWhatsThis::add( leDistinctName, tr2i18n( "Unique name of the <b>Useraction</b>. It is only used in the <i>Konfigurator</i> and doesn't appear in any other menu.<p><b>Note</b>: The <i>Title</i> shown in the <b>Usermenu</b> can be set below." ) );
    TQWhatsThis::add( cbCategory, tr2i18n( "<b>Useractions</b> can be grouped in categories for better distinction. Choose a existing <i>Category</i> or create a new one by entering a name." ) );
    ButtonIcon->setText( tr2i18n( "Icon" ) );
    TQWhatsThis::add( ButtonIcon, tr2i18n( "Each <b>Useraction</b> can have its own icon. It will appear in front of the title in the <b>Usermenu</b>." ) );
    LabelDistinctName->setText( tr2i18n( "Identifier:" ) );
    TQWhatsThis::add( LabelDistinctName, tr2i18n( "<p>Unique name of the <b>Useraction</b>. It is only used in the <i>Konfigurator</i> and doesn't appear in any other menu.</p><p><b>Note</b>: The <i>Title</i> shown in the <b>Usermenu</b> can be set below.</p>" ) );
    LabelCommandline->setText( tr2i18n( "Command:" ) );
    TQWhatsThis::add( LabelCommandline, tr2i18n( "<p>The <i>Command</i> defines the command that will be executed when the <b>Useraction</b> is used. It can be a simple shell command or a complex sequence of multiple commands with <b>Placeholders</b>.</p><p>Examples:<ul><code><li>eject /mnt/cdrom</li><li>amarok --append %aList(\"Selected\")%</li></code></ul>\n"
"Please consult the handbook to learn more about the syntax.</p>" ) );
    TQWhatsThis::add( leTooltip, tr2i18n( "The <i>Tooltip</i> is shown when the mouse cursor is hold over an entry of the <b>Useraction Toolbar</b>." ) );
    TQWhatsThis::add( leStartpath, tr2i18n( "The <i>Workdir</i> defines in which directory the <i>Command</i> will be executed." ) );
    LabelTooltip->setText( tr2i18n( "Tooltip:" ) );
    TQWhatsThis::add( LabelTooltip, tr2i18n( "The <i>Tooltip</i> is shown when the mouse cursor is hold over an entry of the <b>Useraction Toolbar</b>." ) );
    leCommandline->setText( TQString() );
    TQWhatsThis::add( leCommandline, tr2i18n( "The <i>Command</i> defines the command that will be executed when the <b>Useraction</b> is used. It can be a simple shell command or a complex sequence of multiple commands with <b>Placeholders</b>.<p>\n"
"Examples:<ul><code><li>eject /mnt/cdrom</li><li>amarok --append %aList(\"Selected\")%</li></code></ul>\n"
"Please consult the handbook to learn more about the syntax." ) );
    LabelCategory->setText( tr2i18n( "Category:" ) );
    TQWhatsThis::add( LabelCategory, tr2i18n( "<b>Useractions</b> can be grouped in categories for better distinction. Choose a existing <i>Category</i> or create a new one by entering a name." ) );
    ButtonAddPlaceholder->setText( tr2i18n( "&Add" ) );
    TQWhatsThis::add( ButtonAddPlaceholder, tr2i18n( "Add <b>Placeholders</b> for the selected files in the panel." ) );
    TQWhatsThis::add( textDescription, tr2i18n( "A detailed description of the <b>Useraction</b>. It is only displayed in the <i>Konfigurator</i> and via <code>Shift-F1</code>." ) );
    LabelStartpath->setText( tr2i18n( "Workdir:" ) );
    TQWhatsThis::add( LabelStartpath, tr2i18n( "The <i>Workdir</i> defines in which directory the <i>Command</i> will be executed." ) );
    LabelShortcut->setText( tr2i18n( "Default shortcut:" ) );
    KeyButtonShortcut->setText( tr2i18n( "None" ) );
    TQWhatsThis::add( KeyButtonShortcut, tr2i18n( "Set a default keyboard shortcut." ) );
    bgExecType->setTitle( tr2i18n( "Execution mode" ) );
    radioCollectOutput->setText( tr2i18n( "Collect output" ) );
    TQWhatsThis::add( radioCollectOutput, tr2i18n( "Collect the output of the executed program." ) );
    chkSeparateStdError->setText( tr2i18n( "Separate standard error" ) );
    TQWhatsThis::add( chkSeparateStdError, tr2i18n( "Separate standard out and standard error in the output collection." ) );
    radioNormal->setText( tr2i18n( "Normal" ) );
    radioTerminal->setText( tr2i18n( "Run in terminal" ) );
    TQWhatsThis::add( radioTerminal, tr2i18n( "Run the command in a terminal." ) );
    tabWidget3->changeTab( tab, tr2i18n( "Basic Properties" ) );
    gbShowonly->setTitle( tr2i18n( "The Useraction is only available for" ) );
    ButtonNewProtocol->setText( tr2i18n( "&New..." ) );
    ButtonEditProtocol->setText( tr2i18n( "Chan&ge..." ) );
    ButtonRemoveProtocol->setText( tr2i18n( "De&lete" ) );
    TQWhatsThis::add( lbShowonlyProtocol, tr2i18n( "Show the <b>Useraction</b> only for the values defined here." ) );
    tabShowonly->changeTab( TabPage, tr2i18n( "Protocol" ) );
    TQWhatsThis::add( lbShowonlyPath, tr2i18n( "Show the <b>Useraction</b> only for the values defined here." ) );
    ButtonAddPath->setText( tr2i18n( "&New..." ) );
    ButtonEditPath->setText( tr2i18n( "Chan&ge..." ) );
    ButtonRemovePath->setText( tr2i18n( "De&lete" ) );
    tabShowonly->changeTab( tab_3, tr2i18n( "Path" ) );
    TQWhatsThis::add( lbShowonlyMime, tr2i18n( "Show the <b>Useraction</b> only for the values defined here." ) );
    ButtonAddMime->setText( tr2i18n( "&New..." ) );
    ButtonEditMime->setText( tr2i18n( "Chan&ge..." ) );
    ButtonRemoveMime->setText( tr2i18n( "De&lete" ) );
    tabShowonly->changeTab( tab_4, tr2i18n( "Mime-type" ) );
    TQWhatsThis::add( lbShowonlyFile, tr2i18n( "Show the <b>Useraction</b> only for the filenames defined here. The wildcards '<code>?</code>' and '<code>*</code>' can be used." ) );
    ButtonNewFile->setText( tr2i18n( "&New..." ) );
    ButtonEditFile->setText( tr2i18n( "Chan&ge..." ) );
    ButtonRemoveFile->setText( tr2i18n( "De&lete" ) );
    tabShowonly->changeTab( TabPage_2, tr2i18n( "Filename" ) );
    chkConfirmExecution->setText( tr2i18n( "Confirm each program call separately" ) );
    TQWhatsThis::add( chkConfirmExecution, tr2i18n( "Allows to tweak the <i>Command</i> before it is executed." ) );
    chkDifferentUser->setText( tr2i18n( "Run as different user:" ) );
    TQWhatsThis::add( chkDifferentUser, tr2i18n( "Execute the <i>Command</i> under a different user-id." ) );
    TQWhatsThis::add( leDifferentUser, tr2i18n( "Execute the <i>Command</i> under a different user-id." ) );
    tabWidget3->changeTab( tab_2, tr2i18n( "Advanced Properties" ) );
}

#include "actionpropertybase.moc"
