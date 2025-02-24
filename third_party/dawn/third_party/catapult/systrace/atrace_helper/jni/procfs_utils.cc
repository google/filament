// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "procfs_utils.h"

#include <stdio.h>
#include <string.h>

#include "file_utils.h"
#include "logging.h"

using file_utils::ForEachPidInProcPath;
using file_utils::ReadProcFile;
using file_utils::ReadProcFileTrimmed;

namespace procfs_utils {

namespace {

const char kJavaAppPrefix[] = "/system/bin/app_process";
const char kZygotePrefix[] = "zygote";

inline void ReadProcString(int pid, const char* path, char* buf, size_t size) {
  if (!file_utils::ReadProcFileTrimmed(pid, path, buf, size))
    buf[0] = '\0';
}

inline void ReadExePath(int pid, char* buf, size_t size) {
  char exe_path[64];
  sprintf(exe_path, "/proc/%d/exe", pid);
  ssize_t res = readlink(exe_path, buf, size - 1);
  if (res >= 0)
    buf[res] = '\0';
  else
    buf[0] = '\0';
}

inline bool IsApp(const char* name, const char* exe) {
  return strncmp(exe, kJavaAppPrefix, sizeof(kJavaAppPrefix) - 1) == 0 &&
         strncmp(name, kZygotePrefix, sizeof(kZygotePrefix) - 1) != 0;
}

}  // namespace

int ReadTgid(int pid) {
  static const char kTgid[] = "\nTgid:";
  char buf[512];
  ssize_t rsize = ReadProcFile(pid, "status", buf, sizeof(buf));
  if (rsize <= 0)
    return -1;
  const char* tgid_line = strstr(buf, kTgid);
  CHECK(tgid_line);
  return atoi(tgid_line + sizeof(kTgid) - 1);
}

std::unique_ptr<ProcessInfo> ReadProcessInfo(int pid) {
  ProcessInfo* process = new ProcessInfo();
  process->pid = pid;
  ReadProcString(pid, "cmdline", process->name, sizeof(process->name));
  if (process->name[0] != 0) {
    ReadExePath(pid, process->exe, sizeof(process->exe));
    process->is_app = IsApp(process->name, process->exe);
  } else {
    ReadProcString(pid, "comm", process->name, sizeof(process->name));
    CHECK(process->name[0]);
    process->in_kernel = true;
  }
  return std::unique_ptr<ProcessInfo>(process);
}

void ReadProcessThreads(ProcessInfo* process) {
  if (process->in_kernel)
    return;

  char tasks_path[64];
  sprintf(tasks_path, "/proc/%d/task", process->pid);
  ForEachPidInProcPath(tasks_path, [process](int tid) {
    if (process->threads.count(tid))
      return;
    ThreadInfo thread = { tid, "" };
    char task_comm[64];
    sprintf(task_comm, "task/%d/comm", tid);
    ReadProcString(process->pid, task_comm, thread.name, sizeof(thread.name));
    if (thread.name[0] == '\0' && process->is_app)
      strcpy(thread.name, "UI Thread");
    process->threads[tid] = thread;
  });
}

bool ReadOomStats(ProcessSnapshot* snapshot) {
  char buf[64];
  if (ReadProcFileTrimmed(snapshot->pid, "oom_score", buf, sizeof(buf)))
    snapshot->oom_score = atoi(buf);
  else
    return false;
  if (ReadProcFileTrimmed(snapshot->pid, "oom_score_adj", buf, sizeof(buf)))
    snapshot->oom_score_adj = atoi(buf);
  else
    return false;
  return true;
}

bool ReadPageFaultsAndCpuTimeStats(ProcessSnapshot* snapshot) {
  char buf[512];
  if (!ReadProcFileTrimmed(snapshot->pid, "stat", buf, sizeof(buf)))
    return false;
  int ret = sscanf(buf,
      "%*d (%*[^)]) %*c %*d %*d %*d %*d %*d %*u %lu %*lu %lu %*lu %lu %lu",
      &snapshot->minor_faults, &snapshot->major_faults,
      &snapshot->utime, &snapshot->stime);
  CHECK(ret == 4);
  return true;
}

bool ReadMemInfoStats(std::map<std::string, uint64_t>* mem_info) {
  char buf[1024];
  ssize_t rsize = file_utils::ReadFile("/proc/meminfo", buf, sizeof(buf));
  if (rsize <= 0)
    return false;

  file_utils::LineReader reader(buf, rsize);
  for (const char* line = reader.NextLine();
       line && line[0];
       line = reader.NextLine()) {

    const char* pos_colon = strstr(line, ":");
    if (pos_colon == nullptr)
      continue;  // Should not happen.
    std::string name(line, pos_colon - line);
    (*mem_info)[name] = strtoull(&pos_colon[1], nullptr, 10);
  }
  return true;
}

}  // namespace procfs_utils
