#ifndef _PANELPOPUP_H
#define _PANELPOPUP_H

#include <tqwidget.h>
#include <tqwidgetstack.h>
#include <tqpixmap.h>
#include <tqvaluelist.h>
#include <tdefileitem.h>
#include <tqguardedptr.h>
#include <tdeio/previewjob.h>
#include <kurl.h>

class TQButtonGroup;
class TQLabel;
class TQListViewItem;
class TQSplitter;
class KFileTreeView;
class TQToolButton;
class KrSqueezedTextLabel;
class KLineEdit;
class KComboBox;
class KrusaderImageFilePreview;
class PanelViewer;
class DiskUsageViewer;

class PanelPopup: public TQWidget {
   TQ_OBJECT
  
   enum Parts { Tree, Preview, QuickPanel, View, DskUsage, Last=0xFF };
public:
   PanelPopup( TQSplitter *splitter, bool left );
   ~PanelPopup();
	inline int currentPage() const { return stack->id(stack->visibleWidget()); }

	void saveSizes();

public slots:
   void update(KURL url);
	void show();
	void hide();
	
signals:
	void selection(const KURL &url);
	void hideMe();
   
protected slots:	
	virtual void setFocus();
	void tabSelected(int id);
	void treeSelection(TQListViewItem*);
	void slotDroppedOnTree(TQWidget *widget, TQDropEvent *e, KURL::List &lst, KURL &);
	void handleOpenURLRequest(const KURL &url);
	void quickSelect();
	void quickSelect(const TQString &);
        void quickSelectStore();

protected:
	bool _left;
	bool _hidden;
	TQWidgetStack *stack;
	KrusaderImageFilePreview *viewer;
	KrSqueezedTextLabel *dataLine;
	TQGuardedPtr<TDEIO::PreviewJob> pjob;
	KFileTreeView *tree;
	TQToolButton *treeBtn, *previewBtn, *quickBtn, *viewerBtn, *duBtn;
	TQButtonGroup *btns;
	KLineEdit *quickFilter;
	KComboBox *quickSelectCombo;
	PanelViewer *panelviewer;
	DiskUsageViewer *diskusage;
	TQValueList<int> splitterSizes;
	TQSplitter *splitter;
};

#endif // _PANELPOPUP_H
