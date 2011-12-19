/***************************************************************************
                          krservices.cpp  -  description
                             -------------------
    begin                : Thu Aug 8 2002
    copyright            : (C) 2002 by Shie Erlich & Rafi Yanai
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

#include <stdlib.h>
#include <unistd.h>
// QT includes
#include <tqdir.h>
// KDE includes
#include <kdebug.h>
// Krusader includes
#include "krservices.h"
#include "krusader.h"

TQMap<TQString,TQString>* KrServices::slaveMap=0;

bool KrServices::cmdExist(TQString cmdName)
{
  TQString lastGroup = krConfig->group();

  krConfig->setGroup( "Dependencies" );
  if( TQFile( krConfig->readEntry( cmdName, TQString() )).exists() )
  {
    krConfig->setGroup( lastGroup );
    return true;
  }

  krConfig->setGroup( lastGroup );
  return !detectFullPathName( cmdName ).isEmpty();  
}

TQString KrServices::detectFullPathName(TQString name)
{
  TQStringList path = TQStringList::split(":",getenv("PATH"));

  for ( TQStringList::Iterator it = path.begin(); it != path.end(); ++it )
  {
    if( TQDir(*it).exists( name ) )
    {
      TQString dir = *it;
      if( !dir.endsWith( "/" ) )
        dir+="/";
        
      return dir+name;
    }
  }

  return "";
}

TQString KrServices::fullPathName( TQString name, TQString confName )
{
  TQString lastGroup = krConfig->group();
  TQString supposedName;

  if( confName.isNull() )
    confName = name;

  krConfig->setGroup( "Dependencies" );
  if( TQFile( supposedName = krConfig->readEntry( confName, "" )).exists() )
  {
    krConfig->setGroup( lastGroup );
    return supposedName;
  }

  if( ( supposedName = detectFullPathName( name ) ).isEmpty() )
  {
    krConfig->setGroup( lastGroup );
    return "";
  }

  krConfig->writeEntry( confName, supposedName );
  krConfig->setGroup( lastGroup );
  return supposedName;
}

// TODO: Document me!
TQStringList KrServices::separateArgs( TQString args )
{
  TQStringList argList;
  int   pointer = 0, tokenStart, len = args.length();
  bool  quoted = false;
  TQChar quoteCh;

  do{
      while( pointer < len && args[ pointer ].isSpace() )
        pointer++;

      if( pointer >= len )
        break;

      tokenStart = pointer;

      TQString result="";
      
      for(; pointer < len && ( quoted || !args[ pointer ].isSpace()) ; pointer++)
      {
        if( !quoted && ( args[pointer] == '"' || args[pointer] == '\'' ) ) {
          quoted = true, quoteCh = args[pointer];
          continue;
        }
        else if( quoted && args[pointer] == quoteCh ) {
          quoted = false;
          continue;
        }
        else if( !quoted && args[pointer] == '\\' )
        {
          pointer++;
          if(pointer>=len) break;
        }

        result += args[pointer];        
      }

      argList.append( result );
      
    }while( pointer < len );
    
  return argList;
}

TQString KrServices::registerdProtocol(TQString mimetype){
	if( slaveMap == 0 ){
		slaveMap = new TQMap<TQString,TQString>();
		
		krConfig->setGroup( "Protocols" );
		TQStringList protList = krConfig->readListEntry( "Handled Protocols" );
		for( TQStringList::Iterator it = protList.begin(); it != protList.end(); it++ ){
			TQStringList mimes = krConfig->readListEntry( TQString( "Mimes For %1" ).arg( *it ) );
			for( TQStringList::Iterator it2 = mimes.begin(); it2 != mimes.end(); it2++ )
				(*slaveMap)[*it2] = *it;
  		}
		
		
	}
	return (*slaveMap)[mimetype];
}

void KrServices::clearProtocolCache()
{
  if( slaveMap )
    delete slaveMap;
  slaveMap = 0;
}

bool KrServices::fileToStringList(TQTextStream *stream, TQStringList& target, bool keepEmptyLines) {
	if (!stream) return false;
	TQString line;
	while ( !stream->atEnd() ) {
		line = stream->readLine().stripWhiteSpace();
		if (keepEmptyLines || !line.isEmpty()) target.append(line);
	}
	return true;
}

TQString KrServices::quote( TQString name ) {
  if( !name.contains( '\'' ) )
    return "'" + name + "'";
  if( !name.contains( '"' ) && !name.contains( '$' ) )
    return "\"" + name + "\"";
  return escape( name );
}

TQStringList KrServices::quote( const TQStringList& names ) {
	TQStringList result;
	for (unsigned i=0; i<names.size(); ++i)
		result.append(quote(names[i]));
	return result;
}

TQString KrServices::escape( TQString name ) {
  const TQString evilstuff = "\\\"'`()[]{}!?;$&<>| \t\r\n";		// stuff that should get escaped
     
    for ( unsigned int i = 0; i < evilstuff.length(); ++i )
        name.replace( evilstuff[ i ], (TQString("\\") + evilstuff[ i ]) );

  return name;
}


// ------- KEasyProcess
KEasyProcess::KEasyProcess(TQObject *parent, const char *name): KProcess(parent, name) {
	init();
}

KEasyProcess::KEasyProcess(): KProcess() {
	init();
}

void KEasyProcess::init() {
	connect(this, TQT_SIGNAL(receivedStdout(KProcess *, char *, int)),
		this, TQT_SLOT(receivedStdout(KProcess *, char *, int)));
	connect(this, TQT_SIGNAL(receivedStderr(KProcess *, char *, int)),
		this, TQT_SLOT(receivedStderr(KProcess *, char *, int)));
}

void KEasyProcess::receivedStdout (KProcess * /* proc */, char *buffer, int buflen) {
	_stdout+=TQString::fromLocal8Bit(buffer, buflen);
}

void KEasyProcess::receivedStderr (KProcess * /* proc */, char *buffer, int buflen) {
	_stderr+=TQString::fromLocal8Bit(buffer, buflen);
}
