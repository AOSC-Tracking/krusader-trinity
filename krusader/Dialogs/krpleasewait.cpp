/***************************************************************************
                                krpleasewait.cpp
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

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "krpleasewait.h"
#include <tqtimer.h>
#include <tqdatetime.h>
#include <tqapplication.h>
#include <tqpushbutton.h>
#include <unistd.h>
#include "../krusader.h"
#include "tdelocale.h"
#include <kcursor.h>

KRPleaseWait::KRPleaseWait( TQString msg, int count, bool cancel ):
	TQProgressDialog( cancel ? 0 : krApp,0, !cancel) , inc(true) {
	
	timer = new TQTimer(this);
	setCaption( i18n( "Krusader::Wait" ) );

	setMinimumDuration(500);
  setAutoClose(false);
	setAutoReset(false);
	
	connect( timer,TQ_SIGNAL(timeout()), this, TQ_SLOT(cycleProgress()));

  TQProgressBar* progress = new TQProgressBar(count,this);
  progress->setCenterIndicator(true);
	setBar(progress);

	TQLabel* label = new TQLabel(this);
  setLabel(label);

	TQPushButton* btn = new TQPushButton(i18n("&Cancel"),this);
  setCancelButton(btn);

	btn->setEnabled(canClose = cancel);
  setLabelText(msg);

	show();
}

void KRPleaseWait::closeEvent ( TQCloseEvent * e )
{
  if( canClose ) {
    emit cancelled();
    e->accept();
  } else              /* if cancel is not allowed, we disable */
    e->ignore();         /* the window closing [x] also */
}

void KRPleaseWait::incProgress(int howMuch){
	setProgress(progress()+howMuch);
}

void KRPleaseWait::cycleProgress(){
  if (inc) setProgress(progress()+1);
  else     setProgress(progress()-1);
	if ( progress() >= 9 ) inc = false;
  if ( progress() <= 0 ) inc = true;
}

KRPleaseWaitHandler::KRPleaseWaitHandler() : TQObject(), job(), dlg( 0 ) {
}

void KRPleaseWaitHandler::stopWait(){
  if( dlg != 0 ) delete dlg;
  dlg=0;
  cycleMutex=incMutex=false;
	// return cursor to normal arrow
	krApp->setCursor(KCursor::arrowCursor());
}


void KRPleaseWaitHandler::startWaiting( TQString msg, int count , bool cancel){
  if ( dlg == 0 ){
    dlg = new KRPleaseWait( msg , count, cancel);
		connect( dlg,TQ_SIGNAL(cancelled()),this,TQ_SLOT(killJob()) );
  }
  incMutex=cycleMutex=_wasCancelled=false;
  dlg->setProgress(0);
	
	dlg->setLabelText(msg);
	if ( count == 0) {
		dlg->setTotalSteps(10);
		cycle = true ;
		cycleProgress();
	}
  else {
		dlg->setTotalSteps(count);
  	cycle = false;
	}
}

void KRPleaseWaitHandler::cycleProgress(){
  if (cycleMutex) return;
  cycleMutex=true;
  if (dlg) dlg->cycleProgress();
  if (cycle) TQTimer::singleShot(2000,this,TQ_SLOT(cycleProgress()));
  cycleMutex=false;
}

void KRPleaseWaitHandler::killJob(){
	if( !job.isNull() ) job->kill(false);
	stopWait();
	_wasCancelled = true;
}

void KRPleaseWaitHandler::setJob(TDEIO::Job* j){ job=j; }

void KRPleaseWaitHandler::incProgress(int i){
  if (incMutex) return;
  incMutex=true;
  if(dlg) dlg->incProgress(i);
  incMutex=false;
}

void KRPleaseWaitHandler::incProgress( TDEProcess *, char *buffer, int buflen ) {
   int howMuch = 0;
   for ( int i = 0 ; i < buflen; ++i )
      if ( buffer[ i ] == '\n' )
         ++howMuch;

   incProgress( howMuch );
}

#include "krpleasewait.moc"
