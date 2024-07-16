#ifndef KRUSADERAPP_H
#define KRUSADERAPP_H

#include <tdeapplication.h>
#include "X11/Xlib.h"

#ifdef KeyPress
#undef KeyPress
#endif

#ifdef GrayScale
#undef GrayScale
#endif

#ifdef Status
#undef Status
#endif

// declare a dummy kapplication, just to get the X focusin focusout events
class KrusaderApp: public TDEApplication {
	TQ_OBJECT
  
public:
	KrusaderApp(): TDEApplication() {}
	bool x11EventFilter ( XEvent *e ) {
		switch (e->type) {
			case FocusIn:
				emit windowActive();
				break;
			case FocusOut:
				emit windowInactive();
				break;
			
		}
		//return false; // event should be processed normally
		return TDEApplication::x11EventFilter(e);
	}
signals:
	void windowActive();
	void windowInactive();
};


#endif // KRUSADERAPP_H
