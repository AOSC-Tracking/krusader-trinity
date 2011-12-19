/*
* This file is part of the KDE project
* Copyright (C) 2001 Martin R. Jones <mjones@kde.org>
*               2001 Carsten Pfeiffer <pfeiffer@kde.org>
*
* You can Freely distribute this program under the GNU Library General Public
* License. See the file "COPYING" for the exact licensing terms.
*/

#include <tqlayout.h>
#include <tqlabel.h>
#include <tqcombobox.h>
#include <tqcheckbox.h>
#include <tqwhatsthis.h>
#include <tqtimer.h>

#include <kapplication.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kfileitem.h>
#include <kio/previewjob.h>

#include "kimagefilepreview.h"

/**** KrusaderImageFilePreview ****/

KrusaderImageFilePreview::KrusaderImageFilePreview( TQWidget *parent )
		: KPreviewWidgetBase( parent ),
m_job( 0L ) {
	TQVBoxLayout *vb = new TQVBoxLayout( this, KDialog::marginHint() );

	imageLabel = new TQLabel( this );
	imageLabel->setFrameStyle( TQFrame::Panel | TQFrame::Sunken );
	imageLabel->setAlignment( TQt::AlignHCenter | TQt::AlignVCenter );
	imageLabel->setSizePolicy( TQSizePolicy( TQSizePolicy::Preferred, TQSizePolicy::Ignored ) );
	vb->addWidget( imageLabel, 1 );

	timer = new TQTimer( this );
	connect( timer, TQT_SIGNAL( timeout() ), TQT_SLOT( showPreview() ) );

	setSupportedMimeTypes( KIO::PreviewJob::supportedMimeTypes() );
}

KrusaderImageFilePreview::~KrusaderImageFilePreview() {
	if ( m_job )
		m_job->kill();
}

void KrusaderImageFilePreview::showPreview() {
	// Pass a copy since clearPreview() will clear currentURL
	KURL url = currentURL;
	showPreview( url, true );
}

// called via KPreviewWidgetBase interface
void KrusaderImageFilePreview::showPreview( const KURL& url ) {
	showPreview( url, false );
}

void KrusaderImageFilePreview::showPreview( const KURL &url, bool force ) {
	if ( !url.isValid() ) {
		clearPreview();
		return ;
	}

	if ( url != currentURL || force ) {
		clearPreview();
		currentURL = url;

		int w = imageLabel->contentsRect().width() - 4;
		int h = imageLabel->contentsRect().height() - 4;

		m_job = createJob( url, w, h );
		connect( m_job, TQT_SIGNAL( result( KIO::Job * ) ),
		         this, TQT_SLOT( slotResult( KIO::Job * ) ) );
		connect( m_job, TQT_SIGNAL( gotPreview( const KFileItem*,
		                                    const TQPixmap& ) ),
		         TQT_SLOT( gotPreview( const KFileItem*, const TQPixmap& ) ) );

		connect( m_job, TQT_SIGNAL( failed( const KFileItem* ) ),
		         this, TQT_SLOT( slotFailed( const KFileItem* ) ) );
	}
}

void KrusaderImageFilePreview::resizeEvent( TQResizeEvent * ) {
	timer->start( 100, true ); // forces a new preview
}

TQSize KrusaderImageFilePreview::sizeHint() const {
	return TQSize( 20, 200 ); // otherwise it ends up huge???
}

KIO::PreviewJob * KrusaderImageFilePreview::createJob( const KURL& url, int w, int h ) {
	KURL::List urls;
	urls.append( url );
	return KIO::filePreview( urls, w, h, 0, 0, true, false );
}

void KrusaderImageFilePreview::gotPreview( const KFileItem* item, const TQPixmap& pm ) {
	if ( item->url() == currentURL )   // should always be the case
		imageLabel->setPixmap( pm );
}

void KrusaderImageFilePreview::slotFailed( const KFileItem* item ) {
	if ( item->isDir() )
		imageLabel->clear();
	else if ( item->url() == currentURL )   // should always be the case
		imageLabel->setPixmap( SmallIcon( "file_broken", KIcon::SizeLarge,
		                                  KIcon::DisabledState ) );
}

void KrusaderImageFilePreview::slotResult( KIO::Job *job ) {
	if ( job == m_job )
		m_job = 0L;
}

void KrusaderImageFilePreview::clearPreview() {
	if ( m_job ) {
		m_job->kill();
		m_job = 0L;
	}

	imageLabel->clear();
	currentURL = KURL();
}

void KrusaderImageFilePreview::virtual_hook( int id, void* data ) {
	KPreviewWidgetBase::virtual_hook( id, data );
}

#include "kimagefilepreview.moc"
