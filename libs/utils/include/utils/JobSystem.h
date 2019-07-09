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

#include <assert.h>

#include <atomic>
#include <functional>
#include <thread>
#include <vector>

#include <utils/Allocator.h>
#include <utils/architecture.h>
#include <utils/Condition.h>
#include <utils/Log.h>
#include <utils/memalign.h>
#include <utils/Mutex.h>
#include <utils/Slice.h>
#include <utils/ThreadLocal.h>
#include <utils/WorkStealingDequeue.h>

namespace utils {

class JobSystem {
    static constexpr size_t MAX_JOB_COUNT = 4096;
    static_assert(MAX_JOB_COUNT <= 0x7FFE, "MAX_JOB_COUNT must be <= 0x7FFE");
    using WorkQueue = WorkStealingDequeue<uint16_t, MAX_JOB_COUNT>;

public:
    class Job;

    using JobFunc = void(*)(void*, JobSystem&, Job*);

    class alignas(CACHELINE_SIZE) Job {
    public:
        Job() noexcept {} /* = default; */ /* clang bug */ // NOLINT(modernize-use-equals-default,cppcoreguidelines-pro-type-member-init)
        Job(const Job&) = delete;
        Job(Job&&) = delete;

    private:
        friend class JobSystem;

        // Size is chosen so that we can store at least std::function<>
        // the alignas() qualifier ensures we're multiple of a cache-line.
        static constexpr size_t JOB_STORAGE_SIZE_BYTES =
                sizeof(std::function<void()>) > 48 ? sizeof(std::function<void()>) : 48;
        static constexpr size_t JOB_STORAGE_SIZE_WORDS =
                (JOB_STORAGE_SIZE_BYTES + sizeof(void*) - 1) / sizeof(void*);

        // keep it first, so it's correctly aligned with all architectures
        // this is were we store the job's data, typically a std::function<>
                                                                // v7 | v8
        void* storage[JOB_STORAGE_SIZE_WORDS];                  // 48 | 48
        JobFunc function;                                       //  4 |  8
        uint16_t parent;                                        //  2 |  2
        std::atomic<uint16_t> runningJobCount = { 1 };          //  2 |  2
        mutable std::atomic<uint16_t> refCount = { 1 };         //  2 |  2
                                                                //  6 |  2 (padding)
                                                                // 64 | 64
    };

    explicit JobSystem(size_t threadCount = 0, size_t adoptableThreadsCount = 1) noexcept;

    ~JobSystem();

    // Make the current thread part of the thread pool.
    void adopt();

    // Remove this adopted thread from the parent. This is intended to be used for
    // shutting down a JobSystem. In particular, this doesn't allow the parent to
    // adopt more thread.
    void emancipate();


    // return the JobSystem this thread is associated with. nullptr if this thread is not
    // part of a Jobsystem.
    static JobSystem* getJobSystem() noexcept;

    // If a parent is not specified when creating a job, that job will automatically take the
    // master job as a parent.
    // The master job is reset when waited on.
    Job* setMasterJob(Job* job) noexcept { return mMasterJob = job; }


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
        struct stub {
            static void call(void* user, JobSystem& js, Job* job) noexcept {
                (*static_cast<T**>(user)->*method)(js, job);
            }
        };
        Job* job = create(parent, &stub::call);
        if (job) {
            job->storage[0] = data;
        }
        return job;
    }

    // creates a job from a KNOWN method pointer w/ object passed by value
    template<typename T, void(T::*method)(JobSystem&, Job*)>
    Job* createJob(Job* parent, T data) noexcept {
        static_assert(sizeof(data) <= sizeof(Job::storage), "user data too large");
        struct stub {
            static void call(void* user, JobSystem& js, Job* job) noexcept {
                T* that = static_cast<T*>(user);
                (that->*method)(js, job);
                that->~T();
            }
        };
        Job* job = create(parent, &stub::call);
        if (job) {
            new(job->storage) T(std::move(data));
        }
        return job;
    }

    // creates a job from a functor passed by value
    template<typename T>
    Job* createJob(Job* parent, T functor) noexcept {
        static_assert(sizeof(functor) <= sizeof(Job::storage), "functor too large");
        struct stub {
            static void call(void* user, JobSystem& js, Job* job) noexcept {
                T& that = *static_cast<T*>(user);
                that(js, job);
                that.~T();
            }
        };
        Job* job = create(parent, &stub::call);
        if (job) {
            new(job->storage) T(std::move(functor));
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
    Job* retain(Job* job) noexcept;

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
     * Add job to this thread's execution queue. It's reference will drop automatically.
     * Current thread must be owned by JobSystem's thread pool. See adopt().
     *
     * The job can't be used after this call.
     */
    enum runFlags { DONT_SIGNAL = 0x1 };
    void run(Job*& job, uint32_t flags = 0) noexcept;
    void run(Job*&& job, uint32_t flags = 0) noexcept { // allows run(createJob(...));
        Job* p = job;
        run(p);
    }

    void signal() noexcept;

    /*
     * Add job to this thread's execution queue and and keep a reference to it.
     * Current thread must be owned by JobSystem's thread pool. See adopt().
     *
     * This job MUST BE waited on with wait(), or released with release().
     */
    Job* runAndRetain(Job* job, uint32_t flags = 0) noexcept;

    /*
     * Wait on a job and destroys it.
     * Current thread must be owned by JobSystem's thread pool. See adopt().
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
    friend utils::io::ostream& operator << (utils::io::ostream& out, JobSystem const& js);


    // utility functions...

    // set the name of the current thread (on OSes that support it)
    static void setThreadName(const char* threadName) noexcept;

    enum class Priority {
        NORMAL,
        DISPLAY,
        URGENT_DISPLAY
    };

    static void setThreadPriority(Priority priority) noexcept;
    static void setThreadAffinityById(size_t id) noexcept;

    size_t getParallelSplitCount() const noexcept {
        return mParallelSplitCount;
    }

private:
    // this is just to avoid using std::default_random_engine, since we're in a public header.
    class default_random_engine {
        static constexpr uint32_t m = 0x7fffffffu;
        uint32_t mState; // must be 0 < seed < 0x7fffffff
    public:
        inline constexpr explicit default_random_engine(uint32_t seed = 1u) noexcept
                : mState(((seed % m) == 0u) ? 1u : seed % m) {
        }
        inline uint32_t operator()() noexcept {
            return mState = uint32_t((uint64_t(mState) * 48271u) % m);
        }
    };

    struct alignas(CACHELINE_SIZE) ThreadState {    // this causes 40-bytes padding
        // make sure storage is cache-line aligned
        WorkQueue workQueue;

        // these are not accessed by the worker threads
        alignas(CACHELINE_SIZE)     // this causes 56-bytes padding
        JobSystem* js;
        std::thread thread;
        default_random_engine rndGen;
        uint32_t id;
    };

    static_assert(sizeof(ThreadState) % CACHELINE_SIZE == 0,
            "ThreadState doesn't align to a cache line");

    static ThreadState& getState() noexcept;

    void incRef(Job const* job) noexcept;
    void decRef(Job const* job) noexcept;

    Job* allocateJob() noexcept;
    JobSystem::ThreadState* getStateToStealFrom(JobSystem::ThreadState& state) noexcept;
    bool hasJobCompleted(Job const* job) noexcept;

    void requestExit() noexcept;
    bool exitRequested() const noexcept;
    bool hasActiveJobs() const noexcept;

    void loop(ThreadState* state) noexcept;
    bool execute(JobSystem::ThreadState& state) noexcept;
    Job* steal(JobSystem::ThreadState& state) noexcept;
    void finish(Job* job) noexcept;

    void put(WorkQueue& workQueue, Job* job) noexcept {
        size_t index = job - mJobStorageBase;
        assert(index >= 0 && index < MAX_JOB_COUNT);
        workQueue.push(uint16_t(index + 1));
    }

    Job* pop(WorkQueue& workQueue) noexcept {
        size_t index = workQueue.pop();
        assert(index <= MAX_JOB_COUNT);
        return !index ? nullptr : &mJobStorageBase[index - 1];
    }

    Job* steal(WorkQueue& workQueue) noexcept {
        size_t index = workQueue.steal();
        assert(index <= MAX_JOB_COUNT);
        return !index ? nullptr : &mJobStorageBase[index - 1];
    }

    void wait(std::unique_lock<Mutex>& lock) noexcept;
    void wake() noexcept;

    // these have thread contention, keep them together
    utils::Mutex mWaiterLock;
    utils::Condition mWaiterCondition;
    uint32_t mWaiterCount = 0;

    std::atomic<uint32_t> mActiveJobs = { 0 };
    utils::Arena<utils::ThreadSafeObjectPoolAllocator<Job>, LockingPolicy::NoLock> mJobPool;

    template <typename T>
    using aligned_vector = std::vector<T, utils::STLAlignedAllocator<T>>;

    // these are essentially const, make sure they're on a different cache-lines than the
    // read-write atomics.
    // We can't use "alignas(CACHELINE_SIZE)" because the standard allocator can't make this
    // guarantee.
    char padding[CACHELINE_SIZE];

    alignas(16) // at least we align to half (or quarter) cache-line
    aligned_vector<ThreadState> mThreadStates;          // actual data is stored offline
    std::atomic<bool> mExitRequested = { false };       // this one is almost never written
    std::atomic<uint16_t> mAdoptedThreads = { 0 };      // this one is almost never written
    Job* const mJobStorageBase;                         // Base for conversion to indices
    uint16_t mThreadCount = 0;                          // total # of threads in the pool
    uint8_t mParallelSplitCount = 0;                    // # of split allowable in parallel_for
    Job* mMasterJob = nullptr;

    static UTILS_DECLARE_TLS(ThreadState *) sThreadState;
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
        std::function<void()> f;
        // Renaming the method below could cause an Arrested Development.
        void gob(JobSystem&, JobSystem::Job*) noexcept { f(); }
    } user{ std::bind(std::forward<CALLABLE>(func),
            std::forward<ARGS>(args)...) };
    return js.createJob<Data, &Data::gob>(parent, std::move(user));
}

template<typename CALLABLE, typename T, typename ... ARGS,
        typename = typename std::enable_if<
                std::is_member_function_pointer<typename std::remove_reference<CALLABLE>::type>::value
        >::type
>
JobSystem::Job* createJob(JobSystem& js, JobSystem::Job* parent,
        CALLABLE&& func, T&& o, ARGS&&... args) noexcept {
    struct Data {
        std::function<void()> f;
        // Renaming the method below could cause an Arrested Development.
        void gob(JobSystem&, JobSystem::Job*) noexcept { f(); }
    } user{ std::bind(std::forward<CALLABLE>(func), std::forward<T>(o),
            std::forward<ARGS>(args)...) };
    return js.createJob<Data, &Data::gob>(parent, std::move(user));
}


namespace details {

template<typename S, typename F>
struct ParallelForJobData {
    using SplitterType = S;
    using Functor = F;
    using JobData = ParallelForJobData;
    using size_type = uint32_t;

    ParallelForJobData(size_type start, size_type count, uint8_t splits,
            Functor functor,
            const SplitterType& splitter) noexcept
            : start(start), count(count),
              functor(std::move(functor)),
              splits(splits),
              splitter(splitter) {
    }

    void parallelWithJobs(JobSystem& js, JobSystem::Job* parent) noexcept {

        // We first split about the number of threads we have, and only then we split the rest
        // in a single thread (but execute the final cut in new jobs, see parallel() below),
        // this way we save a lot of copies of JobData and miss-predicted branches
        if (splits == js.getParallelSplitCount()) {
            parallel(js, parent);
            return;
        }

        // this branch is often miss-predicted (it both sides happen 50% of the calls)
        if (splitter.split(splits, count)) {
            const size_type lc = count / 2;
            JobData ld(start, lc, splits + uint8_t(1), functor, splitter);
            JobSystem::Job* l = js.createJob<JobData, &JobData::parallelWithJobs>(parent, std::move(ld));
            if (UTILS_UNLIKELY(l == nullptr)) {
                // couldn't create a job, just pretend we're done splitting
                goto execute;
            }

            // start the left side before attempting the right side, so we parallelize in case
            // of job creation failure -- rare, but still.
            js.run(l);

            const size_type rc = count - lc;
            JobData rd(start + lc, rc, splits + uint8_t(1), functor, splitter);
            JobSystem::Job* r = js.createJob<JobData, &JobData::parallelWithJobs>(parent, std::move(rd));
            if (UTILS_UNLIKELY(r == nullptr)) {
                // couldn't allocate right side job, execute it right now
                start += lc;
                count = rc;
                goto execute;
            }

            // All good, execute the right side, but don't signal it,
            // so it's more likely to be executed next on the same thread
            js.run(r, JobSystem::DONT_SIGNAL);
        } else {
execute:
            // we're done splitting, do the real work here!
            functor(start, count);
        }
    }

private:
    void parallel(JobSystem& js, JobSystem::Job* parent) noexcept {

        // figure out how many splits we need
        size_type c = count;
        uint8_t   s = splits;
        while (splitter.split(s, c)) {
            c /= 2u;
            ++s;
        }

        // then linearly create all jobs with number of elements required by the splitter
        JobSystem::Job* job = nullptr;
        auto& func = functor;
        size_type const first = start;
        size_type const end = first + count;
        size_type curr = first;

        while (curr + 2u * c < end) {
            // this creates jobs from the end of the buffer because the WorkStealingDequeue
            // is a LIFO, this could help streaming to the d-cache.
            const size_type pos = end - (curr - first) - c;
            job = js.createJob(parent, [func, pos, c](JobSystem&, JobSystem::Job*) {
                func(pos, c);
            });
            if (UTILS_UNLIKELY(!job)) {
                goto finish; // oops, no more job available
            }
            js.run(job, JobSystem::DONT_SIGNAL);
            curr += c;
        }
    finish:
        assert(end >= curr);
        assert(end - curr >= c);
        js.signal();
        functor(start, end - curr);
    }

    size_type start;            // 4
    size_type count;            // 4
    Functor functor;            // ?
    uint8_t splits;             // 1
    SplitterType splitter;      // 1
};

} // namespace details


// parallel jobs with start/count indices
template<typename S, typename F>
JobSystem::Job* parallel_for(JobSystem& js, JobSystem::Job* parent,
        uint32_t start, uint32_t count, F functor, const S& splitter) noexcept {
    using JobData = details::ParallelForJobData<S, F>;
    JobData jobData(start, count, 0, std::move(functor), splitter);
    return js.createJob<JobData, &JobData::parallelWithJobs>(parent, std::move(jobData));
}

// parallel jobs with pointer/count
template<typename T, typename S, typename F>
JobSystem::Job* parallel_for(JobSystem& js, JobSystem::Job* parent,
        T* data, uint32_t count, F functor, const S& splitter) noexcept {
    auto user = [data, f = std::move(functor)](uint32_t s, uint32_t c) {
        f(data + s, c);
    };
    using JobData = details::ParallelForJobData<S, decltype(user)>;
    JobData jobData(0, count, 0, std::move(user), splitter);
    return js.createJob<JobData, &JobData::parallelWithJobs>(parent, std::move(jobData));
}

// parallel jobs on a Slice<>
template<typename T, typename S, typename F>
JobSystem::Job* parallel_for(JobSystem& js, JobSystem::Job* parent,
        utils::Slice<T> slice, F functor, const S& splitter) noexcept {
    return parallel_for(js, parent, slice.data(), slice.size(), functor, splitter);
}


template <size_t COUNT, size_t MAX_SPLITS = 12>
class CountSplitter {
public:
    bool split(size_t splits, size_t count) const noexcept {
        return (splits < MAX_SPLITS && count >= COUNT * 2);
    }
};

} // namespace jobs
} // namespace utils

#endif // TNT_UTILS_JOBSYSTEM_H
