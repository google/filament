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

#ifdef WIN32
// Size is chosen so that we can store at least std::function<> and a job size is a multiple of a
// cacheline.
#    define JOB_PADDING (6+8)
#else
#    define JOB_PADDING (6)
#endif

namespace utils {

class JobSystem {
    static constexpr size_t MAX_JOB_COUNT = 4096;
    static_assert(MAX_JOB_COUNT <= 0x7FFE, "MAX_JOB_COUNT must be <= 0x7FFE");
    using WorkQueue = WorkStealingDequeue<uint16_t, MAX_JOB_COUNT>;

public:
    class Job;

    using JobFunc = void(*)(void*, JobSystem&, Job*);

    class alignas(CACHELINE_SIZE) Job { // NOLINT(cppcoreguidelines-pro-type-member-init)
    public:
        Job() noexcept {} // = default;
        Job(const Job&) = delete;
        Job(Job&&) = delete;

        void* getData() { return padding; }
        void const* getData() const { return padding; }
    private:
        friend class JobSystem;
        JobFunc function;
        uint16_t parent;
        std::atomic<uint16_t> runningJobCount = { 0 };
        // on 64-bits systems, there is an extra 32-bits lost here
        void* padding[JOB_PADDING];
    };

    static_assert(
            (sizeof(Job) % CACHELINE_SIZE == 0) ||
            (CACHELINE_SIZE % sizeof(Job) == 0),
            "A Job must be N cache-lines long or N Jobs must fit in a cache line exactly.");

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
    // The master job is reset when calling reset()
    Job* setMasterJob(Job* job) noexcept { return mMasterJob = job; }

    // Clears the master job
    void reset() noexcept { mMasterJob = nullptr; }


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
            job->padding[0] = data;
        }
        return job;
    }

    // creates a job from a KNOWN method pointer w/ object passed by value
    template<typename T, void(T::*method)(JobSystem&, Job*)>
    Job* createJob(Job* parent, T data) noexcept {
        static_assert(sizeof(data) <= sizeof(Job::padding), "user data too large");
        struct stub {
            static void call(void* user, JobSystem& js, Job* job) noexcept {
                T* that = static_cast<T*>(user);
                (that->*method)(js, job);
                that->~T();
            }
        };
        Job* job = create(parent, &stub::call);
        if (job) {
            new(job->padding) T(std::move(data));
        }
        return job;
    }

    // creates a job from a functor passed by value
    template<typename T>
    Job* createJob(Job* parent, T functor) noexcept {
        static_assert(sizeof(functor) <= sizeof(Job::padding), "functor too large");
        struct stub {
            static void call(void* user, JobSystem& js, Job* job) noexcept {
                T& that = *static_cast<T*>(user);
                that(js, job);
                that.~T();
            }
        };
        Job* job = create(parent, &stub::call);
        if (job) {
            new(job->padding) T(std::move(functor));
        }
        return job;
    }

    // Add job to this thread's execution queue.
    // Current thread must be owned by JobSystem's thread pool. See adopt().
    enum runFlags { DONT_SIGNAL = 0x1 };
    void run(Job* job, uint32_t flags = 0) noexcept;

    // Wait on a job.
    // Current thread must be owned by JobSystem's thread pool. See adopt().
    void wait(Job const* job) noexcept;

    void runAndWait(Job* job) noexcept {
        run(job);
        wait(job);
    }

    // jobs are normally finished automatically, this can be used to cancel a job
    // before it is run.
    void finish(Job* job) noexcept;

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
    static void setThreadAffinity(uint32_t mask) noexcept;

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
        uint32_t mask;
    };

    static_assert(sizeof(ThreadState) % CACHELINE_SIZE == 0,
            "ThreadState doesn't align to a cache line");

    static ThreadState& getState() noexcept;

    Job* create(Job* parent, JobFunc func) noexcept;
    Job* allocateJob() noexcept;
    JobSystem::ThreadState& getStateToStealFrom(JobSystem::ThreadState& state) noexcept;
    bool hasJobCompleted(Job const* job) noexcept;

    void requestExit() noexcept;
    bool exitRequested() const noexcept;

    void loop(ThreadState* threadState) noexcept;
    bool execute(JobSystem::ThreadState& state) noexcept;

    void put(WorkQueue& workQueue, Job* job) noexcept {
        size_t index = job - mJobStorageBase;
        assert(index >= 0 && index < MAX_JOB_COUNT);
        workQueue.push(uint16_t(index + 1));
    }

    Job* pop(WorkQueue& workQueue) noexcept {
        size_t index = workQueue.pop();
        assert(index <= MAX_JOB_COUNT);
        return !index ? nullptr : (mJobStorageBase - 1) + index;
    }

    Job* steal(WorkQueue& workQueue) noexcept {
        size_t index = workQueue.steal();
        assert(index <= MAX_JOB_COUNT);
        return !index ? nullptr : (mJobStorageBase - 1) + index;
    }

    // these have thread contention, keep them together
    utils::Mutex mLock;
    utils::Condition mCondition;
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
    std::atomic<bool> mExitRequested = { 0 };           // this one is almost never written
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
        // this way we save a lot of copies of JobData.
        if (splits == js.getParallelSplitCount()) {
            parallel(js, parent);
            return;
        }

        if (splitter.split(splits, count)) {
            const size_type lc = count / 2;
            JobData ld(start, lc, splits + uint8_t(1), functor, splitter);
            JobSystem::Job* l = js.createJob<JobData, &JobData::parallelWithJobs>(parent, std::move(ld));

            if (UTILS_UNLIKELY(l == nullptr)) {
                // couldn't create a job, just pretend we're done splitting
                goto done;
            }

            // start the left side before attempting the right side, so we parallelize in case
            // of job creation failure -- rare, but still.
            js.run(l);

            const size_type rc = count - lc;
            JobData rd(start + lc, rc, splits + uint8_t(1), functor, splitter);
            JobSystem::Job* r = js.createJob<JobData, &JobData::parallelWithJobs>(parent, std::move(rd));

            if (UTILS_UNLIKELY(r == nullptr)) {
                // couldn't allocate right side job, execute it right now
                functor(start + lc, rc);
                return;
            }

            // All good, execute the right side, but don't signal it,
            // so it's more likely to be executed next on the same thread
            js.run(r, JobSystem::DONT_SIGNAL);
        } else {
            done:
            // we're done splitting, do the real work here!
            functor(start, count);
        }
    }

    void parallel(JobSystem& js, JobSystem::Job* parent) noexcept {
        // here we split the data ona single thread, and launch jobs once we're completely
        // done splitting
        if (splitter.split(splits, count)) {
            auto lc = count / 2;
            auto rc = count - lc;
            auto rd = start + lc;
            auto s  = ++splits;

            // left-side
            count = lc;
            parallel(js, parent);

            // note: in practice the compiler is able to optimize out the call to parallel() below
            // right-side
            start = rd;
            count = rc;
            splits = s;
            parallel(js, parent);
        } else {
            // only capture what we need
            auto job = js.createJob(parent,
                    [f = functor, s = start, c = count](JobSystem&, JobSystem::Job*) {
                // we're done splitting, do the real work here!
                f(s, c);
            });
            if (UTILS_LIKELY(job)) {
                js.run(job);
            } else {
                // oops, no more job available
                functor(start, count);
            }
        }
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


// parallel jobs with start/count indices + sequential 'reduce'
template<typename S, typename F, typename R>
JobSystem::Job* parallel_for(JobSystem& js, JobSystem::Job* parent,
        uint32_t start, uint32_t count, F functor, const S& splitter, R finish) noexcept {
    using JobData = details::ParallelForJobData<S, F>;
    JobData jobData(start, count, 0, std::move(functor), splitter);
    auto wrapper = js.createJob(parent, [jobData, finish](JobSystem& js, JobSystem::Job* p) {
        auto parallelJob = js.createJob<JobData, &JobData::parallelWithJobs>(p, std::move(jobData));
        js.runAndWait(parallelJob);
        finish(js, parallelJob);
    });
    return wrapper;
}

// parallel jobs with pointer/count + sequential 'reduce'
template<typename T, typename S, typename F, typename R>
JobSystem::Job* parallel_for(JobSystem& js, JobSystem::Job* parent,
        T* data, uint32_t count, F functor, const S& splitter, R finish) noexcept {
    auto user = [data, f = std::move(functor)](uint32_t s, uint32_t c) {
        f(data + s, c);
    };
    using JobData = details::ParallelForJobData<S, decltype(user)>;
    JobData jobData(0, count, 0, std::move(user), splitter);
    auto wrapper = js.createJob(parent, [jobData, finish](JobSystem& js, JobSystem::Job* p) {
        auto parallelJob = js.createJob<JobData, &JobData::parallelWithJobs>(p, std::move(jobData));
        js.runAndWait(parallelJob);
        finish(js, parallelJob);
    });
    return wrapper;
}

// parallel jobs on a Slice<>
template<typename T, typename S, typename F>
JobSystem::Job* parallel_for(JobSystem& js, JobSystem::Job* parent,
        utils::Slice<T> slice, F functor, const S& splitter) noexcept {
    return parallel_for(js, parent, slice.data(), slice.size(), functor, splitter);
}

// parallel jobs on a Slice<> + sequential 'reduce'
template<typename T, typename S, typename F, typename R>
JobSystem::Job* parallel_for(JobSystem& js, JobSystem::Job* parent,
        utils::Slice<T> slice, F functor, const S& splitter, R finish) noexcept {
    return parallel_for(js, parent, slice.data(), slice.size(), functor, splitter, finish);
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
