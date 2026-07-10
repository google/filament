/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <utils/CallStack.h>
#include <utils/compiler.h>
#include <utils/CString.h>
#include <utils/debug/Mutex.h>
#include <utils/Log.h>
#include <utils/Logger.h>
#include <utils/Panic.h>
#include <utils/sstream.h>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <thread>
#include <vector>

namespace utils {
namespace debug {

namespace {

struct HeldLockInfo {
    const Mutex* mutex;
    CallStack lockStack;
};

struct ThreadLockState {
    static constexpr size_t MAX_HELD_LOCKS = 64;
    HeldLockInfo heldLocks[MAX_HELD_LOCKS];
    size_t count = 0;
    uint64_t threadId = 0;

    uint64_t getThreadId() noexcept {
        if (UTILS_UNLIKELY(threadId == 0)) {
            threadId = (uint64_t)std::hash<std::thread::id>()(std::this_thread::get_id());
            if (threadId == 0) {
                threadId = (uint64_t)(uintptr_t)this;
            }
        }
        return threadId;
    }
};

static thread_local ThreadLockState t_lockState;

class GraphSpinLock {
    std::atomic_flag mFlag = ATOMIC_FLAG_INIT;
public:
    void lock() noexcept {
        while (mFlag.test_and_set(std::memory_order_acquire)) {
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
            _mm_pause();
#elif defined(_MSC_VER) && (defined(_M_ARM64) || defined(_M_ARM))
            __yield();
#elif defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
            __builtin_ia32_pause();
#elif defined(__aarch64__) || defined(__arm__)
            __asm__ __volatile__("yield" ::: "memory");
#endif
        }
    }
    void unlock() noexcept {
        mFlag.clear(std::memory_order_release);
    }
};

struct LockEdge {
    const Mutex* from;
    const Mutex* to;
    CallStack fromLockStack;
    CallStack toLockStack;
    uint64_t threadId;
};

struct LockGraphState {
    std::vector<LockEdge> edges;
    std::vector<const Mutex*> nodes;
    GraphSpinLock spinlock;
};

LockGraphState& getLockGraphState() noexcept {
    static LockGraphState s_graphState;
    return s_graphState;
}

bool findCyclePath(const Mutex* startNode, const Mutex* targetNode,
        const LockGraphState& graph, std::vector<size_t>& outCycleEdges) noexcept {
    if (startNode == targetNode) {
        return true;
    }

    struct VisitedNode {
        const Mutex* mutex;
        size_t edgeIdx;
        size_t parentVisitedIdx;
    };
    std::vector<VisitedNode> visited;
    visited.reserve(32);
    visited.push_back({ startNode, size_t(-1), size_t(-1) });

    size_t queueHead = 0;
    while (queueHead < visited.size()) {
        const Mutex* curr = visited[queueHead].mutex;

        for (size_t eIdx = 0; eIdx < graph.edges.size(); ++eIdx) {
            const LockEdge& edge = graph.edges[eIdx];
            if (edge.from == curr) {
                if (edge.to == targetNode) {
                    outCycleEdges.push_back(eIdx);
                    size_t idx = queueHead;
                    while (idx != 0 && visited[idx].edgeIdx != size_t(-1)) {
                        outCycleEdges.push_back(visited[idx].edgeIdx);
                        idx = visited[idx].parentVisitedIdx;
                    }
                    std::reverse(outCycleEdges.begin(), outCycleEdges.end());
                    return true;
                }

                bool alreadyVisited = false;
                for (const VisitedNode& v : visited) {
                    if (v.mutex == edge.to) {
                        alreadyVisited = true;
                        break;
                    }
                }
                if (!alreadyVisited) {
                    visited.push_back({ edge.to, eIdx, queueHead });
                }
            }
        }
        queueHead++;
    }
    return false;
}

} // anonymous namespace

Mutex::~Mutex() noexcept {
    for (size_t i = 0; i < t_lockState.count; ++i) {
        if (UTILS_UNLIKELY(t_lockState.heldLocks[i].mutex == this)) {
            io::sstream stream;
            stream << "[FILAMENT CONCURRENCY PANIC] Destroying Mutex (0x" << (const void*)this
                   << ") while it is currently held by Thread (ID " << (unsigned long long)t_lockState.getThreadId() << ")!\n";
            LOG(ERROR) << stream.c_str();
            std::abort();
            for (size_t j = i; j + 1 < t_lockState.count; ++j) {
                t_lockState.heldLocks[j] = t_lockState.heldLocks[j + 1];
            }
            t_lockState.count--;
            break;
        }
    }

    LockGraphState& graph = getLockGraphState();
    graph.spinlock.lock();
    for (size_t i = 0; i < graph.edges.size(); ) {
        if (graph.edges[i].from == this || graph.edges[i].to == this) {
            graph.edges[i] = graph.edges.back();
            graph.edges.pop_back();
        } else {
            i++;
        }
    }
    for (size_t i = 0; i < graph.nodes.size(); ) {
        if (graph.nodes[i] == this) {
            graph.nodes[i] = graph.nodes.back();
            graph.nodes.pop_back();
        } else {
            i++;
        }
    }
    graph.spinlock.unlock();
}

CallStack const& Mutex::getCreationStack() const noexcept {
    ensureCreationStackCaptured();
    return mCreationStack;
}

void Mutex::ensureCreationStackCaptured() const noexcept {
    if (UTILS_UNLIKELY(!mCreationCaptured.load(std::memory_order_relaxed))) {
        bool expected = false;
        if (mCreationCaptured.compare_exchange_strong(expected, true, std::memory_order_relaxed)) {
            mCreationStack.update(4);
        }
    }
}

void Mutex::lock() UTILS_ACQUIRE() {
    ensureCreationStackCaptured();

    for (size_t i = 0; i < t_lockState.count; ++i) {
        if (UTILS_UNLIKELY(t_lockState.heldLocks[i].mutex == this)) {
            CallStack currentStack = CallStack::unwind(3);
            io::sstream stream;
            stream << "================================================================================\n";
            stream << "[FILAMENT CONCURRENCY PANIC] Self-Deadlock / Recursive Lock Detected!\n";
            stream << "================================================================================\n";
            stream << "Thread (ID " << (unsigned long long)t_lockState.getThreadId()
                   << ") attempted to recursively acquire non-recursive Lock (0x" << (const void*)this << ")!\n\n";
            stream << "--- Lock currently held, acquired at:\n" << t_lockState.heldLocks[i].lockStack << "\n";
            stream << "--- Attempted recursive acquisition at:\n" << currentStack << "\n";
            stream << "--- Mutex creation stack trace:\n" << getCreationStack() << "\n";
            stream << "================================================================================\n";
            LOG(ERROR) << stream.c_str();
            std::abort();
        }
    }

    CallStack currentStack = CallStack::unwind(3);

    if (t_lockState.count > 0) {
        LockGraphState& graph = getLockGraphState();
        graph.spinlock.lock();

        for (size_t i = 0; i < t_lockState.count; ++i) {
            const HeldLockInfo& heldInfo = t_lockState.heldLocks[i];
            const Mutex* L_i = heldInfo.mutex;

            bool edgeExists = false;
            for (const LockEdge& edge : graph.edges) {
                if (edge.from == L_i && edge.to == this) {
                    edgeExists = true;
                    break;
                }
            }

            if (!edgeExists) {
                std::vector<size_t> cycleEdges;
                if (findCyclePath(this, L_i, graph, cycleEdges)) {
                    io::sstream stream;
                    stream << "================================================================================\n";
                    stream << "[FILAMENT CONCURRENCY PANIC] Lock-Order Inversion / Potential Deadlock Detected!\n";
                    stream << "================================================================================\n";
                    stream << "Thread (ID " << (unsigned long long)t_lockState.getThreadId()
                           << ") attempted to acquire Lock (0x" << (const void*)this
                           << ") while holding Lock (0x" << (const void*)L_i << ")!\n";
                    stream << "Acquiring this lock creates a cyclic lock dependency graph across threads:\n\n";

                    for (size_t eIdx : cycleEdges) {
                        const LockEdge& edge = graph.edges[eIdx];
                        stream << "--- Lock (0x" << (const void*)edge.from
                               << ") acquired before Lock (0x" << (const void*)edge.to
                               << ") in Thread (ID " << (unsigned long long)edge.threadId << "):\n";
                        stream << "[Lock (0x" << (const void*)edge.from << ") locked at:]\n" << edge.fromLockStack << "\n";
                        stream << "[Lock (0x" << (const void*)edge.to << ") locked at:]\n" << edge.toLockStack << "\n";
                    }

                    stream << "--- BUT Lock (0x" << (const void*)L_i
                           << ") acquired before Lock (0x" << (const void*)this
                           << ") in Thread (ID " << (unsigned long long)t_lockState.getThreadId() << ") [CURRENT THREAD]:\n";
                    stream << "[Lock (0x" << (const void*)L_i << ") locked at:]\n" << heldInfo.lockStack << "\n";
                    stream << "[Lock (0x" << (const void*)this << ") attempted acquisition at:]\n" << currentStack << "\n";

                    stream << "--------------------------------------------------------------------------------\n";
                    stream << "Creation Stack Traces for Involved Locks:\n";
                    std::vector<const Mutex*> involvedLocks;
                    for (size_t eIdx : cycleEdges) {
                        const LockEdge& edge = graph.edges[eIdx];
                        bool foundFrom = false, foundTo = false;
                        for (const Mutex* m : involvedLocks) {
                            if (m == edge.from) foundFrom = true;
                            if (m == edge.to) foundTo = true;
                        }
                        if (!foundFrom) involvedLocks.push_back(edge.from);
                        if (!foundTo) involvedLocks.push_back(edge.to);
                    }
                    bool foundLi = false, foundThis = false;
                    for (const Mutex* m : involvedLocks) {
                        if (m == L_i) foundLi = true;
                        if (m == this) foundThis = true;
                    }
                    if (!foundLi) involvedLocks.push_back(L_i);
                    if (!foundThis) involvedLocks.push_back(this);

                    for (const Mutex* m : involvedLocks) {
                        stream << "Lock (0x" << (const void*)m << ") created at:\n" << m->getCreationStack() << "\n";
                    }
                    stream << "================================================================================\n";

                    graph.spinlock.unlock();
                    LOG(ERROR) << stream.c_str();
                    std::abort();
                } else {
                    graph.edges.push_back({ L_i, this, heldInfo.lockStack, currentStack, t_lockState.getThreadId() });
                }
            }
        }

        graph.spinlock.unlock();
    }

    mUnderlying.lock();

    if (UTILS_LIKELY(t_lockState.count < ThreadLockState::MAX_HELD_LOCKS)) {
        t_lockState.heldLocks[t_lockState.count++] = { this, currentStack };
    }
}

bool Mutex::try_lock() UTILS_TRY_ACQUIRE(true) {
    if (!mUnderlying.try_lock()) {
        return false;
    }

    ensureCreationStackCaptured();

    for (size_t i = 0; i < t_lockState.count; ++i) {
        if (UTILS_UNLIKELY(t_lockState.heldLocks[i].mutex == this)) {
            CallStack currentStack = CallStack::unwind(3);
            io::sstream stream;
            stream << "================================================================================\n";
            stream << "[FILAMENT CONCURRENCY PANIC] Self-Deadlock / Recursive Lock Detected!\n";
            stream << "================================================================================\n";
            stream << "Thread (ID " << (unsigned long long)t_lockState.getThreadId()
                   << ") attempted to recursively acquire non-recursive Lock (0x" << (const void*)this << ")!\n\n";
            stream << "--- Lock currently held, acquired at:\n" << t_lockState.heldLocks[i].lockStack << "\n";
            stream << "--- Attempted recursive acquisition at:\n" << currentStack << "\n";
            stream << "--- Mutex creation stack trace:\n" << getCreationStack() << "\n";
            stream << "================================================================================\n";
            LOG(ERROR) << stream.c_str();
            std::abort();
        }
    }

    CallStack currentStack = CallStack::unwind(3);

    if (t_lockState.count > 0) {
        LockGraphState& graph = getLockGraphState();
        graph.spinlock.lock();

        for (size_t i = 0; i < t_lockState.count; ++i) {
            const HeldLockInfo& heldInfo = t_lockState.heldLocks[i];
            const Mutex* L_i = heldInfo.mutex;

            bool edgeExists = false;
            for (const LockEdge& edge : graph.edges) {
                if (edge.from == L_i && edge.to == this) {
                    edgeExists = true;
                    break;
                }
            }

            if (!edgeExists) {
                std::vector<size_t> cycleEdges;
                if (findCyclePath(this, L_i, graph, cycleEdges)) {
                    io::sstream stream;
                    stream << "================================================================================\n";
                    stream << "[FILAMENT CONCURRENCY PANIC] Lock-Order Inversion / Potential Deadlock Detected!\n";
                    stream << "================================================================================\n";
                    stream << "Thread (ID " << (unsigned long long)t_lockState.getThreadId()
                           << ") attempted to acquire Lock (0x" << (const void*)this
                           << ") while holding Lock (0x" << (const void*)L_i << ")!\n";
                    stream << "Acquiring this lock creates a cyclic lock dependency graph across threads:\n\n";

                    for (size_t eIdx : cycleEdges) {
                        const LockEdge& edge = graph.edges[eIdx];
                        stream << "--- Lock (0x" << (const void*)edge.from
                               << ") acquired before Lock (0x" << (const void*)edge.to
                               << ") in Thread (ID " << (unsigned long long)edge.threadId << "):\n";
                        stream << "[Lock (0x" << (const void*)edge.from << ") locked at:]\n" << edge.fromLockStack << "\n";
                        stream << "[Lock (0x" << (const void*)edge.to << ") locked at:]\n" << edge.toLockStack << "\n";
                    }

                    stream << "--- BUT Lock (0x" << (const void*)L_i
                           << ") acquired before Lock (0x" << (const void*)this
                           << ") in Thread (ID " << (unsigned long long)t_lockState.getThreadId() << ") [CURRENT THREAD]:\n";
                    stream << "[Lock (0x" << (const void*)L_i << ") locked at:]\n" << heldInfo.lockStack << "\n";
                    stream << "[Lock (0x" << (const void*)this << ") attempted acquisition at:]\n" << currentStack << "\n";

                    stream << "--------------------------------------------------------------------------------\n";
                    stream << "Creation Stack Traces for Involved Locks:\n";
                    std::vector<const Mutex*> involvedLocks;
                    for (size_t eIdx : cycleEdges) {
                        const LockEdge& edge = graph.edges[eIdx];
                        bool foundFrom = false, foundTo = false;
                        for (const Mutex* m : involvedLocks) {
                            if (m == edge.from) foundFrom = true;
                            if (m == edge.to) foundTo = true;
                        }
                        if (!foundFrom) involvedLocks.push_back(edge.from);
                        if (!foundTo) involvedLocks.push_back(edge.to);
                    }
                    bool foundLi = false, foundThis = false;
                    for (const Mutex* m : involvedLocks) {
                        if (m == L_i) foundLi = true;
                        if (m == this) foundThis = true;
                    }
                    if (!foundLi) involvedLocks.push_back(L_i);
                    if (!foundThis) involvedLocks.push_back(this);

                    for (const Mutex* m : involvedLocks) {
                        stream << "Lock (0x" << (const void*)m << ") created at:\n" << m->getCreationStack() << "\n";
                    }
                    stream << "================================================================================\n";

                    graph.spinlock.unlock();
                    LOG(ERROR) << stream.c_str();
                    std::abort();
                } else {
                    graph.edges.push_back({ L_i, this, heldInfo.lockStack, currentStack, t_lockState.getThreadId() });
                }
            }
        }

        graph.spinlock.unlock();
    }

    if (UTILS_LIKELY(t_lockState.count < ThreadLockState::MAX_HELD_LOCKS)) {
        t_lockState.heldLocks[t_lockState.count++] = { this, currentStack };
    }
    return true;
}

void Mutex::unlock() UTILS_RELEASE() {
    bool found = false;
    for (size_t i = t_lockState.count; i > 0; --i) {
        if (t_lockState.heldLocks[i - 1].mutex == this) {
            for (size_t j = i - 1; j + 1 < t_lockState.count; ++j) {
                t_lockState.heldLocks[j] = t_lockState.heldLocks[j + 1];
            }
            t_lockState.count--;
            found = true;
            break;
        }
    }

    if (UTILS_UNLIKELY(!found)) {
        io::sstream stream;
        stream << "[FILAMENT CONCURRENCY PANIC] Attempting to unlock Mutex (0x" << (const void*)this
               << ") that is not held by Thread (ID " << (unsigned long long)t_lockState.getThreadId() << ")!\n";
        LOG(ERROR) << stream.c_str();
        std::abort();
    }

    mUnderlying.unlock();
}

} // namespace debug
} // namespace utils
