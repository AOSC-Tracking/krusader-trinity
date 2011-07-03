/***************************************************************************
                             searchobject.h
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

#ifndef SEARCHOBJECT_H
#define SEARCHOBJECT_H

#include <tqstring.h>
#include <tqvaluevector.h>
#include "../krservices.h"

/**
@author Dirk Eschler <deschler@users.sourceforge.net>
*/
class SearchObject
{
public:
  SearchObject();
  SearchObject(const TQString& name, bool found, const TQString& note);
  virtual ~SearchObject();

  const TQString& getSearchName() const { return _searchName; }
  const TQString& getNote() const { return _note; }
  const bool getFound() const { return _found; }
  void setSearchName(const TQString& s) { _searchName = s; }
  void setNote(const TQString& s) { _note = s; }
  void setFound(const bool& b) { _found = b; }

protected:
  TQString _searchName;
  bool _found;
  TQString _note;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
@author Dirk Eschler <deschler@users.sourceforge.net>
*/
class Application : public SearchObject
{
public:
  Application();
  Application(const TQString& searchName, bool found, const TQString& appName, const TQString& website=TQString(), const TQString& note=TQString());
  Application(const TQString& searchName, const TQString& website, bool found, const TQString& note=TQString());
  virtual ~Application();

  const TQString& getWebsite() const { return _website; }
  const TQString& getAppName() const { return _appName; }
  const TQString& getPath() const { return _path; }
  void setWebsite(const TQString& s) { _website = s; }
  void setAppName(const TQString& s) { _appName = s; }
  void setPath(const TQString& s) { _path = s; }

protected:
  TQString _appName;
  TQString _website;
  TQString _path;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
@author Dirk Eschler <deschler@users.sourceforge.net>
*/
class Archiver : public Application
{
public:
  Archiver();
  Archiver(const TQString& searchName, const TQString& website, bool found, bool isPacker, bool isUnpacker, const TQString& note=TQString());
  ~Archiver();

  const bool getIsPacker() const { return _isPacker; }
  const bool getIsUnpacker() const { return _isUnpacker; }
  void setIsPacker(const bool& b) { _isPacker = b; }
  void setIsUnpacker(const bool& b) { _isUnpacker = b; }

protected:
  bool _isPacker;
  bool _isUnpacker;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
@author Dirk Eschler <deschler@users.sourceforge.net>
*/
class ApplicationGroup : public SearchObject
{
public:
  ApplicationGroup(const TQString& searchName, bool foundGroup, const TQValueVector<Application*>& apps, const TQString& note=TQString());
  ~ApplicationGroup();

  const TQValueVector<Application*>& getAppVec() const { return _apps; }
  const bool getFoundGroup() const { return _foundGroup; }

protected:
  TQValueVector<Application*> _apps;
  bool _foundGroup;
};

#endif
