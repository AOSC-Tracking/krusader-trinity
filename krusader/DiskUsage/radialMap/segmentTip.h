// Author: Max Howell <max.howell@methylblue.com>, (C) 2004
// Copyright: See COPYING file that comes with this distribution

#ifndef SEGMENTTIP_H
#define SEGMENTTIP_H

#include <kpixmap.h>
#include <tqwidget.h>

class File;
class Directory;

namespace RadialMap
{
    class SegmentTip : public TQWidget
    {
    public:
        SegmentTip( uint );

        void updateTip( const File*, const Directory* );
        void moveto( TQPoint, const TQWidget&, bool );

    private:
        virtual bool eventFilter( TQObject*, TQEvent* );
        virtual bool event( TQEvent* );

        uint    m_cursorHeight;
        KPixmap m_pixmap;
        TQString m_text;
    };
}

#endif
