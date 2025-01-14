/***************************************************************************
                          krpreviewpopup.cpp  -  description
                             -------------------
    begin                : Sun Dec 29 2002
    copyright            : (C) 2002 by Shie Erlich & Rafi Yanai
    web site             : http://krusader.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <tdeio/previewjob.h>
#include "krpreviewpopup.h"
#include <kdebug.h>
#include <tdelocale.h>
#include <keditcl.h>
#include "../KViewer/krviewer.h"
#include "../krusader.h"

KrPreviewPopup::KrPreviewPopup(): id(1),noPreview(true){
	connect(this,TQ_SIGNAL(activated(int)),this,TQ_SLOT(view(int)));
}

void KrPreviewPopup::setUrls(const KURL::List* urls){
	//insertItem(i18n("Configure preview"),0);
	insertItem(i18n("Preview not available"),0);

	TDEIO::PreviewJob* pjob;
	TQStringList plugins = TDEIO::PreviewJob::availablePlugins();

	for( unsigned int i=0; i< urls->count(); ++i){
		KFileItem* kfi = new KFileItem(KFileItem::Unknown,KFileItem::Unknown,*(urls->at(i)));
		files.append(kfi);
	}

	pjob = new TDEIO::PreviewJob(files,200,200,200,1,true,true,0);
	connect(pjob,TQ_SIGNAL(gotPreview(const KFileItem*,const TQPixmap&)),
          this,TQ_SLOT(addPreview(const KFileItem*,const TQPixmap&)));
}

KrPreviewPopup::~KrPreviewPopup(){}

void KrPreviewPopup::addPreview(const KFileItem* file,const TQPixmap& preview){
	if(noPreview){
		removeItem(0);
		noPreview = false;
	}
	insertItem(preview,id);
  insertItem(file->text(),id++);
	insertSeparator();
	availablePreviews.append(file->url());
}

void KrPreviewPopup::view(int id){
	if( id==0 ) return;
	else {
		KURL url = *(availablePreviews.at(id-1));
		KrViewer::view(url);
	}
}

#include "krpreviewpopup.moc"
