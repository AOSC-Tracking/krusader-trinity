//Author:    Max Howell <max.howell@methylblue.com>, (C) 2004
//Copyright: See COPYING file that comes with this distribution

#ifndef WIDGET_H
#define WIDGET_H

#include <kurl.h>
#include <tqtimer.h>
#include "segmentTip.h"

template <class T> class Chain;
class Directory;
class File;
namespace KIO { class Job; }
class KURL;

namespace RadialMap
{
    class Segment;

    class Map : public KPixmap
    {
    public:
        Map();
        ~Map();

        void make( const Directory *, bool = false );
        bool resize( const TQRect& );

        bool isNull() const { return ( m_signature == 0 ); }
        void tqinvalidate( const bool );

        friend class Builder;
        friend class Widget;

    private:
        void paint( uint = 1 );
        void aaPaint();
        void colorise();
        void setRingBreadth();

        Chain<Segment> *m_signature;

        TQRect   m_rect;
        uint    m_ringBreadth;  ///ring breadth
        uint    m_innerRadius;  ///radius of inner circle
        uint    m_visibleDepth; ///visible level depth of system
        TQString m_centerText;

        uint MAP_2MARGIN;
    };

    class Widget : public TQWidget
    {
    Q_OBJECT
  TQ_OBJECT

    public:
        Widget( TQWidget* = 0, const char* = 0 );

        TQString path() const;
        KURL url( File const * const = 0 ) const;

        bool isValid() const { return m_tree != 0; }

        friend class Label; //FIXME badness

    public slots:
        void zoomIn();
        void zoomOut();
        void create( const Directory* );
        void tqinvalidate( const bool = true );
        void refresh( int );

    private slots:
        void resizeTimeout();
        void sendFakeMouseEvent();
        void deleteJobFinished( KIO::Job* );
        void createFromCache( const Directory* );

    signals:
        void activated( const KURL& );
        void invalidated( const KURL& );
        void created( const Directory* );
        void mouseHover( const TQString& );

    protected:
        virtual void paintEvent( TQPaintEvent* );
        virtual void resizeEvent( TQResizeEvent* );
        virtual void mouseMoveEvent( TQMouseEvent* );
        virtual void mousePressEvent( TQMouseEvent* );

    protected:
        const Segment *segmentAt( TQPoint& ) const; //FIXME const reference for a library others can use
        const Segment *rootSegment() const { return m_rootSegment; } ///never == 0
        const Segment *focusSegment() const { return m_focus; } ///0 == nothing in focus

    private:
        void paintExplodedLabels( TQPainter& ) const;

        const Directory *m_tree;
        const Segment   *m_focus;
        TQPoint           m_offset;
        TQTimer           m_timer;
        Map              m_map;
        SegmentTip       m_tip;
        Segment         *m_rootSegment;
    };
}

#endif
