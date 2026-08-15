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

#include <utils/Allocator.h>
#include <utils/JobSystem.h>
#include <utils/WorkStealingDequeue.h>

#include <math/mat3.h>
#include <math/vec3.h>

#include <gtest/gtest.h>

#include <array>
#include <thread>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#else
#include <time.h>
#endif

using namespace utils;
using namespace jobs;

TEST(JobSystem, WorkStealingDequeueSingleThreaded) {
    struct MyJob {
    };
    WorkStealingDequeue<MyJob*, 4096> queue;

    std::vector<MyJob> jobs;
    jobs.resize(4096);

    // Make sure a simple push/pop works
    MyJob aJob;
    queue.push(&aJob);
    EXPECT_EQ(&aJob, queue.pop());


    // Make sure multiple push/pop work
    for (size_t i=0 ; i<4096 ; i++) {
        queue.push(&jobs[i]);
    }
    for (size_t i=0 ; i<4096 ; i++) {
        MyJob* j = queue.pop();
        EXPECT_EQ(&jobs[4095-i], j);
    }

    // Make sure multiple pop/steal work
    for (size_t i=0 ; i<4096 ; i++) {
        queue.push(&jobs[i]);
    }
    for (size_t i=0 ; i<4096 ; i++) {
        MyJob* j = queue.steal();
        EXPECT_EQ(&jobs[i], j);
    }
}

TEST(JobSystem, WorkStealingDequeue_PopSteal) {
    struct MyJob {
    };
    WorkStealingDequeue<MyJob*, 65536> queue;
    size_t size = queue.getSize();

    MyJob pJob;
    MyJob sJob;

    // fill the queue
    for (size_t i=0 ; i<size/2 ; i++) {
        queue.push(&sJob);
    }
    for (size_t i=0 ; i<size/2 ; i++) {
        queue.push(&pJob);
    }

    // check concurrency of pop() and several steal()

    size_t pop_size = size / 2;
    std::thread pop_thread([&]() {
        for (int i=0 ; i<pop_size ; i++) {
            EXPECT_EQ(&pJob, queue.pop());
        }
    });

    size_t steal_size = size / (2*4);
    std::thread steal_thread0([&]() {
        for (int i=0 ; i<steal_size ; i++) {
            EXPECT_EQ(&sJob, queue.steal());
        }
    });
    std::thread steal_thread1([&]() {
        for (int i=0 ; i<steal_size ; i++) {
            EXPECT_EQ(&sJob, queue.steal());
        }
    });
    std::thread steal_thread2([&]() {
        for (int i=0 ; i<steal_size ; i++) {
            EXPECT_EQ(&sJob, queue.steal());
        }
    });
    std::thread steal_thread3([&]() {
        for (int i=0 ; i<steal_size ; i++) {
            EXPECT_EQ(&sJob, queue.steal());
        }
    });

    steal_thread0.join();
    steal_thread1.join();
    steal_thread2.join();
    steal_thread3.join();
    pop_thread.join();

    EXPECT_TRUE(queue.getCount() == 0);
}

TEST(JobSystem, WorkStealingDequeue_PushPopSteal) {
    struct MyJob {
    };
    WorkStealingDequeue<MyJob*, 65536> queue;
    size_t size = queue.getSize();

    MyJob pJob;

    int pop = 0;
    int steal0 = 0;
    int steal1 = 0;
    int steal2 = 0;
    int steal3 = 0;

    size_t push_size = size;
    std::thread push_pop_thread([&]() {
        for (int i=0 ; i<push_size/4 ; i++) {
            queue.push(&pJob);
            queue.push(&pJob);
            queue.push(&pJob);
            queue.push(&pJob);

            if (queue.pop())
                pop++;
            if (queue.pop())
                pop++;
            if (queue.pop())
                pop++;
            if (queue.pop())
                pop++;
        }
    });

    size_t steal_size = size;
    std::thread steal_thread0([&]() {
        for (int i=0 ; i<steal_size ; i++) {
            if (queue.steal())
                steal0++;
        }
    });
    std::thread steal_thread1([&]() {
        for (int i=0 ; i<steal_size ; i++) {
            if (queue.steal())
                steal1++;
        }
    });
    std::thread steal_thread2([&]() {
        for (int i=0 ; i<steal_size ; i++) {
            if (queue.steal())
                steal2++;
        }
    });
    std::thread steal_thread3([&]() {
        for (int i=0 ; i<steal_size ; i++) {
            if (queue.steal())
                steal3++;
        }
    });

    steal_thread0.join();
    steal_thread1.join();
    steal_thread2.join();
    steal_thread3.join();
    push_pop_thread.join();

    EXPECT_EQ(pop+steal0+steal1+steal2+steal3, size);
    EXPECT_TRUE(queue.getCount() == 0);
}



static std::atomic_int v = {0};
TEST(JobSystem, JobSystemParallelChildren) {
    v = 0;

    JobSystem js;
    js.adopt();

    struct User {
        std::atomic_int calls = {0};
        void func(JobSystem&, JobSystem::Job*) {
            v++;
            calls++;
        };
    } j;

    JobSystem::Job* root = js.createJob<User, &User::func>(nullptr, &j);
    for (int i=0 ; i<256 ; i++) {
        JobSystem::Job* job = js.createJob<User, &User::func>(root, &j);
        js.run(job);
    }
    js.runAndWait(root);

    EXPECT_EQ(257, v.load());
    EXPECT_EQ(257, j.calls);

    js.emancipate();
}


TEST(JobSystem, JobSystemSequentialChildren) {
    JobSystem js;
    js.adopt();

    struct User {
        int c;
        int i, j;
        void func(JobSystem& js, JobSystem::Job* job) {
            if (c < 43) {
                User u{ c + 1 };
                JobSystem::Job* p = js.createJob<User, &User::func>(job, &u);
                js.runAndWait(p);

                i = u.i + u.j;
                j = u.i;
            } else {
                i = 0;
                j = 1;
            }
        }
    };

    User u{0};

    JobSystem::Job* root = js.createJob<User, &User::func>(nullptr, &u);
    js.runAndWait(root);

    // 43rd fibonacci number
    EXPECT_EQ(433494437, u.i);

    js.emancipate();
}


TEST(JobSystem, JobSystemParallelFor) {
    JobSystem js;
    js.adopt();

    std::array<filament::math::float3, 4096*16> vertices;
    for (size_t j = 0; j<vertices.size(); ++j) {
        vertices[j] = filament::math::float3(j);
    }

    struct Executor {
        void operator()(filament::math::float3* v, size_t c) {
            for (size_t i=0 ; i<c; ++i) {
                v[i] = matrix * v[i];
            }
        }
        filament::math::mat3f matrix;
    } state;
    state.matrix = filament::math::mat3f(2);

    JobSystem::Job* job = parallel_for(js, nullptr, vertices.data(), vertices.size(),
            std::ref(state), CountSplitter<4>());
    js.runAndWait(job);

    const filament::math::mat3f matrix(2);
    for (size_t j = 0; j<vertices.size(); ++j) {
        EXPECT_TRUE(vertices[j] == matrix*filament::math::float3(j));
    }

    js.emancipate();
}

TEST(JobSystem, JobSystemDelegates) {
    JobSystem js;
    js.adopt();

    int result = 0;

    // capturing lambda
    JobSystem::Job* job = jobs::createJob(js, nullptr,
            [ &result ](int answerToEverything) {
        result = 1;
        EXPECT_EQ(42, answerToEverything);
    }, 42);
    js.runAndWait(job);
    EXPECT_EQ(1, result);

    static int promise = 0;

    // std::ref to a functor
    struct Functor {
        int result = 0;
        void operator()(utils::JobSystem&, utils::JobSystem::Job*, int answerToEverything) {
            result = 1;
            EXPECT_EQ(42, answerToEverything);
        }
        void execute(int answerToEverything) {
            result = 2;
            EXPECT_EQ(42, answerToEverything);
        }
        void operator()(utils::JobSystem&, utils::JobSystem::Job*) {
            result = 3;
            promise = 42;
        }
    } functor;
    job = jobs::createJob(js, nullptr, std::ref(functor), std::ref(js), nullptr, 42);
    js.runAndWait(job);
    EXPECT_EQ(1, functor.result);

    // member function pointer
    job = jobs::createJob(js, nullptr, &Functor::execute, &functor, 42);
    js.runAndWait(job);
    EXPECT_EQ(2, functor.result);

    job = js.createJob(nullptr, functor);
    js.runAndWait(job);
    EXPECT_EQ(42, promise);

    job = js.createJob(nullptr, std::ref(functor));
    js.runAndWait(job);
    EXPECT_EQ(3, functor.result);

    size_t a=0,b=0,c=0,d=0,e=0;
    job = js.createJob(nullptr, [&functor, a,b,c](JobSystem&, JobSystem::Job*){
        functor.result = 4;
    });
    js.runAndWait(job);
    EXPECT_EQ(4, functor.result);


    js.emancipate();
}

TEST(JobSystem, JobSystemConcurrentStress) {
    // 8 worker threads + 16 adoptable user threads
    JobSystem js(8, 16);

    constexpr size_t THREAD_COUNT = 16;
    constexpr size_t ITERATIONS = 20;
    constexpr size_t ARRAY_SIZE = 1024;

    std::atomic<bool> start{false};
    std::vector<std::thread> threads;
    threads.reserve(THREAD_COUNT);

    std::atomic<uint64_t> totalSum{0};

    for (size_t t = 0; t < THREAD_COUNT; ++t) {
        threads.emplace_back([&, t]() {
            while (!start.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }

            for (size_t iter = 0; iter < ITERATIONS; ++iter) {
                js.adopt();

                std::vector<uint32_t> data(ARRAY_SIZE, uint32_t(t + 1));
                auto* job = parallel_for(js, nullptr, data.data(), data.size(),
                        [](uint32_t* slice, size_t count) {
                            for (size_t i = 0; i < count; ++i) {
                                slice[i] *= 2;
                            }
                        }, CountSplitter<4>());
                js.runAndWait(job);

                uint64_t localSum = 0;
                for (uint32_t val : data) {
                    localSum += val;
                }
                totalSum.fetch_add(localSum, std::memory_order_relaxed);

                js.emancipate();
            }
        });
    }

    start.store(true, std::memory_order_release);
    for (auto& thread : threads) {
        thread.join();
    }

    uint64_t expectedTotal = 0;
    for (size_t t = 0; t < THREAD_COUNT; ++t) {
        expectedTotal += uint64_t(t + 1) * 2 * ARRAY_SIZE * ITERATIONS;
    }

    EXPECT_EQ(totalSum.load(), expectedTotal);
}

TEST(JobSystem, JobSystemBurstyWake) {
    JobSystem js(4, 1);
    js.adopt();

    std::atomic<uint32_t> completedJobs{0};

    // Part 1: Submit 1 job, wait for completion, sleep to ensure workers go to sleep, repeat
    for (size_t i = 0; i < 30; ++i) {
        JobSystem::Job* job = js.createJob(nullptr, [&completedJobs](JobSystem&, JobSystem::Job*) {
            completedJobs.fetch_add(1, std::memory_order_relaxed);
        });
        js.runAndWait(job);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    EXPECT_EQ(30u, completedJobs.load());

    // Part 2: Sudden burst of 100 jobs after idle period
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    JobSystem::Job* root = js.createJob(nullptr, [](JobSystem&, JobSystem::Job*) {});
    for (size_t i = 0; i < 100; ++i) {
        JobSystem::Job* child = js.createJob(root, [&completedJobs](JobSystem&, JobSystem::Job*) {
            completedJobs.fetch_add(1, std::memory_order_relaxed);
        });
        js.run(child);
    }
    js.runAndWait(root);

    EXPECT_EQ(130u, completedJobs.load());

    js.emancipate();
}

TEST(JobSystem, JobSystemDeepTreeMultiWait) {
    JobSystem js(8, 8);
    js.adopt();

    std::atomic<uint32_t> leafCount{0};

    auto buildTree = [&](auto& self, JobSystem::Job* parent, int depth) -> void {
        if (depth == 0) {
            JobSystem::Job* leaf = js.createJob(parent, [&leafCount](JobSystem&, JobSystem::Job*) {
                leafCount.fetch_add(1, std::memory_order_relaxed);
            });
            js.run(leaf);
            return;
        }
        for (int i = 0; i < 4; ++i) {
            JobSystem::Job* node = js.createJob(parent, [](JobSystem&, JobSystem::Job*) {});
            self(self, node, depth - 1);
            js.run(node);
        }
    };

    JobSystem::Job* root = js.createJob(nullptr, [](JobSystem&, JobSystem::Job*) {});
    buildTree(buildTree, root, 4); // 4^4 = 256 leaves

    // Retain root so we can wait on it from multiple threads
    js.retain(root);
    js.retain(root);
    js.retain(root);

    std::atomic<bool> start{false};
    std::vector<std::thread> waitingThreads;
    for (int t = 0; t < 3; ++t) {
        waitingThreads.emplace_back([&, root]() {
            js.adopt();
            while (!start.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }
            JobSystem::Job* r = root;
            js.waitAndRelease(r);
            js.emancipate();
        });
    }

    start.store(true, std::memory_order_release);
    js.runAndWait(root);

    for (auto& t : waitingThreads) {
        t.join();
    }

    EXPECT_EQ(256u, leafCount.load());

    js.emancipate();
}

TEST(JobSystem, JobSystemPoolExhaustionAndChurn) {
    JobSystem js(8, 8);

    constexpr size_t THREAD_COUNT = 8;
    constexpr size_t BATCH_SIZE = 1200; // 8 * 1200 = 9600 active jobs simultaneously
    std::atomic<uint32_t> jobsExecuted{0};

    std::vector<std::thread> threads;
    threads.reserve(THREAD_COUNT);

    for (size_t t = 0; t < THREAD_COUNT; ++t) {
        threads.emplace_back([&]() {
            js.adopt();
            for (size_t iter = 0; iter < 3; ++iter) {
                JobSystem::Job* root = js.createJob(nullptr, [](JobSystem&, JobSystem::Job*) {});
                for (size_t i = 0; i < BATCH_SIZE; ++i) {
                    JobSystem::Job* child = js.createJob(root, [&jobsExecuted](JobSystem&, JobSystem::Job*) {
                        jobsExecuted.fetch_add(1, std::memory_order_relaxed);
                    });
                    js.run(child);
                }
                js.runAndWait(root);
            }
            js.emancipate();
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(THREAD_COUNT * BATCH_SIZE * 3, jobsExecuted.load());
}

TEST(JobSystem, JobSystemSingleThreaded) {
    JobSystem js(JobSystem::SINGLE_THREADED);
    js.adopt();

    std::vector<uint32_t> data(512, 10);
    auto* job = parallel_for(js, nullptr, data.data(), data.size(),
            [](uint32_t* slice, size_t count) {
                for (size_t i = 0; i < count; ++i) {
                    slice[i] += 5;
                }
            }, CountSplitter<4>());
    js.runAndWait(job);

    for (uint32_t val : data) {
        EXPECT_EQ(15u, val);
    }

    js.emancipate();
}

TEST(JobSystem, JobSystemMultipleInstances) {
    JobSystem js1(4, 4);
    JobSystem js2(4, 4);

    std::atomic<uint32_t> count1{0};
    std::atomic<uint32_t> count2{0};

    std::thread t1([&]() {
        js1.adopt();
        for (int i = 0; i < 100; ++i) {
            JobSystem::Job* job = js1.createJob(nullptr, [&count1](JobSystem&, JobSystem::Job*) {
                count1.fetch_add(1, std::memory_order_relaxed);
            });
            js1.runAndWait(job);
        }
        js1.emancipate();
    });

    std::thread t2([&]() {
        js2.adopt();
        for (int i = 0; i < 100; ++i) {
            JobSystem::Job* job = js2.createJob(nullptr, [&count2](JobSystem&, JobSystem::Job*) {
                count2.fetch_add(1, std::memory_order_relaxed);
            });
            js2.runAndWait(job);
        }
        js2.emancipate();
    });

    t1.join();
    t2.join();

    EXPECT_EQ(100u, count1.load());
    EXPECT_EQ(100u, count2.load());
}

TEST(JobSystem, JobSystemRapidAdoptEmancipate) {
    JobSystem js(4, 16);

    constexpr size_t THREAD_COUNT = 16;
    constexpr size_t ITERATIONS = 100;

    std::atomic<uint32_t> jobsCompleted{0};
    std::vector<std::thread> threads;
    threads.reserve(THREAD_COUNT);

    for (size_t t = 0; t < THREAD_COUNT; ++t) {
        threads.emplace_back([&]() {
            for (size_t iter = 0; iter < ITERATIONS; ++iter) {
                js.adopt();
                JobSystem::Job* job = js.createJob(nullptr, [&jobsCompleted](JobSystem&, JobSystem::Job*) {
                    jobsCompleted.fetch_add(1, std::memory_order_relaxed);
                });
                js.runAndWait(job);
                js.emancipate();
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(THREAD_COUNT * ITERATIONS, jobsCompleted.load());
}

template<typename F>
static bool runWithTimeout(F&& fn, std::chrono::milliseconds timeout) {
    std::atomic<bool> completed{false};
    std::thread t([&]() {
        fn();
        completed.store(true, std::memory_order_release);
    });
    auto const start = std::chrono::steady_clock::now();
    while (!completed.load(std::memory_order_acquire)) {
        if (std::chrono::steady_clock::now() - start > timeout) {
            t.detach();
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    t.join();
    return true;
}

TEST(JobSystem, JobSystemWorkerWakeOnSubsequentPut) {
    bool const finished = runWithTimeout([]() {
        // 3 worker threads in the pool, 1 adoptable
        JobSystem js(3, 1);
        js.adopt();

        // Ensure all 3 workers are idle and sleeping
        std::this_thread::sleep_for(std::chrono::milliseconds(30));

        JobSystem::Job* root = js.createJob(nullptr, [](JobSystem&, JobSystem::Job*) {});

        std::atomic<int> concurrentWorkers{0};
        std::atomic<int> maxConcurrent{0};

        auto jobFunc = [&](JobSystem&, JobSystem::Job*) {
            int const cur = concurrentWorkers.fetch_add(1, std::memory_order_relaxed) + 1;
            int prevMax = maxConcurrent.load(std::memory_order_relaxed);
            while (cur > prevMax && !maxConcurrent.compare_exchange_weak(prevMax, cur,
                    std::memory_order_relaxed, std::memory_order_relaxed)) {}
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            concurrentWorkers.fetch_sub(1, std::memory_order_relaxed);
        };

        // Push 10 jobs in rapid succession
        for (int i = 0; i < 10; ++i) {
            JobSystem::Job* j = js.createJob(root, jobFunc);
            js.run(j);
        }

        // Wait for all 10 jobs to complete via root
        js.runAndWait(root);

        // With 3 pool workers and 1 caller thread in runAndWait, all 3 pool workers should be woken
        EXPECT_GE(maxConcurrent.load(), 3)
                << "Not all sleeping workers were woken! maxConcurrent: " << maxConcurrent.load();

        js.emancipate();
    }, std::chrono::seconds(2));

    EXPECT_TRUE(finished);
}

TEST(JobSystem, JobSystemStealFromEmancipatedQueue) {
    bool const finished = runWithTimeout([]() {
        JobSystem js(4, 2);
        js.adopt();

        JobSystem::Job* root = js.createJob(nullptr, [](JobSystem&, JobSystem::Job*) {});
        JobSystem::Job* child = js.createJob(root, [](JobSystem&, JobSystem::Job*) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });

        std::thread runner([&]() {
            js.adopt();
            js.run(child);
            js.emancipate();
        });
        runner.join();

        js.runAndWait(root);
        js.emancipate();
    }, std::chrono::seconds(2));

    EXPECT_TRUE(finished);
}

namespace {

static double getThreadCpuTimeMs() noexcept {
#if defined(_WIN32)
    FILETIME creationTime, exitTime, kernelTime, userTime;
    if (GetThreadTimes(GetCurrentThread(), &creationTime, &exitTime, &kernelTime, &userTime)) {
        ULARGE_INTEGER kernel, user;
        kernel.LowPart = kernelTime.dwLowDateTime;
        kernel.HighPart = kernelTime.dwHighDateTime;
        user.LowPart = userTime.dwLowDateTime;
        user.HighPart = userTime.dwHighDateTime;
        return double(kernel.QuadPart + user.QuadPart) / 10000.0;
    }
    return 0.0;
#elif defined(CLOCK_THREAD_CPUTIME_ID)
    struct timespec ts {};
    if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts) == 0) {
        return (double(ts.tv_sec) * 1000.0) + (double(ts.tv_nsec) / 1000000.0);
    }
    return 0.0;
#else
    return 0.0;
#endif
}

} // namespace

TEST(JobSystem, JobSystemWaitAndReleaseDoesNotBusySpin) {
    JobSystem js(4, 28);
    js.adopt();

    // 4 worker threads run 100ms tasks
    std::atomic<int> runningWorkers{0};
    JobSystem::Job* targetJob = nullptr;
    for (int i = 0; i < 4; ++i) {
        JobSystem::Job* w = js.createJob(nullptr, [&](JobSystem&, JobSystem::Job*) {
            runningWorkers.fetch_add(1, std::memory_order_relaxed);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
        if (i == 0) {
            targetJob = js.runAndRetain(w);
        } else {
            js.run(w);
        }
    }

    while (runningWorkers.load(std::memory_order_relaxed) < 4) {
        std::this_thread::yield();
    }

    // Push 1 job into a separate adopted thread's queue
    std::atomic<bool> bgDone{false};
    std::thread bgThread([&]() {
        js.adopt();
        JobSystem::Job* extra = js.createJob(nullptr, [](JobSystem&, JobSystem::Job*) {});
        js.run(extra);
        while (!bgDone.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        js.emancipate();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    double const startCpuTime = getThreadCpuTimeMs();

    js.waitAndRelease(targetJob);

    double const endCpuTime = getThreadCpuTimeMs();

    bgDone.store(true, std::memory_order_release);
    bgThread.join();

    double const cpuTimeMs = endCpuTime - startCpuTime;

    EXPECT_LT(cpuTimeMs, 30.0)
            << "waitAndRelease() busy-spun at 100% CPU! Consumed " << cpuTimeMs
            << "ms CPU time while waiting for 100ms child job.";

    js.emancipate();
}

TEST(JobSystem, JobSystemStealingWithManyAdoptableSlots) {
    bool const finished = runWithTimeout([]() {
        // 4 pool threads, 28 adoptable slots (32 total)
        JobSystem js(4, 28);
        js.adopt();

        std::atomic<uint32_t> completed{0};
        constexpr size_t JOB_COUNT = 2000;

        JobSystem::Job* root = js.createJob(nullptr, [](JobSystem&, JobSystem::Job*) {});
        for (size_t i = 0; i < JOB_COUNT; ++i) {
            JobSystem::Job* child = js.createJob(root, [&completed](JobSystem&, JobSystem::Job*) {
                completed.fetch_add(1, std::memory_order_relaxed);
            });
            js.run(child);
        }
        js.runAndWait(root);

        EXPECT_EQ(JOB_COUNT, completed.load());

        js.emancipate();
    }, std::chrono::seconds(2));

    EXPECT_TRUE(finished);
}




