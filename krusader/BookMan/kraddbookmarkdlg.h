#ifndef KRADDBOOKMARKDLG_H
#define KRADDBOOKMARKDLG_H

#include "krbookmark.h"
#include "../VFS/vfs.h"
#include <kdialogbase.h>
#include <kurl.h>
#include <klineedit.h>
#include <tqmap.h>
#include <tdelistview.h>
#include <tqtoolbutton.h>

class KrAddBookmarkDlg: public KDialogBase {
	Q_OBJECT
  
public:
	KrAddBookmarkDlg(TQWidget *parent, KURL url = 0);
	KURL url() const { return vfs::fromPathOrURL(_url->text()); }
	TQString name() const { return _name->text(); }
	KrBookmark *folder() const { return _xr[static_cast<TDEListViewItem*>(_createIn->selectedItem())]; }

protected:
	TQWidget *createInWidget();
	void populateCreateInWidget(KrBookmark *root, TDEListViewItem *parent);

protected slots:
	void toggleCreateIn(bool show);
	void createInSelection(TQListViewItem *item);
	void newFolder();
	
private:
	KLineEdit *_name;
	KLineEdit *_url;
	KLineEdit *_folder;
	TDEListView *_createIn;
	TQMap<TDEListViewItem*, KrBookmark*> _xr;
	TQToolButton *_createInBtn;
};

#endif // KRADDBOOKMARKDLG_H
