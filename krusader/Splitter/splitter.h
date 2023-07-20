/***************************************************************************
                          splitter.h  -  description
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

#ifndef __SPLITTER_H__
#define __SPLITTER_H__

#include "crc32.h"
#include <tqstring.h>
#include <tqprogressdialog.h>
#include <kurl.h>
#include <tdeio/jobclasses.h>
 
class Splitter : public TQProgressDialog
{
  TQ_OBJECT
  
  
private:
  KURL            fileName;
  KURL            destinationDir;
  TDEIO::filesize_t splitSize;

  TDEIO::filesize_t fileSize;
  int             permissions;
  TQString         splitFile;

  KURL            writeURL;
  int             fileNumber;
  TDEIO::filesize_t outputFileSize;
  bool            noValidWriteJob;
  CRC32          *crcContext;
  TQByteArray      transferArray;    
  
  TDEIO::TransferJob *splitReadJob;
  TDEIO::TransferJob *splitWriteJob;
    
public:
  Splitter( TQWidget* parent,  KURL fileNameIn, KURL destinationDirIn );
  ~Splitter();
  
  void split( TDEIO::filesize_t splitSizeIn );

private:
  void splitCreateWriteJob();
  void splitAbortJobs();
  
public slots:
  void splitDataReceived(TDEIO::Job *, const TQByteArray &);
  void splitDataSend(TDEIO::Job *, TQByteArray &);
  void splitSendFinished(TDEIO::Job *);
  void splitReceiveFinished(TDEIO::Job *);
  void splitReceivePercent (TDEIO::Job *, unsigned long);
  void splitFileSend(TDEIO::Job *, TQByteArray &);
  void splitFileFinished(TDEIO::Job *);
};

#endif /* __SPLITTER_H__ */
