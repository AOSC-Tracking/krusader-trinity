/***************************************************************************
                              virt.h
                         -------------------
    begin                : Fri Dec 5 2003
    copyright            : (C) 2003 by Shie Erlich & Rafi Yanai
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _VIRT_H
#define _VIRT_H

#include <sys/types.h>
#include <tqdict.h>
#include <kconfig.h>
#include <kio/slavebase.h>

class VirtProtocol : public TDEIO::SlaveBase {
public:
	VirtProtocol( const TQCString &pool, const TQCString &app );
	virtual ~VirtProtocol();

	virtual void listDir ( const KURL & url );
	virtual void stat    ( const KURL & url );
	virtual void get    ( const KURL & url );
	virtual void mkdir(const KURL& url,int permissions);
	virtual void copy( const KURL &src, const KURL &dest, int permissions, bool overwrite );
	virtual void del    (KURL const & url, bool isFile);

protected:
	bool lock();
	bool unlock();
	bool save();
	bool load();

	void local_entry(const KURL& url,TDEIO::UDSEntry& entry);
	bool addDir(TQString& path);


	static TQDict<KURL::List> kioVirtDict;
	static TDEConfig* kio_virt_db;

	bool rewriteURL(const KURL&, KURL&);

};

#endif
