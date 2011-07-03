#include <kmessagebox.h>
#include <klocale.h>
#include <tqpushbutton.h>
#include <klistview.h>
#include <kiconloader.h>
#include <klistviewsearchline.h>
#include <tqheader.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <ktoolbarbutton.h>
#include "../krusader.h"
#include "../krslots.h"
#include "popularurls.h"

#define STARTING_RANK	20
#define INCREASE			2
#define DECREASE			1

PopularUrls::PopularUrls(TQObject *tqparent, const char *name) : TQObject(tqparent, name), 
	head(0), tail(0), count(0) {
	dlg = new PopularUrlsDlg();
}

PopularUrls::~PopularUrls() {
	clearList();
	delete dlg;
}

void PopularUrls::clearList() {
	if (head) {
		UrlNodeP p=head, tmp;
		while (p) {
			tmp = p;
			p=p->next;
			delete tmp;
		}
	}
	ranks.clear();
	head = tail = 0;
}

void PopularUrls::save() {
	KConfigGroupSaver svr(krConfig, "Private");
	// prepare the string list containing urls and int list with ranks
	TQStringList urlList;
	TQValueList<int> rankList;
	UrlNodeP p = head;
	while (p) {
		urlList << p->url.prettyURL();
		rankList << p->rank;
		p = p->next;
	}
	krConfig->writeEntry("PopularUrls", urlList);
	krConfig->writeEntry("PopularUrlsRank", rankList);
}

void PopularUrls::load() {
	KConfigGroupSaver svr(krConfig, "Private");
	TQStringList urlList = krConfig->readListEntry("PopularUrls");
	TQValueList<int> rankList = krConfig->readIntListEntry("PopularUrlsRank");
	if (urlList.count() != rankList.count()) {
		KMessageBox::error(krApp, i18n("Saved 'Popular Urls' are invalid. List will be cleared"));
		return;
	}
	clearList();
	count = 0;
	// iterate through both lists and
	TQStringList::Iterator uit;
	TQValueList<int>::Iterator rit;
	for (uit=urlList.begin(), rit=rankList.begin(); uit!=urlList.end() && rit!=rankList.end(); ++uit, ++rit) {
		UrlNodeP node = new UrlNode;
		node->url = KURL::fromPathOrURL( *uit );
		node->rank = *rit;
		appendNode(node);
		ranks.insert(*uit, node);
	}
}


// returns a url list with the 'max' top popular urls 
KURL::List PopularUrls::getMostPopularUrls(int max) {
	// get at most 'max' urls
	KURL::List list;
	UrlNodeP p = head;
	int tmp = 0;
	if (maxUrls < max) max = maxUrls; // don't give more than maxUrls
	while (p && tmp < max) {
		list << p->url;
		p = p->next;
		++tmp;
	}
	
	return list;
}

// adds a url to the list, or increase rank of an existing url, making
// sure to bump it up the list if needed
void PopularUrls::addUrl(const KURL& url) {
	KURL tmpurl = url;
	tmpurl.adjustPath(1); // make a uniform trailing slash policy
	UrlNodeP pnode;
	
	decreaseRanks();
	if (!head) { // if the list is empty ... (assumes dict to be empty as well)
		pnode = new UrlNode;
		pnode->rank = STARTING_RANK;
		pnode->url = tmpurl;
		appendNode(pnode);
		ranks.insert(tmpurl.url(), head);
	} else {
		pnode = ranks.tqfind(tmpurl.url());
		if (!pnode) { // is the added url new? if so, append it
			pnode = new UrlNode;
			pnode->rank = STARTING_RANK;
			pnode->url = tmpurl;
			appendNode(pnode);
			ranks.insert(tmpurl.url(), pnode);
		} else {
			pnode->rank += INCREASE;
		}
	}
	
	// do we need to change location for this one?
	relocateIfNeeded(pnode);

	// too many urls?
	if (count > maxUrls) removeNode(tail);
	
	//dumpList();
}

// checks if 'node' needs to be bumped-up the ranking list and does it if needed
void PopularUrls::relocateIfNeeded(UrlNodeP node) {
	if (node->prev && (node->prev->rank < node->rank)) {
		// iterate until we find the correct place to put it
		UrlNodeP tmp = node->prev->prev;
		while (tmp) {
			if (tmp->rank >= node->rank)
				break; // found it!
			else tmp = tmp->prev;
		}
		// now, if tmp isn't null, we need to move node to tmp->next
		// else move it to become head
		removeNode(node);
		insertNode(node, tmp);
	}
}
	
	
// iterate over the list, decreasing each url's rank
// this is very naive, but a 1..30 for loop is acceptable (i hope)
void PopularUrls::decreaseRanks() {
	if (head) {
		UrlNodeP p=head;
		while (p) {
			if (p->rank-DECREASE>=0)
				p->rank -= DECREASE;
			else p->rank = 0;
			p=p->next;
		}
	}
}

// removes a node from the list, but doesn't free memory!
// note: this will be buggy in case the list becomes empty (which should never happen)
void PopularUrls::removeNode(UrlNodeP node) {
	if (node->prev) {
		if (tail == node) tail = node->prev;
		node->prev->next = node->next;
	}
	if (node->next) {
		if (head == node) head = node->next;
		node->next->prev = node->prev;
	}
	--count;
}

void PopularUrls::insertNode(UrlNodeP node, UrlNodeP after) {
	if (!after) { // make node head
		node->next = head;
		node->prev = 0;
		head->prev = node;
		head = node;
	} else {
		if (tail == after) tail = node;
		node->prev = after;
		node->next = after->next;
		if( node->next ) {
			after->next->prev = node;
			after->next = node;
		}
	}
	++count;
}

// appends 'node' to the end of the list, collecting garbage if needed
void PopularUrls::appendNode(UrlNodeP node) {
	if (!tail) { // creating the first element
		head = tail = node;
		node->prev = node->next = 0;
	} else {
		node->next = 0;
		node->prev = tail;
		tail->next = node;
		tail = node;
	}
	++count;
}

void PopularUrls::dumpList() {
	UrlNodeP p = head;
	printf("====start %d====\n",count);
	while (p) {
		printf("%d : %s\n", p->rank, p->url.url().latin1());
		p = p->next;
	}
	fflush(stdout);
}

void PopularUrls::showDialog() {
	KURL::List list = getMostPopularUrls(maxUrls);
	dlg->run(list);
	if (dlg->result() == -1) return;
	SLOTS->refresh(list[dlg->result()]);
	//printf("running %s\n", list[dlg->result()].url().latin1());fflush(stdout);
}

// ===================================== PopularUrlsDlg ======================================
PopularUrlsDlg::PopularUrlsDlg(): 
	KDialogBase(Plain, i18n("Popular Urls"), Close, KDialogBase::NoDefault, krApp) {
	TQGridLayout *tqlayout = new TQGridLayout( plainPage(), 0, KDialog::spacingHint() );
	
	// listview to contain the urls
	urls = new KListView(plainPage());
	urls->header()->hide();
	urls->addColumn("");
	urls->setSorting(-1);
	urls->setVScrollBarMode(TQScrollView::AlwaysOn);
	
	// quick search
	TQToolButton *btn = new TQToolButton(plainPage());
	btn->setIconSet(SmallIcon("locationbar_erase"));
	search = new KListViewSearchLine(plainPage(), urls);
	search->setTrapReturnKey(true);
	TQLabel *lbl = new TQLabel(search, i18n(" &Search: "), plainPage());

	tqlayout->addWidget(btn,0,0);
	tqlayout->addWidget(lbl,0,1);
	tqlayout->addWidget(search,0,2);
	tqlayout->addMultiCellWidget(urls,1,1,0,2);
	setMaximumSize(600, 500);
	
	setTabOrder(search, urls);
	setTabOrder(urls, actionButton(Close));
	
	connect(urls, TQT_SIGNAL(executed(TQListViewItem*)), 
		this, TQT_SLOT(slotItemSelected(TQListViewItem*)));
	connect(urls, TQT_SIGNAL(returnPressed(TQListViewItem*)), 
		this, TQT_SLOT(slotItemSelected(TQListViewItem*)));		
	connect(btn, TQT_SIGNAL(clicked()), search, TQT_SLOT(clear()));
	connect(search, TQT_SIGNAL(returnPressed(const TQString&)), 
		this, TQT_SLOT(slotSearchReturnPressed(const TQString&)));
}

void PopularUrlsDlg::slotItemSelected(TQListViewItem *it) {
	selection = urls->itemIndex(it);
	accept();
}

void PopularUrlsDlg::slotSearchReturnPressed(const TQString&) {
	urls->setFocus();
	// select the first visible item
	TQListViewItemIterator it( urls );
   while ( it.current() ) {
		if ( it.current()->isVisible() ) {
			urls->setSelected(it.current(), true);
			urls->setCurrentItem(it.current());
			break;
		} else ++it;
	} 
}

PopularUrlsDlg::~PopularUrlsDlg() {
	delete search;
	delete urls;
}

void PopularUrlsDlg::run(KURL::List list) {
	// populate the listview
	urls->clear();
	KURL::List::Iterator it;
	for (it = list.begin(); it!=list.end(); ++it) {
		KListViewItem *item = new KListViewItem(urls, urls->lastItem());
		item->setText(0, (*it).isLocalFile() ? (*it).path() : (*it).prettyURL());
		item->setPixmap(0, (*it).isLocalFile() ? SmallIcon("folder") : SmallIcon("folder_html"));
	}
	//urls->setCurrentItem(urls->firstChild());
	//urls->setSelected(urls->firstChild(), true);
	setMinimumSize(urls->tqsizeHint().width()+45, 400);
	
	search->clear();
	search->setFocus();
	selection = -1;
	exec();
}

#include "popularurls.moc"
