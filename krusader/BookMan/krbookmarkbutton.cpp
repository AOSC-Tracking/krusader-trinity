#include "krbookmarkbutton.h"
#include "krbookmarkhandler.h"
#include "../krusader.h"
#include <tqpixmap.h>
#include <kiconloader.h>
#include <kaction.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kdebug.h>

KrBookmarkButton::KrBookmarkButton(TQWidget *parent): TQToolButton(parent) {
	TQPixmap icon = krLoader->loadIcon("bookmark", KIcon::Toolbar, 16);
	setFixedSize(icon.width() + 4, icon.height() + 4);
	setPixmap(icon);
	setTextLabel(i18n("BookMan II"), true);
	setPopupDelay(10); // 0.01 seconds press
	setAcceptDrops(false);

	acmBookmarks = new TDEActionMenu(i18n("Bookmarks"), "bookmark", 0, 0);
	acmBookmarks->setDelayed(false);
	acmBookmarks->popupMenu()->setKeyboardShortcutsEnabled(true);
	acmBookmarks->popupMenu()->setKeyboardShortcutsExecute(true);

	setPopup(acmBookmarks->popupMenu());
	connect(this, TQT_SIGNAL(pressed()), this, TQT_SLOT(populate()));
	populate();
}

void KrBookmarkButton::populate() {
	krBookMan->populate(static_cast<TDEPopupMenu*>(popup()));
}

void KrBookmarkButton::openPopup() {
	populate();
	popup()->exec(mapToGlobal(TQPoint(0, height())));
}

#include "krbookmarkbutton.moc"
