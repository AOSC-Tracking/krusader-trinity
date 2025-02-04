/***************************************************************************
                                krpleasewait.h
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
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

#ifndef KRPLEASEWAIT_H
#define KRPLEASEWAIT_H

#include <tqprogressdialog.h>
#include <tqtimer.h>
#include <tqguardedptr.h>
#include <tdeio/jobclasses.h>

class TDEProcess;
class KRPleaseWait;

class KRPleaseWaitHandler : public TQObject {
  TQ_OBJECT
  

public:
  KRPleaseWaitHandler();

public slots:

  void startWaiting(TQString msg, int count = 0, bool cancel = false);
  void stopWait();
  void cycleProgress();
  void incProgress(int i);
  void incProgress( TDEProcess *, char *buffer, int buflen );
  void killJob();
  void setJob(TDEIO::Job* j);
  bool wasCancelled() const { return _wasCancelled; }

private:
  TQGuardedPtr<TDEIO::Job> job;
  KRPleaseWait * dlg;
  bool cycle, cycleMutex, incMutex, _wasCancelled;
};


class KRPleaseWait : public TQProgressDialog {
  TQ_OBJECT
  
public:
	KRPleaseWait( TQString msg, int count = 0 ,bool cancel = false );
	
public slots:
	void incProgress(int howMuch);
	void cycleProgress();

protected:
	bool inc;
  TQTimer* timer;
  virtual void closeEvent ( TQCloseEvent * e );
  bool canClose;
};

#endif
