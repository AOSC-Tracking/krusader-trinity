/***************************************************************************
                          qfilehack.h  -  description
                             -------------------
    begin                : Tue Oct 29 2002
    copyright            : (C) 2002 by Szombathelyi György
    email                : gyurco@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TQFILEHACK_H
#define TQFILEHACK_H

#include <tqfile.h>
#include <tqstring.h>

/**
  *@author Szombathelyi György
  * TQt thinks if a file is not S_IFREG, you cannot seek in it. It's false (what about
  * block devices for example?
  */

class TQFileHack : public TQFile  {
public: 
    TQFileHack();
    TQFileHack( const TQString & name );
    ~TQFileHack();
    virtual bool open ( int m );
};

#endif
