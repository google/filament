// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//	http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Reactor.hpp"

#include <memory>

#ifndef rr_ReactorCoroutine_hpp
#	define rr_ReactorCoroutine_hpp

namespace rr {

// Base class for the template Stream<T>
class StreamBase
{
protected:
	StreamBase(const std::shared_ptr<Routine> &routine, Nucleus::CoroutineHandle handle)
	    : routine(routine)
	    , handle(handle)
	{}

	~StreamBase()
	{
		auto pfn = (Nucleus::CoroutineDestroy *)routine->getEntry(Nucleus::CoroutineEntryDestroy);
		pfn(handle);
	}

	bool await(void *out)
	{
		auto pfn = (Nucleus::CoroutineAwait *)routine->getEntry(Nucleus::CoroutineEntryAwait);
		return pfn(handle, out);
	}

private:
	std::shared_ptr<Routine> routine;
	Nucleus::CoroutineHandle handle;
};

// Stream is the interface to a running Coroutine instance.
// A Coroutine may Yield() values of type T, which can be retrieved with
// await().
template<typename T>
class Stream : public StreamBase
{
public:
	inline Stream(const std::shared_ptr<Routine> &routine, Nucleus::CoroutineHandle handle)
	    : StreamBase(routine, handle)
	{}

	// await() retrieves the next yielded value from the coroutine.
	// Returns true if the coroutine yieled a value and out was assigned a
	// new value. If await() returns false, the coroutine has finished
	// execution and await() will return false for all future calls.
	inline bool await(T &out) { return StreamBase::await(&out); }
};

template<typename FunctionType>
class Coroutine;

// Coroutine constructs a reactor Coroutine function.
// rr::Coroutine is similar to rr::Function in that it builds a new
// executable function, but Coroutines have the following differences:
//  (1) Coroutines do not support Return() statements.
//  (2) Coroutines support Yield() statements to suspend execution of the
//      coroutine and pass a value up to the caller. Yield can be called
//      multiple times in a single execution of a coroutine.
//  (3) The template argument T to Coroutine<T> is a C-style function
//      signature.
//  (4) Coroutine::operator() returns a rr::Stream<T> instead of an
//      rr::Routine.
//  (5) operator() starts execution of the coroutine immediately.
//  (6) operator() uses the Coroutine's template function signature to
//      ensure the argument types match the generated function signature.
//
// Example usage:
//
//   // Build the coroutine function
//   Coroutine<int()> coroutine;
//   {
//       Yield(Int(0));
//       Yield(Int(1));
//       Int current = 1;
//       Int next = 1;
//       While(true) {
//           Yield(next);
//           auto tmp = current + next;
//           current = next;
//           next = tmp;
//       }
//   }
//
//   // Start the execution of the coroutine.
//   auto s = coroutine();
//
//   // Grab the first 20 yielded values and print them.
//   for(int i = 0; i < 20; i++)
//   {
//       int val = 0;
//       s->await(val);
//       printf("Fibonacci(%d): %d", i, val);
//   }
//
template<typename Return, typename... Arguments>
class Coroutine<Return(Arguments...)>
{
public:
	Coroutine();

	template<int index>
	using CArgumentType = typename std::tuple_element<index, std::tuple<Arguments...>>::type;

	template<int index>
	using RArgumentType = CToReactorT<CArgumentType<index>>;

	// Return the argument value with the given index.
	template<int index>
	Argument<RArgumentType<index>> Arg() const
	{
		Value *arg = Nucleus::getArgument(index);
		return Argument<RArgumentType<index>>(arg);
	}

	// Completes building of the coroutine and generates the coroutine's
	// executable code. After calling, no more reactor functions may be
	// called without building a new rr::Function or rr::Coroutine.
	// While automatically called by operator(), finalize() should be called
	// as soon as possible once the coroutine has been fully built.
	// finalize() *must* be called explicitly on the same thread that
	// instantiates the Coroutine instance if operator() is to be invoked on
	// different threads.
	inline void finalize(const char *name = "coroutine");

	// Starts execution of the coroutine and returns a unique_ptr to a
	// Stream<> that exposes the await() function for obtaining yielded
	// values.
	std::unique_ptr<Stream<Return>> operator()(Arguments...);

protected:
	std::unique_ptr<Nucleus> core;
	std::shared_ptr<Routine> routine;
	std::vector<Type *> arguments;
};

template<typename Return, typename... Arguments>
Coroutine<Return(Arguments...)>::Coroutine()
    : core(new Nucleus())
{
	std::vector<Type *> types = { CToReactorT<Arguments>::type()... };
	for(auto type : types)
	{
		if(type != Void::type())
		{
			arguments.push_back(type);
		}
	}

	Nucleus::createCoroutine(CToReactorT<Return>::type(), arguments);
}

template<typename Return, typename... Arguments>
void Coroutine<Return(Arguments...)>::finalize(const char *name /*= "coroutine"*/)
{
	if(core != nullptr)
	{
		routine = core->acquireCoroutine(name);
		core.reset(nullptr);
	}
}

template<typename Return, typename... Arguments>
std::unique_ptr<Stream<Return>>
Coroutine<Return(Arguments...)>::operator()(Arguments... args)
{
	finalize();

	// TODO(b/148400732): Go back to just calling the CoroutineEntryBegin function directly.
	std::function<Nucleus::CoroutineHandle()> coroutineBegin = [this, args...] {
		using Sig = Nucleus::CoroutineBegin<Arguments...>;
		auto pfn = (Sig *)routine->getEntry(Nucleus::CoroutineEntryBegin);
		auto handle = pfn(args...);
		return handle;
	};

	auto handle = Nucleus::invokeCoroutineBegin(*routine, coroutineBegin);

	return std::make_unique<Stream<Return>>(routine, handle);
}

#	ifdef Yield  // Defined in WinBase.h
#		undef Yield
#	endif

// Suspends execution of the coroutine and yields val to the caller.
// Execution of the coroutine will resume after val is retrieved.
template<typename T>
inline void Yield(const T &val)
{
	Nucleus::yield(ValueOf(val));
}

}  // namespace rr

#endif  // rr_ReactorCoroutine_hpp