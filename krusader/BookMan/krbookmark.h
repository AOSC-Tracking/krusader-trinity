#ifndef KRBOOKMARK_H
#define KRBOOKMARK_H

#include <tdeaction.h>
#include <tqptrlist.h>
#include <kurl.h>

class TDEActionCollection;

class KrBookmark: public TDEAction {
	TQ_OBJECT
  
public:
	KrBookmark(TQString name, KURL url, TDEActionCollection *parent, TQString icon = "", TQString actionName = TQString() );
	KrBookmark(TQString name, TQString icon = ""); // creates a folder
	// text() and setText() to change the name of the bookmark
	// icon() and setIcon() to change icons (by name)
	inline const KURL& url() const { return _url; }
	inline void setURL(const KURL& url) { _url = url; }
	inline bool isFolder() const { return _folder; }
	inline bool isSeparator() const { return _separator; }
	TQPtrList<KrBookmark>& children() { return _children; }

	static KrBookmark* getExistingBookmark(TQString actionName, TDEActionCollection *collection);	
	// ----- special bookmarks
	static KrBookmark* devices(TDEActionCollection *collection);
	static KrBookmark* virt(TDEActionCollection *collection);
	static KrBookmark* lan(TDEActionCollection *collection);
	static KrBookmark* separator();

signals:
	void activated(const KURL& url);

protected slots:
	void activatedProxy();
	
	
private:
	KURL _url;
	TQString _icon;
	bool _folder;
	bool _separator;
	TQPtrList<KrBookmark> _children;
};

#endif // KRBOOKMARK_H
