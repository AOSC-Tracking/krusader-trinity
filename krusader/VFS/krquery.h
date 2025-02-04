/***************************************************************************
                                 krquery.h
                             -------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
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
#ifndef KRQUERY_H
#define KRQUERY_H

#include <tqstringlist.h>
#include <tqdatetime.h>
#include <time.h>
#include <kurl.h>
#include <tdeio/jobclasses.h>
#include "vfile.h"

class KFileItem;

class KRQuery : public TQObject {
  TQ_OBJECT
  

public:
  // null query
  KRQuery();
  // query only with name filter
  KRQuery( const TQString &name, bool matchCase = true );
  // copy constructor
  KRQuery( const KRQuery & );
  // let operator
  KRQuery& operator=(const KRQuery &);
  // destructor
  virtual ~KRQuery();

  // matching a file with the query
  bool match( vfile *file ) const;// checks if the given vfile object matches the conditions
  // matching a TDEIO file with the query
  bool match( KFileItem *file ) const;// checks if the given vfile object matches the conditions
  // matching a name with the query
  bool match( const TQString &name ) const;// matching the filename only
  // matching the name of the directory
  bool matchDirName( const TQString &name ) const;

  // sets the text for name filtering
  void setNameFilter( const TQString &text, bool cs=true );
  // returns the current filter mask
  const TQString& nameFilter() const { return origFilter; }
  // returns whether the filter is case sensitive
  bool isCaseSensitive() { return matchesCaseSensitive; }

  // returns if the filter is null (was cancelled)
  bool isNull() {return bNull;};

  // sets the content part of the query
  void setContent( const TQString &content, bool cs=true, bool wholeWord=false, bool remoteSearch=false );

  // sets the minimum file size limit
  void setMinimumFileSize( TDEIO::filesize_t );
  // sets the maximum file size limit
  void setMaximumFileSize( TDEIO::filesize_t );

  // sets the time the file newer than
  void setNewerThan( time_t time );
  // sets the time the file older than
  void setOlderThan( time_t time );

  // sets the owner
  void setOwner( const TQString &ownerIn );
  // sets the group
  void setGroup( const TQString &groupIn );
  // sets the permissions
  void setPermissions( const TQString &permIn );

  // sets the mimetype for the query
  // type, must be one of the following:
  // 1. a valid mime type name
  // 2. one of: i18n("Archives"),   i18n("Directories"), i18n("Image Files")
  //            i18n("Text Files"), i18n("Video Files"), i18n("Audio Files")
  // 3. i18n("Custom") in which case you must supply a list of valid mime-types
  //    in the member TQStringList customType
  void setMimeType( const TQString &typeIn, TQStringList customList = TQStringList() );
  // true if setMimeType was called
  bool hasMimeType()  { return type.isEmpty(); }

  // sets the search in archive flag
  void setSearchInArchives( bool flag ) { inArchive = flag; }
  // gets the search in archive flag
  bool searchInArchives() { return inArchive; }
  // sets the recursive flag
  void setRecursive( bool flag ) { recurse = flag; }
  // gets the recursive flag
  bool isRecursive() { return recurse; }
  // sets whether to follow symbolic links
  void setFollowLinks( bool flag ) { followLinksP = flag; }
  // gets whether to follow symbolic links
  bool followLinks() { return followLinksP; }

  // sets the folders where the searcher will search
  void setSearchInDirs( const KURL::List &urls );
  // gets the folders where the searcher searches
  const KURL::List & searchInDirs() { return whereToSearch; }
  // sets the folders where search is not permitted
  void setDontSearchInDirs( const KURL::List &urls );
  // gets the folders where search is not permitted
  const KURL::List & dontSearchInDirs() { return whereNotToSearch; }
  // checks if a URL is excluded
  bool isExcluded( const KURL &url );
  // gives whether we search for content
  bool isContentSearched() const { return !contain.isEmpty(); }
  
  const TQString& foundText() const { return lastSuccessfulGrep; }

protected:
  // important to know whether the event processor is connected
  virtual void connectNotify ( const char * signal );
  // important to know whether the event processor is connected
  virtual void disconnectNotify ( const char * signal );

protected:
  TQStringList matches;           // what to search
  TQStringList excludes;          // what to exclude
  TQStringList includedDirs;      // what dirs to include
  TQStringList excludedDirs;      // what dirs to exclude
  bool matchesCaseSensitive;

  bool bNull;                    // flag if the query is null

  TQString contain;               // file must contain this string
  bool containCaseSensetive;
  bool containWholeWord;
  bool containOnRemote;

  TDEIO::filesize_t minSize;
  TDEIO::filesize_t maxSize;

  time_t newerThen;
  time_t olderThen;

  TQString owner;
  TQString group;
  TQString perm;

  TQString type;
  TQStringList customType;

  bool inArchive;                // if true- search in archive.
  bool recurse;                  // if true recurse ob sub-dirs...
  bool followLinksP;

  KURL::List whereToSearch;     // directorys to search
  KURL::List whereNotToSearch;  // directorys NOT to search

signals:
  void status( const TQString &name );
  void processEvents( bool & stopped );

private:
  bool matchCommon( const TQString &, const TQStringList &, const TQStringList & ) const;
  bool checkPerm(TQString perm) const;
  bool checkType(TQString mime) const;
  bool containsContent( TQString file ) const;
  bool containsContent( KURL url ) const;
  bool checkBuffer( const char *buffer, int len ) const;
  bool checkLines( const char *buffer, int len ) const;
  bool checkTimer() const;
  TQStringList split( TQString );

private slots:
  void containsContentData(TDEIO::Job *, const TQByteArray &);
  void containsContentFinished(TDEIO::Job*);

private:
  TQString                  origFilter;
  mutable bool             busy;
  mutable bool             containsContentResult;
  mutable char *           receivedBuffer;
  mutable int              receivedBufferLen;
  mutable TQString          lastSuccessfulGrep;
  mutable TQString          fileName;
  mutable TDEIO::filesize_t  receivedBytes;
  mutable TDEIO::filesize_t  totalBytes;
  mutable int              processEventsConnected;
  mutable TQTime            timer;
};

#endif
