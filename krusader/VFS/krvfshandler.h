/***************************************************************************
                          krvfshandler.h  -  description
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

#ifndef KRVFSHANDLER_H
#define KRVFSHANDLER_H

#include <tqobject.h>

#include <kurl.h>

#include "vfs.h"

/**
  *@author Shie Erlich & Rafi Yanai
  */

class KrVfsHandler : public TQObject  {
public: 
	KrVfsHandler();
	~KrVfsHandler();

  static vfs::VFS_TYPE getVfsType(const KURL& url);
  static vfs* getVfs(const KURL& url,TQObject* tqparent=0,vfs* oldVfs=0);
};

#endif
