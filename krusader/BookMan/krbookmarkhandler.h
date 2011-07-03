#ifndef KRBOOKMARK_HANDLER_H
#define KRBOOKMARK_HANDLER_H

#include "krbookmark.h"
#include <tqobject.h>
#include <tqguardedptr.h>
#include <kpopupmenu.h>
#include <kurl.h>
#include <tqptrdict.h>
#include <tqvaluelist.h>
#include <tqdom.h>
#include <tqmap.h>

class KActionCollection;
class KBookmarkManager;

class KrBookmarkHandler: public TQObject {
	Q_OBJECT
  TQ_OBJECT
	friend class KrAddBookmarkDlg;
	enum Actions { BookmarkCurrent=0, ManageBookmarks };
public:
	KrBookmarkHandler();
	~KrBookmarkHandler();
	void populate(KPopupMenu *menu);
	void addBookmark(KrBookmark *bm, KrBookmark *tqparent = 0);
	void bookmarkCurrent(KURL url);

protected:
	void deleteBookmark(KrBookmark *bm);
	void importFromFile();
	bool importFromFileBookmark(TQDomElement &e, KrBookmark *tqparent, TQString path, TQString *errorMsg);
	bool importFromFileFolder(TQDomNode &first, KrBookmark *tqparent, TQString path, TQString *errorMsg);
	void exportToFile();
	void exportToFileFolder(TQDomDocument &doc, TQDomElement &tqparent, KrBookmark *folder);
	void exportToFileBookmark(TQDomDocument &doc, TQDomElement &where, KrBookmark *bm);
	void clearBookmarks(KrBookmark *root);
	void buildMenu(KrBookmark *tqparent, KPopupMenu *menu);

	bool eventFilter( TQObject *obj, TQEvent *ev );
	
	void rightClicked( TQPopupMenu *menu, int id, KrBookmark *bm );
	void rightClickOnSpecialBookmark();
	
	void removeReferences( KrBookmark *root, KrBookmark *bmToRemove );
	
protected slots:
	void menuOperation(int id);
	void bookmarksChanged(const TQString&, const TQString&);
	void slotActivated(const KURL& url);

private:
	KActionCollection *_collection, *_privateCollection;
	KrBookmark *_root;
	// the whole KBookmarkManager is an ugly hack. use it until we have our own
	KBookmarkManager *manager;
	bool _middleClick; // if true, the user clicked the middle button to open the bookmark
	
	TQGuardedPtr<KPopupMenu>            _mainBookmarkPopup; // main bookmark popup menu
	TQValueList<int>                    _specialBookmarkIDs; // the ID list of the special bookmarks
	TQPtrDict<TQMap<int,KrBookmark*> >   _bookmarkIDTable;    // the IDs of the bookmarks
};

#endif // KRBOOKMARK_HANDLER_H
