#ifndef TQUEUE_WIDGET_H
#define TQUEUE_WIDGET_H

#include <ktabwidget.h>

class QueueWidget: public KTabWidget
{
	Q_OBJECT
  TQ_OBJECT
public:
	QueueWidget();
	~QueueWidget();
};

#endif // TQUEUE_WIDGET_H
