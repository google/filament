/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef TNT_UTILS_JOBSYSTEM_H
#define TNT_UTILS_JOBSYSTEM_H

#include <utils/Allocator.h>
#include <utils/architecture.h>
#include <utils/compiler.h>
#include <utils/Condition.h>
#include <utils/Logger.h>
#include <utils/memalign.h>
#include <utils/Mutex.h>
#include <utils/ostream.h>
#include <utils/Slice.h>
#include <utils/WorkStealingDequeue.h>

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

namespace utils {

template <typename VALUE>
class ThreadMap;

namespace jobs::details {
template<typename S, typename F>
struct ParallelForJobData;
}

class JobSystem {
    static constexpr uint32_t MAX_THREADS = 32;
    static constexpr size_t MAX_JOB_COUNT = 1 << 14; // 16384
    static constexpr uint32_t JOB_COUNT_MASK = MAX_JOB_COUNT - 1;
    static constexpr uint32_t WAITER_COUNT_SHIFT = 24;
    static_assert(MAX_JOB_COUNT <= 0x7FFE, "MAX_JOB_COUNT must be <= 0x7FFE");
    using WorkQueue = WorkStealingDequeue<uint16_t, MAX_JOB_COUNT>;
    using Mutex = utils::Mutex;
    using Condition = utils::Condition;
    using UniqueLock = utils::UniqueLock<Mutex>;
    using LockGuard = utils::LockGuard<Mutex>;

public:
    class Job;

    using ThreadId = uint8_t;

    using JobFunc = void(*)(void*, JobSystem&, Job*);

    static constexpr ThreadId invalidThreadId = 0xff;

    class alignas(CACHELINE_SIZE) Job {
    public:
        Job() noexcept {} /* = default; */ /* clang bug */ // NOLINT(modernize-use-equals-default,cppcoreguidelines-pro-type-member-init)
        Job(const Job&) = delete;
        Job(Job&&) = delete;

        // Size is chosen so that we can store at least std::function<>
        // the alignas() qualifier ensures we're multiple of a cache-line.
        static constexpr size_t JOB_STORAGE_SIZE_BYTES =
                sizeof(std::function<void()>) > 48 ? sizeof(std::function<void()>) : 48;
        static constexpr size_t JOB_STORAGE_SIZE_WORDS =
                (JOB_STORAGE_SIZE_BYTES + sizeof(void*) - 1) / sizeof(void*);

    private:
        friend class JobSystem;

        // keep it first, so it's correctly aligned with all architectures
        // this is where we store the job's data, typically a std::function<>
                                                                // v7 | v8
        void* storage[JOB_STORAGE_SIZE_WORDS];                  // 48 | 48
        JobFunc function;                                       //  4 |  8
        uint16_t parent;                                        //  2 |  2
        mutable ThreadId id = invalidThreadId;                  //  1 |  1
        mutable std::atomic<uint8_t> refCount = { 1 };          //  1 |  1
        std::atomic<uint32_t> runningJobCount = { 1 };          //  4 |  4
                                                                //  4 |  0 (padding)
                                                                // 64 | 64
    };

#ifndef WIN32
    // on windows std::function<void()> is bigger and forces the whole structure to be larger
    static_assert(sizeof(Job) == 64);
#endif

    /**
     * Special value for userThreadCount to configure the JobSystem in single-threaded mode.
     */
    static constexpr uint32_t SINGLE_THREADED = std::numeric_limits<uint32_t>::max();
    /**
     * Create a JobSystem and initialize its thread pool.
     *
     * Total thread capacity across pool workers and adopted threads is capped at MAX_THREADS (32).
     *
     * Parameter constraints & behavior:
     * - `userThreadCount`: Number of dedicated worker threads to spawn in the pool.
     *   - `0` (default): Automatically selects `(hardware_concurrency - 1)`.
     *   - `SINGLE_THREADED`: Configures the JobSystem in single-threaded mode with 0 pool worker
     *     threads and forces `adoptableThreadsCount` to 1.
     *   - In multi-threaded mode (any value other than `SINGLE_THREADED`), the thread pool is
     *     guaranteed to contain at least 1 worker thread (subject to system threading support).
     * - `adoptableThreadsCount`: Maximum number of external calling threads that can concurrently
     *   call `adopt()`.
     *   - Must be between 1 and `MAX_THREADS - 1` (31) in multi-threaded mode to ensure capacity
     *     for at least 1 pool worker thread.
     *   - If `adoptableThreadsCount >= MAX_THREADS`, it is clamped to `MAX_THREADS - 1` with an error.
     *   - If `userThreadCount + adoptableThreadsCount > MAX_THREADS`, `userThreadCount` is clamped to
     *     `(MAX_THREADS - adoptableThreadsCount)` with a warning.
     *
     * @param userThreadCount Number of worker threads in the pool, 0 for auto-detect, or SINGLE_THREADED.
     * @param adoptableThreadsCount Maximum number of external threads that can be adopted concurrently (max: MAX_THREADS - 1).
     */
    explicit JobSystem(uint32_t userThreadCount = 0, uint32_t adoptableThreadsCount = 1) noexcept;

    ~JobSystem();

    // Make the current thread part of the thread pool.
    void adopt();

    // Remove this adopted thread from the parent. This is intended to be used for
    // shutting down a JobSystem. In particular, this doesn't allow the parent to
    // adopt more thread.
    void emancipate();


    // If a parent is not specified when creating a job, that job will automatically take the
    // root job as a parent.
    // The root job is reset when waited on.
    Job* setRootJob(Job* job) noexcept { return mRootJob = job; }

     // use setRootJob() instead
    UTILS_DEPRECATED
    Job* setMasterJob(Job* job) noexcept { return setRootJob(job); }


    Job* create(Job* parent, JobFunc func) noexcept;

    // NOTE: All methods below must be called from the same thread and that thread must be
    // owned by JobSystem's thread pool.

    /*
     * Job creation examples:
     * ----------------------
     *
     *  struct Functor {
     *   uintptr_t storage[6];
     *   void operator()(JobSystem&, Jobsystem::Job*);
     *  } functor;
     *
     *  struct Foo {
     *   uintptr_t storage[6];
     *   void method(JobSystem&, Jobsystem::Job*);
     *  } foo;
     *
     *  Functor and Foo size muse be <= uintptr_t[6]
     *
     *   createJob()
     *   createJob(parent)
     *   createJob<Foo, &Foo::method>(parent, &foo)
     *   createJob<Foo, &Foo::method>(parent, foo)
     *   createJob<Foo, &Foo::method>(parent, std::ref(foo))
     *   createJob(parent, functor)
     *   createJob(parent, std::ref(functor))
     *   createJob(parent, [ up-to 6 uintptr_t ](JobSystem*, Jobsystem::Job*){ })
     *
     *  Utility functions:
     *  ------------------
     *    These are less efficient, but handle any size objects using the heap if needed.
     *    (internally uses std::function<>), and don't require the callee to take
     *    a (JobSystem&, Jobsystem::Job*) as parameter.
     *
     *  struct BigFoo {
     *   uintptr_t large[16];
     *   void operator()();
     *   void method(int answerToEverything);
     *   static void exec(BigFoo&) { }
     *  } bigFoo;
     *
     *   jobs::createJob(js, parent, [ any-capture ](int answerToEverything){}, 42);
     *   jobs::createJob(js, parent, &BigFoo::method, &bigFoo, 42);
     *   jobs::createJob(js, parent, &BigFoo::exec, std::ref(bigFoo));
     *   jobs::createJob(js, parent, bigFoo);
     *   jobs::createJob(js, parent, std::ref(bigFoo));
     *   etc...
     *
     *  struct SmallFunctor {
     *   uintptr_t storage[3];
     *   void operator()(T* data, size_t count);
     *  } smallFunctor;
     *
     *   jobs::parallel_for(js, data, count, [ up-to 3 uintptr_t ](T* data, size_t count) { });
     *   jobs::parallel_for(js, data, count, smallFunctor);
     *   jobs::parallel_for(js, data, count, std::ref(smallFunctor));
     *
     */

    // creates an empty (no-op) job with an optional parent
    Job* createJob(Job* parent = nullptr) noexcept {
        return create(parent, nullptr);
    }

    // creates a job from a KNOWN method pointer w/ object passed by pointer
    // the caller must ensure the object will outlive the Job
    template<typename T, void(T::*method)(JobSystem&, Job*)>
    Job* createJob(Job* parent, T* data) noexcept {
        Job* job = create(parent, +[](void* storage, JobSystem& js, Job* job) {
            T* const that = static_cast<T*>(static_cast<void**>(storage)[0]);
            (that->*method)(js, job);
        });
        if (job) {
            job->storage[0] = data;
        }
        return job;
    }

    // creates a job from a KNOWN method pointer w/ object passed by value
    template<typename T, void(T::*method)(JobSystem&, Job*)>
    Job* createJob(Job* parent, T data) noexcept {
        static_assert(sizeof(data) <= sizeof(Job::storage), "user data too large");
        Job* job = create(parent, [](void* storage, JobSystem& js, Job* job) {
            T* const that = static_cast<T*>(storage);
            (that->*method)(js, job);
            that->~T();
        });
        if (job) {
            new(job->storage) T(std::move(data));
        }
        return job;
    }

    // creates a job from a KNOWN method pointer w/ object passed by value
    template<typename T, void(T::*method)(JobSystem&, Job*), typename ... ARGS>
    Job* emplaceJob(Job* parent, ARGS&& ... args) noexcept {
        static_assert(sizeof(T) <= sizeof(Job::storage), "user data too large");
        Job* job = create(parent, [](void* storage, JobSystem& js, Job* job) {
            T* const that = static_cast<T*>(storage);
            (that->*method)(js, job);
            that->~T();
        });
        if (job) {
            new(job->storage) T(std::forward<ARGS>(args)...);
        }
        return job;
    }

    // creates a job from a functor passed by value
    template<typename T>
    Job* createJob(Job* parent, T functor) noexcept {
        static_assert(sizeof(functor) <= sizeof(Job::storage), "functor too large");
        Job* job = create(parent, [](void* storage, JobSystem& js, Job* job){
            T* const that = static_cast<T*>(storage);
            that->operator()(js, job);
            that->~T();
        });
        if (job) {
            new(job->storage) T(std::move(functor));
        }
        return job;
    }

    // creates a job from a functor passed by value
    template<typename T, typename ... ARGS>
    Job* emplaceJob(Job* parent, ARGS&& ... args) noexcept {
        static_assert(sizeof(T) <= sizeof(Job::storage), "functor too large");
        Job* job = create(parent, [](void* storage, JobSystem& js, Job* job){
            T* const that = static_cast<T*>(storage);
            that->operator()(js, job);
            that->~T();
        });
        if (job) {
            new(job->storage) T(std::forward<ARGS>(args)...);
        }
        return job;
    }

    // creates a job with in-place storage initialized with ARGS, without automatic destruction
    template<typename T, typename ... ARGS>
    Job* emplaceJobRaw(Job* parent, JobFunc const func, ARGS&& ... args) noexcept {
        static_assert(sizeof(T) <= sizeof(Job::storage), "user data too large");
        Job* job = create(parent, func);
        if (UTILS_LIKELY(job)) {
            new(job->storage) T(std::forward<ARGS>(args)...);
        }
        return job;
    }


    /*
     * Jobs are normally finished automatically, this can be used to cancel a job before it is run.
     *
     * Never use this once a flavor of run() has been called.
     */
    void cancel(Job*& job) noexcept;

    /*
     * Adds a reference to a Job.
     *
     * This allows the caller to waitAndRelease() on this job from multiple threads.
     * Use runAndWait() if waiting from multiple threads is not needed.
     *
     * This job MUST BE waited on with waitAndRelease(), or released with release().
     */
    static Job* retain(Job* job) noexcept;

    /*
     * Releases a reference from a Job obtained with runAndRetain() or a call to retain().
     *
     * The job can't be used after this call.
     */
    void release(Job*& job) noexcept;
    void release(Job*&& job) noexcept {
        Job* p = job;
        release(p);
    }

    /*
     * Add job to this thread's execution queue. Its reference will drop automatically.
     * The current thread must be owned by JobSystem's thread pool. See adopt().
     *
     * The job can't be used after this call.
     */
    void run(Job*& job) noexcept;
    void run(Job*&& job) noexcept { // allows run(createJob(...));
        Job* p = job;
        run(p);
    }

    /*
     * Add job to this thread's execution queue. Its reference will drop automatically.
     * The current thread must be owned by JobSystem's thread pool. See adopt().
     * id must be the current thread id obtained with JobSystem::getThreadId(Job*). This
     * API is more efficient than the methods above.
     *
     * The job can't be used after this call.
     */
    void run(Job*& job, ThreadId id) noexcept;
    void run(Job*&& job, ThreadId const id) noexcept { // allows run(createJob(...));
        Job* p = job;
        run(p, id);
    }

    /*
     * Add job to this thread's execution queue and keep a reference to it.
     * The current thread must be owned by JobSystem's thread pool. See adopt().
     *
     * This job MUST BE waited on with wait(), or released with release().
     */
    Job* runAndRetain(Job* job) noexcept;

    /*
     * Wait on a job and destroys it.
     * The current thread must be owned by JobSystem's thread pool. See adopt().
     *
     * The job must first be obtained from runAndRetain() or retain().
     * The job can't be used after this call.
     */
    void waitAndRelease(Job*& job) noexcept;

    /*
     * Runs and wait for a job. This is equivalent to calling
     *  runAndRetain(job);
     *  wait(job);
     *
     * The job can't be used after this call.
     */
    void runAndWait(Job*& job) noexcept;
    void runAndWait(Job*&& job) noexcept { // allows runAndWait(createJob(...));
        Job* p = job;
        runAndWait(p);
    }

    // for debugging
    friend io::ostream& operator << (io::ostream& out, JobSystem const& js);


    // utility functions...

    // set the name of the current thread (on OSes that support it)
    static void setThreadName(const char* threadName) noexcept;

    enum class Priority {
        NORMAL,
        DISPLAY,
        URGENT_DISPLAY,
        BACKGROUND
    };

    static void setThreadPriority(Priority priority) noexcept;
    static void setThreadAffinityById(size_t id) noexcept;

    size_t getThreadCount() const noexcept { return mThreadCount; }

    // returns the high-water mark of active threads (pool threads + adopted threads)
    size_t getActiveThreadCount() const noexcept {
        return mActiveThreadCount.load(std::memory_order_relaxed);
    }

    // returns the current ThreadId, which can be used with run(). This method can only be
    // called from a job's function.
    static ThreadId getThreadId(Job const* job) noexcept {
        assert(job->id != invalidThreadId);
        return job->id;
    }

private:
    // this is just to avoid using std::default_random_engine, since we're in a public header.
    class default_random_engine {
        static constexpr uint32_t m = 0x7fffffffu;
        uint32_t mState; // must be 0 < seed < 0x7fffffff
    public:
        using result_type = uint32_t;

        static constexpr result_type min() noexcept {
            return 1;
        }

        static constexpr result_type max() noexcept {
            return m - 1;
        }

        constexpr explicit default_random_engine(uint32_t const seed = 1u) noexcept
                : mState(((seed % m) == 0u) ? 1u : seed % m) {
        }

        uint32_t operator()() noexcept {
            return mState = uint32_t((uint64_t(mState) * 48271u) % m);
        }
    };

    struct alignas(CACHELINE_SIZE) ThreadState {
        // Keep nextJob at the beginning so that nextJob, workQueue's top/bottom indices,
        // and the initial workQueue slots all share the first 64-byte cache line.
        std::atomic<uint16_t> nextJob = { 0 };
        WorkQueue workQueue;

        // these are not accessed frequently by other threads
        alignas(CACHELINE_SIZE)         // this causes 40-bytes padding
        JobSystem* js;                  // this is in fact const and always initialized
        std::thread thread;             // unused for adopted threads
        default_random_engine rndGen;
    };

    static_assert(sizeof(ThreadState) % CACHELINE_SIZE == 0,
            "ThreadState doesn't align to a cache line");

    ThreadState& getState();

    static void incRef(Job const* job) noexcept;
    void decRef(Job const* job) noexcept;

    Job* allocateJob() noexcept;
    ThreadState* getStateToStealFrom(ThreadState& state) noexcept;
    static bool hasJobCompleted(Job const* job) noexcept;

    void requestExit() noexcept;
    bool exitRequested() const noexcept;
    bool hasActiveJobs() const noexcept;

    void loop(ThreadState* state);
    bool execute(ThreadState& state) noexcept;
    Job* steal(ThreadState& state) noexcept;
    void finish(Job* job) noexcept;

    void put(ThreadState& state, Job const* job) noexcept;
    Job* pop(ThreadState& state) noexcept;
    Job* stealFrom(ThreadState& victim) noexcept;

    [[nodiscard]]
    uint32_t wait(UniqueLock& lock, Job* job) noexcept;
    void waitForWork(UniqueLock& lock) noexcept;
    void waitForJob(UniqueLock& lock) noexcept;
    void wakeWaiters() noexcept;
    void wakeOne() noexcept;

    static constexpr uint32_t SLEEPING_WORKER_ONE = 1 << 16;
    static constexpr uint32_t SLEEPING_WAITER_ONE = 1;
    static constexpr uint32_t SLEEPING_WORKER_MASK = 0xFFFF0000;
    static constexpr uint32_t SLEEPING_WAITER_MASK = 0x0000FFFF;

    // these have thread contention, keep them together
    Mutex mWaiterLock;
    Condition mWorkCondition;
    Condition mJobCondition;

    std::atomic<int32_t> mActiveJobs = { 0 };
    std::atomic<uint32_t> mSleepingCounts = { 0 };
    Arena<ThreadSafeObjectPoolAllocator<Job>, LockingPolicy::NoLock> mJobPool;

    template <typename T>
    using aligned_vector = std::vector<T, STLAlignedAllocator<T>>;

    // These are essentially const, make sure they're on a different cache-lines than the
    // read-write atomics.
    // We can't use "alignas(CACHELINE_SIZE)" because the standard allocator can't make this
    // guarantee.
    char padding[CACHELINE_SIZE];

    alignas(16) // at least we align to half (or quarter) cache-line
    aligned_vector<ThreadState> mThreadStates;          // actual data is stored offline
    std::atomic<bool> mExitRequested = { false };       // this one is almost never written
    std::atomic<uint32_t> mAdoptableSlotsMask = { 0 };  // available slots for adoptable threads
    std::atomic<uint16_t> mActiveThreadCount = { 0 };   // high-water mark of active threads
    Job* const mJobStorageBase;                         // Base for conversion to indices
    uint16_t mThreadCount = 0;                          // total # of threads in the pool
    Job* mRootJob = nullptr;

    std::unique_ptr<ThreadMap<ThreadState*>> mThreadMap;
};

// -------------------------------------------------------------------------------------------------
// Utility functions built on top of JobSystem

namespace jobs {

// These are convenience C++11 style job creation methods that support lambdas
//
// IMPORTANT: these are less efficient to call and may perform heap allocation
//            depending on the capture and parameters
//
template<typename CALLABLE, typename ... ARGS>
JobSystem::Job* createJob(JobSystem& js, JobSystem::Job* parent,
        CALLABLE&& func, ARGS&&... args) noexcept {
    struct Data {
        explicit Data(std::function<void()> f) noexcept: f(std::move(f)) {}
        std::function<void()> f;
        // Renaming the method below could cause an Arrested Development.
        void gob(JobSystem&, JobSystem::Job*) noexcept { f(); }
    };
    return js.emplaceJob<Data, &Data::gob>(parent,
            std::bind(std::forward<CALLABLE>(func), std::forward<ARGS>(args)...));
}

template<typename CALLABLE, typename T, typename ... ARGS,
        typename = std::enable_if_t<
                std::is_member_function_pointer_v<std::remove_reference_t<CALLABLE>>
        >
>
JobSystem::Job* createJob(JobSystem& js, JobSystem::Job* parent,
        CALLABLE&& func, T&& o, ARGS&&... args) noexcept {
    struct Data {
        explicit Data(std::function<void()> f) noexcept: f(std::move(f)) {}
        std::function<void()> f;
        // Renaming the method below could cause an Arrested Development.
        void gob(JobSystem&, JobSystem::Job*) noexcept { f(); }
    };
    return js.emplaceJob<Data, &Data::gob>(parent,
            std::bind(std::forward<CALLABLE>(func), std::forward<T>(o), std::forward<ARGS>(args)...));
}


/**
 * Policy for dividing parallel work into chunks.
 *
 * @tparam COUNT Minimum chunk size (number of elements per work unit) dynamically claimed by threads.
 * @tparam MAX_SPLITS Maximum split depth (caps the total number of chunks to 2^MAX_SPLITS).
 */
template<size_t COUNT = 1, size_t MAX_SPLITS = 12>
class CountSplitter {
public:
    static constexpr size_t CHUNK_SIZE = COUNT;
    static constexpr size_t MAX_CHUNKS = 1u << MAX_SPLITS;

    size_t getChunkSize(size_t const totalCount, size_t const /* threadCount */) const noexcept {
        size_t const dynamicChunk = (totalCount + MAX_CHUNKS - 1) >> MAX_SPLITS;
        return std::max<size_t>(COUNT, dynamicChunk);
    }
};

namespace details {

// Traits helper to extract the chunk size from a splitter.
// Splitters must define getChunkSize() or getChunkSize(count, threadCount).
template<typename S>
struct SplitterTraits {
    template<typename T>
    static auto test_0(int) -> decltype(std::declval<T>().getChunkSize(), std::true_type{});
    template<typename>
    static auto test_0(...) -> std::false_type;

    template<typename T>
    static auto test_2(int) -> decltype(std::declval<T>().getChunkSize(uint32_t(), uint32_t()), std::true_type{});
    template<typename>
    static auto test_2(...) -> std::false_type;

    static constexpr bool has_chunk_size_0 = decltype(test_0<S>(0))::value;
    static constexpr bool has_chunk_size_2 = decltype(test_2<S>(0))::value;

    static_assert(has_chunk_size_0 || has_chunk_size_2,
            "Custom splitter must define getChunkSize() or getChunkSize(count, threadCount). "
            "The old split() method is no longer supported.");

    static uint32_t getChunkSize(const S& splitter, uint32_t const totalCount, uint32_t const threadCount) noexcept {
        if constexpr (has_chunk_size_2) {
            return uint32_t(std::max<size_t>(1, splitter.getChunkSize(totalCount, std::max(1u, threadCount))));
        } else if constexpr (has_chunk_size_0) {
            return uint32_t(std::max<size_t>(1, splitter.getChunkSize()));
        }
    }
};

/*
 * ParallelForJobData coordinates dynamic range-based work stealing across threads.
 *
 * Architecture & Concurrency Model:
 * 1. Range-Based Stealing:
 *    Instead of recursively splitting child jobs into an O(N) binary tree, parallel_for creates
 *    a single root job and at most (threadCount - 1) child helper jobs.
 *    All workers share a single atomic iteration cursor (nextOffset). Each worker claims
 *    contiguous slices of chunkSize items via relaxed fetch_add until nextOffset >= count.
 *
 * 2. Memory Layout & Zero Heap Allocation:
 *    ParallelForJobData is constructed entirely within the root Job's inline storage
 *    (Job::storage, 48 bytes) with zero dynamic memory allocation.
 *    Functors must fit in Job::storage (<= 24 bytes for 8-byte aligned closures, or 28 bytes
 *    for 4-byte aligned closures). Large closures must be passed by reference via
 *    std::cref() or std::ref() (which are 8 bytes).
 *
 * 3. Distributed Lifetime Management:
 *    Because the root thread and helper tasks execute asynchronously, the lifetime of
 *    ParallelForJobData in the root Job's storage is ref-counted by activeWorkers.
 *    Every finishing worker (root or helper) atomically decrements activeWorkers with acq_rel.
 *    The last worker to finish explicitly calls `~ParallelForJobData()`.
 *    The Job struct in JobPool is safely reclaimed when all child jobs and the root job finish.
 */
template<typename S, typename F>
struct ParallelForJobData {
    using Functor = F;

    static_assert(std::is_invocable_v<const Functor&, uint32_t, uint32_t>,
            "parallel_for functor must be const-invocable because a single functor instance "
            "is shared and invoked concurrently across all worker threads. "
            "Mutable lambdas or functors with non-const operator() are prohibited. "
            "If mutable state is required across workers, pass it via thread-safe references (e.g. std::ref) "
            "with atomic or synchronized operations.");

    ParallelForJobData(uint32_t const start, uint32_t const count, uint32_t const chunkSize,
            uint32_t const totalWorkers, Functor functor) noexcept
            : start(start),
              count(count),
              chunkSize(std::max<uint32_t>(1, chunkSize)),
              activeWorkers(totalWorkers),
              functor(std::move(functor)) {
    }

    // Work-stealing loop: dynamically claims slices of chunkSize items until exhaustion.
    void process() noexcept {
        uint32_t offset;
        while ((offset = nextOffset.fetch_add(chunkSize, std::memory_order_relaxed)) < count) {
            uint32_t const chunk = std::min<uint32_t>(chunkSize, count - offset);
            std::as_const(functor)(start + offset, chunk);
        }
    }

    // Invoked by the root job: spawns helper jobs for other threads, then processes chunks itself.
    void runRoot(JobSystem& js, JobSystem::Job* root) noexcept {
        uint32_t const total = activeWorkers.load(std::memory_order_relaxed);
        uint32_t const helperCount = total > 0 ? total - 1 : 0;
        JobSystem::ThreadId const id = JobSystem::getThreadId(root);

        for (uint32_t i = 0; i < helperCount; ++i) {
            JobSystem::Job* helper =
                    js.createJob<ParallelForJobData, &ParallelForJobData::runHelper>(root, this);
            if (UTILS_LIKELY(helper)) {
                js.run(helper, id);
            } else {
                // Failed to allocate a helper job; decrement active workers refcount.
                finishWorker();
            }
        }

        process();
        finishWorker();
    }

    // Invoked by child helper jobs on other worker threads.
    void runHelper(JobSystem&, JobSystem::Job*) noexcept {
        process();
        finishWorker();
    }

    // Decrements active worker count and destroys when the last worker finishes.
    void finishWorker() noexcept {
        if (activeWorkers.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            this->~ParallelForJobData();
        }
    }

    // Raw job entry point for inline storage in the root Job.
    static void jobFuncInline(void* storage, JobSystem& js, JobSystem::Job* root) noexcept {
        auto* const that = static_cast<ParallelForJobData*>(storage);
        that->runRoot(js, root);
    }

    // Packed struct members: total header = 20 bytes (5 x 4-byte fields)
    std::atomic<uint32_t> nextOffset = { 0 };
    uint32_t start = 0;
    uint32_t count = 0;
    uint32_t chunkSize = 1;
    std::atomic<uint32_t> activeWorkers = { 0 };
    Functor functor;
};

// Check whether this struct fits inline inside Job::storage (typically 48 bytes).
template<typename S, typename F>
inline constexpr bool IsParallelForInline =
        (sizeof(ParallelForJobData<S, F>) <= JobSystem::Job::JOB_STORAGE_SIZE_BYTES);

} // namespace details


/**
 * Execute a function in parallel over a range of indices [start, start + count).
 *
 * The range is divided into chunks and dynamically claimed by available worker threads
 * using lock-free range-based stealing.
 *
 * A single shared instance of the functor is invoked concurrently across all worker threads.
 * As such, the functor MUST be const-invocable (i.e. have a `const operator()`).
 * Mutable lambdas or functors with non-const `operator()` are rejected at compile time via static_assert.
 * If mutable state across workers is necessary, pass it via `std::ref()` with appropriate synchronization.
 *
 * The functor is invoked with `(uint32_t start, uint32_t count)` representing contiguous,
 * non-overlapping sub-ranges covering the entire iteration space.
 *
 * If count is 0, a valid no-op job is returned and functor is not invoked.
 * If count exceeds 2^31 - 1 (0x7FFFFFFF), an error is logged and nullptr is returned.
 *
 * ParallelForJobData is stored entirely inline within Job storage with zero heap allocations.
 * Functors must fit inline (<= 24 bytes for 8-byte aligned closures); pass larger closures
 * via std::cref() or std::ref().
 *
 * @param js The JobSystem instance.
 * @param parent Optional parent job.
 * @param start First index of the range.
 * @param count Total number of items to process (max 2^31 - 1; start + count must not exceed 2^32; if 0, functor is not called).
 * @param functor Callable invoked with `(uint32_t start, uint32_t count)`. Must be const-invocable.
 * @param splitter Chunking policy (default: CountSplitter<16>).
 * @return The root JobSystem::Job* representing this parallel operation, or nullptr if count or range is invalid.
 */
template<typename S, typename F>
JobSystem::Job* parallel_for(JobSystem& js, JobSystem::Job* parent,
        uint32_t const start, uint32_t const count, F functor, const S& splitter) noexcept {
    static_assert(details::IsParallelForInline<S, F>,
            "parallel_for functor too large for inline Job storage. "
            "Pass large closures by reference via std::cref() or std::ref().");

    if (UTILS_UNLIKELY(count == 0)) {
        return js.createJob(parent, [](JobSystem&, JobSystem::Job*) {});
    }

    if (UTILS_UNLIKELY(count > 0x7FFFFFFFu || uint64_t(start) + count > 0x100000000ull)) {
        LOG(ERROR) << "parallel_for: invalid range [start=" << start << ", count=" << count
                   << "), start + count exceeds 2^32 or count exceeds 2^31 - 1";
        return nullptr;
    }

    uint32_t const poolThreads = uint32_t(js.getThreadCount());
    uint32_t const totalThreads = (poolThreads == 0) ? 0 :
            std::max(poolThreads, uint32_t(js.getActiveThreadCount()));
    uint32_t const effectiveThreads = std::max(1u, totalThreads);

    uint32_t const chunkSize = details::SplitterTraits<S>::getChunkSize(splitter, count, effectiveThreads);
    uint32_t const numChunks = ((count - 1) / chunkSize) + 1;
    uint32_t const helperCount = (totalThreads > 0 && numChunks > 1) ?
            std::min<uint32_t>(totalThreads - 1, numChunks - 1) : 0;
    uint32_t const totalWorkers = helperCount + 1;

    using JobData = details::ParallelForJobData<S, F>;

    return js.emplaceJobRaw<JobData>(parent, &JobData::jobFuncInline,
            start, count, chunkSize, totalWorkers, std::move(functor));
}

template<typename F>
JobSystem::Job* parallel_for(JobSystem& js, JobSystem::Job* parent,
        uint32_t const start, uint32_t const count, F functor) noexcept {
    return parallel_for(js, parent, start, count, std::move(functor), CountSplitter<16>{});
}

/**
 * Execute a function in parallel over an array of elements [data, data + count).
 *
 * The functor is invoked with `(T* data, size_t count)` for contiguous sub-slices.
 * A single shared instance of the functor is invoked concurrently across all worker threads;
 * the functor must be const-invocable.
 * If count is 0, a valid no-op job is returned and functor is not invoked.
 * If count exceeds 2^31 - 1 (0x7FFFFFFF), an error is logged and nullptr is returned.
 *
 * @param js The JobSystem instance.
 * @param parent Optional parent job.
 * @param data Pointer to the start of the data array.
 * @param count Total number of elements (if 0, functor is not called).
 * @param functor Callable invoked with `(T* data, size_t count)`. Must be const-invocable.
 * @param splitter Chunking policy (default: CountSplitter<16>).
 * @return The root JobSystem::Job* representing this parallel operation, or nullptr if count exceeds 2^31 - 1.
 */
template<typename T, typename S, typename F>
JobSystem::Job* parallel_for(JobSystem& js, JobSystem::Job* parent,
        T* const data, uint32_t const count, F functor, const S& splitter) noexcept {
    static_assert(std::is_invocable_v<const std::remove_reference_t<F>&, T*, size_t> ||
                  std::is_invocable_v<const std::remove_reference_t<F>&, T*, uint32_t>,
            "parallel_for functor must be const-invocable because a single functor instance "
            "is shared and invoked concurrently across all worker threads.");
    auto user = [data, f = std::move(functor)](uint32_t const s, uint32_t const c) {
        std::as_const(f)(data + s, c);
    };
    return parallel_for(js, parent, 0, count, std::move(user), splitter);
}

template<typename T, typename F>
JobSystem::Job* parallel_for(JobSystem& js, JobSystem::Job* parent,
        T* const data, uint32_t const count, F functor) noexcept {
    return parallel_for(js, parent, data, count, std::move(functor), CountSplitter<16>{});
}

/**
 * Execute a function in parallel over a Slice<T>.
 *
 * The functor is invoked with `(T* data, size_t count)` for contiguous sub-slices.
 * A single shared instance of the functor is invoked concurrently across all worker threads;
 * the functor must be const-invocable.
 * If slice is empty (size 0), a valid no-op job is returned and functor is not invoked.
 * If slice.size() exceeds 2^31 - 1 (0x7FFFFFFF), an error is logged and nullptr is returned.
 *
 * @param js The JobSystem instance.
 * @param parent Optional parent job.
 * @param slice Slice of elements to process (if empty, functor is not called).
 * @param functor Callable invoked with `(T* data, size_t count)`. Must be const-invocable.
 * @param splitter Chunking policy (default: CountSplitter<16>).
 * @return The root JobSystem::Job* representing this parallel operation, or nullptr if count exceeds 2^31 - 1.
 */
template<typename T, typename S, typename F>
JobSystem::Job* parallel_for(JobSystem& js, JobSystem::Job* parent,
        Slice<T> slice, F functor, const S& splitter) noexcept {
    return parallel_for(js, parent, slice.data(), uint32_t(slice.size()), std::move(functor), splitter);
}

template<typename T, typename F>
JobSystem::Job* parallel_for(JobSystem& js, JobSystem::Job* parent,
        Slice<T> slice, F functor) noexcept {
    return parallel_for(js, parent, slice.data(), uint32_t(slice.size()), std::move(functor), CountSplitter<16>{});
}
} // namespace jobs
} // namespace utils

#endif // TNT_UTILS_JOBSYSTEM_H
