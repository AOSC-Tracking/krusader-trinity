#include "krbookmark.h"
#include "../krusader.h"
#include "../VFS/krarchandler.h"
#include <tdeactioncollection.h>
#include <kiconloader.h>
#include <tdelocale.h>
#include <kdebug.h>

#define BM_NAME(X)		(TQString("Bookmark:")+X)

#if KDE_IS_VERSION(3,4,0)
static const char* NAME_DEVICES = I18N_NOOP("Media");
#else
static const char* NAME_DEVICES = I18N_NOOP("Devices");
#endif
static const char* NAME_VIRTUAL = I18N_NOOP("Virtual Filesystem");
static const char* NAME_LAN = I18N_NOOP("Local Network");

KrBookmark::KrBookmark(TQString name, KURL url, TDEActionCollection *parent, TQString icon, TQString actionName ):
	TDEAction(name, 0, 0, 0, parent, actionName.isNull() ? BM_NAME(name).latin1() : BM_NAME(actionName).latin1()), 
	_url(url), _folder(false), _separator(false) {
	connect(this, TQ_SIGNAL(activated()), this, TQ_SLOT(activatedProxy()));
	// do we have an icon?
	if (!icon.isEmpty())
		setIcon(icon);
	else {
		// what kind of a url is it?
		if (_url.isLocalFile()) {
			setIcon("folder");
		} else { // is it an archive?
			if (KRarcHandler::isArchive(_url))
				setIcon("application-x-tar");
			else setIcon("folder_html");
		}
	}

	_children.setAutoDelete(true);
}

KrBookmark::KrBookmark(TQString name, TQString icon):
	TDEAction(name, 0, 0, 0, 0), _folder(true), _separator(false) {
	setIcon(icon=="" ? "folder" : icon);
}

KrBookmark* KrBookmark::getExistingBookmark(TQString actionName, TDEActionCollection *collection) {
	return static_cast<KrBookmark*>(collection->action(BM_NAME(actionName).latin1()));
}

KrBookmark* KrBookmark::devices(TDEActionCollection *collection) {
	KrBookmark *bm = getExistingBookmark(i18n(NAME_DEVICES), collection);	
	if (!bm) {
#if KDE_IS_VERSION(3,4,0)
		bm = new KrBookmark(i18n(NAME_DEVICES), "media:/", collection);
#else
		bm = new KrBookmark(i18n(NAME_DEVICES), "devices:/", collection);
#endif
		bm->setIconSet(krLoader->loadIcon("blockdevice", TDEIcon::Small));
	}
	return bm;
}

KrBookmark* KrBookmark::virt(TDEActionCollection *collection) {
	KrBookmark *bm = getExistingBookmark(i18n(NAME_VIRTUAL), collection);	
	if (!bm) {
		bm = new KrBookmark(i18n(NAME_VIRTUAL), "virt:/", collection);
		bm->setIconSet(krLoader->loadIcon("pipe", TDEIcon::Small));
	}
	return bm;
}

KrBookmark* KrBookmark::lan(TDEActionCollection *collection) {
	KrBookmark *bm = getExistingBookmark(i18n(NAME_LAN), collection);	
	if (!bm) {
		bm = new KrBookmark(i18n(NAME_LAN), "lan:/", collection);
		bm->setIconSet(krLoader->loadIcon("network", TDEIcon::Small));
	}
	return bm;
}

KrBookmark* KrBookmark::separator() {
	KrBookmark *bm = new KrBookmark("");
	bm->_separator = true;
	bm->_folder = false;
	return bm;
}


void KrBookmark::activatedProxy() {
	emit activated(url());
}

#include "krbookmark.moc"
