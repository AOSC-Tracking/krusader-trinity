/*  This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _TAR_H
#define _TAR_H

#include <tdeio/slavebase.h>
#include <sys/types.h>

class ArchiveProtocol : public TDEIO::SlaveBase {
public:
	ArchiveProtocol( const TQCString &pool, const TQCString &app );
	virtual ~ArchiveProtocol();

	virtual void listDir( const KURL & url );
	virtual void stat( const KURL & url );
	virtual void get( const KURL & url );
	virtual void put( const KURL& url, int permissions, bool overwrite, bool resume );
	virtual void mkdir(const KURL& url,int permissions);

protected:
	void createUDSEntry( const KArchiveEntry * tarEntry, TDEIO::UDSEntry & entry );
	bool checkNewFile( const KURL & url, TQString & path );

	KArchive * m_archiveFile;
	TQString m_archiveName;
	TQString user;
	TQString group;
	time_t m_mtime;
};

#endif
