/***************************************************************************
                          combiner.h  -  description
                             -------------------
    copyright            : (C) 2003 by Csaba Karai
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

#ifndef __COMBINER_H__
#define __COMBINER_H__

#include "crc32.h"
#include <tqstring.h>
#include <tqprogressdialog.h>
#include <kurl.h>
#include <tdeio/jobclasses.h>

class Combiner : public TQProgressDialog
{
  TQ_OBJECT
  

private:
  KURL            splURL;
  KURL            readURL;
  KURL            writeURL;

  KURL            baseURL;
  KURL            destinationURL;
  CRC32          *crcContext;
  TQByteArray      transferArray;

  TQString         splitFile;
  TQString         error;


  bool            hasValidSplitFile;
  TQString         expectedFileName;
  TDEIO::filesize_t expectedSize;
  TQString         expectedCrcSum;

  int             fileCounter;
  int             permissions;
  TDEIO::filesize_t receivedSize;
  
  TDEIO::TransferJob *combineReadJob;
  TDEIO::TransferJob *combineWriteJob;

  bool            unixNaming;
  
public:
  Combiner( TQWidget* parent,  KURL baseURLIn, KURL destinationURLIn, bool unixNamingIn=false );
  ~Combiner();

  void combine();

public slots:
  void combineSplitFileDataReceived(TDEIO::Job *, const TQByteArray &byteArray);
  void combineSplitFileFinished(TDEIO::Job *job);
  void combineDataReceived(TDEIO::Job *, const TQByteArray &);
  void combineReceiveFinished(TDEIO::Job *);
  void combineDataSend(TDEIO::Job *, TQByteArray &);
  void combineSendFinished(TDEIO::Job *);
  void combineWritePercent(TDEIO::Job *, unsigned long);

private:
  void openNextFile();
  void combineAbortJobs();
};

#endif /* __COMBINER_H__ */
