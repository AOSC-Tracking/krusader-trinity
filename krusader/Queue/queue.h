#ifndef TQUEUE_H
#define TQUEUE_H

#include <tqobject.h>
#include <kio/jobclasses.h>
#include <tqptrlist.h>

/**
 * Queue can hold anything which inherits KIO::Job, and schedule it, start it, stop etc...
 * the main reason to hold the Job itself (at least for phase 1) is to keep the code 
 * in krusader relatively unchaged, and allow to create the job as usual and choose if 
 * to start it, or queue it.
 *
 */
class Queue: public TQObject
{
	Q_OBJECT
  
public:
	Queue(const TQString& name);
	virtual ~Queue();
	
	inline const TQString& name() const { return _name; }
	void enqueue(KIO::Job *job);

protected:
	void dumpQueue();

	TQString _name;
	TQPtrList<KIO::Job> _jobs;
};

#endif // TQUEUE_H
