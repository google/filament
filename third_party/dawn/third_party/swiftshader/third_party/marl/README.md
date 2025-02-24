# Marl

Marl is a hybrid thread / fiber task scheduler written in C++ 11.

## About

Marl is a C++ 11 library that provides a fluent interface for running tasks across a number of threads.

Marl uses a combination of fibers and threads to allow efficient execution of tasks that can block, while keeping a fixed number of hardware threads.

Marl supports Windows, macOS, Linux, FreeBSD, Fuchsia, Emscripten, Android and iOS (arm, aarch64, loongarch64, mips64, ppc64, rv64, x86 and x64).

Marl has no dependencies on other libraries (with an exception on googletest for building the optional unit tests).

Example:

```cpp
#include "marl/defer.h"
#include "marl/event.h"
#include "marl/scheduler.h"
#include "marl/waitgroup.h"

#include <cstdio>

int main() {
  // Create a marl scheduler using all the logical processors available to the process.
  // Bind this scheduler to the main thread so we can call marl::schedule()
  marl::Scheduler scheduler(marl::Scheduler::Config::allCores());
  scheduler.bind();
  defer(scheduler.unbind());  // Automatically unbind before returning.

  constexpr int numTasks = 10;

  // Create an event that is manually reset.
  marl::Event sayHello(marl::Event::Mode::Manual);

  // Create a WaitGroup with an initial count of numTasks.
  marl::WaitGroup saidHello(numTasks);

  // Schedule some tasks to run asynchronously.
  for (int i = 0; i < numTasks; i++) {
    // Each task will run on one of the 4 worker threads.
    marl::schedule([=] {  // All marl primitives are capture-by-value.
      // Decrement the WaitGroup counter when the task has finished.
      defer(saidHello.done());

      printf("Task %d waiting to say hello...\n", i);

      // Blocking in a task?
      // The scheduler will find something else for this thread to do.
      sayHello.wait();

      printf("Hello from task %d!\n", i);
    });
  }

  sayHello.signal();  // Unblock all the tasks.

  saidHello.wait();  // Wait for all tasks to complete.

  printf("All tasks said hello.\n");

  // All tasks are guaranteed to complete before the scheduler is destructed.
}
```

## Benchmarks

Graphs of several microbenchmarks can be found [here](https://google.github.io/marl/benchmarks).

## Building

Marl contains many unit tests and examples that can be built using CMake.

Unit tests require fetching the `googletest` external project, which can be done by typing the following in your terminal:

```bash
cd <path-to-marl>
git submodule update --init
```

### Linux and macOS

To build the unit tests and examples, type the following in your terminal:

```bash
cd <path-to-marl>
mkdir build
cd build
cmake .. -DMARL_BUILD_EXAMPLES=1 -DMARL_BUILD_TESTS=1
make
```

The resulting binaries will be found in `<path-to-marl>/build`

### Emscripten

1. install and activate the emscripten sdk following [standard instructions for your platform](https://emscripten.org/docs/getting_started/downloads.html).
2. build an example from the examples folder using emscripten, say `hello_task`. 
```bash
cd <path-to-marl>
mkdir build
cd build
emcmake cmake .. -DMARL_BUILD_EXAMPLES=1
make hello_task -j 8
```
NOTE: you want to change the value of the linker flag `sPTHREAD_POOL_SIZE` that must be at least as large as the number of threads used by your application.
3. Test the emscripten output.
You can use the provided python script to create a local web server:
```bash
../run_webserver
```
In your browser, navigate to the example URL: [http://127.0.0.1:8080/hello_task.html](http://127.0.0.1:8080/hello_task.html).  
Voilà - you should see the log output appear on the web page.

### Installing Marl (vcpkg)

Alternatively, you can build and install Marl using [vcpkg](https://github.com/Microsoft/vcpkg/) dependency manager:

```bash or powershell
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
./vcpkg install marl
```

The Marl port in vcpkg is kept up to date by Microsoft team members and community contributors. If the version is out of date, please [create an issue or pull request](https://github.com/Microsoft/vcpkg) on the vcpkg repository.

### Windows

Marl can be built using [Visual Studio 2019's CMake integration](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=vs-2019).

### Using Marl in your CMake project

You can build and link Marl using `add_subdirectory()` in your project's `CMakeLists.txt` file:

```cmake
set(MARL_DIR <path-to-marl>) # example <path-to-marl>: "${CMAKE_CURRENT_SOURCE_DIR}/third_party/marl"
add_subdirectory(${MARL_DIR})
```

This will define the `marl` library target, which you can pass to `target_link_libraries()`:

```cmake
target_link_libraries(<target> marl) # replace <target> with the name of your project's target
```

You may also wish to specify your own paths to the third party libraries used by `marl`.
You can do this by setting any of the following variables before the call to `add_subdirectory()`:

```cmake
set(MARL_THIRD_PARTY_DIR <third-party-root-directory>) # defaults to ${MARL_DIR}/third_party
set(MARL_GOOGLETEST_DIR  <path-to-googletest>)         # defaults to ${MARL_THIRD_PARTY_DIR}/googletest
add_subdirectory(${MARL_DIR})
```

### Usage Recommendations

#### Capture marl synchronization primitives by value

All marl synchronization primitives aside from `marl::ConditionVariable` should be lambda-captured by **value**:

```c++
marl::Event event;
marl::schedule([=]{ // [=] Good, [&] Bad.
  event.signal();
})
```

Internally, these primitives hold a shared pointer to the primitive state. By capturing by value we avoid common issues where the primitive may be destructed before the last reference is used.

#### Create one instance of `marl::Scheduler`, use it for the lifetime of the process

The `marl::Scheduler` constructor can be expensive as it may spawn a number of hardware threads. \
Destructing the `marl::Scheduler` requires waiting on all tasks to complete.

Multiple `marl::Scheduler`s may fight each other for hardware thread utilization.

For these reasons, it is recommended to create a single `marl::Scheduler` for the lifetime of your process.

For example:

```c++
int main() {
  marl::Scheduler scheduler(marl::Scheduler::Config::allCores());
  scheduler.bind();
  defer(scheduler.unbind());

  return do_program_stuff();
}
```

#### Bind the scheduler to externally created threads

In order to call `marl::schedule()` the scheduler must be bound to the calling thread. Failure to bind the scheduler to the thread before calling `marl::schedule()` will result in undefined behavior.

`marl::Scheduler` may be simultaneously bound to any number of threads, and the scheduler can be retrieved from a bound thread with `marl::Scheduler::get()`.

A typical way to pass the scheduler from one thread to another would be:

```c++
std::thread spawn_new_thread() {
  // Grab the scheduler from the currently running thread.
  marl::Scheduler* scheduler = marl::Scheduler::get();

  // Spawn the new thread.
  return std::thread([=] {
    // Bind the scheduler to the new thread.
    scheduler->bind();
    defer(scheduler->unbind());

    // You can now safely call `marl::schedule()`
    run_thread_logic();
  });
}

```

Always remember to unbind the scheduler before terminating the thread. Forgetting to unbind will result in the `marl::Scheduler` destructor blocking indefinitely.

#### Don't use externally blocking calls in marl tasks

The `marl::Scheduler` internally holds a number of worker threads which will execute the scheduled tasks. If a marl task becomes blocked on a marl synchronization primitive, marl can yield from the blocked task and continue execution of other scheduled tasks.

Calling a non-marl blocking function on a marl worker thread will prevent that worker thread from being able to switch to execute other tasks until the blocking function has returned. Examples of these non-marl blocking functions include: [`std::mutex::lock()`](https://en.cppreference.com/w/cpp/thread/mutex/lock), [`std::condition_variable::wait()`](https://en.cppreference.com/w/cpp/thread/condition_variable/wait), [`accept()`](http://man7.org/linux/man-pages/man2/accept.2.html).

Short blocking calls are acceptable, such as a mutex lock to access a data structure. However be careful that you do not use a marl blocking call with a `std::mutex` lock held - the marl task may yield with the lock held, and block other tasks from re-locking the mutex. This sort of situation may end up with a deadlock.

If you need to make a blocking call from a marl worker thread, you may wish to use [`marl::blocking_call()`](https://github.com/google/marl/blob/main/include/marl/blockingcall.h), which will spawn a new thread for performing the call, allowing the marl worker to continue processing other scheduled tasks.

---

Note: This is not an officially supported Google product
