// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <memory>
#include <set>
#include <string>
#include <sstream>

#include "atrace_process_dump.h"
#include "logging.h"

namespace {

std::unique_ptr<AtraceProcessDump> g_prog;

void ParseFullDumpConfig(const std::string& config, AtraceProcessDump* prog) {
  using FullDumpMode = AtraceProcessDump::FullDumpMode;
  if (config == "all") {
    prog->set_full_dump_mode(FullDumpMode::kAllProcesses);
  } else if (config == "apps") {
    prog->set_full_dump_mode(FullDumpMode::kAllJavaApps);
  } else {
    std::set<std::string> whitelist;
    std::istringstream ss(config);
    std::string entry;
    while (std::getline(ss, entry, ',')) {
      whitelist.insert(entry);
    }
    if (whitelist.empty())
      return;
    prog->set_full_dump_mode(FullDumpMode::kOnlyWhitelisted);
    prog->set_full_dump_whitelist(whitelist);
  }
}

}  // namespace

int main(int argc, char** argv) {
  if (argc == 2 && !strcmp(argv[1], "--echo-ts")) {
    // Used by clock sync marker to correct the difference between
    // Linux monotonic clocks on the device and host.
    printf("%" PRIu64 "\n", time_utils::GetTimestamp());
    return 0;
  }

  bool background = false;
  int dump_interval_ms = 5000;
  char out_file[PATH_MAX] = {};
  bool dump_to_file = false;
  int count = -1;

  AtraceProcessDump* prog = new AtraceProcessDump();
  g_prog = std::unique_ptr<AtraceProcessDump>(prog);

  if (geteuid()) {
    fprintf(stderr, "Must run as root\n");
    exit(EXIT_FAILURE);
  }

  int opt;
  while ((opt = getopt(argc, argv, "bm:gst:o:c:")) != -1) {
    switch (opt) {
      case 'b':
        background = true;
        break;
      case 'm':
        ParseFullDumpConfig(optarg, prog);
        break;
      case 'g':
        prog->enable_graphics_stats();
        break;
      case 's':
        prog->enable_print_smaps();
        break;
      case 't':
        dump_interval_ms = atoi(optarg);
        CHECK(dump_interval_ms > 0);
        break;
      case 'c':
        count = atoi(optarg);
        CHECK(count > 0);
        break;
      case 'o':
        strncpy(out_file, optarg, sizeof(out_file));
        dump_to_file = true;
        break;
      default:
        fprintf(stderr,
                "Usage: %s [-b] [-m full_dump_filter] [-g] [-s] "
                "[-t dump_interval_ms] "
                "[-c dumps_count] [-o out.json]\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  prog->set_dump_count(count);
  prog->SetDumpInterval(dump_interval_ms);

  FILE* out_stream = stdout;
  char tmp_file[PATH_MAX];
  if (dump_to_file) {
    unlink(out_file);
    sprintf(tmp_file, "%s.tmp", out_file);
    out_stream = fopen(tmp_file, "w");
    CHECK(out_stream);
  }

  if (background) {
    if (!dump_to_file) {
      fprintf(stderr, "-b requires -o for output dump path.\n");
      exit(EXIT_FAILURE);
    }
    printf("Continuing in background. kill -TERM to terminate the daemon.\n");
    CHECK(daemon(0 /* nochdir */, 0 /* noclose */) == 0);
  }

  auto on_exit = [](int) { g_prog->Stop(); };
  signal(SIGINT, on_exit);
  signal(SIGTERM, on_exit);

  prog->RunAndPrintJson(out_stream);
  fclose(out_stream);

  if (dump_to_file)
    rename(tmp_file, out_file);
}
