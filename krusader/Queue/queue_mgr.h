#ifndef TQUEUE_MGR_H
#define TQUEUE_MGR_H

#include "queue.h"
#include <tqmap.h>

/**
 * QueueManager holds multiple queues and has a static
 * method that fetches a queue by name. calling it with
 * no arguments will fetch the default queue
 */
class QueueManager
{
	static const TQString defaultName;
public:
	QueueManager();
	~QueueManager();
	
	static Queue* queue(const TQString& queueName=defaultName);
	TQValueList<TQString> queues() const;

protected:
	static TQMap<TQString, Queue*> _queues;
};

#endif // TQUEUE_MGR_H
