#ifndef _SUPERVIEW_H
#define _SUPERVIEW_H

#include <kurl.h>
#include <tqstring.h>
#include <tqwidgetstack.h>
#include <tdeparts/part.h>
#include <tdeio/job.h>
#include <ktempfile.h>
#include <tqdict.h>
#include <tqlabel.h>

#include "krviewer.h"


class PanelViewerBase: public TQWidgetStack {
	Q_OBJECT
  

public:
	PanelViewerBase( TQWidget *parent = 0 );
	virtual ~PanelViewerBase();
	inline KURL url() const { return curl; }
	inline void setUrl( KURL url ) { emit urlChanged( this, url ); curl = url; }
	inline KParts::ReadOnlyPart* part() const { return cpart; }
	virtual bool isModified() { return false; }
	virtual bool isEditor() = 0;

public slots:
	virtual KParts::ReadOnlyPart* openURL( const KURL&, KrViewer::Mode=KrViewer::Generic ){ return 0;} 
	virtual bool closeURL(){ return false; }
	virtual bool queryClose() { return true; }

signals:
	void openURLRequest( const KURL &url );
	void urlChanged( PanelViewerBase *, const KURL & );

protected:
	TQDict<KParts::ReadOnlyPart> *mimes;
	KParts::ReadOnlyPart *cpart;

	TQString cmimetype;
	KURL curl;
	TQLabel *fallback;

};

class PanelViewer: public PanelViewerBase {
	Q_OBJECT
  
public slots:
	KParts::ReadOnlyPart* openURL( const KURL &url, KrViewer::Mode mode=KrViewer::Generic );
	bool closeURL();

public:
	PanelViewer( TQWidget *parent = 0 );
	~PanelViewer();

	virtual bool isEditor() { return false; }

protected:
	KParts::ReadOnlyPart *getPart( TQString mimetype );
	KParts::ReadOnlyPart*  getHexPart();
	void oldHexViewer(KTempFile& tmpFile);
};

class PanelEditor: public PanelViewerBase {
	Q_OBJECT
  
public:
	virtual bool isModified();
	virtual bool isEditor() { return true; }

public slots:
	KParts::ReadOnlyPart* openURL( const KURL &url, KrViewer::Mode mode=KrViewer::Generic );
	bool closeURL();
	bool queryClose();
	void slotStatResult( TDEIO::Job* job );

public:
	PanelEditor( TQWidget *parent = 0 );
	~PanelEditor();

protected:
	KParts::ReadWritePart* getPart( TQString mimetype );

	bool busy;
	TDEIO::UDSEntry entry;
};

#endif
