//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SyncQueriesTest.cpp: Tests of the GL_CHROMIUM_sync_query extension

#include "test_utils/ANGLETest.h"

namespace angle
{

class SyncQueriesTest : public ANGLETest<>
{
  protected:
    SyncQueriesTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void testTearDown() override
    {
        if (mQuery != 0)
        {
            glDeleteQueriesEXT(1, &mQuery);
            mQuery = 0;
        }
    }

    GLuint mQuery = 0;
};

// Test basic usage of sync queries
TEST_P(SyncQueriesTest, Basic)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_CHROMIUM_sync_query"));

    glGenQueriesEXT(1, &mQuery);
    glBeginQueryEXT(GL_COMMANDS_COMPLETED_CHROMIUM, mQuery);
    EXPECT_GL_NO_ERROR();

    glClearColor(0.0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEndQueryEXT(GL_COMMANDS_COMPLETED_CHROMIUM);

    glFlush();
    GLuint result = 0;
    glGetQueryObjectuivEXT(mQuery, GL_QUERY_RESULT_EXT, &result);
    EXPECT_GL_TRUE(result);
    EXPECT_GL_NO_ERROR();
}

// Test that the sync query enums are not accepted unless the extension is available
TEST_P(SyncQueriesTest, Validation)
{
    // Need the GL_EXT_occlusion_query_boolean extension for the entry points
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_occlusion_query_boolean"));

    bool extensionAvailable = IsGLExtensionEnabled("GL_CHROMIUM_sync_query");

    glGenQueriesEXT(1, &mQuery);

    glBeginQueryEXT(GL_COMMANDS_COMPLETED_CHROMIUM, mQuery);
    if (extensionAvailable)
    {
        EXPECT_GL_NO_ERROR();
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }

    glDeleteQueriesEXT(1, &mQuery);

    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(SyncQueriesTest);

}  // namespace angle
