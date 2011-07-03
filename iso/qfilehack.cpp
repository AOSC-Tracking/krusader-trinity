/***************************************************************************
                          qfilehack.cpp  -  description
                             -------------------
    begin                : Tue Oct 29 2002
    copyright            : (C) 2002 by Szombathelyi Gy�rgy
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

#include "tqfilehack.h"

TQFileHack::TQFileHack(){
}

TQFileHack::TQFileHack( const TQString & name ) : TQFile(name) {
}

TQFileHack::~TQFileHack(){
}

bool TQFileHack::open ( int m ) {
    bool ret;

#ifdef __linux__
    m |= IO_Async; //On linux, set O_NONBLOCK, opens CD-ROMs faster
#endif
    ret=TQFile::open(m);
    if (ret && isSequentialAccess() ) {
        setType(IO_Direct);
    }
    return ret;
}
