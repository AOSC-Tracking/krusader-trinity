#ifndef QUEUE_H
#define QUEUE_H

#include <tqobject.h>
#include <tdeio/jobclasses.h>
#include <tqptrlist.h>

/**
 * Queue can hold anything which inherits TDEIO::Job, and schedule it, start it, stop etc...
 * the main reason to hold the Job itself (at least for phase 1) is to keep the code 
 * in krusader relatively unchaged, and allow to create the job as usual and choose if 
 * to start it, or queue it.
 *
 */
class Queue: public TQObject
{
	TQ_OBJECT
  
public:
	Queue(const TQString& name);
	virtual ~Queue();
	
	inline const TQString& name() const { return _name; }
	void enqueue(TDEIO::Job *job);

protected:
	void dumpQueue();

	TQString _name;
	TQPtrList<TDEIO::Job> _jobs;
};

#endif // QUEUE_H
