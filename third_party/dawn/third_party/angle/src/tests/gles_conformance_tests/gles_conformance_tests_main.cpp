//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gles_conformance_tests.h"

#include "gtest/gtest.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <map>
#include <string>
#include <vector>

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    int rt = RUN_ALL_TESTS();
    return rt;
}
