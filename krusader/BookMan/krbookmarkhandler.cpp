#include "krbookmarkhandler.h"
#include "kraddbookmarkdlg.h"
#include "../krusader.h"
#include "../krslots.h"
#include "../Dialogs/popularurls.h"
#include "../VFS/vfs.h"
#include <kiconloader.h>
#include <tdemessagebox.h>
#include <tqptrlist.h>
#include <tdeactioncollection.h>
#include <tdelocale.h>
#include <kdebug.h>
#include <kbookmarkmanager.h>
#include <kstandarddirs.h>
#include <tqfile.h>
#include <tqcursor.h>

#define SPECIAL_BOOKMARKS	true

// ------------------------ for internal use
#define BOOKMARKS_FILE	"krusader/krbookmarks.xml"
#define CONNECT_BM(X)	{ disconnect(X, TQ_SIGNAL(activated(const KURL&)), 0, 0); connect(X, TQ_SIGNAL(activated(const KURL&)), this, TQ_SLOT(slotActivated(const KURL&))); }
											
KrBookmarkHandler::KrBookmarkHandler(): TQObject(0), _middleClick(false), _mainBookmarkPopup( 0 ), _specialBookmarkIDs(), _bookmarkIDTable() {
	// create our own action collection and make the shortcuts apply only to parent
	_privateCollection = new TDEActionCollection(krApp, "private collection");
	_collection = krApp->actionCollection();

	// create _root: father of all bookmarks. it is a dummy bookmark and never shown
	_root = new KrBookmark(i18n("Bookmarks"));
	
	_bookmarkIDTable.setAutoDelete( true );
	
	// load bookmarks 
	importFromFile();

	// hack
	manager = KBookmarkManager::managerForFile(locateLocal( "data", BOOKMARKS_FILE ), false);
	connect(manager, TQ_SIGNAL(changed(const TQString&, const TQString& )), this, TQ_SLOT(bookmarksChanged(const TQString&, const TQString& )));
}

KrBookmarkHandler::~KrBookmarkHandler() {
	delete manager;
	delete _privateCollection;
}

void KrBookmarkHandler::menuOperation(int id) {
	switch (id) {
		case BookmarkCurrent:
			bookmarkCurrent(ACTIVE_PANEL->virtualPath());
			break;
		case ManageBookmarks:
			manager->slotEditBookmarks();
			break;
	}
}

void KrBookmarkHandler::bookmarkCurrent(KURL url) {
	KrAddBookmarkDlg dlg(krApp, url);
	if (dlg.exec() == KDialog::Accepted) {
		KrBookmark *bm = new KrBookmark(dlg.name(), dlg.url(), _collection);
		addBookmark(bm, dlg.folder());
	}
}

void KrBookmarkHandler::addBookmark(KrBookmark *bm, KrBookmark *folder) {
	if (folder == 0)
		folder = _root;
		
	// add to the list (bottom)
	folder->children().append(bm);

	exportToFile();
}

void KrBookmarkHandler::deleteBookmark(KrBookmark *bm) {
	if( bm->isFolder() )
		clearBookmarks( bm ); // remove the child bookmarks
	removeReferences( _root, bm );
	bm->unplugAll();
	delete bm;
	
	exportToFile();
}

void KrBookmarkHandler::removeReferences( KrBookmark *root, KrBookmark *bmToRemove ) {
	int index = root->children().find( bmToRemove );
	if( index >= 0 )
		root->children().take( index );
	
	KrBookmark *bm = root->children().first();
	while (bm) {
		if (bm->isFolder())
			removeReferences(bm, bmToRemove);
		bm = root->children().next();
	}
}

void KrBookmarkHandler::exportToFileBookmark(TQDomDocument &doc, TQDomElement &where, KrBookmark *bm) {
	if( bm->isSeparator() ) {
		TQDomElement bookmark = doc.createElement("separator");
		where.appendChild(bookmark);
	}
	else {
		TQDomElement bookmark = doc.createElement("bookmark");
		// url
		bookmark.setAttribute("href", bm->url().prettyURL());
		// icon
		bookmark.setAttribute("icon", bm->icon());
		// title
		TQDomElement title = doc.createElement("title");	
		title.appendChild(doc.createTextNode(bm->text()));
		bookmark.appendChild(title);
		
		where.appendChild(bookmark);
	}
}

void KrBookmarkHandler::exportToFileFolder(TQDomDocument &doc, TQDomElement &parent, KrBookmark *folder) {
	for (KrBookmark *bm = folder->children().first(); bm; bm = folder->children().next()) {
		if (bm->isFolder()) {
			TQDomElement newFolder = doc.createElement("folder");
			newFolder.setAttribute("icon", bm->icon());
			parent.appendChild(newFolder);
			TQDomElement title = doc.createElement("title");
			title.appendChild(doc.createTextNode(bm->text()));
			newFolder.appendChild(title);
			exportToFileFolder(doc, newFolder, bm);
		} else {
			exportToFileBookmark(doc, parent, bm);
		}
	}
}

// export to file using the xbel standard
//
//  <xbel>
//    <bookmark href="http://developer.kde.org"><title>Developer Web Site</title></bookmark>
//    <folder folded="no">
//      <title>Title of this folder</title>
//      <bookmark icon="kde" href="http://www.kde.org"><title>KDE Web Site</title></bookmark>
//      <folder toolbar="yes">
//        <title>My own bookmarks</title>
//        <bookmark href="http://www.koffice.org"><title>KOffice Web Site</title></bookmark>
//        <separator/>
//        <bookmark href="http://www.kdevelop.org"><title>KDevelop Web Site</title></bookmark>
//      </folder>
//    </folder>
//  </xbel>
void KrBookmarkHandler::exportToFile() {
	TQDomDocument doc( "xbel" );
   TQDomElement root = doc.createElement( "xbel" );
   doc.appendChild( root );

	exportToFileFolder(doc, root, _root);
	if (!doc.firstChild().isProcessingInstruction()) {
		// adding: <?xml version="1.0" encoding="UTF-8" ?> if not already present 
		TQDomProcessingInstruction instr = doc.createProcessingInstruction( "xml", 
				"version=\"1.0\" encoding=\"UTF-8\" ");
		doc.insertBefore( instr, doc.firstChild() ); 
	}

	
	TQString filename = locateLocal( "data", BOOKMARKS_FILE );
	TQFile file(filename);
	if ( file.open( IO_WriteOnly ) ) {
		TQTextStream stream( &file );
		stream.setEncoding(stream.UnicodeUTF8);
		stream << doc.toString();
		file.close();
	} else {
		KMessageBox::error(krApp, i18n("Unable to write to %1").arg(filename), i18n("Error"));
	}
}

bool KrBookmarkHandler::importFromFileBookmark(TQDomElement &e, KrBookmark *parent, TQString path, TQString *errorMsg) {
	TQString url, name, icon;
	// verify tag
	if (e.tagName() != "bookmark") {
		*errorMsg = e.tagName() + i18n(" instead of ")+"bookmark";
		return false;
	}
	// verify href
	if (!e.hasAttribute("href")) {
		*errorMsg = i18n("missing tag ")+ "href";
		return false;
	} else url = e.attribute("href");
	// verify title
	TQDomElement te = e.firstChild().toElement();
	if (te.tagName() != "title") {
		*errorMsg = i18n("missing tag ")+"title";
		return false;
	} else name = te.text();
	// do we have an icon?
	if (e.hasAttribute("icon")) {
		icon=e.attribute("icon");
	}
	// ok: got name and url, let's add a bookmark
	KrBookmark *bm = KrBookmark::getExistingBookmark(path+name, _collection);
	if (!bm) {
		bm = new KrBookmark(name, vfs::fromPathOrURL( url ), _collection, icon, path+name);
	parent->children().append(bm);
	}

	return true;
}

bool KrBookmarkHandler::importFromFileFolder(TQDomNode &first, KrBookmark *parent, TQString path, TQString *errorMsg) {
	TQString name;
	TQDomNode n = first;
	while (!n.isNull()) {
		TQDomElement e = n.toElement();
		if (e.tagName() == "bookmark") {
			if (!importFromFileBookmark(e, parent, path, errorMsg))
				return false;
		} else if (e.tagName() == "folder") {
			TQString iconName = "";
			if (e.hasAttribute("icon")) iconName=e.attribute("icon");
			// the title is the first child of the folder
			TQDomElement tmp = e.firstChild().toElement();
			if (tmp.tagName() != "title") {
				*errorMsg = i18n("missing tag ")+"title";
				return false;
			} else name = tmp.text();
			KrBookmark *folder = new KrBookmark(name, iconName);
			parent->children().append(folder);

			TQDomNode nextOne = tmp.nextSibling();
			if (!importFromFileFolder(nextOne, folder, path + name + "/", errorMsg))
				return false;
		} else if (e.tagName() == "separator") {
			parent->children().append(KrBookmark::separator());
		}
		n = n.nextSibling();
	}
	return true;
}


void KrBookmarkHandler::importFromFile() {
	clearBookmarks(_root);
	
	TQString filename = locateLocal( "data", BOOKMARKS_FILE );
	TQFile file( filename );
	if ( !file.open(IO_ReadOnly))
		return; // no bookmarks file

	TQString errorMsg;
	TQDomNode n;
	TQDomElement e;
	TQDomDocument doc( "xbel" );
	if ( !doc.setContent( &file, &errorMsg ) ) {
		goto ERROR;
	}
	// iterate through the document: first child should be "xbel" (skip all until we find it)
	n = doc.firstChild();	
	while (!n.isNull() && n.toElement().tagName()!="xbel")
		n = n.nextSibling();

	if (n.isNull() || n.toElement().tagName()!="xbel") {
		errorMsg = i18n("%1 doesn't seem to be a valid Bookmarks file").arg(filename);
		goto ERROR;
	} else n = n.firstChild(); // skip the xbel part
	importFromFileFolder(n, _root, "", &errorMsg);
	goto SUCCESS;
	
ERROR:
	KMessageBox::error(krApp, i18n("Error reading bookmarks file: %1").arg(errorMsg), i18n( "Error" ));

SUCCESS:
	file.close();
}

void KrBookmarkHandler::populate(TDEPopupMenu *menu) {
	_mainBookmarkPopup = menu;
	menu->clear();
	_bookmarkIDTable.clear();
	_specialBookmarkIDs.clear();
	buildMenu(_root, menu);
}

void KrBookmarkHandler::buildMenu(KrBookmark *parent, TDEPopupMenu *menu) {
	static int inSecondaryMenu = 0; // used to know if we're on the top menu

	// run the loop twice, in order to put the folders on top. stupid but easy :-)
	// note: this code drops the separators put there by the user
	for (KrBookmark *bm = parent->children().first(); bm; bm = parent->children().next()) {
		if (!bm->isFolder()) continue;
		TDEPopupMenu *newMenu = new TDEPopupMenu(menu);
		int id = menu->insertItem(TQIconSet(krLoader->loadIcon(bm->icon(), TDEIcon::Small)),
									bm->text(), newMenu, -1 /* dummy id */, -1 /* end of list */);
		
		if( !_bookmarkIDTable.find( menu ) )
			_bookmarkIDTable.insert( menu, new TQMap<int, KrBookmark *> );
		(*_bookmarkIDTable[ menu ])[ id ] = bm;
		
		++inSecondaryMenu;
		buildMenu(bm, newMenu);
		--inSecondaryMenu;
	}
	for (KrBookmark *bm = parent->children().first(); bm; bm = parent->children().next()) {
		if (bm->isFolder()) continue;
		if (bm->isSeparator() ) {
			menu->insertSeparator();
			continue;
		}
		int itemIndex = bm->plug(menu, -1 /* end of list */);
		CONNECT_BM(bm);
		
		int id = bm->itemId( itemIndex );
		if( !_bookmarkIDTable.find( menu ) )
			_bookmarkIDTable.insert( menu, new TQMap<int, KrBookmark *> );
		(*_bookmarkIDTable[ menu ])[ id ] = bm;
	}

	if (!inSecondaryMenu) {
		krConfig->setGroup( "Private" );
		bool hasPopularURLs = krConfig->readBoolEntry( "BM Popular URLs", true );
		bool hasDevices     = krConfig->readBoolEntry( "BM Devices",      true );
		bool hasLan         = krConfig->readBoolEntry( "BM Lan",          true );
		bool hasVirtualFS   = krConfig->readBoolEntry( "BM Virtual FS",   true );
		bool hasJumpback    = krConfig->readBoolEntry( "BM Jumpback",     true );
		
		int itemIndex;
		
		if( hasPopularURLs ) {
			menu->insertSeparator();
			
			// add the popular links submenu
			TDEPopupMenu *newMenu = new TDEPopupMenu(menu);
			itemIndex = menu->insertItem(TQIconSet(krLoader->loadIcon("bookmark_folder", TDEIcon::Small)),
										i18n("Popular URLs"), newMenu, -1 /* dummy id */, -1 /* end of list */);
			_specialBookmarkIDs.append( itemIndex );
			// add the top 15 urls
			#define MAX 15
			KURL::List list = krApp->popularUrls->getMostPopularUrls(MAX);
			KURL::List::Iterator it;
			for (it = list.begin(); it != list.end(); ++it) {
				TQString name;
				if ((*it).isLocalFile()) name = (*it).path();
				else name = (*it).prettyURL();
				// note: these bookmark are put into the private collection
				// as to not spam the general collection
				KrBookmark *bm = KrBookmark::getExistingBookmark(name, _privateCollection);
				if (!bm)
					bm = new KrBookmark(name, *it, _privateCollection);
				bm->plug(newMenu);
				CONNECT_BM(bm);
			}
			
			newMenu->insertSeparator();
			krPopularUrls->plug(newMenu);
			newMenu->installEventFilter(this);
		}
		
		// do we need to add special bookmarks?
		if (SPECIAL_BOOKMARKS) {
			if( hasDevices || hasLan || hasVirtualFS || hasJumpback )
				menu->insertSeparator();
			
			KrBookmark *bm;
			
			// note: special bookmarks are not kept inside the _bookmarks list and added ad-hoc
			if( hasDevices ) {
				bm = KrBookmark::devices(_collection);
				itemIndex = bm->plug(menu);
				_specialBookmarkIDs.append( bm->itemId( itemIndex ) );
				CONNECT_BM(bm);
			}
			
			if( hasLan ) {
				bm = KrBookmark::lan(_collection);
				itemIndex = bm->plug(menu);
				_specialBookmarkIDs.append( bm->itemId( itemIndex ) );
				CONNECT_BM(bm);
			}
			
			if( hasVirtualFS ) {
				bm = KrBookmark::virt(_collection);
				itemIndex = bm->plug(menu);
				_specialBookmarkIDs.append( bm->itemId( itemIndex ) );
				CONNECT_BM(bm);
			}
			
			if( hasJumpback ) {
				// add the jump-back button
				itemIndex = krJumpBack->plug(menu);
				_specialBookmarkIDs.append( krJumpBack->itemId( itemIndex ) );
				menu->insertSeparator();
				itemIndex = krSetJumpBack->plug(menu);
				_specialBookmarkIDs.append( krSetJumpBack->itemId( itemIndex ) );
			}
		} 
		
		if( !hasJumpback )
			menu->insertSeparator();
		
		itemIndex = menu->insertItem(krLoader->loadIcon("bookmark_add", TDEIcon::Small),
			i18n("Bookmark Current"), BookmarkCurrent);
		_specialBookmarkIDs.append( itemIndex );
		itemIndex = menu->insertItem(krLoader->loadIcon("bookmark", TDEIcon::Small),
			i18n("Manage Bookmarks"), ManageBookmarks);
		_specialBookmarkIDs.append( itemIndex );
	
		// make sure the menu is connected to us
		disconnect(menu, TQ_SIGNAL(activated(int)), 0, 0);
		connect(menu, TQ_SIGNAL(activated(int)), this, TQ_SLOT(menuOperation(int)));
	}

	menu->installEventFilter(this);
}

void KrBookmarkHandler::clearBookmarks(KrBookmark *root) {
	KrBookmark *bm = root->children().first();
	while (bm) {	
		if (bm->isFolder())
			clearBookmarks(bm);
		else {
			bm->unplugAll();
			delete bm;
		}

		bm = root->children().next();
	}
	root->children().clear();
}

void KrBookmarkHandler::bookmarksChanged(const TQString&, const TQString&) {
	importFromFile();
}

bool KrBookmarkHandler::eventFilter( TQObject *obj, TQEvent *ev ) {
	if (ev->type() == TQEvent::MouseButtonRelease) {
		switch (static_cast<TQMouseEvent*>(ev)->button()) {
			case TQt::RightButton:
				_middleClick = false;
				if( obj->inherits( "TQPopupMenu" ) ) {
					int id = static_cast<TQPopupMenu*>(obj)->idAt( static_cast<TQMouseEvent*>(ev)->pos() );
					
					if( obj == _mainBookmarkPopup && _specialBookmarkIDs.contains( id ) ) {
						rightClickOnSpecialBookmark();
						return true;
					}
					
					if( _bookmarkIDTable.find( obj ) ) {
						TQMap<int, KrBookmark*> * table = _bookmarkIDTable[ obj ];
						if( table && table->count( id ) ) {
							KrBookmark *bm = (*table)[ id ];
							rightClicked( static_cast<TQPopupMenu*>(obj), id, bm );
							return true;
						}
					}
				}
			case TQt::LeftButton:
				_middleClick = false;
				break;
			case TQt::MidButton:
				_middleClick = true;
				break;
			default:
				break;
		}
	}
	return TQObject::eventFilter(obj, ev);
}

#define POPULAR_URLS_ID        100100
#define DEVICES_ID             100101
#define LAN_ID                 100103
#define VIRTUAL_FS_ID          100102
#define JUMP_BACK_ID           100104

void KrBookmarkHandler::rightClickOnSpecialBookmark() {
	krConfig->setGroup( "Private" );
	bool hasPopularURLs = krConfig->readBoolEntry( "BM Popular URLs", true );
	bool hasDevices     = krConfig->readBoolEntry( "BM Devices",      true );
	bool hasLan         = krConfig->readBoolEntry( "BM Lan",          true );
	bool hasVirtualFS   = krConfig->readBoolEntry( "BM Virtual FS",   true );
	bool hasJumpback    = krConfig->readBoolEntry( "BM Jumpback",     true );
	
	TQPopupMenu menu( _mainBookmarkPopup );
	menu.setCaption( i18n( "Enable special bookmarks" ) );
	menu.setCheckable( true );
	
	menu.insertItem( i18n( "Popular URLs" ), POPULAR_URLS_ID );
	menu.setItemChecked( POPULAR_URLS_ID, hasPopularURLs );
	menu.insertItem( i18n( "Devices" ), DEVICES_ID );
	menu.setItemChecked( DEVICES_ID, hasDevices );
	menu.insertItem( i18n( "Local Network" ), LAN_ID );
	menu.setItemChecked( LAN_ID, hasLan );
	menu.insertItem( i18n( "Virtual Filesystem" ), VIRTUAL_FS_ID );
	menu.setItemChecked( VIRTUAL_FS_ID, hasVirtualFS );
	menu.insertItem( i18n( "Jump back" ), JUMP_BACK_ID );
	menu.setItemChecked( JUMP_BACK_ID, hasJumpback );
	
	connect( _mainBookmarkPopup, TQ_SIGNAL( highlighted( int ) ), &menu, TQ_SLOT( close() ) );
	connect( _mainBookmarkPopup, TQ_SIGNAL( activated( int ) ), &menu, TQ_SLOT( close() ) );
	
	int result = menu.exec( TQCursor::pos() );
	bool doCloseMain = true;
	
	krConfig->setGroup( "Private" );
	
	switch( result ) {
	case POPULAR_URLS_ID:
		krConfig->writeEntry( "BM Popular URLs", !hasPopularURLs );
		break;
	case DEVICES_ID:
		krConfig->writeEntry( "BM Devices", !hasDevices );
		break;
	case LAN_ID:
		krConfig->writeEntry( "BM Lan", !hasLan );
		break;
	case VIRTUAL_FS_ID:
		krConfig->writeEntry( "BM Virtual FS", !hasVirtualFS );
		break;
	case JUMP_BACK_ID:
		krConfig->writeEntry( "BM Jumpback", !hasJumpback );
		break;
	default:
		doCloseMain = false;
		break;
	}
	
	menu.close();
	
	if( doCloseMain && _mainBookmarkPopup )
		_mainBookmarkPopup->close();
}

#define OPEN_ID           100200
#define OPEN_NEW_TAB_ID   100201
#define DELETE_ID         100202

void KrBookmarkHandler::rightClicked( TQPopupMenu *menu, int /*id*/, KrBookmark * bm ) {
	TQPopupMenu popup( _mainBookmarkPopup );
	
	popup.insertItem( krLoader->loadIcon( "document-open", TDEIcon::Panel ), i18n( "Open" ), OPEN_ID );
	popup.insertItem( krLoader->loadIcon( "tab_new", TDEIcon::Panel ), i18n( "Open in a new tab" ), OPEN_NEW_TAB_ID );
	popup.insertSeparator();
	popup.insertItem( krLoader->loadIcon( "edit-delete", TDEIcon::Panel ), i18n( "Delete" ), DELETE_ID );
	
	connect( menu, TQ_SIGNAL( highlighted( int ) ), &popup, TQ_SLOT( close() ) );
	connect( menu, TQ_SIGNAL( activated( int ) ), &popup, TQ_SLOT( close() ) );
	
	int result = popup.exec( TQCursor::pos() );
	
	popup.close();
	if( _mainBookmarkPopup && result >= OPEN_ID && result <= DELETE_ID ) {
		_mainBookmarkPopup->close();
	}
	
	switch( result ) {
	case OPEN_ID:
		SLOTS->refresh( bm->url() );
		break;
	case OPEN_NEW_TAB_ID:
		SLOTS->newTab( bm->url() );
		break;
	case DELETE_ID:
		deleteBookmark( bm );
		break;
	}
}

// used to monitor middle clicks. if mid is found, then the
// bookmark is opened in a new tab. ugly, but easier than overloading
// TDEAction and TDEActionCollection.
void KrBookmarkHandler::slotActivated(const KURL& url) {
	if (_middleClick)
		SLOTS->newTab(url);
	else SLOTS->refresh(url);
}


#include "krbookmarkhandler.moc"
