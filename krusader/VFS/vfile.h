/***************************************************************************
                          vfile.h
                      -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
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
#ifndef VFILE_H
#define VFILE_H

// QT includes
#include <tqstring.h>
#include <tqobject.h>
// System includes
#include <sys/types.h>
// KDE includes
#include <kio/global.h>
#include <kmimetype.h>

#define PERM_ALL          -2

/**
 * The Virtual File class handles all the details of maintaining a single
 * file component within the virtual file system (vfs). a vfile object
 * contains the nessecery details about a file and member functions which
 *  allow the object to give out the needed details about the file.
 */
class vfile : public TQObject{
  Q_OBJECT
  
  
public:
	vfile(){}

  /**
	 * Use this constructor when you know the following files properties: \n
	 * file name, file size, file permissions,is the file a link,owner uid & group uid.
	 */
	vfile(const TQString& name,
	      const TDEIO::filesize_t size,
	      const TQString& perm,
	      const time_t mtime,
	      const bool symLink,
	      const uid_t	owner,
	      const gid_t group,
	      const TQString& mime,
	      const TQString& symDest,
	      const mode_t  mode,
	      const int rwx = -1 );
	
	vfile(const TQString& name,	
	      const TDEIO::filesize_t size,	
	      const TQString& perm,
	      const time_t mtime,
	      const bool symLink,
	      const TQString& owner,
	      const TQString& group,
	      const TQString& userName,
	      const TQString& mime,
	      const TQString& symDest,
	      const mode_t  mode,
	      const int rwx = -1,
	      const TQString& aclString = TQString(),
	      const TQString& aclDfltString = TQString() );
	
	bool        operator==(const vfile& vf) const;
	vfile&      operator= (const vfile& vf); 
	inline bool operator!=(const vfile& vf){ return !((*this)==vf); }
	
	// following functions give-out file details
	inline const TQString&   vfile_getName()    const { return vfile_name;           }
	inline TDEIO::filesize_t  vfile_getSize()    const { return vfile_size;           }
	inline const TQString&   vfile_getPerm()    const { return vfile_perm;           }
	inline bool             vfile_isDir()      const { return vfile_isdir;          }
	inline bool             vfile_isSymLink()  const { return vfile_symLink;        }
	inline const TQString&   vfile_getSymDest() const { return vfile_symDest;        }
	inline mode_t           vfile_getMode()    const { return vfile_mode;           }
	inline uid_t            vfile_getUid()     const { return vfile_ownerId;        }
	inline gid_t            vfile_getGid()     const { return vfile_groupId;        }
	inline time_t           vfile_getTime_t()  const { return vfile_time_t;         }
	inline const KURL&      vfile_getUrl()     const { return vfile_url;            }
  
	const TQString&          vfile_getMime(bool fast=false);
	const TQString&          vfile_getOwner();
	const TQString&          vfile_getGroup();
	const TQString&          vfile_getACL();
	const TQString&          vfile_getDefaultACL();
	const TDEIO::UDSEntry     vfile_getEntry(); //< return the UDSEntry from the vfile
	char                    vfile_isReadable()   const;
	char                    vfile_isWriteable()  const;
	char                    vfile_isExecutable() const;
	/**
	 * Set the file size.
	 * used ONLY when calculating a directory's space, needs to change the
	 * displayed size of the viewitem and thus the vfile. For INTERNAL USE !
	 */
	inline void             vfile_setSize(TDEIO::filesize_t size) {vfile_size = size;}
	inline void             vfile_setUrl(const KURL& url)       {vfile_url = url;  }

	inline void             vfile_setIcon(const TQString& icn)   {vfile_icon = icn; }
	inline TQString          vfile_getIcon();

	virtual ~vfile(){}

private:
	void                    vfile_loadACL();

protected:
	// the file information list
	TQString          vfile_name;     //< file name
	TDEIO::filesize_t  vfile_size;     //< file size
	mode_t           vfile_mode;     //< file mode
	uid_t            vfile_ownerId;  //< file owner id
	gid_t            vfile_groupId;  //< file group id
	TQString          vfile_owner;    //< file owner name
	TQString          vfile_group;    //< file group name
	TQString          vfile_userName; //< the current username
	TQString          vfile_perm;     //< file permissions string
	time_t           vfile_time_t;   //< file modification in time_t format
	bool             vfile_symLink;  //< true if the file is a symlink
	TQString          vfile_mimeType; //< file mimetype
	TQString          vfile_symDest;  //< if it's a sym link - its detination
	KURL             vfile_url;      //< file URL - empty by default
	TQString          vfile_icon;     //< the name of the icon file
	bool             vfile_isdir;    //< flag, if it's a directory
	int              vfile_rwx;      //< flag, showing read, write, execute properties
	bool             vfile_acl_loaded;//<flag, indicates that ACL permissions already loaded
	bool             vfile_has_acl;  //< flag, indicates ACL permissions
	TQString          vfile_acl;      //< ACL permission string
	TQString          vfile_def_acl;  //< ACL default string
};

	
TQString vfile::vfile_getIcon(){
	if( vfile_icon.isEmpty() ){
		TQString mime = this->vfile_getMime();
      if ( mime == "Broken Link !" )
         vfile_icon = "file_broken";
      else {
         vfile_icon = KMimeType::mimeType( mime ) ->icon( TQString(), true );
      }		
	}
	return vfile_icon;
}

#endif
