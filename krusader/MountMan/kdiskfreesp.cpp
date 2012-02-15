/*
 * kdiskfreesp.cpp
 *
 * Copyright (c) 1999 Michael Kropfberger <michael.kropfberger@gmx.net>
 *
 * Requires the TQt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kdiskfreesp.h"
#include <tqfile.h>
#include <tqtextstream.h>

#include <kdebug.h>
#include <kprocess.h>
#include <kio/global.h>

#include "kdiskfreesp.moc"

#define DF_COMMAND    "df"
#define DF_ARGS       "-k"
#define NO_FS_TYPE    true

#define BLANK ' '
#define FULL_PERCENT 95.0

/***************************************************************************
  * constructor
**/
KDiskFreeSp::KDiskFreeSp(TQObject *parent, const char *name)
    : TQObject(parent,name)
{
    dfProc = new KProcess(); TQ_CHECK_PTR(dfProc);
    dfProc->setEnvironment("LANGUAGE", "C");
    connect( dfProc, TQT_SIGNAL(receivedStdout(KProcess *, char *, int) ),
             this, TQT_SLOT (receivedDFStdErrOut(KProcess *, char *, int)) );
    connect(dfProc,TQT_SIGNAL(processExited(KProcess *) ),
            this, TQT_SLOT(dfDone() ) );

    readingDFStdErrOut=false;
}


/***************************************************************************
  * destructor
**/
KDiskFreeSp::~KDiskFreeSp()
{
    delete dfProc;
}

/***************************************************************************
  * is called, when the df-command writes on StdOut
**/
void KDiskFreeSp::receivedDFStdErrOut(KProcess *, char *data, int len)
{
  TQCString tmp(data,len+1);  // adds a zero-byte
  dfStringErrOut.append(tmp);
}

/***************************************************************************
  * reads the df-commands results
**/
int KDiskFreeSp::readDF( const TQString & mountPoint )
{
  if (readingDFStdErrOut || dfProc->isRunning())
    return -1;
  m_mountPoint = mountPoint;
  dfStringErrOut=""; // yet no data received
  dfProc->clearArguments();
  (*dfProc) << TQString::fromLocal8Bit(DF_COMMAND) << TQString::fromLocal8Bit(DF_ARGS) << mountPoint;
  if (!dfProc->start( KProcess::NotifyOnExit, KProcess::AllOutput ))
     kdError() << "could not execute ["<< DF_COMMAND << "]" << endl;
  return 1;
}


/***************************************************************************
  * is called, when the df-command has finished
**/
void KDiskFreeSp::dfDone()
{
  readingDFStdErrOut=true;

  TQTextStream t (dfStringErrOut, IO_ReadOnly);
  TQString s=t.readLine();
  if ( (s.isEmpty()) || ( s.left(10) != TQString::fromLatin1("Filesystem") ) )
    kdError() << "Error running df command... got [" << s << "]" << endl;
  while ( !t.eof() ) {
    TQString u,v;
    s=t.readLine();
    s=s.simplifyWhiteSpace();
    if ( !s.isEmpty() ) {
      //kdDebug(kfile_area) << "GOT: [" << s << "]" << endl;

      if (s.find(BLANK)<0)      // devicename was too long, rest in next line
	if ( !t.eof() ) {       // just appends the next line
            v=t.readLine();
            s=s.append(v);
            s=s.simplifyWhiteSpace();
            //kdDebug(kfile_area) << "SPECIAL GOT: [" << s << "]" << endl;
	 }//if silly linefeed

      //kdDebug(kfile_area) << "[" << s << "]" << endl;

      //TQString deviceName = s.left(s.find(BLANK));
      s=s.remove(0,s.find(BLANK)+1 );
      //kdDebug(kfile_area) << "    DeviceName:    [" << deviceName << "]" << endl;

      if (!NO_FS_TYPE)
          s=s.remove(0,s.find(BLANK)+1 ); // eat fs type

      u=s.left(s.find(BLANK));
      unsigned long kBSize = u.toULong();
      s=s.remove(0,s.find(BLANK)+1 );
      //kdDebug(kfile_area) << "    Size:       [" << kBSize << "]" << endl;

      u=s.left(s.find(BLANK));
      unsigned long kBUsed = u.toULong();
      s=s.remove(0,s.find(BLANK)+1 );
      //kdDebug(kfile_area) << "    Used:       [" << kBUsed << "]" << endl;

      u=s.left(s.find(BLANK));
      unsigned long kBAvail = u.toULong();
      s=s.remove(0,s.find(BLANK)+1 );
      //kdDebug(kfile_area) << "    Avail:       [" << kBAvail << "]" << endl;


      s=s.remove(0,s.find(BLANK)+1 );  // delete the capacity 94%
      TQString mountPoint = s.stripWhiteSpace();
      //kdDebug(kfile_area) << "    MountPoint:       [" << mountPoint << "]" << endl;

      if ( mountPoint == m_mountPoint )
      {
        //kdDebug(kfile_area) << "Found mount point. Emitting" << endl;
        emit foundMountPoint( mountPoint, kBSize, kBUsed, kBAvail );
        emit foundMountPoint( kBSize, kBUsed, kBAvail, mountPoint ); // sic!
      }
    }//if not header
  }//while further lines available

  readingDFStdErrOut=false;
  emit done();
  delete this;
}

KDiskFreeSp * KDiskFreeSp::findUsageInfo( const TQString & path )
{	
    KDiskFreeSp * job = new KDiskFreeSp;
    TQString mountPoint = KIO::findPathMountPoint( path );
    job->readDF( mountPoint );
    return job;
}
