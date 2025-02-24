// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "process_memory_stats.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include <memory>

#include "file_utils.h"
#include "libmemtrack_wrapper.h"
#include "logging.h"

namespace {

const int kKbPerPage = 4;

const char kRss[] = "Rss";
const char kPss[] = "Pss";
const char kSwap[] = "Swap";
const char kSharedClean[] = "Shared_Clean";
const char kSharedDirty[] = "Shared_Dirty";
const char kPrivateClean[] = "Private_Clean";
const char kPrivateDirty[] = "Private_Dirty";

bool ReadSmapsMetric(
    const char* line, const char* metric, int metric_size, uint64_t* res) {
  if (strncmp(line, metric, metric_size - 1))
    return false;
  if (line[metric_size - 1] != ':')
    return false;
  *res = strtoull(line + metric_size, nullptr, 10);
  return true;
}

}  // namespace

bool ProcessMemoryStats::ReadLightStats(int pid) {
  char buf[64];
  if (file_utils::ReadProcFile(pid, "statm", buf, sizeof(buf)) <= 0)
    return false;
  uint32_t vm_size_pages;
  uint32_t rss_pages;
  int res = sscanf(buf, "%u %u", &vm_size_pages, &rss_pages);
  CHECK(res == 2);
  rss_kb_ = rss_pages * kKbPerPage;
  virt_kb_ = vm_size_pages * kKbPerPage;
  return true;
}

bool ProcessMemoryStats::ReadFullStats(int pid) {
  const size_t kBufSize = 8u * 1024 * 1024;
  std::unique_ptr<char[]> buf(new char[kBufSize]);
  ssize_t rsize = file_utils::ReadProcFile(pid, "smaps", &buf[0], kBufSize);
  if (rsize <= 0)
    return false;
  MmapInfo* last_mmap_entry = nullptr;
  std::unique_ptr<MmapInfo> new_mmap(new MmapInfo());
  CHECK(mmaps_.empty());
  CHECK(rss_kb_ == 0);

  // Iterate over all lines in /proc/PID/smaps.
  file_utils::LineReader rd(&buf[0], rsize);
  for (const char* line = rd.NextLine(); line; line = rd.NextLine()) {
    if (!line[0])
      continue;
    // Performance optimization (hack).
    // Any header line starts with lowercase hex digit but subsequent lines
    // start with uppercase letter.
    if (line[0] < 'A' || line[0] > 'Z') {
      // Note that the mapped file name ([stack]) is optional and won't be
      // present on anonymous memory maps (hence res >= 3 below).
      int res = sscanf(line,
          "%" PRIx64 "-%" PRIx64 " %4s %*" PRIx64 " %*[:0-9a-f] "
          "%*[0-9a-f]%*[ \t]%127[^\n]",
          &new_mmap->start_addr, &new_mmap->end_addr, new_mmap->prot_flags,
          new_mmap->mapped_file);
      last_mmap_entry = new_mmap.get();
      CHECK(new_mmap->end_addr >= new_mmap->start_addr);
      new_mmap->virt_kb =
          (new_mmap->end_addr - new_mmap->start_addr) / 1024;
      if (res == 3)
        new_mmap->mapped_file[0] = '\0';
      virt_kb_ += new_mmap->virt_kb;
      mmaps_.push_back(std::move(new_mmap));
      new_mmap.reset(new MmapInfo());
    } else {
      // The current line is a metrics line within a mmap entry, e.g.:
      // Size:   4 kB
      uint64_t size = 0;
      CHECK(last_mmap_entry);
      if (ReadSmapsMetric(line, kRss, sizeof(kRss), &size)) {
        last_mmap_entry->rss_kb = size;
        rss_kb_ += size;
      } else if (ReadSmapsMetric(line, kPss, sizeof(kPss), &size)) {
        last_mmap_entry->pss_kb = size;
        pss_kb_ += size;
      } else if (ReadSmapsMetric(line, kSwap, sizeof(kSwap), &size)) {
        last_mmap_entry->swapped_kb = size;
        swapped_kb_ += size;
      } else if (ReadSmapsMetric(
                     line, kSharedClean, sizeof(kSharedClean), &size)) {
        last_mmap_entry->shared_clean_kb = size;
        shared_clean_kb_ += size;
      } else if (ReadSmapsMetric(
                     line, kSharedDirty, sizeof(kSharedDirty), &size)) {
        last_mmap_entry->shared_dirty_kb = size;
        shared_dirty_kb_ += size;
      } else if (ReadSmapsMetric(
                     line, kPrivateClean, sizeof(kPrivateClean), &size)) {
        last_mmap_entry->private_clean_kb = size;
        private_clean_kb_ += size;
      } else if (ReadSmapsMetric(
                     line, kPrivateDirty, sizeof(kPrivateDirty), &size)) {
        last_mmap_entry->private_dirty_kb = size;
        private_dirty_kb_ += size;
      }
    }
  }
  full_stats_ = true;
  return true;
}

bool ProcessMemoryStats::ReadGpuStats(int pid) {
  MemtrackProc mt(pid);
  gpu_graphics_kb_ = mt.graphics_total() / 1024;
  gpu_graphics_pss_kb_ = mt.graphics_pss() / 1024;
  gpu_gl_kb_ = mt.gl_total() / 1024;
  gpu_gl_pss_kb_ = mt.gl_pss() / 1024;
  gpu_other_kb_ = mt.other_total() / 1024;
  gpu_other_pss_kb_ = mt.other_pss() / 1024;

  gpu_stats_ = !mt.has_errors() &&
      (gpu_graphics_kb_ != 0 || gpu_gl_kb_ != 0 || gpu_other_kb_ != 0);
  return gpu_stats_;
}
