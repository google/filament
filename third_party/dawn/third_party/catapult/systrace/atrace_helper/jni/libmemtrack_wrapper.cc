// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libmemtrack_wrapper.h"

#include <dlfcn.h>

#include "logging.h"

namespace {

// Init memtrack service. Removed in the latest version.
int (*memtrack_init)(void);

// Allocate and dispose memory stats.
libmemtrack_proc* (*memtrack_proc_new)(void);
void (*memtrack_proc_destroy)(libmemtrack_proc* p);

// Query memory stats for given process.
int (*memtrack_proc_get)(libmemtrack_proc* p, pid_t pid);

// Since memory stats is opaque structure, there are helpers to parse it.
ssize_t (*memtrack_proc_graphics_total)(libmemtrack_proc* p);
ssize_t (*memtrack_proc_graphics_pss)(libmemtrack_proc* p);
ssize_t (*memtrack_proc_gl_total)(libmemtrack_proc* p);
ssize_t (*memtrack_proc_gl_pss)(libmemtrack_proc* p);
ssize_t (*memtrack_proc_other_total)(libmemtrack_proc* p);
ssize_t (*memtrack_proc_other_pss)(libmemtrack_proc* p);

typedef ssize_t (*libmemtrack_getter_t)(libmemtrack_proc*);

bool g_initialized = false;
bool g_broken = false;

template <typename T>
void Import(T** func, void* lib, const char* name) {
  *(reinterpret_cast<void**>(func)) = dlsym(lib, name);
}

bool ImportLibmemtrackSymbols(void* handle) {
  Import(&memtrack_init, handle, "memtrack_init");
  Import(&memtrack_proc_new, handle, "memtrack_proc_new");
  Import(&memtrack_proc_destroy, handle, "memtrack_proc_destroy");
  Import(&memtrack_proc_get, handle, "memtrack_proc_get");
  Import(&memtrack_proc_graphics_total, handle, "memtrack_proc_graphics_total");
  Import(&memtrack_proc_graphics_pss, handle, "memtrack_proc_graphics_pss");
  Import(&memtrack_proc_gl_total, handle, "memtrack_proc_gl_total");
  Import(&memtrack_proc_gl_pss, handle, "memtrack_proc_gl_pss");
  Import(&memtrack_proc_other_total, handle, "memtrack_proc_other_total");
  Import(&memtrack_proc_other_pss, handle, "memtrack_proc_other_pss");

  if (!memtrack_proc_new || !memtrack_proc_destroy || !memtrack_proc_get) {
    LogError("Couldn't use libmemtrack. Probably it's API has been changed.");
    return false;
  }
  // Initialization is required on pre-O Android.
  if (memtrack_init && memtrack_init() != 0) {
    LogError("Failed to initialize libmemtrack. "
             "Probably implementation is missing in the ROM.");
    return false;
  }
  return true;
}

bool LazyOpenLibmemtrack() {
  if (g_initialized)
    return true;
  if (g_broken)
    return false;

  void *handle = dlopen("libmemtrack.so", RTLD_GLOBAL | RTLD_NOW);
  if (handle == nullptr) {
    LogError("Failed to open libmemtrack library.");
    g_broken = true;
    return false;
  }

  if (!ImportLibmemtrackSymbols(handle)) {
    dlclose(handle);
    g_broken = true;
    return false;
  }

  g_initialized = true;
  return true;
}

uint64_t GetOrZero(libmemtrack_getter_t getter, libmemtrack_proc* proc) {
  if (!getter || !proc)
    return 0;
  return static_cast<uint64_t>(getter(proc));
}

}  // namespace

MemtrackProc::MemtrackProc(int pid) {
  if (!LazyOpenLibmemtrack())
    return;

  proc_ = memtrack_proc_new();
  if (!proc_) {
    LogError("Failed to create libmemtrack proc. "
             "Probably it's API has been changed.");
    return;
  }

  if (memtrack_proc_get(proc_, pid) != 0) {
    // Don't log an error since not every process has memtrack stats.
    memtrack_proc_destroy(proc_);
    proc_ = nullptr;
  }
}

MemtrackProc::~MemtrackProc() {
  if (proc_)
    memtrack_proc_destroy(proc_);
}

uint64_t MemtrackProc::graphics_total() const {
  return GetOrZero(memtrack_proc_graphics_total, proc_);
}

uint64_t MemtrackProc::graphics_pss() const {
  return GetOrZero(memtrack_proc_graphics_pss, proc_);
}

uint64_t MemtrackProc::gl_total() const {
  return GetOrZero(memtrack_proc_gl_total, proc_);
}

uint64_t MemtrackProc::gl_pss() const {
  return GetOrZero(memtrack_proc_gl_pss, proc_);
}

uint64_t MemtrackProc::other_total() const {
  return GetOrZero(memtrack_proc_other_total, proc_);
}

uint64_t MemtrackProc::other_pss() const {
  return GetOrZero(memtrack_proc_other_pss, proc_);
}
