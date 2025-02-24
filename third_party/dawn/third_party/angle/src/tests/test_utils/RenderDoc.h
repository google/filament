//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RenderDoc:
//   Connection to renderdoc for capturing tests through its API.
//

#ifndef TESTS_TEST_UTILS_RENDERDOC_H_
#define TESTS_TEST_UTILS_RENDERDOC_H_

#include "common/system_utils.h"

class RenderDoc
{
  public:
    RenderDoc();
    ~RenderDoc();

    void attach();
    void startFrame();
    void endFrame();

  private:
    angle::Library *mRenderDocModule;
    void *mApi;
};

#endif  // TESTS_TEST_UTILS_RENDERDOC_H_
