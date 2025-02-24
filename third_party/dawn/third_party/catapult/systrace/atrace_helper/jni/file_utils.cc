// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "file_utils.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace {

bool IsNumeric(const char* str) {
  if (!str[0])
    return false;
  for (const char* c = str; *c; c++) {
    if (!isdigit(*c))
      return false;
  }
  return true;
}

}  // namespace

namespace file_utils {

void ForEachPidInProcPath(const char* proc_path,
                          std::function<void(int)> predicate) {
  DIR* root_dir = opendir(proc_path);
  ScopedDir autoclose(root_dir);
  struct dirent* child_dir;
  while ((child_dir = readdir(root_dir))) {
    if (child_dir->d_type != DT_DIR || !IsNumeric(child_dir->d_name))
      continue;
    predicate(atoi(child_dir->d_name));
  }
}

ssize_t ReadFile(const char* path, char* buf, size_t length) {
  buf[0] = '\0';
  int fd = open(path, O_RDONLY);
  if (fd < 0 && errno == ENOENT)
    return -1;
  ScopedFD autoclose(fd);
  size_t tot_read = 0;
  do {
    ssize_t rsize = read(fd, buf + tot_read, length - tot_read);
    if (rsize == 0)
      break;
    if (rsize == -1 && errno == EINTR)
      continue;
    else if (rsize < 0)
      return -1;
    tot_read += static_cast<size_t>(rsize);
  } while (tot_read < length);
  buf[tot_read < length ? tot_read : length - 1] = '\0';
  return tot_read;
}

bool ReadFileTrimmed(const char* path, char* buf, size_t length) {
  ssize_t rsize = ReadFile(path, buf, length);
  if (rsize < 0)
    return false;
  for (ssize_t i = 0; i < rsize; i++) {
    const char c = buf[i];
    if (c == '\0' || c == '\r' || c == '\n') {
      buf[i] = '\0';
      break;
    }
    buf[i] = isprint(c) ? c : '?';
  }
  return true;
}

ssize_t ReadProcFile(int pid, const char* proc_file, char* buf, size_t length) {
  char proc_path[128];
  snprintf(proc_path, sizeof(proc_path), "/proc/%d/%s", pid, proc_file);
  return ReadFile(proc_path, buf, length);
}

// Reads a single-line proc file, stripping out any \0, \r, \n and replacing
// non-printable charcters with '?'.
bool ReadProcFileTrimmed(int pid,
                         const char* proc_file,
                         char* buf,
                         size_t length) {
  char proc_path[128];
  snprintf(proc_path, sizeof(proc_path), "/proc/%d/%s", pid, proc_file);
  return ReadFileTrimmed(proc_path, buf, length);
}

LineReader::LineReader(char* buf, size_t size)
    : ptr_(buf), end_(buf + size) {
}

LineReader::~LineReader() {
}

const char* LineReader::NextLine() {
  if (ptr_ >= end_)
    return nullptr;
  const char* cur = ptr_;
  char* next = strchr(ptr_, '\n');
  if (next) {
    *next = '\0';
    ptr_ = next + 1;
  } else {
    ptr_ = end_;
  }
  return cur;
}

}  // namespace file_utils
