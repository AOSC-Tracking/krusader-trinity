#ifndef KRSQUEEZEDTEXTLABEL_H
#define KRSQUEEZEDTEXTLABEL_H

#include <ksqueezedtextlabel.h>

class TQMouseEvent;
class TQDropEvent;
class TQDragEnterEvent;

/**
This class overloads KSqueezedTextLabel and simply adds a clicked signal,
so that users will be able to click the label and switch focus between panels.

NEW: a special setText() method allows to choose which part of the string should
     be displayed (example: make sure that search results won't be cut out)
*/
class KrSqueezedTextLabel : public KSqueezedTextLabel {
Q_OBJECT
  TQ_OBJECT
  public:
    KrSqueezedTextLabel(TQWidget *parent = 0, const char *name = 0);
    ~KrSqueezedTextLabel();

    void enableDrops( bool flag );

  public slots:
  	 void setText( const TQString &text, int index=-1, int length=-1 );

  signals:
    void clicked(); /**< emitted when someone clicks on the label */
    void dropped(TQDropEvent *); /**< emitted when someone drops URL onto the label */

  protected:
  	 void resizeEvent( TQResizeEvent * ) { squeezeTextToLabel(_index, _length); }
    virtual void mousePressEvent(TQMouseEvent *e);
    virtual void dropEvent(TQDropEvent *e);
    virtual void dragEnterEvent(TQDragEnterEvent *e);
    void squeezeTextToLabel(int index=-1, int length=-1);

  private:
    bool  acceptDrops;
    int _index, _length;
};

#endif
