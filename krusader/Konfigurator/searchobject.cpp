/***************************************************************************
                             searchobject.cpp
                             -------------------
    copyright            : (C) 2005 by Dirk Eschler & Krusader Krew
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

#include "searchobject.h"

SearchObject::SearchObject()
{
}

SearchObject::SearchObject(const TQString& searchName, bool found, const TQString& note)
  : _searchName(searchName),
    _found(found),
    _note(note)
{
}

SearchObject::~SearchObject()
{
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

Application::Application()
{
}

Application::Application(const TQString& searchName, bool found, const TQString& appName, const TQString& website, const TQString& note)
  : SearchObject(searchName, found, note),
    _appName(appName),
    _website(website),
    _path(KrServices::fullPathName(appName))
{
}

Application::Application(const TQString& searchName, const TQString& website, bool found, const TQString& note)
  : SearchObject(searchName, found, note),
    _appName(searchName),
    _website(website),
    _path(KrServices::fullPathName(searchName))
{
}

Application::~Application()
{
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

Archiver::Archiver()
  : Application()
{
}

Archiver::Archiver(const TQString& searchName, const TQString& website, bool found, bool isPacker, bool isUnpacker, const TQString& note)
  : Application(searchName, website, found, note),
    _isPacker(isPacker),
    _isUnpacker(isUnpacker)
{
}

Archiver::~Archiver()
{
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

ApplicationGroup::ApplicationGroup(const TQString& searchName, bool foundGroup, const TQValueVector<Application*>& apps, const TQString& note)
 : SearchObject(searchName, foundGroup, note),
   _apps(apps),
  _foundGroup(foundGroup)
{
}

ApplicationGroup::~ApplicationGroup()
{
}
