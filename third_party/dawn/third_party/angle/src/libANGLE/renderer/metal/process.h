//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// process.h:
//    Process manages a child process. This is largely copied from chrome.
//

#ifndef LIBANGLE_RENDERER_METAL_PROCESS_H_
#define LIBANGLE_RENDERER_METAL_PROCESS_H_

#include <sys/types.h>
#include <string>
#include <vector>

namespace rx
{
namespace mtl
{

class Process
{
  public:
    Process(const std::vector<std::string> &argv);
    Process(const Process &)            = delete;
    Process &operator=(const Process &) = delete;
    ~Process();

    bool WaitForExit(int &exit_code);
    bool DidLaunch() const;

  private:
    const pid_t pid_;
};

}  // namespace mtl
}  // namespace rx

#endif /* LIBANGLE_RENDERER_METAL_PROCESS_H_ */
