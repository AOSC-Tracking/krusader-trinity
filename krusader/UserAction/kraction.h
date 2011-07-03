//
// C++ Interface: kraction
//
// Description: 
//
//
// Author: Krusader Krew <http://www.krusader.org>, (C) 2004, 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef KRACTION_H
#define KRACTION_H

#include <kaction.h>
#include <kprocess.h>
#include <kdialogbase.h>
#include "kractionbase.h"

class UserActionProperties;
class TQTextEdit;
class KActionCollection;
class TQDomElement;
class TQDomDocument;

/**
 * This subclass of KAction extends it with an individual executor and a struct UserActionProperties. It is used to integrate useractions into KDE's KAction-System
 * @author Jonas Bähr (http://www.jonas-baehr.de)
 */
class KrAction: public KAction, public KrActionBase {
   Q_OBJECT
  TQ_OBJECT
   public:
      KrAction( KActionCollection *tqparent, const char* name );
      ~KrAction();

      /**
       * This chekcs if the KrAction is for a specific file / location available
       * @param currentURL Check for this file
       * @return true if the KrAction if available
       */
      bool isAvailable( const KURL& currentURL );

      bool xmlRead( const TQDomElement& element );
      TQDomElement xmlDump( TQDomDocument& doc ) const;

      void setName( const char* ) { /* empty reimplementation to prevent a name-change */ };

      TQString category() const { return _category; };
      void setCategory( const TQString& category ) { _category = category; };

      TQString command() const { return _command; };
      void setCommand( const TQString& command ) { _command = command; };

      TQString user() const { return _user; };
      void setUser( const TQString& user ) { _user = user; };

      TQString startpath() const { return _startpath; };
      void setStartpath( const TQString& startpath ) { _startpath = startpath; };

      ExecType execType() const { return _execType; };
      void setExecType( ExecType execType ) { _execType = execType; };

      bool acceptURLs() const { return _acceptURLs; };
      void setAcceptURLs(const bool& acceptURLs) { _acceptURLs = acceptURLs; };

      bool confirmExecution() const { return _confirmExecution; };
      void setConfirmExecution(const bool& confirmExecution) { _confirmExecution = confirmExecution; };

      TQStringList showonlyProtocol() const { return _showonlyProtocol; };
      void setShowonlyProtocol( const TQStringList& showonlyProtocol ) { _showonlyProtocol = showonlyProtocol; };

      TQStringList showonlyPath() const { return _showonlyPath; };
      void setShowonlyPath( const TQStringList& showonlyPath ) { _showonlyPath = showonlyPath; };

      TQStringList showonlyMime() const { return _showonlyMime; };
      void setShowonlyMime( const TQStringList& showonlyMime ) { _showonlyMime = showonlyMime; };

      TQStringList showonlyFile() const { return _showonlyFile; };
      void setShowonlyFile( const TQStringList& showonlyFile ) { _showonlyFile = showonlyFile; };

      bool doSubstitution() const {
        return true;
      }

      TQString text() const {
        return KAction::text();
      }

   public slots:
    void exec() {
      KrActionBase::exec();
    }


   private:
      void readCommand( const TQDomElement& element );
      TQDomElement dumpCommand( TQDomDocument& doc ) const;

      void readAvailability( const TQDomElement& element );
      TQDomElement dumpAvailability( TQDomDocument& doc ) const;

      TQString _category;
      TQString _command;
      TQString _user;
      TQString _startpath;
      ExecType _execType;
      bool _acceptURLs;
      bool _confirmExecution;
      TQStringList _showonlyProtocol;
      TQStringList _showonlyPath;
      TQStringList _showonlyMime;
      TQStringList _showonlyFile;

};

class TQFont;
/**
 * This displays the output of a process
 * @author Shie Erlich, Jonas Bähr
 */
class KrActionProcDlg: public KDialogBase {
      Q_OBJECT
  TQ_OBJECT
   public:
      KrActionProcDlg( TQString caption, bool enableStderr = false, TQWidget *tqparent = 0 );

   protected slots:
      void addStderr( KProcess *proc, char *buffer, int buflen );
      void addStdout( KProcess *proc, char *buffer, int buflen );
      void toggleFixedFont( bool state );
      void slotUser1(); ///< This is used to save the buffer to disc

   private:
      TQTextEdit *_stdout, *_stderr, *_currentTextEdit;
      TQFont normalFont, fixedFont;
   private slots:
      void currentTextEditChanged();
};

/**
 * This executes a command of a UserAction
 * @author Shie Erlich, Jonas Bähr
 * @todo jonas: call a list of commands separately (I began it but it doesn't work)
 */
class KrActionProc: public TQObject {
      Q_OBJECT
  TQ_OBJECT
   public:

      KrActionProc( KrActionBase* action );
      virtual ~KrActionProc();
      void start( TQString cmdLine );
      void start( TQStringList cmdLineList );

   protected slots:
      void kill() { _proc->kill( SIGINT ); }
      void processExited( KProcess *proc );

   private:
      KrActionBase* _action;
      KProcess *_proc;
      TQString _stdout;
      TQString _stderr;
      KrActionProcDlg *_output;
};


#endif //KRACTION_H
