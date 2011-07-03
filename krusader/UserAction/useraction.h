//
// C++ Interface: useraction
//
// Description: This manages all useractions
//
//
// Author: Jonas Bähr (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef USERACTION_H
#define USERACTION_H

#include <tqptrlist.h>

class TQDomDocument;
class TQDomElement;
class TQString;
class TQStringList;
class KrAction;
class KURL;
class KPopupMenu;

/**
 * Useractions are Krusaders backend for user-defined actions on current/selected files in its panels
 * and for krusader's internal actions which need some parameter. @n
 * There are several komponents:
 * - The UserAction class as a Manager
 * - The interface to KDE's action-system (the KrAction)
 * - The Expander, which parses the commandline for placeholders and calls the internal actions
 * - A widget to manipulate the UserAction's Properties via GUI (ActionProperty)
 * .
 * The Useractions are stored in XML-files. Currently there are two main files. The first is a global example-file
 * which is read only (read after the other actionfiles, doublicates are ignored) and a local file where the actions are saved.
 * This class reads only the container and passes each action-tag to the new KrAction, which reads it's data itself.
 *
 * @author Jonas Bähr (http://www.jonas-baehr.de)
 */

class UserAction {
public:

  typedef TQPtrList<KrAction> KrActionList;

  enum ReadMode { renameDoublicated, ignoreDoublicated };

  /**
   * The constructor reads all useractions, see readAllFiles()
   */
  UserAction();
  ~UserAction();
  
  /**
   * adds an action to the collection.
   */
  void addKrAction( KrAction* action ) { _actions.append( action ); };
  
  /**
   * Use this to access the whole list of registerd KrActions.
   * currently only used to fill the usermenu with all available actions. This should change...
   * @return A reference to the internal KrActionList
   */
   const KrActionList &actionList() { return _actions; };

  /**
   * @return how many useractions exist
   */
  int count() const { return _actions.count(); };

  /**
   * removes a KrAction from the internal list but does not delete it.
   * @param action the KrAction which should be removed
   */
  void removeKrAction( KrAction* action ) { _actions.remove( action ); };
  
  /**
   * check for each KrAction if it is available for the currend location / file and disables it if not
   */
  void setAvailability();
  /**
   * same as above but check for a specitic file
   * @param currentURL Check for this file
   */
  void setAvailability(const KURL& currentURL);
  
  /**
   * Fills a KPopupMenu with all available UserActions in the list
   * @param  popupmenu to populate
   */
  void populateMenu(KPopupMenu* menu);

   TQStringList allCategories();
   TQStringList allNames();

   /**
    * reads all predefined useractionfiles. 
    */
   void readAllFiles();
   /**
    * writes all actions to the local actionfile
    */
   bool writeActionFile();
   /**
    * Reads UserActions from a xml-file.
    * @param list If provided, all new actions will also be added to this list
    */
   void readFromFile( const TQString& filename, ReadMode mode = renameDoublicated, KrActionList* list = 0 );
   /**
    * Reads UserActions from a XML-Element.
    * @param element a container with action-elements
    * @param list If provided, all new actions will also be added to this list
    */
   void readFromElement( const TQDomElement& element, ReadMode mode = renameDoublicated, KrActionList* list = 0 );

   /**
    * creates an empty TQDomDocument for the UserActions
    */
   static TQDomDocument createEmptyDoc();
   /**
    * Writes a TQDomDocument to an UTF-8 encodes text-file
    * @param doc the XML-Tree
    * @param filename the filename where to save
    * @return true on success, false otherwise
    * @warning any existing file will get overwritten!
    */
   static bool writeToFile( const TQDomDocument& doc, const TQString& filename );

private:
  KrActionList _actions;
};


#define ACTION_XML				"krusader/useractions.xml"
#define ACTION_XML_EXAMPLES	"krusader/useraction_examples.xml"

#define ACTION_DOCTYPE		"KrusaderUserActions"
// in well formed XML the root-element has to have the same name then the doctype:
#define ACTION_ROOT	ACTION_DOCTYPE
#define ACTION_PROCESSINSTR	"version=\"1.0\" encoding=\"UTF-8\" "



#endif // ifndef USERACTION_H
