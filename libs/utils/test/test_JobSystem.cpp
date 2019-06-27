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

#include <gtest/gtest.h>

#include <utils/JobSystem.h>
#include <utils/WorkStealingDequeue.h>

#include <math/vec3.h>
#include <math/mat3.h>

#include <array>
#include <thread>
#include <utils/Allocator.h>

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
