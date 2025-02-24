// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This file contains a number of synchronization primitives for concurrency.
//
// You may be tempted to change this code to unlock the mutex before calling
// std::condition_variable::notify_[one,all]. Please read
// https://issuetracker.google.com/issues/133135427 before making this sort of
// change.

#ifndef sw_Synchronization_hpp
#define sw_Synchronization_hpp

#include "Debug.hpp"

#include <assert.h>
#include <chrono>
#include <condition_variable>
#include <queue>

#include "marl/event.h"
#include "marl/mutex.h"
#include "marl/waitgroup.h"

namespace sw {

// CountedEvent is an event that is signalled when the internal counter is
// decremented and reaches zero.
// The counter is incremented with calls to add() and decremented with calls to
// done().
class CountedEvent
{
public:
	// Constructs the CountedEvent with the initial signalled state set to the
	// provided value.
	CountedEvent(bool signalled = false)
	    : ev(marl::Event::Mode::Manual, signalled)
	{}

	// add() increments the internal counter.
	// add() must not be called when the event is already signalled.
	void add() const
	{
		ASSERT(!ev.isSignalled());
		wg.add();
	}

	// done() decrements the internal counter, signalling the event if the new
	// counter value is zero.
	// done() must not be called when the event is already signalled.
	void done() const
	{
		ASSERT(!ev.isSignalled());
		if(wg.done())
		{
			ev.signal();
		}
	}

	// reset() clears the signal state.
	// done() must not be called when the internal counter is non-zero.
	void reset() const
	{
		ev.clear();
	}

	// signalled() returns the current signal state.
	bool signalled() const
	{
		return ev.isSignalled();
	}

	// wait() waits until the event is signalled.
	void wait() const
	{
		ev.wait();
	}

	// wait() waits until the event is signalled or the timeout is reached.
	// If the timeout was reached, then wait() return false.
	template<class CLOCK, class DURATION>
	bool wait(const std::chrono::time_point<CLOCK, DURATION> &timeout) const
	{
		return ev.wait_until(timeout);
	}

	// event() returns the internal marl event.
	const marl::Event &event() { return ev; }

private:
	const marl::WaitGroup wg;
	const marl::Event ev;
};

// Chan is a thread-safe FIFO queue of type T.
// Chan takes its name after Golang's chan.
template<typename T>
class Chan
{
public:
	Chan();

	// take returns the next item in the chan, blocking until an item is
	// available.
	T take();

	// tryTake returns a <T, bool> pair.
	// If the chan is not empty, then the next item and true are returned.
	// If the chan is empty, then a default-initialized T and false are returned.
	std::pair<T, bool> tryTake();

	// put places an item into the chan, blocking if the chan is bounded and
	// full.
	void put(const T &v);

	// Returns the number of items in the chan.
	// Note: that this may change as soon as the function returns, so should
	// only be used for debugging.
	size_t count();

private:
	marl::mutex mutex;
	std::queue<T> queue GUARDED_BY(mutex);
	std::condition_variable added;
};

template<typename T>
Chan<T>::Chan()
{}

template<typename T>
T Chan<T>::take()
{
	marl::lock lock(mutex);
	// Wait for item to be added.
	lock.wait(added, [this]() REQUIRES(mutex) { return queue.size() > 0; });
	T out = queue.front();
	queue.pop();
	return out;
}

template<typename T>
std::pair<T, bool> Chan<T>::tryTake()
{
	marl::lock lock(mutex);
	if(queue.size() == 0)
	{
		return std::make_pair(T{}, false);
	}
	T out = queue.front();
	queue.pop();
	return std::make_pair(out, true);
}

template<typename T>
void Chan<T>::put(const T &item)
{
	marl::lock lock(mutex);
	queue.push(item);
	added.notify_one();
}

template<typename T>
size_t Chan<T>::count()
{
	marl::lock lock(mutex);
	return queue.size();
}

}  // namespace sw

#endif  // sw_Synchronization_hpp
