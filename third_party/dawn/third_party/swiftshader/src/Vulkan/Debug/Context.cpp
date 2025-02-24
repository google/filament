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

#include "Context.hpp"

#include "EventListener.hpp"
#include "File.hpp"
#include "Thread.hpp"
#include "Variable.hpp"
#include "WeakMap.hpp"

#include "System/Debug.hpp"

#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#if !defined(NDEBUG) || defined(DCHECK_ALWAYS_ON)
#	define CHECK_REENTRANT_CONTEXT_LOCKS 1
#else
#	define CHECK_REENTRANT_CONTEXT_LOCKS 0
#endif

namespace {

#if CHECK_REENTRANT_CONTEXT_LOCKS
thread_local std::unordered_set<const void *> contextsWithLock;
#endif

////////////////////////////////////////////////////////////////////////////////
// Broadcaster - template base class for ServerEventBroadcaster and
// ClientEventBroadcaster
////////////////////////////////////////////////////////////////////////////////
template<typename Listener>
class Broadcaster : public Listener
{
public:
	void add(Listener *);
	void remove(Listener *);

protected:
	template<typename F>
	inline void foreach(F &&);

	template<typename F>
	inline void modify(F &&);

	using ListenerSet = std::unordered_set<Listener *>;
	std::recursive_mutex mutex;
	std::shared_ptr<ListenerSet> listeners = std::make_shared<ListenerSet>();
	int listenersInUse = 0;
};

template<typename Listener>
void Broadcaster<Listener>::add(Listener *l)
{
	modify([&]() { listeners->emplace(l); });
}

template<typename Listener>
void Broadcaster<Listener>::remove(Listener *l)
{
	modify([&]() { listeners->erase(l); });
}

template<typename Listener>
template<typename F>
void Broadcaster<Listener>::foreach(F &&f)
{
	std::unique_lock<std::recursive_mutex> lock(mutex);
	++listenersInUse;
	auto copy = listeners;
	for(auto l : *copy) { f(l); }
	--listenersInUse;
}

template<typename Listener>
template<typename F>
void Broadcaster<Listener>::modify(F &&f)
{
	std::unique_lock<std::recursive_mutex> lock(mutex);
	if(listenersInUse > 0)
	{
		// The listeners map is current being iterated over.
		// Make a copy before making the edit.
		listeners = std::make_shared<ListenerSet>(*listeners);
	}
	f();
}

////////////////////////////////////////////////////////////////////////////////
// ServerEventBroadcaster
////////////////////////////////////////////////////////////////////////////////
class ServerEventBroadcaster : public Broadcaster<vk::dbg::ServerEventListener>
{
public:
	using Thread = vk::dbg::Thread;

	void onThreadStarted(Thread::ID id) override
	{
		foreach([&](auto *l) { l->onThreadStarted(id); });
	}

	void onThreadStepped(Thread::ID id) override
	{
		foreach([&](auto *l) { l->onThreadStepped(id); });
	}

	void onLineBreakpointHit(Thread::ID id) override
	{
		foreach([&](auto *l) { l->onLineBreakpointHit(id); });
	}

	void onFunctionBreakpointHit(Thread::ID id) override
	{
		foreach([&](auto *l) { l->onFunctionBreakpointHit(id); });
	}
};

////////////////////////////////////////////////////////////////////////////////
// ClientEventBroadcaster
////////////////////////////////////////////////////////////////////////////////
class ClientEventBroadcaster : public Broadcaster<vk::dbg::ClientEventListener>
{
public:
	void onSetBreakpoint(const vk::dbg::Location &location, bool &handled) override
	{
		foreach([&](auto *l) { l->onSetBreakpoint(location, handled); });
	}

	void onSetBreakpoint(const std::string &func, bool &handled) override
	{
		foreach([&](auto *l) { l->onSetBreakpoint(func, handled); });
	}

	void onBreakpointsChanged() override
	{
		foreach([&](auto *l) { l->onBreakpointsChanged(); });
	}
};

}  // namespace

namespace vk {
namespace dbg {

////////////////////////////////////////////////////////////////////////////////
// Context::Impl
////////////////////////////////////////////////////////////////////////////////
class Context::Impl : public Context
{
public:
	// Context compliance
	Lock lock() override;
	void addListener(ClientEventListener *) override;
	void removeListener(ClientEventListener *) override;
	ClientEventListener *clientEventBroadcast() override;
	void addListener(ServerEventListener *) override;
	void removeListener(ServerEventListener *) override;
	ServerEventListener *serverEventBroadcast() override;

	void addFile(const std::shared_ptr<File> &file);

	ServerEventBroadcaster serverEventBroadcaster;
	ClientEventBroadcaster clientEventBroadcaster;

	std::mutex mutex;
	std::unordered_map<std::thread::id, std::shared_ptr<Thread>> threadsByStdId;
	std::unordered_set<std::string> functionBreakpoints;
	std::unordered_map<std::string, std::vector<int>> pendingBreakpoints;
	WeakMap<Thread::ID, Thread> threads;
	WeakMap<File::ID, File> files;
	WeakMap<Frame::ID, Frame> frames;
	WeakMap<Scope::ID, Scope> scopes;
	WeakMap<Variables::ID, Variables> variables;
	Thread::ID nextThreadID = 1;
	File::ID nextFileID = 1;
	Frame::ID nextFrameID = 1;
	Scope::ID nextScopeID = 1;
};

Context::Lock Context::Impl::lock()
{
	return Lock(this);
}

void Context::Impl::addListener(ClientEventListener *l)
{
	clientEventBroadcaster.add(l);
}

void Context::Impl::removeListener(ClientEventListener *l)
{
	clientEventBroadcaster.remove(l);
}

ClientEventListener *Context::Impl::clientEventBroadcast()
{
	return &clientEventBroadcaster;
}

void Context::Impl::addListener(ServerEventListener *l)
{
	serverEventBroadcaster.add(l);
}

void Context::Impl::removeListener(ServerEventListener *l)
{
	serverEventBroadcaster.remove(l);
}

ServerEventListener *Context::Impl::serverEventBroadcast()
{
	return &serverEventBroadcaster;
}

void Context::Impl::addFile(const std::shared_ptr<File> &file)
{
	files.add(file->id, file);

	auto it = pendingBreakpoints.find(file->name);
	if(it != pendingBreakpoints.end())
	{
		for(auto line : it->second)
		{
			file->addBreakpoint(line);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Context
////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Context> Context::create()
{
	return std::shared_ptr<Context>(new Context::Impl());
}

////////////////////////////////////////////////////////////////////////////////
// Context::Lock
////////////////////////////////////////////////////////////////////////////////
Context::Lock::Lock(Impl *ctx)
    : ctx(ctx)
{
#if CHECK_REENTRANT_CONTEXT_LOCKS
	ASSERT_MSG(contextsWithLock.count(ctx) == 0, "Attempting to acquire Context lock twice on same thread. This will deadlock");
	contextsWithLock.emplace(ctx);
#endif
	ctx->mutex.lock();
}

Context::Lock::Lock(Lock &&o)
    : ctx(o.ctx)
{
	o.ctx = nullptr;
}

Context::Lock::~Lock()
{
	unlock();
}

Context::Lock &Context::Lock::operator=(Lock &&o)
{
	ctx = o.ctx;
	o.ctx = nullptr;
	return *this;
}

void Context::Lock::unlock()
{
	if(ctx)
	{
#if CHECK_REENTRANT_CONTEXT_LOCKS
		contextsWithLock.erase(ctx);
#endif

		ctx->mutex.unlock();
		ctx = nullptr;
	}
}

std::shared_ptr<Thread> Context::Lock::currentThread()
{
	auto threadIt = ctx->threadsByStdId.find(std::this_thread::get_id());
	if(threadIt != ctx->threadsByStdId.end())
	{
		return threadIt->second;
	}
	auto id = ++ctx->nextThreadID;
	char name[256];
	snprintf(name, sizeof(name), "Thread<0x%x>", id.value());

	auto thread = std::make_shared<Thread>(id, ctx);
	ctx->threads.add(id, thread);
	thread->setName(name);
	ctx->threadsByStdId.emplace(std::this_thread::get_id(), thread);

	ctx->serverEventBroadcast()->onThreadStarted(id);

	return thread;
}

std::shared_ptr<Thread> Context::Lock::get(Thread::ID id)
{
	return ctx->threads.get(id);
}

std::vector<std::shared_ptr<Thread>> Context::Lock::threads()
{
	std::vector<std::shared_ptr<Thread>> out;
	out.reserve(ctx->threads.approx_size());
	for(auto it : ctx->threads)
	{
		out.push_back(it.second);
	}
	return out;
}

std::shared_ptr<File> Context::Lock::createVirtualFile(const std::string &name,
                                                       const std::string &source)
{
	auto file = File::createVirtual(ctx->nextFileID++, name, source);
	ctx->addFile(file);
	return file;
}

std::shared_ptr<File> Context::Lock::createPhysicalFile(const std::string &path)
{
	auto file = File::createPhysical(ctx->nextFileID++, path);
	ctx->addFile(file);
	return file;
}

std::shared_ptr<File> Context::Lock::get(File::ID id)
{
	return ctx->files.get(id);
}

std::shared_ptr<File> Context::Lock::findFile(const std::string &path)
{
	for(auto it : ctx->files)
	{
		auto &file = it.second;
		if(file->path() == path)
		{
			return file;
		}
	}
	return nullptr;
}

std::vector<std::shared_ptr<File>> Context::Lock::files()
{
	std::vector<std::shared_ptr<File>> out;
	out.reserve(ctx->files.approx_size());
	for(auto it : ctx->files)
	{
		out.push_back(it.second);
	}
	return out;
}

std::shared_ptr<Frame> Context::Lock::createFrame(
    const std::shared_ptr<File> &file, std::string function)
{
	auto frame = std::make_shared<Frame>(ctx->nextFrameID++, std::move(function));
	ctx->frames.add(frame->id, frame);
	frame->arguments = createScope(file);
	frame->locals = createScope(file);
	frame->registers = createScope(file);
	frame->hovers = createScope(file);
	frame->location.file = file;
	return frame;
}

std::shared_ptr<Frame> Context::Lock::get(Frame::ID id)
{
	return ctx->frames.get(id);
}

std::shared_ptr<Scope> Context::Lock::createScope(
    const std::shared_ptr<File> &file)
{
	auto scope = std::make_shared<Scope>(ctx->nextScopeID++, file, std::make_shared<VariableContainer>());
	ctx->scopes.add(scope->id, scope);
	return scope;
}

std::shared_ptr<Scope> Context::Lock::get(Scope::ID id)
{
	return ctx->scopes.get(id);
}

void Context::Lock::track(const std::shared_ptr<Variables> &vars)
{
	ctx->variables.add(vars->id, vars);
}

std::shared_ptr<Variables> Context::Lock::get(Variables::ID id)
{
	return ctx->variables.get(id);
}

void Context::Lock::clearFunctionBreakpoints()
{
	ctx->functionBreakpoints.clear();
}

void Context::Lock::addFunctionBreakpoint(const std::string &name)
{
	ctx->functionBreakpoints.emplace(name);
}

void Context::Lock::addPendingBreakpoints(const std::string &filename, const std::vector<int> &lines)
{
	ctx->pendingBreakpoints.emplace(filename, lines);
}

bool Context::Lock::isFunctionBreakpoint(const std::string &name)
{
	return ctx->functionBreakpoints.count(name) > 0;
}

std::unordered_set<std::string> Context::Lock::getFunctionBreakpoints()
{
	return ctx->functionBreakpoints;
}

}  // namespace dbg
}  // namespace vk
