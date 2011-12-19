/*
*
* This file is part of the KDE project.
* Copyright (C) 2001 Martin R. Jones <mjones@kde.org>
*               2001 Carsten Pfeiffer <pfeiffer@kde.org>
*
* Modified for Krusader by Shie Erlich, October 2004
*
* You can Freely distribute this program under the GNU Library General Public
* License. See the file "COPYING" for the exact licensing terms.
*/

#ifndef KrusaderImageFilePreview_H
#define KrusaderImageFilePreview_H

#include <tqpixmap.h>

#include <kurl.h>
#include <kpreviewwidgetbase.h>

class TQCheckBox;
class TQPushButton;
class TQLabel;
class TQTimer;

class KFileDialog;
class KFileItem;

class KrusaderImageFilePreview : public KPreviewWidgetBase {
		Q_OBJECT
  TQ_OBJECT

	public:
		KrusaderImageFilePreview( TQWidget *parent );
		~KrusaderImageFilePreview();

		virtual TQSize sizeHint() const;

	public slots:
		virtual void showPreview( const KURL &url );
		virtual void clearPreview();

	protected slots:
		void showPreview();
		void showPreview( const KURL& url, bool force );

		virtual void gotPreview( const KFileItem*, const TQPixmap& );

	protected:
		virtual void resizeEvent( TQResizeEvent *e );
		virtual KIO::PreviewJob * createJob( const KURL& url,
		                                     int w, int h );

	private slots:
		void slotResult( KIO::Job * );
		virtual void slotFailed( const KFileItem* );

	private:
		KURL currentURL;
		TQTimer *timer;
		TQLabel *imageLabel;
		TQLabel *infoLabel;
		KIO::PreviewJob *m_job;
	protected:
		virtual void virtual_hook( int id, void* data );
	private:
		class KrusaderImageFilePreviewPrivate;
		KrusaderImageFilePreviewPrivate *d;
};

#endif // KrusaderImageFilePreview_H
