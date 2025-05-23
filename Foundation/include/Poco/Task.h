//
// Task.h
//
// Library: Foundation
// Package: Tasks
// Module:  Tasks
//
// Definition of the Task class.
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//


#ifndef Foundation_Task_INCLUDED
#define Foundation_Task_INCLUDED


#include "Poco/Foundation.h"
#include "Poco/Runnable.h"
#include "Poco/RefCountedObject.h"
#include "Poco/Mutex.h"
#include "Poco/Event.h"


namespace Poco {


class TaskManager;
class Notification;
class NotificationCenter;


class Foundation_API Task: public Runnable, public RefCountedObject
	/// A Task is a subclass of Runnable that has a name
	/// and supports progress reporting and cancellation.
	///
	/// A TaskManager object can be used to take care of the
	/// lifecycle of a Task.
{
public:
	enum TaskState
	{
		TASK_IDLE,
		TASK_STARTING,
		TASK_RUNNING,
		TASK_CANCELLING,
		TASK_FINISHED
	};

	Task(const std::string& name);
		/// Creates the Task.

	const std::string& name() const;
		/// Returns the task's name.

	float progress() const;
		/// Returns the task's progress.
		/// The value will be between 0.0 (just started)
		/// and 1.0 (completed).

	virtual void cancel();
		/// Requests the task to cancel itself. For cancellation
		/// to work, the task's runTask() method must periodically
		/// call isCancelled() and react accordingly.
		///
		/// Can be overridden to implement custom behavior,
		/// but the base class implementation of cancel() should
		/// be called to ensure proper behavior.

	bool isCancelled() const;
		/// Returns true if cancellation of the task has been
		/// requested.
		///
		/// A Task's runTask() method should periodically
		/// call this method and stop whatever it is doing in an
		/// orderly way when this method returns true.
		/// If task is cancelled before it had a chance to run,
		/// runTask() will never be called.

	TaskState state() const;
		/// Returns the task's current state.

	void reset();
		/// Sets the task's progress to zero and clears the
		/// cancel flag.

	virtual void runTask() = 0;
		/// Do whatever the task needs to do. Must
		/// be overridden by subclasses.

	void run() override;
		/// If task has not been cancelled prior to this call, it
		/// calls the task's runTask() method and notifies the owner of
		/// the task's start and completion.
		/// If task has been cancelled prior to this call, it only sets
		/// the state to TASK_FINISHED and notifies the owner.

	bool hasOwner() const;
		/// Returns true iff the task has an owner.
protected:
	bool sleep(long milliseconds);
		/// Suspends the current thread for the specified
		/// amount of time.
		///
		/// If the task is cancelled while it is sleeping,
		/// sleep() will return immediately and the return
		/// value will be true. If the time interval
		/// passes without the task being cancelled, the
		/// return value is false.
		///
		/// A Task should use this method in favor of Thread::sleep().

	bool yield();
		/// Yields cpu to other threads
		///
		/// If the task is cancelled while it is suspended,
		/// yield() will return true. If the tasks resumes
		/// without being cancelled, the
		/// return value is false.
		///
		/// A Task should use this method in favor of Thread::yield().

	void setProgress(float progress);
		/// Sets the task's progress.
		/// The value should be between 0.0 (just started)
		/// and 1.0 (completed).

	virtual void postNotification(Notification* pNf);
		/// Posts a notification to the task manager's
		/// notification center.
		///
		/// A task can use this method to post custom
		/// notifications about its progress.

	void setOwner(TaskManager* pOwner);
		/// Sets the (optional) owner of the task.

	TaskManager* getOwner() const;
		/// Returns the owner of the task, which may be NULL.

	TaskState setState(TaskState state);
		/// Sets the task's state.

	~Task() override;
		/// Destroys the Task.

private:
	Task();
	Task(const Task&);
	Task& operator = (const Task&);

	std::string               _name;
	std::atomic<TaskManager*> _pOwner;
	std::atomic<float>        _progress;
	std::atomic<TaskState>    _state;
	Event                     _cancelEvent;
	mutable FastMutex         _mutex;

	friend class TaskManager;
};


//
// inlines
//
inline const std::string& Task::name() const
{
	return _name;
}


inline float Task::progress() const
{
	return _progress;
}


inline bool Task::isCancelled() const
{
	return _state == TASK_CANCELLING;
}


inline Task::TaskState Task::state() const
{
	return _state;
}


inline TaskManager* Task::getOwner() const
{
	return _pOwner;
}


inline bool Task::hasOwner() const
{
	return _pOwner != nullptr;
}


} // namespace Poco


#endif // Foundation_Task_INCLUDED
