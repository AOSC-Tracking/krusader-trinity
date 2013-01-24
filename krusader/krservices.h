/***************************************************************************
                          krservices.h  -  description
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

#ifndef KRSERVICES_H
#define KRSERVICES_H

#include <tqstring.h>
#include <tqstringlist.h>
#include <tqmap.h>
#include <kprocess.h>

/**
  *@author Shie Erlich & Rafi Yanai
  */

class KrServices {
public: 
	KrServices(){}
	~KrServices(){}

	static bool         cmdExist(TQString cmdName);
	static TQString      detectFullPathName( TQString name );
	static TQString      fullPathName( TQString name, TQString confName = TQString() );
	static TQStringList  separateArgs( TQString args );
	static TQString      registerdProtocol(TQString mimetype);
	static void         clearProtocolCache();
	static bool         fileToStringList(TQTextStream *stream, TQStringList& target, bool keepEmptyLines=false);
	static TQString		  quote( TQString name );
	static TQStringList  quote( const TQStringList& names );

protected:
	static TQString 	  escape( TQString name );

private:
	static TQMap<TQString,TQString>* slaveMap;

};


// TODO: make KrServices a namespace and move it there

// wraps over kprocess, but buffers stdout and stderr and allows easy access to them later
// note, that you still have to enable stdout,stderr in KEasyProcess::start() for buffering
// to happen (ie: start(KEasyProcess::Block, KEasyProcess::AllOutput);)
class KEasyProcess: public TDEProcess {
	Q_OBJECT
  
public:
	KEasyProcess(TQObject *parent, const char *name=0);
	KEasyProcess();
	virtual ~KEasyProcess() {}

	const TQString& getStdout() const { return _stdout; }
	const TQString& getStderr() const { return _stderr; }

protected slots:
	void receivedStdout (TDEProcess *proc, char *buffer, int buflen);
	void receivedStderr (TDEProcess *proc, char *buffer, int buflen);
	void init();

private:
	TQString _stdout, _stderr;
};

#endif
