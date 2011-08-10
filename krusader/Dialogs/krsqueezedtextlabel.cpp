#include "krsqueezedtextlabel.h"
#include <kstringhandler.h>
#include <kurldrag.h>
#include <tqtooltip.h>

KrSqueezedTextLabel::KrSqueezedTextLabel(TQWidget *tqparent, const char *name):
  KSqueezedTextLabel(tqparent, name), acceptDrops( false ), _index(-1), _length(-1) {
}


KrSqueezedTextLabel::~KrSqueezedTextLabel() {
}

void KrSqueezedTextLabel::mousePressEvent(TQMouseEvent *) {
  emit clicked();
  
}

void KrSqueezedTextLabel::enableDrops( bool flag )
{
  setAcceptDrops( acceptDrops = flag );
}

void KrSqueezedTextLabel::dropEvent(TQDropEvent *e) {
  emit dropped(e);
}

void KrSqueezedTextLabel::dragEnterEvent(TQDragEnterEvent *e) {
  if( acceptDrops )
    e->accept( KURLDrag::canDecode( e ) );
  else
    KSqueezedTextLabel::dragEnterEvent( e );
}

void KrSqueezedTextLabel::squeezeTextToLabel(int index, int length) {
	if (index==-1 || length==-1)
		KSqueezedTextLabel::squeezeTextToLabel();
	else {
		TQString sqtext=fullText;
		TQFontMetrics fm(fontMetrics());
		int labelWidth = size().width();
		int textWidth = fm.width(sqtext);
		if (textWidth > labelWidth) {
			int avgCharSize = textWidth / sqtext.length();
			int numOfExtraChars = (textWidth-labelWidth)/avgCharSize;
			int delta;
			
			// remove as much as possible from the left, and then from the right
			if (index>3) {
				delta=TQMIN(index, numOfExtraChars);
				numOfExtraChars -= delta;
				sqtext.replace(0, delta, "...");
			}
			
			if (numOfExtraChars>0 && ((int)sqtext.length() > length+3)) {
				delta = TQMIN(numOfExtraChars, (int)sqtext.length() - (length+3));
				sqtext.replace(sqtext.length()-delta, delta, "...");
			}
			TQLabel::setText(sqtext);

			TQToolTip::remove( this );
			TQToolTip::add( this, fullText );
		} else {
			TQLabel::setText(fullText);

			TQToolTip::remove( this );
			TQToolTip::hide();
		}
	}
}

void KrSqueezedTextLabel::setText( const TQString &text, int index, int length ) {
	_index=index;
	_length=length;
	fullText = text;
	squeezeTextToLabel(_index,_length);
}

#include "krsqueezedtextlabel.moc"

