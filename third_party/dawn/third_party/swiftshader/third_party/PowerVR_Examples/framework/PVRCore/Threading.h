/*!
\brief MultiThreading tools, including abstract classes for tasks and scheduling, and adaptation of
MoodyCamel's BlockingConcurrentQueue, and
\file PVRCore/Threading.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "../external/concurrent_queue/blockingconcurrentqueue.h"

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <sstream>
#include <deque>

//  ASYNCHRONOUS FRAMEWORK: Framework async loader base etc //
namespace pvr {
namespace async {
/// <summary>A lightweight semaphore with a small spin-wait (typedef from moodycamel)</summary>
typedef moodycamel::details::mpmc_sema::LightweightSemaphore Semaphore;

/// <summary>A simple wrapper for a moodcamel Semaphore object used for synchronising multi threaded access</summary>
class Mutex
{
	Semaphore sema;

public:
	/// <summary>Constructor</summary>
	Mutex() : sema(1) {}
	/// <summary>Waits for semaphore completion</summary>
	void lock() { sema.wait(); }
	/// <summary>Signals the semaphore as completed</summary>
	void unlock() { sema.signal(); }
	/// <summary>Retrieves the semaphore</summary>
	/// <returns>The semaphore</returns>
	Semaphore& getSemaphore() { return sema; }
	/// <summary>Retrieves the semaphore (const)</summary>
	/// <returns>The semaphore (const)</returns>
	const Semaphore& getSemaphore() const { return sema; }
};

/// <summary>A reference-counted pointer to a Semaphore</summary>
typedef std::shared_ptr<Semaphore> SemaphorePtr;

/// <summary>A wrapper object with explicit clean up semantics that is intended
/// to be used as the return value in functions that may need to wrap multiple items.
/// For example, it might be the return value for a texture upload function that
/// returns both a texture object and the fence to know the operatio is complete,
/// and the cleanup function might wait on the fence and then delete it.
/// to void destroying useful resources, does NOT call cleanup in the
/// destructor by default, but that can (should usually) be added to the
/// destructor of the derived class.
/// Implement the abstract cleanup_ function
/// to clean up the resource, then use this object as the return value_comp
/// from a function needing to return (for example) multiple transient
/// values.</summary>
template<typename T>
class IFrameworkCleanupObject
{
	bool _destroyed;

protected:
	IFrameworkCleanupObject() : _destroyed(false) {}

public:
	/// <summary>Virtual destructor to be overriden if required</summary>
	virtual ~IFrameworkCleanupObject() {}
	/// <summary>Call this function when you are done using this object.</summary>
	void cleanup()
	{
		if (!_destroyed)
		{
			cleanup_();
			_destroyed = true;
		}
	}

private:
	/// <summary>Implement this function for your cleanup semantics</summary>
	virtual void cleanup_() = 0;
};

/// <summary>An object that is intended to be used as an asynchronous return value.
/// When you wish to perform a task in another thread, when "kicking" that task you should
/// "immediately" (before the task is complete) return such an object, and use it to
/// check when the task is complete, and retrieve its return value.
/// Will provide both cleanup semantics (which should at least ensure the objects
/// are safe to destroy and operations are pending on them), and functions to both
/// query completion and (blockingly) get the return value.
/// To implement this class, you must implement the abstract functions get_ (that
/// actually waits for and returns the result, and _isComplete, which actually queries
/// if the result is ready.</summary>
/// <typeparam name="T">The type of the return value of the task (for example, Texture)</typeparam>
template<typename T>
class IFrameworkAsyncResult : public IFrameworkCleanupObject<T>
{
public:
	/// <summary>The type of the return value</summary>
	typedef T ValueType;
	/// <summary>A smart pointer to this result object (not the wrapped value)</summary>
	typedef std::shared_ptr<IFrameworkAsyncResult<T>> PointerType;
	/// <summary>A function pointer type that can be used as a callback to call
	/// when the return value is ready</summary>
	typedef void (*Callback)(PointerType);

	/// <summary>Query is the return value is ready to be used</summary>
	/// <returns>True if the value is ready, otherwise false</returns>
	bool isComplete() const { return _isComplete ? true : (_isComplete = isComplete_()); }
	/// <summary>This is the most important function of IFrameworkAsyncResult: it actually
	/// gets the return value. If the value is not yet ready (the task is not complete),
	/// it will wait for the task completion. Use this immediately when other operations
	/// depend on the value. Otherwise, in order to avoid any wait, you can first check
	/// isComplete() and only call this function after isComplete() has returned true.
	/// If the task may fail, you should call isSuccessful() before
	/// using the value to ensure it is what you expect.</summary>
	/// <returns>The return value of the operation.</returns>
	ValueType get() { return get_(); }
	/// <summary>Returns true if the operation was successful. An operation that is not
	/// complete cannot be successful, so check isComplete to ensure that a return value
	/// of false means that the operation actually failed</returns>
	/// <returns>True if the operation is complete and successful, false if the operation
	/// failed or has not yet completed</returns>
	bool isSuccessful() const { return _successful; }

protected:
	Callback _completionCallback; //!< The callback that will be called on completion
	std::atomic<bool> _inCallback; //!< This is a mechanism to query if a function is actually called BY the callback to avoid deadlocking.
	bool _successful; //!< This will / should be set to true when the work is successfully completed.

	/// <summary>Set the callback (a function pointer that will be called whenever processing an item is done)</summary>
	/// <param name="completionCallback">The callback to set</param>
	void setTheCallback(Callback completionCallback) { _completionCallback = completionCallback; }

	/// <summary>Execute the callback (overridable)</summary>
	/// <param name="thisPtr">The object with which the callback will be executed as a parameter.</param>
	virtual void executeCallBack(PointerType thisPtr)
	{
		if (_completionCallback)
		{
			_inCallback = true;
			_completionCallback(thisPtr);
			_inCallback = false;
		}
	}

	IFrameworkAsyncResult() : _completionCallback(nullptr), _inCallback(false), _successful(false), _isComplete(false) {}

private:
	mutable bool _isComplete;

	/// <summary>Implement this by returning true if the task is complete (successful or not). Return false
	/// if the task is not complete (is still running). True should imply that, as far as practical, get()
	/// will return immediately.
	/// <returns>True if the task is complete (or aborted, or failed), false if the task is still running</returns>
	virtual bool isComplete_() const = 0;
	/// <summary>Implement this call to block until the task is completed, and then return the return value
	/// of the task. As much as possible, if the call is complete, this should return immediately. Repeated
	/// calls should function normally (i.e. not block etc.)
	/// <returns>The return value of the task</returns>
	virtual T get_() const = 0;
};

/// <summary>The AsyncScheduler is an abstract Scheduling system of a homogeneous task queue running on a background
/// thread, i.e. a queue of work of a particular type. It provides its child classes wioth access to the actual queue
/// and the synchronization semaphores in order to facilitate easier implementation. Specifically, it is expected that
/// the user will provide functions to enqueue and dequeue work, while this class provides the running loop (actually
/// executing the tasks, calling the callbacks etc</summary>
/// <typeparam name="ValueType">The type of the return value that will be returned by the functions</typeparam>
/// <typeparam name="FutureType">The type of the future (which will also be the input to the worker function)</typeparam>
/// <typeparam name="worker">The function pointer that will be called to perform the work</typeparam>
template<typename ValueType, typename FutureType, void (*worker)(FutureType)>
class AsyncScheduler
{
public:
	/// <summary>The type of result throughout this class.</summary>
	typedef std::shared_ptr<IFrameworkAsyncResult<ValueType>> AsyncResult;

	/// <summary>The approximate number of queued items. (Unsynchronized for performance).</summary>
	/// <returns>The number of queued items (currently visible to this thread)</returns>
	uint32_t getNumApproxQueuedItem() { return static_cast<uint32_t>(_queue.size()); }

	/// <summary>The precise number of queued items at the time of calling. Although it is poosible
	/// that (if the queue is currently producing or consuming) the number has changed by the time
	/// it is used (even if immediately), nevertheless getting the number is synchronized hence precise
	/// at the time of querying it.</summary>
	/// <returns>The number of queued items (access is synchronized)</returns>
	uint32_t getNumQueuedItems()
	{
		_queueSemaphore.wait();
		uint32_t retval = static_cast<uint32_t>(_queue.size());
		_queueSemaphore.signal();
		return retval;
	}

	/// <summary>Destructor (virtual)</summary>
	virtual ~AsyncScheduler()
	{
		bool didit = false;
		_queueSemaphore.wait(); // protect the _done variable
		if (!_done)
		{
			didit = true;
			_done = true;
			_workSemaphore.signal();
		}
		_queueSemaphore.signal();
		if (didit) // Only join if it was actually running. Duh!
		{ _thread.join(); } }

protected:
	/// <summary>Constructor (empty, spawns worker thread and waits)</summary>
	AsyncScheduler()
	{
		_done = false;
		_thread = std::thread(&AsyncScheduler::run, this);
	}

	/// <summary>This semaphore is the counter for enqueued work. Signal it once when adding work to the queue.</summary>
	Semaphore _workSemaphore;
	/// <summary>This is basically a mutex for the queue. Wait on it to lock the queue, then signal it when done.</summary>
	Semaphore _queueSemaphore;
	/// <summary>This is the work queue. Each item enqueued will be processed by worker.</summary>
	std::deque<FutureType> _queue;
	/// <summary>String information regarding the tasks operations.</summary>
	std::string _myInfo;

private:
	std::thread _thread;
	std::atomic_bool _done;
	void run()
	{
		Log(LogLevel::Information,
			"%s : Asynchronous Scheduler starting. "
			"1 worker thread spawned. The worker thread will be sleeping as long as no work is being performed, "
			"and will be released when the async sheduler is destroyed.",
			_myInfo.c_str());

		std::string _str_0_unlock_ = (_myInfo + " : LOOP ENTER: Unlocking the queue.");
		std::string _str_1_wait_work_ = (_myInfo + " : Queue unlocked, waiting for work to be scheduled");
		std::string _str_2_queuelock_ = (_myInfo + " : Work scheduled, Obtaining queue lock");
		std::string _str_3_queueLocked_ = (_myInfo + " : Queue lock obtained, testing to determine work/exit");
		std::string _str_4_exit_signal_ = (_myInfo + " : Queue was signalled without items - must be the exit signal. Exiting...");
		std::string _str_5_obtaining_work_ = (_myInfo + " : Queue had items: Obtaining work!");
		std::string _str_6_obtained_work_ = (_myInfo + " : Work obtained, Releasing the queue.");
		std::string _str_7_executing_work_ = (_myInfo + " : Queue released, Executing work.");
		std::string _str_8_execution_done_ = (_myInfo + " : Work execution done, locking the queue.");
		std::string _str_9_looping_up_ = (_myInfo + " : Queue locked, LOOPING!");

		// Are we done?
		// a) The queue will be empty on the first iteration.
		// b) Even if the queue is empty, consumers may be blocked on results.
		// c) Even if done, consumers may be blocked on results.
		while (!_done || _queue.size())
		{
			DebugLog(_str_0_unlock_.c_str());
			_queueSemaphore.signal(); // First iteration: Signal that we are actually waiting on the queue, essentially priming it,
			// consumers to start adding items.
			// Subsequent iterations: Release the check we are doing on the queue.
			DebugLog(_str_1_wait_work_.c_str());
			_workSemaphore.wait(); // Wait for work to arrive
			DebugLog(_str_2_queuelock_.c_str());
			_queueSemaphore.wait(); // Work has arrived! Or has it? Lock the queue, as we need to check that it is not empty,
			DebugLog(_str_3_queueLocked_.c_str());
			// and to unqueue items.
			if (!_queue.size()) // Are we done? Was this a signal for work or for us to finish?
			{
				DebugLog(_str_4_exit_signal_.c_str());
				// Someone signalled the queue without putting items in. This means we are finished.
				assertion(_done); // duh?
				break;
			}
			else // Yay! Work to do!
			{
				DebugLog(_str_5_obtaining_work_.c_str());

				FutureType future = std::move(_queue.front()); // Get work.
				_queue.pop_front();
				DebugLog(_str_6_obtained_work_.c_str());
				_queueSemaphore.signal(); // Release the krak... QUEUE.
				DebugLog(_str_7_executing_work_.c_str());
				worker(future); // Load a texture! Yay!
				DebugLog(_str_8_execution_done_.c_str());
			}
			_queueSemaphore.wait(); // Continue. Lock the queue to check it out.
			DebugLog(_str_9_looping_up_.c_str());
		}
		_queueSemaphore.signal(); // Finish. Release the queue.
		Log(LogLevel::Information, "%s: Asynchronous asset loader closing down. Freeing workers.", _myInfo.c_str());
	}
};
} // namespace async
} // namespace pvr

namespace pvr {
/// <summary>A simple adapter for the BlockingConcurrentQueue that simplifies common use cases</summary>
template<typename T>
class LockedQueue
{
	moodycamel::BlockingConcurrentQueue<T> _queue;
	std::atomic<int32_t> _unblockingCounter; // 1->32767:unblocking one 2,147,483,648:draining all
	volatile bool _isUnblocking;
	const int TIMEOUT_US = 1000;

public:
	/// <summary>Producer token</summary>
	typedef moodycamel::ProducerToken ProducerToken;
	/// <summary>Consumer token</summary>
	typedef moodycamel::ConsumerToken ConsumerToken;

	/// <summary>Constructor</summary>
	LockedQueue() : _unblockingCounter(0), _isUnblocking(false) {}

	/// <summary>Get a consumer token for this queue</summary>
	/// <returns>A consumer token</returns>
	ConsumerToken getConsumerToken() { return ConsumerToken(_queue); }

	/// <summary>Get a producer token for this queue</summary>
	/// <returns>A producer token</returns>
	ProducerToken getProducerToken() { return ProducerToken(_queue); }

	/// <summary>Produce: Enqueue an item in the queue.</summary>
	/// <param name="item">The item that will be enqueued. Will be copied into the queue.</param>
	void produce(const T& item) { _queue.enqueue(item); }

	/// <summary>Produce: Enqueue an item in the queue, using a producer token.
	/// Will generally be more efficient to use tokens.</summary>
	/// <param name="token">The ProducerToken. May be used to create "private" subqueues to avoid thread contention.</param>
	/// <param name="item">The item that will be enqueued. Will be copied into the queue.</param>
	void produce(ProducerToken& token, const T& item) { _queue.enqueue(token, item); }

	/// <summary>Produce: Enqueue multiple items in the queue.
	/// Will be very efficient.</summary>
	/// <param name="items">A type that will act as an iterator. Typically a C-pointer</param>
	/// <param name="numItems">The number if items to enqueue from <paramRef name="items"/></param>
	/// <typeparam name="iterator">Type Inferred: The type of the iterator <paramRef name="items"></typeparam>
	template<typename iterator>
	void produceMultiple(iterator items, uint32_t numItems)
	{
		_queue.enqueue_bulk(items, numItems);
	}

	/// <summary>Produce: Enqueue multiple items in the queue.
	/// Will be very the most efficient way of enqueueing multiple items.</summary>
	/// <param name="items">A type that will act as an iterator. Typically a C-pointer</param>
	/// <param name="numItems">The number if items to enqueue from <paramRef name="items"/></param>
	/// <param name="token">The ProducerToken. May be used to create "private" subqueues to avoid thread contention.</param>
	/// <typeparam name="iterator">Type Inferred: The type of the iterator <paramRef name="items"></typeparam>
	template<typename iterator>
	void produceMultiple(ProducerToken& token, iterator items, uint32_t numItems)
	{
		_queue.enqueue_bulk(token, items, numItems);
	}

	// This function will test if there is spend an unblock token. Should be called once whenever a dequeue
	inline bool tryUnblockAndReturnTrueIfUnblocked()
	{
		// times out
		if (_isUnblocking)
		{
			// Spend an "unblock token"
			int32_t howManyUnblocks = _unblockingCounter--;
			if (howManyUnblocks == 1)
			{
				// Finish unblocking if all tokens are spent
				_isUnblocking = false;
			} // Last one!
			if (howManyUnblocks > 0)
			{
				// If you didn't spend a token that was not there, break free, you are unblocked
				return true;
			}
			++_unblockingCounter; // Nope, not unblocking, at least not for THIS thread. Revert your change, and keep waiting - token is already spent.
		}
		return false;
	}

	/// <summary>Blocking consume: Dequeue one item from the queue. If returns false, the queue was finishing.</summary>
	/// <param name="item">Output variable: The item that was dequeued.</param>
	/// <returns>True if an item was dequeued, otherwise false</returns>
	bool consume(T& item)
	{
		bool result = false;
		// Spins until you either an item is consumed (returns "result" number of items), or an "unblock" token is consumed.
		while (!(result = _queue.wait_dequeue_timed(item, TIMEOUT_US)))
		{
			if (tryUnblockAndReturnTrueIfUnblocked()) { break; }
		}
		assert(_unblockingCounter >= 0);
		return result;
	}

	/// <summary>Blocking consume: Dequeue one item from the queue. If returns false, the queue was finishing.</summary>
	/// <param name="item">Output variable: The item that was dequeued.</param>
	/// <param name="token">A ConsumerToken. May be used to reduce thread contention.</param>
	/// <returns>True if an item was dequeued, otherwise false</returns>
	bool consume(ConsumerToken& token, T& item)
	{
		bool result = false;
		// Spins until you either an item is consumed (returns "result" number of items), or an "unblock" token is consumed.
		while (!(result = _queue.wait_dequeue_timed(token, item, TIMEOUT_US)))
		{
			if (tryUnblockAndReturnTrueIfUnblocked()) { break; }
		}
		assert(_unblockingCounter >= 0);
		return result;
	}

	/// <summary>Blocking consume multiple: Dequeue multiple items from the queue. Will return at least one item, unless the
	/// queue was closing down. If returns 0 items, the queue was finishing.</summary>
	/// <param name="firstItem">Output iterator variable: The items will be dequeued using this iterator. Usually a C-pointer</param>
	/// <param name="maxItems">The maximum number of items that will be dequeued</param>
	/// <returns>The number of items dequeued. Will only be 0 if the queue was finishing.</returns>
	/// <typeparam name="iterator">Type Inferred: The type of the iterator <paramRef name="items"></typeparam>
	template<typename iterator>
	size_t consumeMultiple(iterator firstItem, uint32_t maxItems)
	{
		bool result = false;
		// Spins until you either an item is consumed (returns "result" number of items), or an "unblock" token is consumed.
		while (!(result = _queue.wait_dequeue_bulk_timed(firstItem, maxItems, TIMEOUT_US)))
		{
			if (tryUnblockAndReturnTrueIfUnblocked()) { break; }
		}
		assert(_unblockingCounter >= 0);
		return result;
	}

	/// <summary>Blocking consume multiple with token: Dequeue multiple items from the queue. Will return at least one item, unless the
	/// queue was closing down. If returns 0 items, the queue was finishing.</summary>
	/// <param name="firstItem">Output iterator variable: The items will be dequeued using this iterator. Usually a C-pointer</param>
	/// <param name="maxItems">The maximum number of items that will be dequeued</param>
	/// <param name="token">A ConsumerToken. May be used to reduce thread contention.</param>
	/// <returns>The number of items dequeued. Will only be 0 if the queue was finishing.</returns>
	/// <typeparam name="iterator">Type Inferred: The type of the iterator <paramRef name="items"></typeparam>
	template<typename iterator>
	size_t consumeMultiple(ConsumerToken& token, iterator firstItem, uint32_t maxItems)
	{
		size_t result = 0;
		while (!(result = _queue.wait_dequeue_bulk_timed(token, firstItem, maxItems, TIMEOUT_US)))
		{
			if (tryUnblockAndReturnTrueIfUnblocked()) { break; }
		}
		assert(_unblockingCounter >= 0);
		return result;
	}

	/// <summary>Release one consumer from the queue. Used to release waiting threads when shutting down.</summary>
	void unblockOne()
	{
		_isUnblocking = true;
		_unblockingCounter += 1;
	}

	/// <summary>Release multiple consumers from the queue. Used to release waiting threads when shutting down.</summary>
	/// <param name="numUnblock">The number of consumers to unblock</param>
	void unblockMultiple(uint16_t numUnblock)
	{
		_isUnblocking = true;
		_unblockingCounter += numUnblock;
	}

	/// <summary>Only call on an empty, otherwise behaviour is undefined.</summary>
	void reset()
	{
		_unblockingCounter = 0;
		_isUnblocking = false;
	}

	/// <summary>Check if the queue is (tentatively) empty - this is an approximation due to multithreading.
	/// (To get a precise number the queue should have be blocked for all access and until it was used.)</summary>
	/// <returns>True if the thread is empty, otherwise false</returns>
	bool isEmpty() { return itemsRemainingApprox() == 0; }

	/// <summary>Get the number of (tentative) items in the queue - this is an approximation due to multithreading.
	/// (To get a precise number the queue should have be blocked for all access and until it was used.)</summary>
	/// <returns>The number of produced items</returns>
	size_t itemsRemainingApprox() { return _queue.size_approx(); }

	/// <summary>Always call this after you stop enqueueing items. Waits for all consumers to finish consuming,
	/// and then signals the queue to finish (unblocks all consumers on an empty queue)</summary>
	void waitUntilEmpty()
	{
		while (_queue.size_approx())
		{
			while (_queue.size_approx())
			{
				// allow the consumers to consume
				std::this_thread::sleep_for(std::chrono::microseconds(10 * _queue.size_approx()));
			}
			// just for good measure...
			std::this_thread::sleep_for(std::chrono::microseconds(50));
		}
		done();
	}

	/// <summary>Immediately signal all consumers to unblock and stop, refusing to enqueue any more items.
	/// The queue will be inconsistent afterwards and should be destroyed.</summary>
	void done()
	{
		_unblockingCounter = (std::numeric_limits<int>::max() / 2); // Robustness. Allow user to still call unblock without overflow
		_isUnblocking = true;
	}
}; // namespace pvr
} // namespace pvr
