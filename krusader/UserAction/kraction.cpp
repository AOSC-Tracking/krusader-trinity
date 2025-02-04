//
// C++ Implementation: kraction
//
// Description: 
//
//
// Author: Shie Erlich and Rafi Yanai <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <kdialogbase.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <kinputdialog.h>
#include <tqtextedit.h>
#include <tqvbox.h>
#include <tqlayout.h>
#include <tqsplitter.h>
#include <tqpushbutton.h>
#include <tqcheckbox.h>
#include <tqfile.h>
#include <tqlabel.h>
#include <tdeaction.h>
#include <kurl.h>
#include <tdemessagebox.h>
#include <tdefiledialog.h>
#include "kraction.h"
#include "expander.h"
#include "useraction.h"
#include "../krusader.h"
#include "../krusaderview.h"
#include "../defaults.h"

//for the availabilitycheck:
#include <kmimetype.h>
#include <tqregexp.h>


//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////  KrActionProcDlg  /////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
#include <tqlayout.h>
KrActionProcDlg::KrActionProcDlg( TQString caption, bool enableStderr, TQWidget *parent ) :
KDialogBase( parent, 0, false, caption, KDialogBase::User1 | KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Cancel ),
_stdout(0), _stderr(0), _currentTextEdit(0) {

   setButtonOK( i18n( "Close" ) );
   enableButtonOK( false ); // disable the close button, until the process finishes

   setButtonCancel( KGuiItem(i18n("Kill"), i18n( "Kill the running process" )) );

   setButtonText(KDialogBase::User1, i18n("Save as") );

   TQVBox *page = makeVBoxMainWidget();
   // do we need to separate stderr and stdout?
   if ( enableStderr ) {
      TQSplitter *splitt = new TQSplitter( TQt::Vertical, page );
      // create stdout
      TQVBox *stdoutBox = new TQVBox( splitt, "stdout VBox" );
      stdoutBox->setSpacing( 6 );
      new TQLabel( i18n( "Standard Output (stdout)" ), stdoutBox );
      _stdout = new TQTextEdit( stdoutBox );
      _stdout->setReadOnly( true );
      _stdout->setMinimumWidth( fontMetrics().maxWidth() * 40 );
      // create stderr
      TQVBox *stderrBox = new TQVBox( splitt, "stderr VBox" );
      stderrBox->setSpacing( 6 );
      new TQLabel( i18n( "Standard Error (stderr)" ), stderrBox );
      _stderr = new TQTextEdit( stderrBox );
      _stderr->setReadOnly( true );
      _stderr->setMinimumWidth( fontMetrics().maxWidth() * 40 );
   } else {
      // create stdout
      new TQLabel( i18n( "Output" ), page );
      _stdout = new TQTextEdit( page );
      _stdout->setReadOnly( true );
      _stdout->setMinimumWidth( fontMetrics().maxWidth() * 40 );
   }

   _currentTextEdit = _stdout;
   connect( _stdout, TQ_SIGNAL( clicked(int, int) ), TQ_SLOT( currentTextEditChanged() ) );
   if (_stderr)
      connect( _stderr, TQ_SIGNAL( clicked(int, int) ), TQ_SLOT( currentTextEditChanged() ) );

   krConfig->setGroup( "UserActions" );
   normalFont = krConfig->readFontEntry( "Normal Font", _UserActions_NormalFont );
   fixedFont = krConfig->readFontEntry( "Fixed Font", _UserActions_FixedFont );
   bool startupState = krConfig->readBoolEntry( "Use Fixed Font", _UserActions_UseFixedFont );
   toggleFixedFont( startupState );

   // HACK This fetches the layout of the buttonbox from KDialogBase, although it is not accessable with KDialogBase's API
   // None the less it's quite save to use since this implementation hasn't changed since KDE-3.3 (I haven't looked at earlier
   // versions since we don't support them) and now all work is done in KDE-4.
   TQWidget* buttonBox = static_cast<TQWidget*>( actionButton(KDialogBase::Ok)->parent() );
   TQBoxLayout* buttonBoxLayout = static_cast<TQBoxLayout*>( buttonBox->layout() );
   TQCheckBox* useFixedFont = new TQCheckBox( i18n("Use font with fixed width"), buttonBox );
   buttonBoxLayout->insertWidget( 0, useFixedFont );
   useFixedFont->setChecked( startupState );
   connect( useFixedFont, TQ_SIGNAL( toggled(bool) ), TQ_SLOT( toggleFixedFont(bool) ) );
}

void KrActionProcDlg::addStderr( TDEProcess *, char *buffer, int buflen ) {
   if (_stderr)
      _stderr->append( TQString::fromLatin1( buffer, buflen ) );
   else {
      _stdout->setItalic(true);
      _stdout->append( TQString::fromLatin1( buffer, buflen ) );
      _stdout->setItalic(false);
   }
}

void KrActionProcDlg::addStdout( TDEProcess *, char *buffer, int buflen ) {
   _stdout->append( TQString::fromLatin1( buffer, buflen ) );
}

void KrActionProcDlg::toggleFixedFont( bool state ) {
   if ( state ) {
      _stdout->setFont( fixedFont );
      if ( _stderr )
         _stderr->setFont( fixedFont );
   }
   else {
      _stdout->setFont( normalFont );
      if ( _stderr )
         _stderr->setFont( normalFont );
   }
}

void KrActionProcDlg::slotUser1() {
   TQString filename = KFileDialog::getSaveFileName(TQString(), i18n("*.txt|Text files\n*|all files"), this);
   if ( filename.isEmpty() )
      return;
   TQFile file( filename );
   int answer = KMessageBox::Yes;
   if ( file.exists() )
      answer = KMessageBox::warningYesNoCancel( this,	//parent
      		i18n("This file already exists.\nDo you want to overwrite it or append the output?"),	//text
      		i18n("Overwrite or append?"),	//caption
      		i18n("Overwrite"),	//label for Yes-Button
      		i18n("Append")	//label for No-Button
      	);
   if ( answer == KMessageBox::Cancel )
      return;
   bool open;
   if ( answer == KMessageBox::No ) // this means to append
      open = file.open( IO_WriteOnly | IO_Append );
   else
      open = file.open( IO_WriteOnly );

   if ( ! open ) {
      KMessageBox::error( this,
      		i18n("Can't open %1 for writing!\nNothing exported.").arg(filename),
      		i18n("Export failed!")
      	);
      return;
   }

   TQTextStream stream( &file );
   stream << _currentTextEdit->text();
   file.close();
}

void KrActionProcDlg::currentTextEditChanged() {
   if ( _stderr && _stderr->hasFocus() )
      _currentTextEdit = _stderr;
   else
      _currentTextEdit = _stdout;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////  KrActionProc  ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

KrActionProc::KrActionProc( KrActionBase* action ) : TQObject(), _action( action ), _proc( new TDEProcess(this) ), _output( 0 ) {
   _proc->setUseShell( true );

   connect( _proc, TQ_SIGNAL( processExited( TDEProcess* ) ),
            this, TQ_SLOT( processExited( TDEProcess* ) ) ) ;
}

KrActionProc::~KrActionProc() {
   delete _proc;
}

void KrActionProc::start( TQString cmdLine ) {
   TQStringList list = cmdLine;
   start( list );
}

void KrActionProc::start( TQStringList cmdLineList ) {
   _proc->clearArguments();
   TQString cmd;

   if ( ! _action->startpath().isEmpty() )
      _proc->setWorkingDirectory( _action->startpath() );

   if ( _action->execType() == KrAction::Terminal && cmdLineList.count() > 1)
      KMessageBox::sorry( 0, i18n("Support for more than one command doesn't work in a terminal. Only the first is executed in the terminal.") );

   if ( _action->execType() == KrAction::RunInTE
         && ( MAIN_VIEW->konsole_part == NULL || MAIN_VIEW->konsole_part->widget() == NULL ) ) {
      KMessageBox::sorry( 0, i18n("Embedded terminal emulator does not work, using output collection instead.") );
   }

   if( _action->execType() == KrAction::Normal || _action->execType() == KrAction::Terminal
       || ( _action->execType() == KrAction::RunInTE && MAIN_VIEW->konsole_part && MAIN_VIEW->konsole_part->widget() )
   ) { // not collect output
      //TODO option to run them in paralell (not available for: collect output)
      for ( TQStringList::Iterator it = cmdLineList.begin(); it != cmdLineList.end(); ++it) {
         if ( ! cmd.isEmpty() )
            cmd += " ; ";	//TODO make this separator configurable (users may want && or || for spec. actions)
         cmd += *it;
      }
      //run in TE
      if ( _action->execType() == KrAction::RunInTE ) {
         //send the commandline contents to the terminal emulator
         TQKeyEvent keyEvent( TQEvent::KeyPress, 0, -1, 0,  cmd+"\n");
         TQApplication::sendEvent( MAIN_VIEW->konsole_part->widget(), &keyEvent );     
      } else { // will start a new process
         // run in terminal
         if ( _action->execType() == KrAction::Terminal ) {
           krConfig->setGroup( "UserActions" );
           TQString term = krConfig->readEntry( "Terminal", _UserActions_Terminal );

            if ( _action->user().isEmpty() )
               ( *_proc ) << term << cmd;
            else
//                ( *_proc )  << "tdesu" << "-u" << *_properties->user() << "-c" << TDEProcess::quote("konsole --noclose -e " + TDEProcess::quote(cmd) );
               ( *_proc )  << "tdesu" << "-u" << _action->user() << "-c" << TDEProcess::quote( term + " " + cmd );
         } else { // no terminal, no output collection, start&forget
            if ( _action->user().isEmpty() )
               ( *_proc ) << cmd;
            else
               ( *_proc ) << "tdesu" << "-u" << _action->user() << "-c" << TDEProcess::quote(cmd);
         }
         _proc->start( TDEProcess::NotifyOnExit, ( TDEProcess::Communication ) ( TDEProcess::Stdout | TDEProcess::Stderr ) );
      }
   }
   else { // collect output
      bool separateStderr = false;
      if ( _action->execType() == KrAction::CollectOutputSeparateStderr )
         separateStderr = true;
      _output = new KrActionProcDlg( _action->text(), separateStderr );
      // connect the output to the dialog
      connect( _proc, TQ_SIGNAL( receivedStderr( TDEProcess*, char*, int ) ), _output, TQ_SLOT( addStderr( TDEProcess*, char *, int ) ) );
      connect( _proc, TQ_SIGNAL( receivedStdout( TDEProcess*, char*, int ) ), _output, TQ_SLOT( addStdout( TDEProcess*, char *, int ) ) );
      connect( _output, TQ_SIGNAL( cancelClicked() ), this, TQ_SLOT( kill() ) );
      _output->show();
      for ( TQStringList::Iterator it = cmdLineList.begin(); it != cmdLineList.end(); ++it) {
         if ( ! cmd.isEmpty() )
            cmd += " ; ";	//TODO make this separator configurable (users may want && or ||)
         //TODO: read header fom config or action-properties and place it on top of each command
         if ( cmdLineList.count() > 1 )
            cmd += "echo --------------------------------------- ; ";
         cmd += *it;
      }
      if ( _action->user().isEmpty() )
         ( *_proc ) << cmd;
      else
         // "-t" is nessesary that tdesu displays the terminal-output of the command
         ( *_proc ) << "tdesu" << "-t" << "-u" << _action->user() << "-c" << TDEProcess::quote(cmd);
      _proc->start( TDEProcess::NotifyOnExit, ( TDEProcess::Communication ) ( TDEProcess::Stdout | TDEProcess::Stderr ) );
   }

}

void KrActionProc::processExited( TDEProcess * ) {
   // enable the 'close' button on the dialog (if active), disable 'kill' button
   if ( _output ) {
      _output->enableButtonOK( true );
      _output->enableButtonCancel( false);
   }
   delete this; // banzai!!
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////  KrAction  ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

KrAction::KrAction( TDEActionCollection *parent, const char* name ) : TDEAction( parent, name ) {
   connect(this, TQ_SIGNAL(activated()), this, TQ_SLOT(exec()) );
}

KrAction::~KrAction() {
   unplugAll();
   krUserAction->removeKrAction( this ); // Importent! Else Krusader will crash when writing the actions to file
}

bool KrAction::isAvailable( const KURL& currentURL ) {
   bool available = true; //show per default (FIXME: make the default an attribute of <availability>)
   
   //check protocol
   if ( ! _showonlyProtocol.empty() ) {
      available = false;
      for ( TQStringList::Iterator it = _showonlyProtocol.begin(); it != _showonlyProtocol.end(); ++it ) {
         //kdDebug() << "KrAction::isAvailable currendProtocol: " << currentURL.protocol() << " =?= " << *it << endl;
         if ( currentURL.protocol() == *it ) {  // FIXME remove trailing slashes at the xml-parsing (faster because done only once)
            available = true;
            break;
         }
      }
   } //check protocol: done
      
   //check the Path-list:
   if ( ! _showonlyPath.empty() ) {
      available = false;
      for ( TQStringList::Iterator it = _showonlyPath.begin(); it != _showonlyPath.end(); ++it ) {
         if ( (*it).right(1) == "*" ){
             if ( currentURL.path().find( (*it).left( (*it).length() - 1 ) ) == 0 ) {
               available = true;
               break;
            }
         } else
         if ( currentURL.directory() == *it ) {  // FIXME remove trailing slashes at the xml-parsing (faster because done only once)
            available = true;
            break;
         }
      }
   } //check the Path-list: done
   
   //check mime-type
   if ( ! _showonlyMime.empty() ) {
      available = false;
      KMimeType::Ptr mime = KMimeType::findByURL( currentURL );
      for ( TQStringList::Iterator it = _showonlyMime.begin(); it != _showonlyMime.end(); ++it ) {
         if ( (*it).contains("/") ) {
            if ( mime->is( *it ) ) {  // don't use ==; use 'is()' instead, which is aware of inheritence (ie: text/x-makefile is also text/plain)
               available = true;
               break;
            }
         }
         else {
            if ( mime->name().find( *it ) == 0 ) {  // 0 is the beginning, -1 is not found
               available = true;
               break;
            }
         }
      } //for
   } //check the mime-type: done
   
   //check filename
   if ( ! _showonlyFile.empty() ) {
      available = false;
      for ( TQStringList::Iterator it = _showonlyFile.begin(); it != _showonlyFile.end(); ++it ) {
         TQRegExp regex = TQRegExp( *it, false, true ); // case-sensitive = false; wildcards = true
         if ( regex.exactMatch( currentURL.fileName() ) ) {
            available = true;
            break;
         }
      }
   } //check the filename: done
   
   return available;
}


bool KrAction::xmlRead( const TQDomElement& element ) {
/*
   This has to be done elsewhere!!

   if ( element.tagName() != "action" )
      return false;

   Also the name has to be checked before the action is created!

   setName( element.attribute( "name" ).latin1() );
*/

   for ( TQDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling() ) {
      TQDomElement e = node.toElement();
      if ( e.isNull() )
         continue; // this should skip nodes which are not elements ( i.e. comments, <!-- -->, or text nodes)

      if ( e.tagName() == "title" )
         setText( e.text() );
      else
      if ( e.tagName() == "tooltip" )
         setToolTip( e.text() );
      else
      if ( e.tagName() == "icon" )
         setIcon( e.text() );
      else
      if ( e.tagName() == "category" )
         setCategory( e.text() );
      else
      if ( e.tagName() == "description" )
         setWhatsThis( e.text() );
      else
      if (e.tagName() == "command")
         readCommand( e );
      else
      if ( e.tagName() == "startpath" )
         setStartpath( e.text() );
      else
      if (e.tagName() == "availability")
         readAvailability( e );
      else
      if ( e.tagName() == "defaultshortcut" )
         setShortcut(  TDEShortcut( e.text() ) );
      else

      // unknown but not empty
      if ( ! e.tagName().isEmpty() )
         krOut << "KrAction::xmlRead() - unrecognized tag found: <action name=\"" << name() << "\"><" << e.tagName() << ">" << endl;

   } // for ( TQDomNode node = action->firstChild(); !node.isNull(); node = node.nextSibling() )

   return true;
} //KrAction::xmlRead

TQDomElement KrAction::xmlDump( TQDomDocument& doc ) const {
   TQDomElement actionElement = doc.createElement("action");
   actionElement.setAttribute( "name", name() );

#define TEXT_ELEMENT( TAGNAME, TEXT ) \
   { \
   TQDomElement e = doc.createElement( TAGNAME ); \
   e.appendChild( doc.createTextNode( TEXT ) ); \
   actionElement.appendChild( e ); \
   }

   TEXT_ELEMENT( "title", text() )

   if ( ! toolTip().isEmpty() )
      TEXT_ELEMENT( "tooltip", toolTip() )

   if ( ! icon().isEmpty() )
      TEXT_ELEMENT( "icon", icon() )

   if ( ! category().isEmpty() )
      TEXT_ELEMENT( "category", category() )

   if ( ! whatsThis().isEmpty() )
      TEXT_ELEMENT( "description", whatsThis() )

   actionElement.appendChild( dumpCommand( doc ) );

   if ( ! startpath().isEmpty() )
      TEXT_ELEMENT( "startpath", startpath() )

   TQDomElement availabilityElement = dumpAvailability( doc );
   if ( availabilityElement.hasChildNodes() )
      actionElement.appendChild( availabilityElement );

   if ( ! shortcut().isNull() )
      TEXT_ELEMENT( "defaultshortcut", shortcut().toStringInternal() )  //.toString() would return a localised string which can't be read again

   return actionElement;
} //KrAction::xmlDump

void KrAction::readCommand( const TQDomElement& element ) {
   TQString attr;

   attr = element.attribute( "executionmode", "normal" ); // default: "normal"
   if ( attr == "normal")
      setExecType( Normal );
   else
   if ( attr == "terminal" )
      setExecType( Terminal );
   else if ( attr == "collect_output")
      setExecType( CollectOutput );
   else if ( attr == "collect_output_separate_stderr")
      setExecType( CollectOutputSeparateStderr );
   else
      krOut << "KrAction::readCommand() - unrecognized attribute value found: <action name=\"" << name() << "\"><command executionmode=\"" << attr << "\""<< endl;

   attr = element.attribute( "accept", "local" ); // default: "local"
   if ( attr == "local" )
      setAcceptURLs( false );
   else if ( attr == "url")
      setAcceptURLs( true );
   else
        krOut << "KrAction::readCommand() - unrecognized attribute value found: <action name=\"" << name() << "\"><command accept=\"" << attr << "\""<< endl;

   attr = element.attribute( "confirmexecution", "false" ); // default: "false"
   if ( attr == "true" )
      setConfirmExecution( true );
   else
      setConfirmExecution( false );

   setUser( element.attribute( "run_as" ) );

   setCommand( element.text() );

} //KrAction::readCommand

TQDomElement KrAction::dumpCommand( TQDomDocument& doc ) const {
   TQDomElement commandElement = doc.createElement("command");

   switch ( execType() ) {
   case Terminal:
      commandElement.setAttribute( "executionmode", "terminal" );
      break;
   case CollectOutput:
      commandElement.setAttribute( "executionmode", "collect_output" );
      break;
   case CollectOutputSeparateStderr:
      commandElement.setAttribute( "executionmode", "collect_output_separate_stderr" );
      break;
   default:
      // don't write the default to file
      break;
   }

   if ( acceptURLs() )
      commandElement.setAttribute( "accept", "url" );

   if ( confirmExecution() )
      commandElement.setAttribute( "confirmexecution", "true" );

   if ( ! user().isEmpty() )
      commandElement.setAttribute( "run_as", user() );

   commandElement.appendChild( doc.createTextNode( command() ) );

   return commandElement;
} //KrAction::dumpCommand

void KrAction::readAvailability( const TQDomElement& element ) {
   for ( TQDomNode node = element.firstChild(); ! node.isNull(); node = node.nextSibling() ) {
      TQDomElement e = node.toElement();
      if ( e.isNull() )
         continue; // this should skip nodes which are not elements ( i.e. comments, <!-- -->, or text nodes)

      TQStringList* showlist = 0;

      if ( e.tagName() == "protocol" )
         showlist = &_showonlyProtocol;
      else
      if ( e.tagName() == "path" )
         showlist = &_showonlyPath;
      else
      if ( e.tagName() == "mimetype" )
         showlist = & _showonlyMime;
      else
      if ( e.tagName() == "filename" )
         showlist = & _showonlyFile;
      else {
         krOut << "KrAction::readAvailability() - unrecognized element found: <action name=\"" << name() << "\"><availability><" << e.tagName() << ">"<< endl;
         showlist = 0;
      }

      if ( showlist ) {
          for ( TQDomNode subnode = e.firstChild(); ! subnode.isNull(); subnode = subnode.nextSibling() ) {
            TQDomElement subelement = subnode.toElement();
            if ( subelement.tagName() == "show" )
              showlist->append( subelement.text() );
          } // for
        } // if ( showlist )

   } // for
} //KrAction::readAvailability

TQDomElement KrAction::dumpAvailability( TQDomDocument& doc ) const {
   TQDomElement availabilityElement = doc.createElement("command");

# define LIST_ELEMENT( TAGNAME, LIST ) \
   { \
   TQDomElement e = doc.createElement( TAGNAME ); \
   for ( TQStringList::const_iterator it = LIST.constBegin(); it != LIST.constEnd(); ++it ) { \
      TQDomElement show = doc.createElement( "show" ); \
      show.appendChild( doc.createTextNode( *it ) ); \
      e.appendChild( show ); \
      } \
   availabilityElement.appendChild( e ); \
   }

   if ( ! _showonlyProtocol.isEmpty() )
      LIST_ELEMENT( "protocol", _showonlyProtocol )

   if ( ! _showonlyPath.isEmpty() )
      LIST_ELEMENT( "path", _showonlyPath )

   if ( ! _showonlyMime.isEmpty() )
      LIST_ELEMENT( "mimetype", _showonlyMime )

   if ( ! _showonlyFile.isEmpty() )
      LIST_ELEMENT( "filename", _showonlyFile )

   return availabilityElement;
} //KrAction::dumpAvailability

#include "kraction.moc"
