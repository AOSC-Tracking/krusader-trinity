//
// C++ Implementation: useractionpage
//
// Description: 
//
//
// Author: Shie Erlich and Rafi Yanai <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "useractionpage.h"

#include <tqsplitter.h>
#include <tqlayout.h>
#include <tqtoolbutton.h>
#include <tqtooltip.h>
#include <klineedit.h>
#include <tdemessagebox.h>
#include <tdefiledialog.h>
#include <kiconloader.h>
#include <tdelocale.h>
#include <kclipboard.h>

#include "actionproperty.h"
#include "useractionlistview.h"
#include "../UserMenu/usermenu.h" //FIXME this should not be needed here!
#include "../UserAction/useraction.h"
#include "../UserAction/kraction.h"
#include "../krusader.h"

#define ICON(N)		TDEGlobal::iconLoader()->loadIcon(N, TDEIcon::Toolbar)
//This is the filter in the KFileDialog of Import/Export:
static const char* FILE_FILTER = I18N_NOOP("*.xml|xml-files\n*|all files");


UserActionPage::UserActionPage( TQWidget* parent )
 : TQWidget( parent, "UserActionPage" )
{
   TQVBoxLayout* layout = new TQVBoxLayout( this, 0, 6, "UserActionPageLayout" ); // 0px margin, 6px item-spacing

   // ======== pseudo-toolbar start ========
   TQHBoxLayout* toolbarLayout = new TQHBoxLayout( layout, 0, 0 ); // neither margin nor spacing for the toolbar with autoRaise

   newButton = new TQToolButton( this, "newButton" );
   newButton->setPixmap( ICON("document-new") );
   newButton->setAutoRaise(true);
   TQToolTip::add( newButton, i18n("Create new useraction") );

   importButton = new TQToolButton( this, "importButton" );
   importButton->setPixmap( ICON("fileimport") );
   importButton->setAutoRaise(true);
   TQToolTip::add( importButton, i18n("Import useractions") );

   exportButton = new TQToolButton( this, "exportButton" );
   exportButton->setPixmap( ICON("fileexport") );
   exportButton->setAutoRaise(true);
   TQToolTip::add( exportButton, i18n("Export useractions") );

   copyButton = new TQToolButton( this, "copyButton" );
   copyButton->setPixmap( ICON("edit-copy") );
   copyButton->setAutoRaise(true);
   TQToolTip::add( copyButton, i18n("Copy useractions to clipboard") );

   pasteButton = new TQToolButton( this, "pasteButton" );
   pasteButton->setPixmap( ICON("edit-paste") );
   pasteButton->setAutoRaise(true);
   TQToolTip::add( pasteButton, i18n("Paste useractions from clipboard") );

   removeButton = new TQToolButton( this, "removeButton" );
   removeButton->setPixmap( ICON("edit-delete") );
   removeButton->setAutoRaise(true);
   TQToolTip::add( removeButton, i18n("Delete selected useractions") );

   toolbarLayout->addWidget( newButton );
   toolbarLayout->addWidget( importButton );
   toolbarLayout->addWidget( exportButton );
   toolbarLayout->addWidget( copyButton );
   toolbarLayout->addWidget( pasteButton );
   toolbarLayout->addSpacing( 6 ); // 6 pixel nothing
   toolbarLayout->addWidget( removeButton );
   toolbarLayout->addStretch( 1000 ); // some very large stretch-factor
   // ======== pseudo-toolbar end ========
/* This seems obsolete now!
   // Display some help
   KMessageBox::information( this,	// parent
   		i18n( "When you apply changes to an action, the modifications "
   			"become available in the current session immediately.\n"
   			"When closing ActionMan, you will be asked to save the changes permanently."
   		),
  		TQString(),	// caption
  		"show UserAction help"	//dontShowAgainName for the config
  	);
*/
   TQSplitter *split = new TQSplitter( this, "useractionpage splitter");
   layout->addWidget( split, 1000 ); // again a very large stretch-factor to fix the height of the toolbar

   actionTree = new UserActionListView( split, "actionTree" );
   actionProperties = new ActionProperty( split, "actionProperties" );
   actionProperties->setEnabled( false ); // if there are any actions in the list, the first is displayed and this widget is enabled

   connect(  actionTree, TQ_SIGNAL( currentChanged(TQListViewItem*) ), TQ_SLOT( slotChangeCurrent() ) );
   connect( newButton, TQ_SIGNAL( clicked() ), TQ_SLOT( slotNewAction() ) );
   connect( removeButton, TQ_SIGNAL( clicked() ), TQ_SLOT( slotRemoveAction() ) );
   connect( importButton, TQ_SIGNAL( clicked() ), TQ_SLOT( slotImport() ) );
   connect( exportButton, TQ_SIGNAL( clicked() ), TQ_SLOT( slotExport() ) );
   connect( copyButton, TQ_SIGNAL( clicked() ), TQ_SLOT( slotToClip() ) );
   connect( pasteButton, TQ_SIGNAL( clicked() ), TQ_SLOT( slotFromClip() ) );

   // forwards the changed signal of the properties
   connect ( actionProperties, TQ_SIGNAL( changed() ), TQ_SIGNAL( changed() ) );

   actionTree->setFirstActionCurrent();
   actionTree->setFocus();
}

UserActionPage::~UserActionPage()
{
}

bool UserActionPage::continueInSpiteOfChanges() {
   if ( ! actionProperties->isModified() )
      return true;

   int answer = KMessageBox::questionYesNoCancel( this,
   		i18n("The current action has been modified. Do you want to apply these changes?")
   	);
   if ( answer == KMessageBox::Cancel ) {
      disconnect(  actionTree, TQ_SIGNAL( currentChanged(TQListViewItem*) ), this, TQ_SLOT( slotChangeCurrent() ) );
      actionTree->setCurrentAction( actionProperties->action() );
      connect(  actionTree, TQ_SIGNAL( currentChanged(TQListViewItem*) ), TQ_SLOT( slotChangeCurrent() ) );
      return false;
   }
   if ( answer == KMessageBox::Yes ) {
      if ( ! actionProperties->validProperties() ) {
         disconnect(  actionTree, TQ_SIGNAL( currentChanged(TQListViewItem*) ), this, TQ_SLOT( slotChangeCurrent() ) );
         actionTree->setCurrentAction( actionProperties->action() );
         connect(  actionTree, TQ_SIGNAL( currentChanged(TQListViewItem*) ), TQ_SLOT( slotChangeCurrent() ) );
         return false;
      }
      slotUpdateAction();
   } // if Yes
   return true;
}

void UserActionPage::slotChangeCurrent() {
   if ( ! continueInSpiteOfChanges() )
      return;

   KrAction* action = actionTree->currentAction();
   if ( action ) {
      actionProperties->setEnabled( true );
      // the discinct name is used as ID it is not allowd to change it afterwards because it is may referenced anywhere else
      actionProperties->leDistinctName->setEnabled( false );
      actionProperties->updateGUI( action );
   }
   else {
      // If the current item in the tree is no action (i.e. a cathegory), disable the properties
      actionProperties->clear();
      actionProperties->setEnabled( false );
   }
   emit applied(); // to disable the apply-button
}


void UserActionPage::slotUpdateAction() {
  // check that we have a command line, title and a name
   if ( ! actionProperties->validProperties() )
     return;

  if ( actionProperties->leDistinctName->isEnabled() ) {
      // := new entry
      KrAction* action = new KrAction( krApp->actionCollection(), actionProperties->leDistinctName->text().latin1() );
      krUserAction->addKrAction( action );
      actionProperties->updateAction( action );
      UserActionListViewItem* item = actionTree->insertAction( action );
      actionTree->setCurrentItem( item );
      krApp->userMenu->update();
    }
    else { // := edit an existing
       actionProperties->updateAction();
       actionTree->update( actionProperties->action() ); // update the listviewitem as well...
    }
   apply();
}


void UserActionPage::slotNewAction() {
   if ( continueInSpiteOfChanges() ) {
      actionTree->clearSelection();  // else the user may think that he is overwriting the selected action
      actionProperties->clear();
      actionProperties->setEnabled( true ); // it may be disabled because the tree has the focus on a category
      actionProperties->leDistinctName->setEnabled( true );
      actionProperties->leDistinctName->setFocus();
   }
}

void UserActionPage::slotRemoveAction() {
   if ( ! dynamic_cast<UserActionListViewItem*>( actionTree->currentItem() ) )
      return;

   int messageDelete = KMessageBox::warningContinueCancel ( this,	//parent
		i18n("Are you sure that you want to remove all selected actions?"),	//text
		i18n("Remove selected actions?"), 	//caption
		i18n("Remove"),	//Label for the continue-button
		"Confirm Remove UserAction",	//dontAskAgainName (for the config-file)
		KMessageBox::Dangerous) ;

   if ( messageDelete != KMessageBox::Continue )
      return;

   actionTree->removeSelectedActions();

   apply();
}

void UserActionPage::slotImport() {
   TQString filename = KFileDialog::getOpenFileName(TQString(), i18n(FILE_FILTER), this);
   if ( filename.isEmpty() )
      return;

   UserAction::KrActionList newActions;
   krUserAction->readFromFile( filename, UserAction::renameDoublicated, &newActions );
   for ( KrAction* action = newActions.first(); action; action = newActions.next() )
      actionTree->insertAction( action );

   if ( newActions.count() > 0 ) {
      apply();
   }
}

void UserActionPage::slotExport() {
   if ( ! dynamic_cast<UserActionListViewItem*>( actionTree->currentItem() ) )
      return;

   TQString filename = KFileDialog::getSaveFileName(TQString(), i18n(FILE_FILTER), this);
   if ( filename.isEmpty() )
      return;

   TQDomDocument doc = TQDomDocument( ACTION_DOCTYPE );
   TQFile file( filename );
   int answer = 0;
   if( file.open( IO_ReadOnly ) ) { // getting here, means the file already exists an can be read
      if( doc.setContent( &file ) ) // getting here means the file exists and already contains an UserAction-XML-tree
         answer = KMessageBox::warningYesNoCancel( this,	//parent
         		i18n("This file already contains some useractions.\nDo you want to overwrite it or should it be merged with the selected actions?"),	//text
         		i18n("Overwrite or merge?"),	//caption
         		i18n("Overwrite"),	//label for Yes-Button
         		i18n("Merge")	//label for No-Button
         	);
      file.close();
   }
   if ( answer == 0 && file.exists() )
      answer = KMessageBox::warningContinueCancel( this,	//parent
      		i18n("This file already exists. Do you want to overwrite it?"),	//text
      		i18n("Overwrite existing file?"),	//caption
      		i18n("Overwrite")	//label for Continue-Button
      	);

   if ( answer == KMessageBox::Cancel )
      return;

   if ( answer == KMessageBox::No ) // that means the merge-button
      doc = actionTree->dumpSelectedActions( &doc ); // merge
   else // Yes or Continue means overwrite
      doc = actionTree->dumpSelectedActions();

   bool success = UserAction::writeToFile( doc, filename );
   if ( ! success )
      KMessageBox::error( this,
      		i18n("Can't open %1 for writing!\nNothing exported.").arg(filename),
      		i18n("Export failed!")
      );
}

void UserActionPage::slotToClip() {
   if ( ! dynamic_cast<UserActionListViewItem*>( actionTree->currentItem() ) )
      return;

   TQDomDocument doc = actionTree->dumpSelectedActions();
   TDEApplication::clipboard()->setText( doc.toString() );
}

void UserActionPage::slotFromClip() {
   TQDomDocument doc( ACTION_DOCTYPE );
   if ( doc.setContent( TDEApplication::clipboard()->text() ) ) {
      TQDomElement root = doc.documentElement();
      UserAction::KrActionList newActions;
      krUserAction->readFromElement( root, UserAction::renameDoublicated, &newActions );
      for ( KrAction* action = newActions.first(); action; action = newActions.next() )
         actionTree->insertAction( action );
      if ( newActions.count() > 0 ) {
         apply();
      }
   } // if ( doc.setContent )
}

bool UserActionPage::readyToQuit() {
   // Check if the current UserAction has changed
   if ( ! continueInSpiteOfChanges() )
      return false;

   krUserAction->writeActionFile();
   return true;
}

void UserActionPage::apply() {
   krUserAction->writeActionFile();
   emit applied();
}

void UserActionPage::applyChanges() {
   slotUpdateAction();
}


#include "useractionpage.moc"
