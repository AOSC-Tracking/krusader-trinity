//
// C++ Interface: useractionlistview
//
// Description: 
//
//
// Author: Jonas Bähr, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef USERACTIONLISTVIEW_H
#define USERACTIONLISTVIEW_H

#include <tdelistview.h>

class KrAction;
class TQString;
class UserActionListViewItem;
class TQDomDocument;

/**
 * @author Jonas Bähr
 */
class UserActionListView : public TDEListView {
public:
   UserActionListView( TQWidget* parent = 0, const char* name = 0 );
   ~UserActionListView();
   virtual TQSize sizeHint() const;

   void update();
   void update( KrAction* action );
   UserActionListViewItem* insertAction( KrAction* action );

   KrAction* currentAction() const;
   void setCurrentAction( const KrAction* );

   TQDomDocument dumpSelectedActions( TQDomDocument* mergeDoc = 0 ) const;

   void removeSelectedActions();

   /**
    * makes the first action in the list current
    */
   void setFirstActionCurrent();

   /**
    * makes @e item current and ensures its visibility
    */
   virtual void setCurrentItem( TQListViewItem* item );

protected:
   TQListViewItem* findCategoryItem( const TQString& category );
   UserActionListViewItem* findActionItem( const KrAction* action );
};


/**
 * @author Jonas Bähr
 */
class UserActionListViewItem : public TDEListViewItem {
public:
   UserActionListViewItem( TQListView* view, KrAction* action );
   UserActionListViewItem( TQListViewItem* item, KrAction* action );
   ~UserActionListViewItem();

   void setAction( KrAction* action );
   KrAction* action() const;
   void update();

   /**
    * This reimplements TQt's compare-function in order to have categories on the top of the list
    */
   int compare ( TQListViewItem * i, int col, bool ascending ) const;

private:
   KrAction* _action;
};


#endif //USERACTIONLISTVIEW_H
