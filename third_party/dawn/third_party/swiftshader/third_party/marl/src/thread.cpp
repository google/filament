// Copyright 2019 The Marl Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "marl/thread.h"

#include "marl/debug.h"
#include "marl/defer.h"
#include "marl/trace.h"

#include <algorithm>  // std::sort

#include <cstdarg>
#include <cstdio>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <array>
#include <cstdlib>  // mbstowcs
#include <limits>   // std::numeric_limits
#include <vector>
#undef max
#elif defined(__APPLE__)
#include <mach/thread_act.h>
#include <pthread.h>
#include <unistd.h>
#include <thread>
#elif defined(__FreeBSD__)
#include <pthread.h>
#include <pthread_np.h>
#include <unistd.h>
#include <thread>
#else
#include <pthread.h>
#include <unistd.h>
#include <thread>
#endif

namespace {

struct CoreHasher {
  inline uint64_t operator()(marl::Thread::Core core) const {
    return core.pthread.index;
  }
};

}  // anonymous namespace

namespace marl {

#if defined(_WIN32)
static constexpr size_t MaxCoreCount =
    std::numeric_limits<decltype(Thread::Core::windows.index)>::max() + 1ULL;
static constexpr size_t MaxGroupCount =
    std::numeric_limits<decltype(Thread::Core::windows.group)>::max() + 1ULL;
static_assert(sizeof(KAFFINITY) * 8ULL <= MaxCoreCount,
              "Thread::Core::windows.index is too small");

namespace {
#define CHECK_WIN32(expr)                                    \
  do {                                                       \
    auto res = expr;                                         \
    (void)res;                                               \
    MARL_ASSERT(res == TRUE, #expr " failed with error: %d", \
                (int)GetLastError());                        \
  } while (false)

struct ProcessorGroup {
  unsigned int count;  // number of logical processors in this group.
  KAFFINITY affinity;  // affinity mask.
};

struct ProcessorGroups {
  std::array<ProcessorGroup, MaxGroupCount> groups;
  size_t count;
};

const ProcessorGroups& getProcessorGroups() {
  static ProcessorGroups groups = [] {
    ProcessorGroups out = {};
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX info[32] = {};
    DWORD size = sizeof(info);
    CHECK_WIN32(GetLogicalProcessorInformationEx(RelationGroup, info, &size));
    DWORD count = size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);
    for (DWORD i = 0; i < count; i++) {
      if (info[i].Relationship == RelationGroup) {
        auto groupCount = info[i].Group.ActiveGroupCount;
        for (WORD groupIdx = 0; groupIdx < groupCount; groupIdx++) {
          auto const& groupInfo = info[i].Group.GroupInfo[groupIdx];
          out.groups[out.count++] = ProcessorGroup{
              groupInfo.ActiveProcessorCount, groupInfo.ActiveProcessorMask};
          MARL_ASSERT(out.count <= MaxGroupCount, "Group index overflow");
        }
      }
    }
    return out;
  }();
  return groups;
}
}  // namespace
#endif  // defined(_WIN32)

////////////////////////////////////////////////////////////////////////////////
// Thread::Affinty
////////////////////////////////////////////////////////////////////////////////

Thread::Affinity::Affinity(Allocator* allocator) : cores(allocator) {}
Thread::Affinity::Affinity(Affinity&& other) : cores(std::move(other.cores)) {}
Thread::Affinity& Thread::Affinity::operator=(Affinity&& other) {
  cores = std::move(other.cores);
  return *this;
}
Thread::Affinity::Affinity(const Affinity& other, Allocator* allocator)
    : cores(other.cores, allocator) {}

Thread::Affinity::Affinity(std::initializer_list<Core> list,
                           Allocator* allocator)
    : cores(allocator) {
  cores.reserve(list.size());
  for (auto core : list) {
    cores.push_back(core);
  }
}

Thread::Affinity::Affinity(const containers::vector<Core, 32>& coreList,
                           Allocator* allocator)
    : cores(coreList, allocator) {}

Thread::Affinity Thread::Affinity::all(
    Allocator* allocator /* = Allocator::Default */) {
  Thread::Affinity affinity(allocator);

#if defined(_WIN32)
  const auto& groups = getProcessorGroups();
  for (size_t groupIdx = 0; groupIdx < groups.count; groupIdx++) {
    const auto& group = groups.groups[groupIdx];
    Core core;
    core.windows.group = static_cast<decltype(Core::windows.group)>(groupIdx);
    for (unsigned int coreIdx = 0; coreIdx < group.count; coreIdx++) {
      if ((group.affinity >> coreIdx) & 1) {
        core.windows.index = static_cast<decltype(core.windows.index)>(coreIdx);
        affinity.cores.emplace_back(std::move(core));
      }
    }
  }
#elif defined(__linux__) && !defined(__ANDROID__) && !defined(__BIONIC__)
  auto thread = pthread_self();
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  if (pthread_getaffinity_np(thread, sizeof(cpu_set_t), &cpuset) == 0) {
    int count = CPU_COUNT(&cpuset);
    for (int i = 0; i < count; i++) {
      Core core;
      core.pthread.index = static_cast<uint16_t>(i);
      affinity.cores.emplace_back(std::move(core));
    }
  }
#elif defined(__FreeBSD__)
  auto thread = pthread_self();
  cpuset_t cpuset;
  CPU_ZERO(&cpuset);
  if (pthread_getaffinity_np(thread, sizeof(cpuset_t), &cpuset) == 0) {
    int count = CPU_COUNT(&cpuset);
    for (int i = 0; i < count; i++) {
      Core core;
      core.pthread.index = static_cast<uint16_t>(i);
      affinity.cores.emplace_back(std::move(core));
    }
  }
#else
  static_assert(!supported,
                "marl::Thread::Affinity::supported is true, but "
                "Thread::Affinity::all() is not implemented for this platform");
#endif

  return affinity;
}

std::shared_ptr<Thread::Affinity::Policy> Thread::Affinity::Policy::anyOf(
    Affinity&& affinity,
    Allocator* allocator /* = Allocator::Default */) {
  struct Policy : public Thread::Affinity::Policy {
    Affinity affinity;
    Policy(Affinity&& affinity) : affinity(std::move(affinity)) {}

    Affinity get(uint32_t threadId, Allocator* allocator) const override {
#if defined(_WIN32)
      auto count = affinity.count();
      if (count == 0) {
        return Affinity(affinity, allocator);
      }
      auto group = affinity[threadId % affinity.count()].windows.group;
      Affinity out(allocator);
      out.cores.reserve(count);
      for (auto core : affinity.cores) {
        if (core.windows.group == group) {
          out.cores.push_back(core);
        }
      }
      return out;
#else
      return Affinity(affinity, allocator);
#endif
    }
  };

  return allocator->make_shared<Policy>(std::move(affinity));
}

std::shared_ptr<Thread::Affinity::Policy> Thread::Affinity::Policy::oneOf(
    Affinity&& affinity,
    Allocator* allocator /* = Allocator::Default */) {
  struct Policy : public Thread::Affinity::Policy {
    Affinity affinity;
    Policy(Affinity&& affinity) : affinity(std::move(affinity)) {}

    Affinity get(uint32_t threadId, Allocator* allocator) const override {
      auto count = affinity.count();
      if (count == 0) {
        return Affinity(affinity, allocator);
      }
      return Affinity({affinity[threadId % affinity.count()]}, allocator);
    }
  };

  return allocator->make_shared<Policy>(std::move(affinity));
}

size_t Thread::Affinity::count() const {
  return cores.size();
}

Thread::Core Thread::Affinity::operator[](size_t index) const {
  return cores[index];
}

Thread::Affinity& Thread::Affinity::add(const Thread::Affinity& other) {
  containers::unordered_set<Core, CoreHasher> set(cores.allocator);
  for (auto core : cores) {
    set.emplace(core);
  }
  for (auto core : other.cores) {
    if (set.count(core) == 0) {
      cores.push_back(core);
    }
  }
  std::sort(cores.begin(), cores.end());
  return *this;
}

Thread::Affinity& Thread::Affinity::remove(const Thread::Affinity& other) {
  containers::unordered_set<Core, CoreHasher> set(cores.allocator);
  for (auto core : other.cores) {
    set.emplace(core);
  }
  for (size_t i = 0; i < cores.size(); i++) {
    if (set.count(cores[i]) != 0) {
      cores[i] = cores.back();
      cores.resize(cores.size() - 1);
    }
  }
  std::sort(cores.begin(), cores.end());
  return *this;
}

#if defined(_WIN32)

class Thread::Impl {
 public:
  Impl(Func&& func, _PROC_THREAD_ATTRIBUTE_LIST* attributes)
      : func(std::move(func)),
        handle(CreateRemoteThreadEx(GetCurrentProcess(),
                                    nullptr,
                                    0,
                                    &Impl::run,
                                    this,
                                    0,
                                    attributes,
                                    nullptr)) {}
  ~Impl() { CloseHandle(handle); }

  Impl(const Impl&) = delete;
  Impl(Impl&&) = delete;
  Impl& operator=(const Impl&) = delete;
  Impl& operator=(Impl&&) = delete;

  void Join() const { WaitForSingleObject(handle, INFINITE); }

  static DWORD WINAPI run(void* self) {
    reinterpret_cast<Impl*>(self)->func();
    return 0;
  }

 private:
  const Func func;
  const HANDLE handle;
};

Thread::Thread(Affinity&& affinity, Func&& func) {
  SIZE_T size = 0;
  InitializeProcThreadAttributeList(nullptr, 1, 0, &size);
  MARL_ASSERT(size > 0,
              "InitializeProcThreadAttributeList() did not give a size");

  std::vector<uint8_t> buffer(size);
  LPPROC_THREAD_ATTRIBUTE_LIST attributes =
      reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(buffer.data());
  CHECK_WIN32(InitializeProcThreadAttributeList(attributes, 1, 0, &size));
  defer(DeleteProcThreadAttributeList(attributes));

  GROUP_AFFINITY groupAffinity = {};

  auto count = affinity.count();
  if (count > 0) {
    groupAffinity.Group = affinity[0].windows.group;
    for (size_t i = 0; i < count; i++) {
      auto core = affinity[i];
      MARL_ASSERT(groupAffinity.Group == core.windows.group,
                  "Cannot create thread that uses multiple affinity groups");
      groupAffinity.Mask |= (1ULL << core.windows.index);
    }
    CHECK_WIN32(UpdateProcThreadAttribute(
        attributes, 0, PROC_THREAD_ATTRIBUTE_GROUP_AFFINITY, &groupAffinity,
        sizeof(groupAffinity), nullptr, nullptr));
  }

  impl = new Impl(std::move(func), attributes);
}

Thread::~Thread() {
  delete impl;
}

void Thread::join() {
  MARL_ASSERT(impl != nullptr, "join() called on unjoinable thread");
  impl->Join();
}

void Thread::setName(const char* fmt, ...) {
  static auto setThreadDescription =
      reinterpret_cast<HRESULT(WINAPI*)(HANDLE, PCWSTR)>(GetProcAddress(
          GetModuleHandleA("kernelbase.dll"), "SetThreadDescription"));
  if (setThreadDescription == nullptr) {
    return;
  }

  char name[1024];
  va_list vararg;
  va_start(vararg, fmt);
  vsnprintf(name, sizeof(name), fmt, vararg);
  va_end(vararg);

  wchar_t wname[1024];
  mbstowcs(wname, name, 1024);
  setThreadDescription(GetCurrentThread(), wname);
  MARL_NAME_THREAD("%s", name);
}

unsigned int Thread::numLogicalCPUs() {
  unsigned int count = 0;
  const auto& groups = getProcessorGroups();
  for (size_t groupIdx = 0; groupIdx < groups.count; groupIdx++) {
    const auto& group = groups.groups[groupIdx];
    count += group.count;
  }
  return count;
}

#else

class Thread::Impl {
 public:
  Impl(Affinity&& affinity, Thread::Func&& f)
      : affinity(std::move(affinity)), func(std::move(f)), thread([this] {
          setAffinity();
          func();
        }) {}

  Affinity affinity;
  Func func;
  std::thread thread;

  void setAffinity() {
    auto count = affinity.count();
    if (count == 0) {
      return;
    }

#if defined(__linux__) && !defined(__ANDROID__) && !defined(__BIONIC__)
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    for (size_t i = 0; i < count; i++) {
      CPU_SET(affinity[i].pthread.index, &cpuset);
    }
    auto thread = pthread_self();
    pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
#elif defined(__FreeBSD__)
    cpuset_t cpuset;
    CPU_ZERO(&cpuset);
    for (size_t i = 0; i < count; i++) {
      CPU_SET(affinity[i].pthread.index, &cpuset);
    }
    auto thread = pthread_self();
    pthread_setaffinity_np(thread, sizeof(cpuset_t), &cpuset);
#else
    MARL_ASSERT(!marl::Thread::Affinity::supported,
                "Attempting to use thread affinity on a unsupported platform");
#endif
  }
};

Thread::Thread(Affinity&& affinity, Func&& func)
    : impl(new Thread::Impl(std::move(affinity), std::move(func))) {}

Thread::~Thread() {
  MARL_ASSERT(!impl, "Thread::join() was not called before destruction");
}

void Thread::join() {
  impl->thread.join();
  delete impl;
  impl = nullptr;
}

void Thread::setName(const char* fmt, ...) {
  char name[1024];
  va_list vararg;
  va_start(vararg, fmt);
  vsnprintf(name, sizeof(name), fmt, vararg);
  va_end(vararg);

#if defined(__APPLE__)
  pthread_setname_np(name);
#elif defined(__FreeBSD__)
  pthread_set_name_np(pthread_self(), name);
#elif !defined(__Fuchsia__) && !defined(__EMSCRIPTEN__)
  pthread_setname_np(pthread_self(), name);
#endif

  MARL_NAME_THREAD("%s", name);
}

unsigned int Thread::numLogicalCPUs() {
  return static_cast<unsigned int>(sysconf(_SC_NPROCESSORS_ONLN));
}

#endif  // OS

Thread::Thread(Thread&& rhs) : impl(rhs.impl) {
  rhs.impl = nullptr;
}

Thread& Thread::operator=(Thread&& rhs) {
  if (impl) {
    delete impl;
    impl = nullptr;
  }
  impl = rhs.impl;
  rhs.impl = nullptr;
  return *this;
}

}  // namespace marl
