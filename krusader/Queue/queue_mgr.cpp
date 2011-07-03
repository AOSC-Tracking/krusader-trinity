#include "queue_mgr.h"

const TQString QueueManager::defaultName="default";
TQMap<TQString, Queue*> QueueManager::_queues;

QueueManager::QueueManager()
{
	Queue *defaultQ = new Queue(defaultName);
	_queues.insert(defaultQ->name(), defaultQ);
}

QueueManager::~QueueManager() 
{
	TQMap<TQString, Queue*>::iterator it;
 	for (it = _queues.begin(); it != _queues.end(); ++it )
 		delete it.data();
	_queues.clear();
}

Queue* QueueManager::queue(const TQString& queueName)
{
	if (!_queues.tqcontains(queueName))
		return 0;
	return _queues[queueName];
}

TQValueList<TQString> QueueManager::queues() const
{
	return _queues.keys();
}

