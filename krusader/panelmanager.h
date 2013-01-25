#ifndef _PANEL_MANAGER_H
#define _PANEL_MANAGER_H

#include <tqwidget.h>
#include <tqlayout.h>
#include "paneltabbar.h"

class TDEConfig;
class ListPanel;
class TQWidgetStack;
class TQToolButton;

/**
 * Implements tabbed-browsing by managing a list of tabs and corrosponding panels.
 */
class PanelManager: public TQWidget {
    Q_OBJECT
  

  public:
    /**
     * PanelManager is created where once panels were created. It accepts three references to pointers
     * (self, other, active), which enables it to manage pointers held by the panels transparently.
     * It also receives a bool (left) which is true if the manager is the left one, or false otherwise.
     */
    PanelManager( TQWidget *parent, bool left );
    /**
     * Called once by KrusaderView to create the first panel. Subsequent called are done internally
     * Note: only creates the panel, but doesn't start the VFS inside it. Use startPanel() for that.
     */
    ListPanel* createPanel( TQString type, bool setCurrent = true );
    /**
     * Called once by KrusaderView to start the first panel. Subsequent called are done internally
     * Only starts the VFS inside the panel, you must first use createPanel() !
     */
    void startPanel(ListPanel *panel, const KURL& path);
    /**
     * Swaps the left / right directions of the panel
     */
    void swapPanels();
    
    void saveSettings(TDEConfig *config, const TQString& key, bool localOnly = true );
    void loadSettings(TDEConfig *config, const TQString& key);
    int  activeTab();
    void setActiveTab( int );
    void setCurrentTab( int );
    void refreshAllTabs( bool invalidate = false );

  public slots:
    /**
     * Called externally to start a new tab. Example of usage would be the "open in a new tab"
     * action, from the context-menu.
     */
    void slotNewTab(const KURL& url, bool setCurrent = true, TQString type = TQString(), int props = 0 );
    void slotNewTab();
    void slotNextTab();
    void slotPreviousTab();	 
    void slotCloseTab();
    void slotCloseTab( int index );
    void slotRecreatePanels();

  protected slots:
    void slotChangePanel(ListPanel *p);
    void slotRefreshActions();

  private:
    void deletePanel( ListPanel *p );
  
    TQGridLayout *_layout;
    TQHBoxLayout *_barLayout;
    bool _left;
    PanelTabBar *_tabbar;
    TQWidgetStack *_stack;
    TQToolButton *_newTab, *_closeTab;
    ListPanel **_selfPtr, **_otherPtr;
};


#endif // _PANEL_MANAGER_H
