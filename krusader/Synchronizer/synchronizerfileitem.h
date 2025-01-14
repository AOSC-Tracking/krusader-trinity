/***************************************************************************
                    synchronizerfileitem.h  -  description
                             -------------------
    copyright            : (C) 2006 + by Csaba Karai
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

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __SYNCHRONIZER_FILE_ITEM_H__
#define __SYNCHRONIZER_FILE_ITEM_H__

#include <tqstring.h>
#include <tdeio/global.h>

typedef enum 
{
  TT_EQUALS        = 0,   // the files are equals     -> do nothing
  TT_DIFFERS       = 1,   // the files are differents -> don't know what to do
  TT_COPY_TO_LEFT  = 2,   // the right file is newer  -> copy from right to left
  TT_COPY_TO_RIGHT = 3,   // the left file is newer   -> copy from left to right
  TT_DELETE        = 4,   // the left file is single  -> delete it
  TT_UNKNOWN       = 5,   // (5-9) the type of the task is not yet known
  TT_MAX           = 10    // the maximum number of task types
} TaskType;

#define SWAP( A, B, TYPE )      {TYPE TMP = A; A = B; B = TMP;}
#define REVERSE_TASK( A, asym ) {switch( A )                                           \
                                 {                                                     \
                                 case TT_COPY_TO_LEFT:                                 \
                                   if( asym )                                          \
                                     A = !m_existsRight ? TT_DELETE : TT_COPY_TO_LEFT; \
                                   else                                                \
                                     A = TT_COPY_TO_RIGHT;                             \
                                   break;                                              \
                                 case TT_COPY_TO_RIGHT:                                \
                                 case TT_DELETE:                                       \
                                   A = TT_COPY_TO_LEFT;                                \
                                 default:                                              \
                                   break;                                              \
                                 }};

class SynchronizerFileItem
{
  private:
    TQString               m_leftName;     // the left file name
    TQString               m_rightName;    // the right file name
    TQString               m_leftDirectory;// the left relative directory path from the base
    TQString               m_rightDirectory;// the left relative directory path from the base
    bool                  m_marked;       // flag, indicates to show the file
    bool                  m_existsLeft;   // flag, the file exists in the left directory
    bool                  m_existsRight;  // flag, the file exists in the right directory
    TDEIO::filesize_t       m_leftSize;     // the file size at the left directory
    TDEIO::filesize_t       m_rightSize;    // the file size at the right directory
    time_t                m_leftDate;     // the file date at the left directory
    time_t                m_rightDate;    // the file date at the left directory
    TQString               m_leftLink;     // the left file's symbolic link destination
    TQString               m_rightLink;    // the right file's symbolic link destination
    TQString               m_leftOwner;    // the left file's owner
    TQString               m_rightOwner;   // the right file's owner
    TQString               m_leftGroup;    // the left file's group
    TQString               m_rightGroup;   // the right file's group
    mode_t                m_leftMode;     // mode for left
    mode_t                m_rightMode;    // mode for right
    TQString               m_leftACL;      // ACL of the left file
    TQString               m_rightACL;     // ACL of the right file
    TaskType              m_task;         // the task with the file
    bool                  m_isDir;        // flag, indicates that the file is a directory
    SynchronizerFileItem *m_parent;       // pointer to the parent directory item or 0
    void                 *m_userData;     // user data
    bool                  m_overWrite;    // overwrite flag
    TQString               m_destination;  // the destination URL at rename
    bool                  m_temporary;    // flag indicates temporary directory
    TaskType              m_originalTask; // the original task type
    
  public:
    SynchronizerFileItem(const TQString &leftNam, const TQString &rightNam, const TQString &leftDir,
                       const TQString &rightDir, bool mark, bool exL, bool exR, TDEIO::filesize_t leftSize,
                       TDEIO::filesize_t rightSize, time_t leftDate, time_t rightDate,
                       const TQString &leftLink, const TQString &rightLink, const TQString &leftOwner,
                       const TQString &rightOwner, const TQString &leftGroup, const TQString &rightGroup,
                       mode_t leftMode, mode_t rightMode, const TQString &leftACL, const TQString &rightACL, 
                       TaskType tsk, bool isDir, bool tmp, SynchronizerFileItem *parent ) :
                       m_leftName( leftNam ), m_rightName( rightNam ), m_leftDirectory( leftDir ),  m_rightDirectory( rightDir ),
                       m_marked( mark ),  m_existsLeft( exL ), m_existsRight( exR ), m_leftSize( leftSize ),
                       m_rightSize( rightSize ), m_leftDate( leftDate ), m_rightDate( rightDate ),
                       m_leftLink( leftLink ), m_rightLink( rightLink ), m_leftOwner( leftOwner ),
                       m_rightOwner( rightOwner ), m_leftGroup( leftGroup ), m_rightGroup( rightGroup ),
                       m_leftMode( leftMode ), m_rightMode( rightMode ), m_leftACL( leftACL ),
                       m_rightACL( rightACL ), m_task( tsk ), m_isDir( isDir ), m_parent(parent),
                       m_userData( 0 ), m_overWrite( false ), m_destination( TQString() ), 
                       m_temporary( tmp ), m_originalTask( tsk ) {}

    inline bool                   isMarked()              {return m_marked;}
    inline void                   setMarked( bool flag )  {m_marked = flag;}
    inline const TQString &        leftName()              {return m_leftName;}
    inline const TQString &        rightName()             {return m_rightName;}
    inline const TQString &        leftDirectory()         {return m_leftDirectory;}
    inline const TQString &        rightDirectory()        {return m_rightDirectory;}
    inline bool                   existsInLeft()          {return m_existsLeft;}
    inline bool                   existsInRight()         {return m_existsRight;}
    inline bool                   overWrite()             {return m_overWrite;}
    inline TDEIO::filesize_t        leftSize()              {return m_leftSize;}
    inline TDEIO::filesize_t        rightSize()             {return m_rightSize;}
    inline time_t                 leftDate()              {return m_leftDate;}
    inline time_t                 rightDate()             {return m_rightDate;}
    inline const TQString &        leftLink()              {return m_leftLink;}
    inline const TQString &        rightLink()             {return m_rightLink;}
    inline const TQString &        leftOwner()             {return m_leftOwner;}
    inline const TQString &        rightOwner()            {return m_rightOwner;}
    inline const TQString &        leftGroup()             {return m_leftGroup;}
    inline const TQString &        rightGroup()            {return m_rightGroup;}
    inline mode_t                 leftMode()              {return m_leftMode;}
    inline mode_t                 rightMode()             {return m_rightMode;}
    inline const TQString &        leftACL()               {return m_leftACL;}
    inline const TQString &        rightACL()              {return m_rightACL;}
    inline TaskType               task()                  {return m_task;}
    inline void                   compareContentResult( bool res )
                                                          {if( res == true )
                                                             m_task = m_originalTask = TT_EQUALS;
                                                           else if( m_originalTask >= TT_UNKNOWN )
                                                             m_task = m_originalTask = (TaskType)(m_originalTask - TT_UNKNOWN);
                                                          }
    inline bool                   isDir()                 {return m_isDir;}
    inline SynchronizerFileItem * parent()                {return m_parent;}
    inline void *                 userData()              {return m_userData;}
    inline void                   setUserData( void *ud)  {m_userData = ud;}
    inline void                   setOverWrite()          {m_overWrite = true;}
    inline const TQString &        destination()           {return m_destination;}
    inline void                   setDestination(TQString d) {m_destination = d;}
    inline bool                   isTemporary()           {return m_temporary;}
    inline void                   setPermanent()          {m_temporary = false;}
    inline TaskType               originalTask()          {return m_originalTask;}
    inline void                   restoreOriginalTask()   {m_task = m_originalTask;}
    inline void                   setTask( TaskType t )   {m_task = t;}
    inline void                   swap( bool asym=false ) {SWAP( m_existsLeft, m_existsRight, bool );
                                                           SWAP( m_leftName, m_rightName, TQString );
                                                           SWAP( m_leftDirectory, m_rightDirectory, TQString );
                                                           SWAP( m_leftSize, m_rightSize, TDEIO::filesize_t );
                                                           SWAP( m_leftDate, m_rightDate, time_t );
                                                           SWAP( m_leftLink, m_rightLink, TQString );
                                                           SWAP( m_leftOwner, m_rightOwner, TQString );
                                                           SWAP( m_leftGroup, m_rightGroup, TQString );
                                                           SWAP( m_leftACL, m_rightACL, TQString );
                                                           REVERSE_TASK( m_originalTask, asym );
                                                           REVERSE_TASK( m_task, asym );}
};

#endif /* __SYNCHRONIZER_FILE_ITEM_H__ */
