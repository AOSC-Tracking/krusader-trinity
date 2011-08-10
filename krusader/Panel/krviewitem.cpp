#include "krviewitem.h"
#include "../VFS/krpermhandler.h"
#include <klocale.h>
#include <kmimetype.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>

#define PROPS	static_cast<const KrViewProperties*>(_viewProperties)

TQString atomicExtensions[] = { 
	".tar.gz", 
	".tar.bz2", 
	".tqmoc.cpp"
};

KrViewItem::KrViewItem(vfile *vf, const KrViewProperties* properties): 
	_vf(vf), dummyVfile(false), _viewProperties(properties), _hasExtension(false), _hidden(false), _extension("") {
	if (vf) {
		// check if the file has an extension
		const TQString& vfName = vf->vfile_getName();
		int loc = vfName.findRev('.');
		if (loc>0) { // avoid mishandling of .bashrc and friend
			// check if it has one of the predefined 'atomic extensions'
			for (TQStringList::const_iterator i = PROPS->atomicExtensions.begin(); i != PROPS->atomicExtensions.end(); ++i) {
				if (vfName.endsWith(*i)){
					loc = vfName.length() - (*i).length();
					break;
				}
			}
			_name = vfName.left(loc);
			_extension = vfName.mid(loc+1);
			_hasExtension=true;
		}
		
		if( vfName.startsWith(".") )
			_hidden = true;
	}
}

const TQString& KrViewItem::name(bool withExtension) const {
	if (!withExtension && _hasExtension) return _name;
	else return _vf->vfile_getName();
}

TQString KrViewItem::description() const {
	if (dummyVfile) return i18n("Climb up the directory tree");
	// else is implied
	TQString text = _vf->vfile_getName();
	TQString comment = KMimeType::mimeType(_vf->vfile_getMime())->comment(text, false);
	TQString myLinkDest = _vf->vfile_getSymDest();
	KIO::filesize_t mySize = _vf->vfile_getSize();
	
	TQString text2 = text.copy();
	mode_t m_fileMode = _vf->vfile_getMode();
	
	if (_vf->vfile_isSymLink() ){
		TQString tmp;
		if ( comment.isEmpty() )	tmp = i18n ( "Symbolic Link" ) ;
		else if( _vf->vfile_getMime() == "Broken Link !" ) tmp = i18n("(broken link !)");
		else tmp = i18n("%1 (Link)").tqarg(comment);
	
		text += "->";
	text += myLinkDest;
	text += "  ";
	text += tmp;
	} else if ( S_ISREG( m_fileMode ) ){
	text = TQString("%1 (%2)").tqarg(text2).tqarg( PROPS->humanReadableSize ?
		KRpermHandler::parseSize(_vf->vfile_getSize()) : KIO::convertSize( mySize ) );
	text += "  ";
	text += comment;
	} else if ( S_ISDIR ( m_fileMode ) ){
	text += "/  ";
		text += comment;
	} else {
	text += "  ";
	text += comment;
	}
	return text;
}

TQString KrViewItem::dateTime() const {
   // convert the time_t to struct tm
   time_t time = _vf->vfile_getTime_t();
   struct tm* t=localtime((time_t *)&time);

   TQDateTime tmp(TQDate(t->tm_year+1900, t->tm_mon+1, t->tm_mday), TQTime(t->tm_hour, t->tm_min));
   return KGlobal::locale()->formatDateTime(tmp);
}

TQPixmap KrViewItem::icon() {
#if 0  
  TQPixmap *p;

  // This is bad - very bad. the function must return a valid reference,
  // This is an interface flow - shie please fix it with a function that return TQPixmap*
  // this way we can return 0 - and do our error checking...
  
  // shie answers: why? what's the difference? if we return an empty pixmap, others can use it as it
  // is, without worrying or needing to do error checking. empty pixmap displays nothing
#endif
	if (dummyVfile || !_viewProperties->displayIcons)
		return TQPixmap();
	else return KrView::getIcon(_vf);
}
