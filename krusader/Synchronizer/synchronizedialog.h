/***************************************************************************
                      synchronizedialog.h  -  description
                             -------------------
    copyright            : (C) 2003 + by Csaba Karai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __SYNCHRONIZE_DIALOG__
#define __SYNCHRONIZE_DIALOG__

#include "../VFS/vfs.h"
#include "synchronizer.h"
#include <tqdialog.h>
#include <tqpushbutton.h>
#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqprogressbar.h>

class SynchronizeDialog : TQDialog
{
  TQ_OBJECT
  
  
  public:
    SynchronizeDialog(  TQWidget*,  const char*, bool, WFlags, Synchronizer *sync,
                        int, TDEIO::filesize_t, int, TDEIO::filesize_t, int, TDEIO::filesize_t, int);
    ~SynchronizeDialog();

    inline bool wasSyncronizationStarted()    { return syncStarted; }
    
  public slots:
    void startSynchronization();
    void synchronizationFinished();
    void processedSizes( int, TDEIO::filesize_t, int, TDEIO::filesize_t, int, TDEIO::filesize_t);
    void pauseOrResume();
    void pauseAccepted();

  private:
    TQProgressBar  *progress;
    
    TQCheckBox     *cbRightToLeft;
    TQCheckBox     *cbLeftToRight;
    TQCheckBox     *cbDeletable;

    TQLabel        *lbRightToLeft;
    TQLabel        *lbLeftToRight;
    TQLabel        *lbDeletable;

    TQCheckBox     *cbOverwrite;
    
    TQPushButton   *btnStart;
    TQPushButton   *btnPause;
    
    Synchronizer  *synchronizer;

    int               leftCopyNr;
    TDEIO::filesize_t   leftCopySize;
    int               rightCopyNr;
    TDEIO::filesize_t   rightCopySize;
    int               deleteNr;
    TDEIO::filesize_t   deleteSize;

    int               parallelThreads;

    bool           isPause;
    bool           syncStarted;
};

#endif /* __SYNCHRONIZE_DIALOG__ */
