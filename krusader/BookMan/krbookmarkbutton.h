#ifndef KRBOOKMARK_BUTTON_H
#define KRBOOKMARK_BUTTON_H

#include <tqtoolbutton.h>
#include "krbookmarkhandler.h"

class KrBookmarkButton: public TQToolButton {
	Q_OBJECT
  TQ_OBJECT
public:
	KrBookmarkButton(TQWidget *tqparent);
	void openPopup();

signals:
	void openUrl(const KURL &url);

protected slots:
	void populate();
	
private:
	KActionMenu *acmBookmarks;
};

#endif // KRBOOKMARK_BUTTON_H
