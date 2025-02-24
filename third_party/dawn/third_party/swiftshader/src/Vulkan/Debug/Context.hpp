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

#ifndef VK_DEBUG_CONTEXT_HPP_
#define VK_DEBUG_CONTEXT_HPP_

#include "ID.hpp"

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace vk {
namespace dbg {

// Forward declarations.
class Thread;
class File;
class Frame;
class Scope;
class Variables;
class ClientEventListener;
class ServerEventListener;

// Context holds the full state of the debugger, including all current files,
// threads, frames and variables. It also holds a list of EventListeners that
// can be broadcast to using the Context::broadcast() interface.
// Context requires locking before accessing any state. The lock is
// non-reentrant and careful use is required to prevent accidentical
// double-locking by the same thread.
class Context
{
	class Impl;

public:
	// Lock is the interface to the Context's state.
	// The lock is automatically released when the Lock is destructed.
	class Lock
	{
	public:
		Lock(Impl *);
		Lock(Lock &&);
		~Lock();

		// move-assignment operator.
		Lock &operator=(Lock &&);

		// unlock() explicitly unlocks before the Lock destructor is called.
		// It is illegal to call any other methods after calling unlock().
		void unlock();

		// currentThread() creates (or returns an existing) a Thread that
		// represents the currently executing thread.
		std::shared_ptr<Thread> currentThread();

		// get() returns the thread with the given ID, or null if the thread
		// does not exist or no longer has any external shared_ptr references.
		std::shared_ptr<Thread> get(ID<Thread>);

		// threads() returns the full list of threads that still have an
		// external shared_ptr reference.
		std::vector<std::shared_ptr<Thread>> threads();

		// createVirtualFile() returns a new file that is not backed by the
		// filesystem.
		// name is the unique name of the file.
		// source is the content of the file.
		std::shared_ptr<File> createVirtualFile(const std::string &name,
		                                        const std::string &source);

		// createPhysicalFile() returns a new file that is backed by the file
		// at path.
		std::shared_ptr<File> createPhysicalFile(const std::string &path);

		// get() returns the file with the given ID, or null if the file
		// does not exist or no longer has any external shared_ptr references.
		std::shared_ptr<File> get(ID<File>);

		// findFile() returns the file with the given path, or nullptr if not
		// found.
		std::shared_ptr<File> findFile(const std::string &path);

		// files() returns the full list of files.
		std::vector<std::shared_ptr<File>> files();

		// createFrame() returns a new frame for the given file and function
		// name.
		std::shared_ptr<Frame> createFrame(
		    const std::shared_ptr<File> &file, std::string function);

		// get() returns the frame with the given ID, or null if the frame
		// does not exist or no longer has any external shared_ptr references.
		std::shared_ptr<Frame> get(ID<Frame>);

		// createScope() returns a new scope for the given file.
		std::shared_ptr<Scope> createScope(
		    const std::shared_ptr<File> &file);

		// get() returns the scope with the given ID, or null if the scope
		// does not exist.
		std::shared_ptr<Scope> get(ID<Scope>);

		// track() registers the variables with the context so it can be
		// retrieved by get(). Note that the context does not hold a strong
		// reference to the variables, and get() will return nullptr if all
		// strong external references are dropped.
		void track(const std::shared_ptr<Variables> &);

		// get() returns the variables with the given ID, or null if the
		// variables does not exist or no longer has any external shared_ptr
		// references.
		std::shared_ptr<Variables> get(ID<Variables>);

		// clearFunctionBreakpoints() removes all function breakpoints.
		void clearFunctionBreakpoints();

		// addFunctionBreakpoint() adds a breakpoint to the start of the
		// function with the given name.
		void addFunctionBreakpoint(const std::string &name);

		// addPendingBreakpoints() adds a number of breakpoints to the file with
		// the given name which has not yet been created with a call to
		// createVirtualFile() or createPhysicalFile().
		void addPendingBreakpoints(const std::string &name, const std::vector<int> &lines);

		// isFunctionBreakpoint() returns true if the function with the given
		// name has a function breakpoint set.
		bool isFunctionBreakpoint(const std::string &name);

		// getFunctionBreakpoints() returns all the set function breakpoints.
		std::unordered_set<std::string> getFunctionBreakpoints();

	private:
		Lock(const Lock &) = delete;
		Lock &operator=(const Lock &) = delete;
		Impl *ctx;
	};

	// create() creates and returns a new Context.
	static std::shared_ptr<Context> create();

	virtual ~Context() = default;

	// lock() returns a Lock which exclusively locks the context for state
	// access.
	virtual Lock lock() = 0;

	// addListener() registers an ClientEventListener for event notifications.
	virtual void addListener(ClientEventListener *) = 0;

	// removeListener() unregisters an ClientEventListener that was previously
	// registered by a call to addListener().
	virtual void removeListener(ClientEventListener *) = 0;

	// clientEventBroadcast() returns an ClientEventListener that will broadcast
	// all method calls on to all registered ServerEventListeners.
	virtual ClientEventListener *clientEventBroadcast() = 0;

	// addListener() registers an ServerEventListener for event notifications.
	virtual void addListener(ServerEventListener *) = 0;

	// removeListener() unregisters an ServerEventListener that was previously
	// registered by a call to addListener().
	virtual void removeListener(ServerEventListener *) = 0;

	// serverEventBroadcast() returns an ServerEventListener that will broadcast
	// all method calls on to all registered ServerEventListeners.
	virtual ServerEventListener *serverEventBroadcast() = 0;
};

}  // namespace dbg
}  // namespace vk

#endif  // VK_DEBUG_CONTEXT_HPP_
