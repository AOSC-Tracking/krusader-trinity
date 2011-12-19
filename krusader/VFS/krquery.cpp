/***************************************************************************
                                 krquery.cpp
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

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#include "krquery.h"
#include "../krusader.h"
#include "../resources.h"
#include "vfs.h"
#include "krarchandler.h"
#include "krpermhandler.h"

#include <tqtextstream.h>
#include <tqtextcodec.h>
#include <tqregexp.h>
#include <klargefile.h>
#include <klocale.h>
#include <kmimetype.h>
#include <tqfile.h>
#include <kurlcompletion.h>
#include <kio/job.h>
#include <kfileitem.h>

#define  STATUS_SEND_DELAY     250
#define  MAX_LINE_LEN          500

// set the defaults
KRQuery::KRQuery(): TQObject(), matchesCaseSensitive(true), bNull( true ),
                    contain(TQString()),containCaseSensetive(true),
                    containWholeWord(false),containOnRemote(false),
                    minSize(0),maxSize(0),newerThen(0),olderThen(0),
                    owner(TQString()),group(TQString()),perm(TQString()),
                    type(TQString()),inArchive(false),recurse(true),
                    followLinksP(true), receivedBuffer( 0 ), processEventsConnected(0)  {}

// set the defaults
KRQuery::KRQuery( const TQString &name, bool matchCase ) : TQObject(),
                    bNull( true ),contain(TQString()),containCaseSensetive(true),
                    containWholeWord(false), containOnRemote(false),
                    minSize(0),maxSize(0),newerThen(0),olderThen(0),
                    owner(TQString()),group(TQString()),perm(TQString()),
                    type(TQString()),inArchive(false),recurse(true),
                    followLinksP(true), receivedBuffer( 0 ), processEventsConnected(0) {
  setNameFilter( name, matchCase );
}

KRQuery::KRQuery( const KRQuery & that ) : TQObject(), receivedBuffer( 0 ), processEventsConnected(0) {
  *this = that;
}

KRQuery::~KRQuery() {
  if( receivedBuffer )
    delete []receivedBuffer;
  receivedBuffer = 0;
}

KRQuery& KRQuery::operator=(const KRQuery &old) {
  matches = old.matches;
  excludes = old.excludes;
  includedDirs = old.includedDirs;
  excludedDirs = old.excludedDirs;
  matchesCaseSensitive = old.matchesCaseSensitive;
  bNull = old.bNull;
  contain = old.contain;
  containCaseSensetive = old.containCaseSensetive;
  containWholeWord = old.containWholeWord;
  containOnRemote = old.containOnRemote;
  minSize = old.minSize;
  maxSize = old.maxSize;
  newerThen = old.newerThen;
  olderThen = old.olderThen;
  owner = old.owner;
  group = old.group;
  perm = old.perm;
  type = old.type;
  customType = old.customType;
  inArchive = old.inArchive;
  recurse = old.recurse;
  followLinksP = old.followLinksP;
  whereToSearch = old.whereToSearch;
  whereNotToSearch = old.whereNotToSearch;
  origFilter = old.origFilter;
  
  return *this;
}

void KRQuery::connectNotify ( const char * signal ) {
  TQString signalString  = TQString( signal ).replace( " ", "" );
  TQString processString = TQString( TQT_SIGNAL( processEvents( bool & ) ) ).replace( " ", "" );
  if( signalString == processString )
    processEventsConnected++;
}

void KRQuery::disconnectNotify ( const char * signal ) {
  TQString signalString  = TQString( signal ).replace( " ", "" );
  TQString processString = TQString( TQT_SIGNAL( processEvents( bool & ) ) ).replace( " ", "" );
  if( signalString == processString )
    processEventsConnected--;
}

bool KRQuery::checkPerm( TQString filePerm ) const
{
  for ( int i = 0; i < 9; ++i )
    if ( perm[ i ] != '?' && perm[ i ] != filePerm[ i + 1 ] ) return false;
  return true;
}

bool KRQuery::checkType( TQString mime ) const
{
  if ( type == mime ) return true;
  if ( type == i18n( "Archives" ) ) return KRarcHandler::arcSupported( mime.right( 4 ) );
  if ( type == i18n( "Directories" ) ) return mime.contains( "directory" );
  if ( type == i18n( "Image Files" ) ) return mime.contains( "image/" );
  if ( type == i18n( "Text Files" ) ) return mime.contains( "text/" );
  if ( type == i18n( "Video Files" ) ) return mime.contains( "video/" );
  if ( type == i18n( "Audio Files" ) ) return mime.contains( "audio/" );
  if ( type == i18n( "Custom" ) ) return customType.contains( mime );
  return false;
}

bool KRQuery::match( const TQString & name ) const {
  return matchCommon( name, matches, excludes );
}

bool KRQuery::matchDirName( const TQString & name ) const {
  return matchCommon( name, includedDirs, excludedDirs );
}

bool KRQuery::matchCommon( const TQString &nameIn, const TQStringList &matchList, const TQStringList &excludeList ) const
{
  if( excludeList.count() == 0 && matchList.count() == 0 ) /* true if there's no match condition */
    return true;

  TQString name( nameIn );
  int ndx = nameIn.findRev( '/' ); // virtual filenames may contain '/'
  if( ndx != -1 )                  // but the end of the filename is OK
    name = nameIn.mid( ndx + 1 );

  unsigned int len;
  for ( unsigned int i = 0; i < excludeList.count(); ++i )
  {
    TQRegExp( *excludeList.at( i ), matchesCaseSensitive, true ).match( name, 0, ( int* ) & len );
    if ( len == name.length() ) return false;
  }

  if( matchList.count() == 0 )
    return true;

  for ( unsigned int i = 0; i < matchList.count(); ++i )
  {
    TQRegExp( *matchList.at( i ), matchesCaseSensitive, true ).match( name, 0, ( int* ) & len );
    if ( len == name.length() ) return true;
  }
  return false;
}

bool KRQuery::match( vfile *vf ) const
{
  if( vf->vfile_isDir() && !matchDirName( vf->vfile_getName() ) ) return false;
  // see if the name matches
  if ( !match( vf->vfile_getName() ) ) return false;
  // checking the mime
  if( !type.isEmpty() && !checkType( vf->vfile_getMime( true ) ) ) return false;
  // check that the size fit
  KIO::filesize_t size = vf->vfile_getSize();
  if ( minSize && size < minSize ) return false;
  if ( maxSize && size > maxSize ) return false;
  // check the time frame
  time_t mtime = vf->vfile_getTime_t();
  if ( olderThen && mtime > olderThen ) return false;
  if ( newerThen && mtime < newerThen ) return false;
  // check owner name
  if ( !owner.isEmpty() && vf->vfile_getOwner() != owner ) return false;
  // check group name
  if ( !group.isEmpty() && vf->vfile_getGroup() != group ) return false;
  //check permission
  if ( !perm.isEmpty() && !checkPerm( vf->vfile_getPerm() ) ) return false;

  if ( !contain.isEmpty() )
  {
    if( (totalBytes = vf->vfile_getSize()) == 0 )
      totalBytes++; // sanity
    receivedBytes = 0;
    if( receivedBuffer ) {
      delete []receivedBuffer;
      receivedBuffer = 0;
    }
    fileName = vf->vfile_getName();
    timer.start();

    if( vf->vfile_getUrl().isLocalFile() )
    {
      if( !containsContent( vf->vfile_getUrl().path() ) ) return false;
    }
    else
    {
      if( containOnRemote ) {
        if( processEventsConnected == 0 ) return false;
        if( !containsContent( vf->vfile_getUrl() ) ) return false;
      }
    }
  }

  return true;
}

bool KRQuery::match( KFileItem *kfi ) const {
  mode_t mode = kfi->mode() | kfi->permissions();
  TQString perm = KRpermHandler::mode2TQString( mode );
  if ( kfi->isDir() ) 
    perm[ 0 ] = 'd';

  vfile temp( kfi->text(), kfi->size(), perm, kfi->time( KIO::UDS_MODIFICATION_TIME ),
              kfi->isLink(), kfi->user(), kfi->group(), kfi->user(), 
              kfi->mimetype(), kfi->linkDest(), mode );

  return match( &temp );
}

// takes the string and adds BOLD to it, so that when it is displayed,
// the grepped text will be bold
void fixFoundTextForDisplay(TQString& haystack, int start, int length) {
  TQString before = haystack.left( start );
  TQString text = haystack.mid( start, length );
  TQString after = haystack.mid( start + length );

  before.replace( '&', "&amp;" );
  before.replace( '<', "&lt;" );
  before.replace( '>', "&gt;" );

  text.replace( '&', "&amp;" );
  text.replace( '<', "&lt;" );
  text.replace( '>', "&gt;" );

  after.replace( '&', "&amp;" );
  after.replace( '<', "&lt;" );
  after.replace( '>', "&gt;" );

  haystack = ("<qt>"+before+"<b>"+text+"</b>"+after+"</qt>" );
}

bool KRQuery::checkBuffer( const char *buf, int len ) const {
  if( len == 0 )  { // last block?
    if( receivedBuffer ) {
      bool result = checkLines( receivedBuffer, receivedBufferLen );
      delete []receivedBuffer;
      receivedBuffer = 0;
      return result;
    }
    return false;
  }

  int after = len;
  while( buf[ after-1 ] != '\n' && buf[ after-1 ] != 0 ) {
    after--;
    if( after <= 0 || after <= len - MAX_LINE_LEN ) {
      after = len;  // if there's no <ENTER> in MAX_LINE_LEN, we break the line
      break;        // breaking the line causes problems at Unicode characters
    }
  }

  if( receivedBuffer ) {
    int previous = 0;
    while( previous < after && previous < MAX_LINE_LEN && buf[previous] != '\n' && buf[previous] != 0 )
      previous++;

    char * str = new char[ receivedBufferLen + previous ];
    memcpy( str, receivedBuffer, receivedBufferLen );
    delete []receivedBuffer;
    receivedBuffer = 0;
    memcpy( str + receivedBufferLen, buf, previous );

    if( checkLines( str, receivedBufferLen+previous ) ) {
      delete []str;
      return true;
    }
    delete []str;

    if( after > previous && checkLines( buf+previous, after-previous ) )
      return true; 

  } else if( checkLines( buf, after ) )
      return true;

  if( after < len ) {
    receivedBufferLen = len-after;
    receivedBuffer = new char [ receivedBufferLen ];
    memcpy( receivedBuffer, buf+after, receivedBufferLen );
  }
  return false;
}

bool KRQuery::checkLines( const char * buf, int len ) const
{
  TQStringList list;

  int start = 0;
  int k = 0;
  while( k < len )
  {
    if( buf[ k ] == 0 || buf[ k ] == '\n' )
    {
      if( k != start )
      {
        TQString line = TQTextCodec::codecForLocale()->toUnicode( buf + start, k - start );
        if( !line.isEmpty() )
          list << line;
      }
      start = k + 1;
    }
    k++;
  }
  if( start != k )
  {
    TQString line = TQTextCodec::codecForLocale()->toUnicode( buf + start, k - start );
    if( !line.isEmpty() )
      list << line;
  }

  for( unsigned int i=0; i != list.count(); i++ ) {
    TQString line = list[ i ];

    int ndx = 0;
    if ( line.isNull() ) return false;
    if ( containWholeWord )
    {
      while ( ( ndx = line.find( contain, ndx, containCaseSensetive ) ) != -1 )
      {
        TQChar before = line.at( ndx - 1 );
        TQChar after = line.at( ndx + contain.length() );

        if ( !before.isLetterOrNumber() && !after.isLetterOrNumber() &&
          after != '_' && before != '_' ) {
            lastSuccessfulGrep = line;
            fixFoundTextForDisplay(lastSuccessfulGrep, ndx, contain.length());
            return true;
           }
        ndx++;
      }
    }
    else if ( (ndx = line.find( contain, 0, containCaseSensetive )) != -1 ) {
      lastSuccessfulGrep = line;
      fixFoundTextForDisplay(lastSuccessfulGrep, ndx, contain.length());
      return true;
    }
  }
  return false;
}

bool KRQuery::containsContent( TQString file ) const
{
  TQFile qf( file );
  if( !qf.open( IO_ReadOnly ) )
    return false;

  char buffer[ 1440 ]; // 2k buffer

  while ( !qf.atEnd() )
  {
    int bytes = qf.readBlock( buffer, sizeof( buffer ) );
    if( bytes <= 0 )
      break;

    receivedBytes += bytes;

    if( checkBuffer( buffer, bytes ) )
      return true;

    if( checkTimer() ) {
      bool stopped = false;
      emit ((KRQuery *)this)->processEvents( stopped );
      if( stopped )
        return false;
    }
  }
  if( checkBuffer( buffer, 0 ) )
    return true;

  lastSuccessfulGrep = TQString(); // nothing was found
  return false;
}

bool KRQuery::containsContent( KURL url ) const
{
  KIO::TransferJob *contentReader = KIO::get( url, false, false );
  connect(contentReader, TQT_SIGNAL(data(KIO::Job *, const TQByteArray &)),
          this, TQT_SLOT(containsContentData(KIO::Job *, const TQByteArray &)));
  connect(contentReader, TQT_SIGNAL( result( KIO::Job* ) ),
          this, TQT_SLOT(containsContentFinished( KIO::Job* ) ) );

  busy = true;
  containsContentResult = false;
  bool stopped = false;

  while( busy && !stopped ) {
    checkTimer();
    emit ((KRQuery *)this)->processEvents( stopped );
  }

  if( busy ) {
    contentReader->kill();
    busy = false;
  }

  return containsContentResult;
}

void KRQuery::containsContentData(KIO::Job *job, const TQByteArray &array) {
  receivedBytes  += array.size();
  if( checkBuffer( array.data(), array.size() ) ) {
    containsContentResult = true;
    containsContentFinished( job );
    job->kill();
    return;
  }
  checkTimer();
}

void KRQuery::containsContentFinished( KIO::Job * ) {
  busy = false;
}

bool KRQuery::checkTimer() const {
  if( timer.elapsed() >= STATUS_SEND_DELAY ) {
    int pcnt = (int)(100.*(double)receivedBytes/(double)totalBytes + .5);
    TQString message = i18n( "Searching content of '%1' (%2%)" )
                      .tqarg( fileName ).tqarg( pcnt );
    timer.start();
    emit ((KRQuery *)this)->status( message );
    return true;
  }
  return false;
}

TQStringList KRQuery::split( TQString str )
{
  TQStringList list;
  unsigned splitNdx = 0;
  unsigned startNdx = 0;
  bool quotation = false;

  while( splitNdx < str.length() )
  {
    if( str[ splitNdx ] == '"' )
      quotation = !quotation;

    if( !quotation && str[ splitNdx ] == ' ')
    {
      TQString section = str.mid( startNdx, splitNdx - startNdx );
      startNdx = splitNdx+1;
      if( section.startsWith( "\"" ) && section.endsWith( "\"" ) && section.length() >=2 )
        section = section.mid( 1, section.length()-2 );
      if( !section.isEmpty() )
        list.append( section );
    }
    splitNdx++;
  }

  if( startNdx < splitNdx )
  {
    TQString section = str.mid( startNdx, splitNdx - startNdx );
    if( section.startsWith( "\"" ) && section.endsWith( "\"" ) && section.length() >=2 )
      section = section.mid( 1, section.length()-2 );
    if( !section.isEmpty() )
      list.append( section );
  }

  return list;
}

void KRQuery::setNameFilter( const TQString &text, bool cs )
{
  bNull = false;
  matchesCaseSensitive = cs;
  origFilter = text;

  TQString matchText = text;
  TQString excludeText;
  
  unsigned excludeNdx = 0;
  bool quotationMark = 0;
  while( excludeNdx < matchText.length() )
  {
    if( matchText[ excludeNdx ] == '"' )
      quotationMark = !quotationMark;
    if( !quotationMark )
    {
      if( matchText[ excludeNdx ] == '|' )
        break; 
    }
    excludeNdx++;
  }

  if( excludeNdx < matchText.length() )
  {
    excludeText = matchText.mid( excludeNdx + 1 ).stripWhiteSpace();
    matchText.truncate( excludeNdx );
    matchText = matchText.stripWhiteSpace();
    if( matchText.isEmpty() )
      matchText = "*";
  }

  unsigned i;

  matches  = split( matchText );
  includedDirs.clear();

  for( i=0; i < matches.count(); ) {
    if( matches[ i ].endsWith( "/" ) ) {
      includedDirs.push_back( matches[ i ].left( matches[ i ].length() - 1 ) );
      matches.remove( matches.at( i ) );
      continue;
    }

    if( !matches[ i ].contains( "*" ) && !matches[ i ].contains( "?" ) ) 
      matches[ i ] = "*" + matches[ i ] + "*";

    i++;
  }

  excludes = split( excludeText );
  excludedDirs.clear();

  for( i=0; i < excludes.count(); ) {
    if( excludes[ i ].endsWith( "/" ) ) {
      excludedDirs.push_back( excludes[ i ].left( excludes[ i ].length() - 1 ) );
      excludes.remove( excludes.at( i ) );
      continue;
    }

    if( !excludes[ i ].contains( "*" ) && !excludes[ i ].contains( "?" ) ) 
      excludes[ i ] = "*" + excludes[ i ] + "*";

    i++;
  }
}

void KRQuery::setContent( const TQString &content, bool cs, bool wholeWord, bool remoteSearch )
{
  bNull = false;
  contain = content;
  containCaseSensetive = cs;
  containWholeWord = wholeWord;
  containOnRemote = remoteSearch;
}

void KRQuery::setMinimumFileSize( KIO::filesize_t minimumSize )
{
  bNull = false;
  minSize = minimumSize;
}

void KRQuery::setMaximumFileSize( KIO::filesize_t maximumSize )
{
  bNull = false;
  maxSize = maximumSize;
}

void KRQuery::setNewerThan( time_t time )
{
  bNull = false;
  newerThen = time;
}

void KRQuery::setOlderThan( time_t time )
{
  bNull = false;
  olderThen = time;
}

void KRQuery::setOwner( const TQString &ownerIn )
{
  bNull = false;
  owner = ownerIn;
}

void KRQuery::setGroup( const TQString &groupIn )
{
  bNull = false;
  group = groupIn;
}

void KRQuery::setPermissions( const TQString &permIn )
{
  bNull = false;
  perm = permIn;
}

void KRQuery::setMimeType( const TQString &typeIn, TQStringList customList )
{
  bNull = false;
  type = typeIn;
  customType = customList;
}

bool KRQuery::isExcluded( const KURL &url )
{
  for ( unsigned int i = 0; i < whereNotToSearch.count(); ++i )
    if( whereNotToSearch [ i ].isParentOf( url ) || url.equals( whereNotToSearch [ i ], true ) )
      return true;

  if( !matchDirName( url.fileName() ) )
    return true;

  return false;
}

void KRQuery::setSearchInDirs( const KURL::List &urls ) { 
  whereToSearch.clear();
  for( unsigned int i = 0; i < urls.count(); ++i ) {
    TQString url = urls[ i ].url();
    KURL completed = vfs::fromPathOrURL( KURLCompletion::replacedPath( url, true, true ) );
    whereToSearch.append( completed );
  }
}

void KRQuery::setDontSearchInDirs( const KURL::List &urls ) { 
  whereNotToSearch.clear();
  for( unsigned int i = 0; i < urls.count(); ++i ) {
    TQString url = urls[ i ].url();
    KURL completed = vfs::fromPathOrURL( KURLCompletion::replacedPath( url, true, true ) );
    whereNotToSearch.append( completed );
  }
}

#include "krquery.moc"
