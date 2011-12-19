#include "kraddbookmarkdlg.h"
#include "../krusader.h"
#include "krbookmarkhandler.h"
#include <klocale.h>
#include <tqheader.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <kinputdialog.h>
#include <kiconloader.h>
#include <kdebug.h>

KrAddBookmarkDlg::KrAddBookmarkDlg(TQWidget *parent, KURL url):
	KDialogBase(KDialogBase::Swallow, i18n("Add Bookmark"),
				 KDialogBase::User1 | KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, parent) {
	// create the 'new folder' button
	setButtonText(KDialogBase::User1, i18n("New Folder"));
	showButton(KDialogBase::User1, false); // hide it until _createIn is shown
	connect(this, TQT_SIGNAL(user1Clicked()), this, TQT_SLOT(newFolder()));

	// create the main widget
	TQWidget *page = new TQWidget(this);
	setMainWidget(page);

	TQGridLayout *tqlayout = new TQGridLayout(page, 1, 1, 0, spacingHint()); // expanding
	// name and url
	TQLabel *lb1 = new TQLabel(i18n("Name:"), page);
	_name = new KLineEdit(page);
	_name->setText(url.prettyURL()); // default name is the url
	_name->selectAll(); // make the text selected
	tqlayout->addWidget(lb1, 0, 0);	
	tqlayout->addWidget(_name, 0, 1);
	
	TQLabel *lb2 = new TQLabel(i18n("URL:"), page);
	_url = new KLineEdit(page);
	tqlayout->addWidget(lb2, 1, 0);	
	tqlayout->addWidget(_url, 1, 1);
	_url->setText(url.prettyURL()); // set the url in the field

	// create in linedit and button
	TQLabel *lb3 = new TQLabel(i18n("Create in:"), page);
	_folder = new KLineEdit(page);
	tqlayout->addWidget(lb3, 2, 0);
	tqlayout->addWidget(_folder, 2, 1);
	_folder->setReadOnly(true);

	_createInBtn = new TQToolButton(page);
	_createInBtn->setPixmap(krLoader->loadIcon("down", KIcon::Small));
	_createInBtn->setToggleButton(true);
	connect(_createInBtn, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(toggleCreateIn(bool )));
	tqlayout->addWidget(_createInBtn, 2, 2);

	setDetailsWidget(createInWidget());
	
	_name->setFocus();
}

void KrAddBookmarkDlg::toggleCreateIn(bool show) {
	_createInBtn->setPixmap(krLoader->loadIcon(show ? "up" :"down", KIcon::Small));
	showButton(KDialogBase::User1, show);
	setDetails(show);
}

// creates the widget that lets you decide where to put the new bookmark
TQWidget *KrAddBookmarkDlg::createInWidget() {
	_createIn = new KListView(this);
	_createIn->addColumn("Folders");
	_createIn->header()->hide();
	_createIn->setRootIsDecorated(true);
	_createIn->setAlternateBackground(TQColor()); // disable alternate coloring 
	
	KListViewItem *item = new KListViewItem(_createIn, i18n("Bookmarks"));
	item->setOpen(true);
	item->setSelected(true);
	_xr[item] = krBookMan->_root;

	populateCreateInWidget(krBookMan->_root, item);
	_createIn->setCurrentItem(item);
	createInSelection(item);
	connect(_createIn, TQT_SIGNAL(selectionChanged(TQListViewItem*)), this, TQT_SLOT(createInSelection(TQListViewItem*)));
	
	return _createIn;
}

void KrAddBookmarkDlg::createInSelection(TQListViewItem *item) {
	if (item) {
		_folder->setText(_xr[static_cast<KListViewItem*>(item)]->text());
	}
}

void KrAddBookmarkDlg::populateCreateInWidget(KrBookmark *root, KListViewItem *parent) {
	for (KrBookmark *bm = root->children().first(); bm; bm = root->children().next()) {
		if (bm->isFolder()) {
			KListViewItem *item = new KListViewItem(parent, bm->text());
			item->setOpen(true);
			_xr[item] = bm;
			populateCreateInWidget(bm, item);
		}
	}
}

void KrAddBookmarkDlg::newFolder() {
	// get the name
	TQString newFolder = KInputDialog::getText(i18n("New Folder"), i18n("Folder name:"), TQString(), 0, this);
	if (newFolder == TQString())
		return;
	// add to the list in bookman
	KrBookmark *bm = new KrBookmark(newFolder);
	krBookMan->addBookmark(bm, _xr[static_cast<KListViewItem*>(_createIn->selectedItem())]);
	// fix the gui
	KListViewItem *item = new KListViewItem(_createIn->selectedItem(), bm->text());
	_xr[item] = bm;

	_createIn->setCurrentItem(item);
	item->setSelected(true);
}

#include "kraddbookmarkdlg.moc"
