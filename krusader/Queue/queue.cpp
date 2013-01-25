#include "queue.h"

Queue::Queue(const TQString& name): _name(name)
{
}

Queue::~Queue() 
{
	// TODO: save queue on delete? or just delete jobs
}

void Queue::enqueue(TDEIO::Job *job)
{
	_jobs.append(job);
	
	dumpQueue();
}

void Queue::dumpQueue()
{
	tqDebug("Queue: %s", name().latin1());
}
