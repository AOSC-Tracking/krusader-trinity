/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

/** This file was modified for Krusader by Shie Erlich & Rafi Yanai **/

#ifndef __kr_progress_h__
#define __kr_progress_h__

#include <tqlabel.h>

#include <tdeio/global.h>

#include <kprogress.h>
#include <ksqueezedtextlabel.h>

#include <tdeio/progressbase.h>

#include <tqobject.h>

class KrProgress : public TDEIO::ProgressBase {
  Q_OBJECT
  
public:

  KrProgress(TDEIO::Job* job);
  virtual ~KrProgress();

public slots:
  virtual void slotTotalSize( TDEIO::Job*, TDEIO::filesize_t bytes );
  virtual void slotTotalFiles( TDEIO::Job*, unsigned long files );
  virtual void slotTotalDirs( TDEIO::Job*, unsigned long dirs );

  virtual void slotProcessedSize( TDEIO::Job*, TDEIO::filesize_t bytes );
  virtual void slotProcessedFiles( TDEIO::Job*, unsigned long files );
  virtual void slotProcessedDirs( TDEIO::Job*, unsigned long dirs );

  virtual void slotSpeed( TDEIO::Job*, unsigned long bytes_per_second );
  virtual void slotPercent( TDEIO::Job*, unsigned long percent );
  virtual void slotInfoMessage( TDEIO::Job*, const TQString & msg );
	
	virtual void slotStop();
  virtual void closeEvent( TQCloseEvent* );

protected:
  void showTotals();
  void setDestVisible( bool visible );

  KSqueezedTextLabel* sourceLabel;
  KSqueezedTextLabel* destLabel;
  TQLabel* progressLabel;
  TQLabel* destInvite;
  TQLabel* speedLabel;
  TQLabel* sizeLabel;
  TQLabel* resumeLabel;

  KProgress* m_pProgressBar;

  TDEIO::filesize_t m_iTotalSize;
  unsigned long m_iTotalFiles;
  unsigned long m_iTotalDirs;

  TDEIO::filesize_t m_iProcessedSize;
  unsigned long m_iProcessedDirs;
  unsigned long m_iProcessedFiles;

protected:
  virtual void virtual_hook( int id, void* data );
};


#endif // __kr_progress_h__

