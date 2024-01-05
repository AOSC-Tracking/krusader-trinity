/***************************************************************************
                         krviewer.h  -  description
                            -------------------
   begin                : Thu Apr 18 2002
   copyright            : (C) 2002 by Shie Erlich & Rafi Yanai
   email                : 
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRVIEWER_H
#define KRVIEWER_H

#include <tqwidget.h>
#include <tqptrlist.h>
#include <tdeparts/mainwindow.h>
#include <tdetempfile.h>
#include <tdeparts/partmanager.h>
#include <tdeparts/browserextension.h>
#include <tqguardedptr.h>
#include <ktabwidget.h>

#include "../krusader.h"


/**
  *@author Shie Erlich & Rafi Yanai
  */

class TQPopupMenu;
class PanelViewerBase;

class KrViewer : public KParts::MainWindow {
	TQ_OBJECT
  
public:
	virtual ~KrViewer();
	
	enum Mode{Generic,Text,Hex};

	static void view( KURL url, TQWidget * parent = krApp );
	static void view( KURL url, Mode mode, bool new_window, TQWidget * parent = krApp );
	static void edit( KURL url, TQWidget * parent );
	static void edit( KURL url, Mode mode=Text, int new_window=-1, TQWidget * parent = krApp );
	
	virtual bool eventFilter ( TQObject * watched, TQEvent * e );

public slots:
	void keyPressEvent( TQKeyEvent *e );
	void createGUI( KParts::Part* );

	void viewGeneric();
	void viewText();
	void viewHex();
	void editText();

	void print();
	void copy();

	void tabChanged(TQWidget* w);
	void tabURLChanged( PanelViewerBase * pvb, const KURL &url );
	void tabCloseRequest(TQWidget *w);
	void tabCloseRequest();

	void nextTab();
	void prevTab();
	void detachTab();
	
	void checkModified();

protected:
	virtual bool queryClose();
	virtual bool queryExit();
	virtual void windowActivationChange ( bool oldActive );

	virtual void focusInEvent( TQFocusEvent * ){ if( viewers.remove( this ) ) viewers.prepend( this ); } // move to first

private:
	KrViewer( TQWidget *parent = 0, const char *name = 0 );
	void addTab(PanelViewerBase* pvb, TQString msg,TQString iconName, KParts::Part* part);
	PanelViewerBase * getPanelViewerBase( KParts::Part* part);
	void updateActions( PanelViewerBase * base );
	
	static KrViewer* getViewer(bool new_window);	

	KParts::PartManager manager;
	TQPopupMenu* viewerMenu;
	KTempFile tmpFile;
	KTabWidget tabBar;
	TQGuardedPtr<TQWidget> returnFocusTo;
	PanelViewerBase * returnFocusTab;
	
	int detachActionIndex;

	TDEAction *printAction;
	TDEAction *copyAction;

	int tabCloseID;
	int closeID;

	static TQPtrList<KrViewer> viewers; // the first viewer is the active one
	TQValueList<int>    reservedKeys;   // the reserved key sequences
	TQValueList<int>    reservedKeyIDs; // the IDs of the reserved keys
};

class Invoker : public TQObject {
	TQ_OBJECT
  
	
public:
	Invoker( TQObject *recv, const char * slot ) {
		connect( this, TQ_SIGNAL( invokeSignal() ), recv, slot );
	}
	
	void invoke() {
		emit invokeSignal();
	}
	
signals:
	void invokeSignal();
};

#endif
