//
// C++ Implementation: actionproperty
//
// Description: 
//
//
// Author: Jonas B�r (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "actionproperty.h"
#include "addplaceholderpopup.h"

#include "../UserAction/useraction.h"
#include "../UserAction/kraction.h"
#include "../krusader.h"

#include <tqtoolbutton.h>
#include <tqcheckbox.h>
#include <tqradiobutton.h>
#include <tqbuttongroup.h>
#include <klocale.h>
#include <tdefiledialog.h>
#include <klineedit.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <kkeybutton.h>
#include <kcombobox.h>
#include <kicondialog.h>
#include <ktextedit.h>
#include <kiconloader.h>

#define ICON(N)		TDEGlobal::iconLoader()->loadIcon(N, KIcon::Small)

ActionProperty::ActionProperty( TQWidget *parent, const char *name, KrAction *action )
 : ActionPropertyBase( parent, name ), _modified(false)
 {
   if ( action ) {
      _action = action;
      updateGUI( _action );
   }

   ButtonAddPlaceholder->setPixmap( ICON("add") );
   ButtonAddStartpath->setPixmap( ICON("fileopen") );

   // fill with all existing categories
   cbCategory->insertStringList( krUserAction->allCategories() );

   connect( ButtonAddPlaceholder, TQT_SIGNAL( clicked() ), this, TQT_SLOT( addPlaceholder() ) );
   connect( ButtonAddStartpath, TQT_SIGNAL( clicked() ), this, TQT_SLOT( addStartpath() ) );
   connect( ButtonNewProtocol, TQT_SIGNAL( clicked() ), this, TQT_SLOT( newProtocol() ) );
   connect( ButtonEditProtocol, TQT_SIGNAL( clicked() ), this, TQT_SLOT( editProtocol() ) );
   connect( ButtonRemoveProtocol, TQT_SIGNAL( clicked() ), this, TQT_SLOT( removeProtocol() ) );
   connect( ButtonAddPath, TQT_SIGNAL( clicked() ), this, TQT_SLOT( addPath() ) );
   connect( ButtonEditPath, TQT_SIGNAL( clicked() ), this, TQT_SLOT( editPath() ) );
   connect( ButtonRemovePath, TQT_SIGNAL( clicked() ), this, TQT_SLOT( removePath() ) );
   connect( ButtonAddMime, TQT_SIGNAL( clicked() ), this, TQT_SLOT( addMime() ) );
   connect( ButtonEditMime, TQT_SIGNAL( clicked() ), this, TQT_SLOT( editMime() ) );
   connect( ButtonRemoveMime, TQT_SIGNAL( clicked() ), this, TQT_SLOT( removeMime() ) );
   connect( ButtonNewFile, TQT_SIGNAL( clicked() ), this, TQT_SLOT( newFile() ) );
   connect( ButtonEditFile, TQT_SIGNAL( clicked() ), this, TQT_SLOT( editFile() ) );
   connect( ButtonRemoveFile, TQT_SIGNAL( clicked() ), this, TQT_SLOT( removeFile() ) );
   connect( KeyButtonShortcut, TQT_SIGNAL( capturedShortcut(const KShortcut&) ), this, TQT_SLOT( changedShortcut(const KShortcut&) ) );
   // track modifications:
   connect( leDistinctName, TQT_SIGNAL( textChanged(const TQString&) ), TQT_SLOT( setModified() ) );
   connect( leTitle, TQT_SIGNAL( textChanged(const TQString&) ), TQT_SLOT( setModified() ) );
   connect( ButtonIcon, TQT_SIGNAL( iconChanged(TQString) ), TQT_SLOT( setModified() ) );
   connect( cbCategory, TQT_SIGNAL( textChanged(const TQString&) ), TQT_SLOT( setModified() ) );
   connect( leTooltip, TQT_SIGNAL( textChanged(const TQString&) ), TQT_SLOT( setModified() ) );
   connect( textDescription, TQT_SIGNAL( textChanged() ), TQT_SLOT( setModified() ) );
   connect( leDistinctName, TQT_SIGNAL( textChanged(const TQString&) ), TQT_SLOT( setModified() ) );
   connect( leCommandline, TQT_SIGNAL( textChanged(const TQString&) ), TQT_SLOT( setModified() ) );
   connect( leStartpath, TQT_SIGNAL( textChanged(const TQString&) ), TQT_SLOT( setModified() ) );
   connect( bgExecType, TQT_SIGNAL( clicked(int) ), TQT_SLOT( setModified() ) );
   connect( bgAccept, TQT_SIGNAL( clicked(int) ), TQT_SLOT( setModified() ) );
   connect( KeyButtonShortcut, TQT_SIGNAL( capturedShortcut(const KShortcut&) ), TQT_SLOT( setModified() ) );
   connect( leDifferentUser, TQT_SIGNAL( textChanged(const TQString&) ), TQT_SLOT( setModified() ) );
   connect( chkDifferentUser, TQT_SIGNAL( clicked() ), TQT_SLOT( setModified() ) );
   connect( chkConfirmExecution, TQT_SIGNAL( clicked() ), TQT_SLOT( setModified() ) );
   // The modified-state of the ShowOnly-lists is tracked in the access-functions below
}

ActionProperty::~ActionProperty() {
}

void ActionProperty::changedShortcut( const KShortcut& shortcut ) {
  KeyButtonShortcut->setShortcut( shortcut, false );
}


void ActionProperty::clear() {
   _action = 0;

   // This prevents the changed-signal from being emited during the GUI-update
   _modified = true; // The real state is set at the end of this function.

   leDistinctName->clear();
   cbCategory->clearEdit();
   leTitle->clear();
   leTooltip->clear();
   textDescription->clear();
   leCommandline->clear();
   leStartpath->clear();
   KeyButtonShortcut->setShortcut( KShortcut(), false );

   lbShowonlyProtocol->clear();
   lbShowonlyPath->clear();
   lbShowonlyMime->clear();
   lbShowonlyFile->clear();

   chkSeparateStdError->setChecked( false );
   radioNormal->setChecked( true );

   radioLocal->setChecked( true );

   chkConfirmExecution->setChecked( false );

   ButtonIcon->resetIcon();

    leDifferentUser->clear();
    chkDifferentUser->setChecked( false );

   setModified( false );
}

void ActionProperty::updateGUI( KrAction *action ) {
   if ( action )
      _action = action;
   if ( ! _action )
      return;

   // This prevents the changed-signal from being emited during the GUI-update.
   _modified = true; // The real state is set at the end of this function.

   leDistinctName->setText( _action->name() );
   cbCategory->setCurrentText( _action->category() );
   leTitle->setText( _action->text() );
   leTooltip->setText( _action->toolTip() );
   textDescription->setText( _action->whatsThis() );
   leCommandline->setText( _action->command() );
   leCommandline->home(false);
   leStartpath->setText( _action->startpath() );
   KeyButtonShortcut->setShortcut( _action->shortcut(), false );

   lbShowonlyProtocol->clear();
   lbShowonlyProtocol->insertStringList( _action->showonlyProtocol() );
   lbShowonlyPath->clear();
   lbShowonlyPath->insertStringList( _action->showonlyPath() );
   lbShowonlyMime->clear();
   lbShowonlyMime->insertStringList( _action->showonlyMime() );
   lbShowonlyFile->clear();
   lbShowonlyFile->insertStringList( _action->showonlyFile() );

   chkSeparateStdError->setChecked( false );
   switch ( _action->execType() ) {
   case KrAction::CollectOutputSeparateStderr:
      chkSeparateStdError->setChecked( true );
      radioCollectOutput->setChecked( true );
      break;
   case KrAction::CollectOutput:
      radioCollectOutput->setChecked( true );
      break;
   case KrAction::Terminal:
      radioTerminal->setChecked( true );
      break;
   default: // case KrAction::Normal:
      radioNormal->setChecked( true );
      break;
   }

   if ( _action->acceptURLs() )
      radioUrl->setChecked( true );
   else
      radioLocal->setChecked( true );

   chkConfirmExecution->setChecked( _action->confirmExecution() );

   if ( ! _action->icon().isEmpty() )
      ButtonIcon->setIcon( _action->icon() );
   else
      ButtonIcon->resetIcon();

    leDifferentUser->setText( _action->user() );
    if ( _action->user().isEmpty() )
        chkDifferentUser->setChecked( false );
    else
        chkDifferentUser->setChecked( true );

   setModified( false );
}

void ActionProperty::updateAction( KrAction *action ) {
   if ( action )
      _action = action;
   if ( ! _action )
      return;

   if ( _action->category() != cbCategory->currentText() ) {
      _action->setCategory( cbCategory->currentText() );
      // Update the category-list
      cbCategory->clear();
      cbCategory->insertStringList( krUserAction->allCategories() );
      cbCategory->setCurrentText( _action->category() );
   }

   _action->setName( leDistinctName->text().latin1() );
   _action->setText( leTitle->text() );
   _action->setToolTip( leTooltip->text() );
   _action->setWhatsThis( textDescription->text() );
   _action->setCommand( leCommandline->text() );
   _action->setStartpath( leStartpath->text() );
   _action->setShortcut( KeyButtonShortcut->shortcut() );

   TQListBoxItem* lbi = lbShowonlyProtocol->firstItem();
   TQStringList list;
   while ( lbi ) {
      list << lbi->text();
      lbi = lbi->next();
   }
   _action->setShowonlyProtocol( list );

   lbi = lbShowonlyPath->firstItem();
   list = TQStringList();
   while ( lbi ) {
      list << lbi->text();
      lbi = lbi->next();
   }
   _action->setShowonlyPath( list );

   lbi = lbShowonlyMime->firstItem();
   list = TQStringList();
   while ( lbi ) {
      list << lbi->text();
      lbi = lbi->next();
   }
   _action->setShowonlyMime( list );

   lbi = lbShowonlyFile->firstItem();
   list = TQStringList();
   while ( lbi ) {
      list << lbi->text();
      lbi = lbi->next();
   }
   _action->setShowonlyFile( list );

   if ( radioCollectOutput->isChecked() && chkSeparateStdError->isChecked() )
      _action->setExecType( KrAction::CollectOutputSeparateStderr );
   else if ( radioCollectOutput->isChecked() && ! chkSeparateStdError->isChecked() )
      _action->setExecType( KrAction::CollectOutput );
   else if ( radioTerminal->isChecked() )
      _action->setExecType( KrAction::Terminal );
   else
      _action->setExecType( KrAction::Normal );

   if ( radioUrl->isChecked() )
      _action->setAcceptURLs( true );
   else
      _action->setAcceptURLs( false );

   _action->setConfirmExecution( chkConfirmExecution->isChecked()  );

   _action->setIcon( ButtonIcon->icon() );

   _action->setUser( leDifferentUser->text() );

   setModified( false );
}

void ActionProperty::addPlaceholder() {
   AddPlaceholderPopup popup( this );
   TQString exp = popup.getPlaceholder( mapToGlobal(
   		TQPoint(
   			ButtonAddPlaceholder->pos().x() + ButtonAddPlaceholder->width()+6, // 6 is the default margin
   			ButtonAddPlaceholder->pos().y()
   		)
   ) );
   leCommandline->insert( exp );
}


void ActionProperty::addStartpath() {
   TQString folder = KFileDialog::getExistingDirectory(TQString(), this);
   if (folder != TQString()) {
      leStartpath->setText( folder );
   }
}


void ActionProperty::newProtocol() {
  bool ok;
  TQString text = KInputDialog::getText(
		i18n( "New protocol" ),
		i18n( "Set a protocol:" ),
		lbShowonlyProtocol->currentText(),
		&ok, this );
    if ( ok && !text.isEmpty() ) {
      lbShowonlyProtocol->insertStringList( TQStringList::split( ";", text ) );
      setModified();
   }
}

void ActionProperty::editProtocol() {
  if (lbShowonlyProtocol->currentItem() == -1)
    return;

  bool ok;
  TQString text = KInputDialog::getText(
		i18n( "Edit protocol" ),
		i18n( "Set another protocol:" ),
		lbShowonlyProtocol->currentText(),
		&ok, this );
    if ( ok && !text.isEmpty() ) {
      lbShowonlyProtocol->changeItem( text, lbShowonlyProtocol->currentItem() );
      setModified();
   }
}

void ActionProperty::removeProtocol() {
   if (lbShowonlyProtocol->currentItem() != -1) {
     lbShowonlyProtocol->removeItem( lbShowonlyProtocol->currentItem() );
      setModified();
  }
}

void ActionProperty::addPath() {
   TQString folder = KFileDialog::getExistingDirectory(TQString(), this);
   if (folder != TQString()) {
     lbShowonlyPath->insertItem( folder );
     setModified();
   }
}

void ActionProperty::editPath() {
  if (lbShowonlyPath->currentItem() == -1)
    return;

  bool ok;
  TQString text = KInputDialog::getText(
		i18n( "Edit path" ),
		i18n( "Set another path:" ),
		lbShowonlyPath->currentText(),
		&ok, this );
    if ( ok && !text.isEmpty() ) {
      lbShowonlyPath->changeItem( text, lbShowonlyPath->currentItem() );
      setModified();
   }
}

void ActionProperty::removePath() {
   if (lbShowonlyPath->currentItem() != -1) {
     lbShowonlyPath->removeItem( lbShowonlyPath->currentItem() );
     setModified();
  }
}

void ActionProperty::addMime() { 
  bool ok;
  TQString text = KInputDialog::getText(
		i18n( "New mime-type" ),
		i18n( "Set a mime-type:" ),
		lbShowonlyMime->currentText(),
		&ok, this );
    if ( ok && !text.isEmpty() ) {
      lbShowonlyMime->insertStringList( TQStringList::split( ";", text ) );
      setModified();
   }
}

void ActionProperty::editMime() { 
  if (lbShowonlyMime->currentItem() == -1)
    return;

  bool ok;
  TQString text = KInputDialog::getText(
		i18n( "Edit mime-type" ),
		i18n( "Set another mime-type:" ),
		lbShowonlyMime->currentText(),
		&ok, this );
    if ( ok && !text.isEmpty() ) {
      lbShowonlyMime->changeItem( text, lbShowonlyMime->currentItem() );
      setModified();
   }
}

void ActionProperty::removeMime() { 
   if (lbShowonlyMime->currentItem() != -1) {
     lbShowonlyMime->removeItem( lbShowonlyMime->currentItem() );
     setModified();
  }
}

void ActionProperty::newFile() {
  bool ok;
  TQString text = KInputDialog::getText(
		i18n( "New filename" ),
		i18n( "Set a filename:" ),
		lbShowonlyFile->currentText(),
		&ok, this );
    if ( ok && !text.isEmpty() ) {
      lbShowonlyFile->insertStringList( TQStringList::split( ";", text ) );
      setModified();
   }
}

void ActionProperty::editFile() {
  if (lbShowonlyFile->currentItem() == -1)
    return;

  bool ok;
  TQString text = KInputDialog::getText(
		i18n( "Edit filename" ),
		i18n( "Set another filename:" ),
		lbShowonlyFile->currentText(),
		&ok, this );
    if ( ok && !text.isEmpty() ) {
      lbShowonlyFile->changeItem( text, lbShowonlyFile->currentItem() );
      setModified();
   }
}

void ActionProperty::removeFile() {
   if (lbShowonlyFile->currentItem() != -1) {
     lbShowonlyFile->removeItem( lbShowonlyFile->currentItem() );
     setModified();
  }
}


bool ActionProperty::validProperties() {
  if ( leDistinctName->text().simplifyWhiteSpace().isEmpty() ) {
    KMessageBox::error( this, i18n("Please set a unique name for the useraction") );
    leDistinctName->setFocus();
    return false;
  }
  if ( leTitle->text().simplifyWhiteSpace().isEmpty() ) {
    KMessageBox::error( this, i18n("Please set a title for the menu entry") );
    leTitle->setFocus();
    return false;
  }
  if ( leCommandline->text().simplifyWhiteSpace().isEmpty() ) {
    KMessageBox::error( this, i18n("Command line is empty") );
    leCommandline->setFocus();
    return false;
  }
  if ( leDistinctName->isEnabled() )
    if ( krApp->actionCollection()->action( leDistinctName->text().latin1() ) ) {
      KMessageBox::error( this,
      		i18n("There already is an action with this name\n"
      		"If you don't have such an useraction the name is used by Krusader for an internal action")
      	);
      leDistinctName->setFocus();
      return false;
    }

  return true;
}

void ActionProperty::setModified( bool m )
{
   if ( m && !_modified ) { // emit only when the state _changes_to_true_,
      emit changed();
      }
   _modified = m;
}


#include "actionproperty.moc"
