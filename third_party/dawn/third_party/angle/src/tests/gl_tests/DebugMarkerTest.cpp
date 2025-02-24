//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DebugMarkerTest:
//   Basic tests to ensure EXT_debug_marker entry points work.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class DebugMarkerTest : public ANGLETest<>
{
  protected:
    DebugMarkerTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// Simple test to ensure the various EXT_debug_marker entry points don't crash.
// The debug markers can be validated by capturing this test under a graphics debugger.
TEST_P(DebugMarkerTest, BasicValidation)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_debug_marker"));

    std::string eventMarkerCaption = "Test event marker caption";
    std::string groupMarkerCaption = "Test group marker caption";

    glPushGroupMarkerEXT(static_cast<GLsizei>(groupMarkerCaption.length()),
                         groupMarkerCaption.c_str());

    // Do some basic operations between calls to extension entry points
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glInsertEventMarkerEXT(static_cast<GLsizei>(eventMarkerCaption.length()),
                           eventMarkerCaption.c_str());
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glPushGroupMarkerEXT(0, nullptr);
    glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glPopGroupMarkerEXT();
    glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glPopGroupMarkerEXT();

    ASSERT_GL_NO_ERROR();
}

// Test EXT_debug_marker markers before, during and after rendering.  The debug markers can be
// validated by capturing this test under a graphics debugger.
TEST_P(DebugMarkerTest, Rendering)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_debug_marker"));

    // The test produces the following hierarchy:
    //
    // Event: Before Draw Marker
    // Group: Before Draw
    //   Event: In Group 1 Marker
    //   glDrawArrays
    //   Group: After Draw 1
    //      Event: In Group 2 Marker
    //      glDrawArrays
    //
    //      glCopyTexImage <-- this breaks the render pass
    //
    //      glDrawArrays
    //   End Group
    //   glDrawArrays
    //   Group: After Draw 2
    //      glDrawArrays
    //      Event: In Group 3 Marker
    //
    //      glCopyTexImage <-- this breaks the render pass
    //   End Group
    // End Group
    // Event: After Draw Marker
    const std::string beforeDrawGroup = "Before Draw";
    const std::string drawGroup1      = "Group 1";
    const std::string drawGroup2      = "Group 2";

    const std::string beforeDrawMarker = "Before Draw Marker";
    const std::string inGroup1Marker   = "In Group 1 Marker";
    const std::string inGroup2Marker   = "In Group 2 Marker";
    const std::string inGroup3Marker   = "In Group 3 Marker";
    const std::string afterDrawMarker  = "After Draw Marker";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    glUseProgram(program);

    glInsertEventMarkerEXT(static_cast<GLsizei>(beforeDrawMarker.length()),
                           beforeDrawMarker.c_str());
    glPushGroupMarkerEXT(static_cast<GLsizei>(beforeDrawGroup.length()), beforeDrawGroup.c_str());
    {
        glInsertEventMarkerEXT(static_cast<GLsizei>(inGroup1Marker.length()),
                               inGroup1Marker.c_str());

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glPushGroupMarkerEXT(static_cast<GLsizei>(drawGroup1.length()), drawGroup1.c_str());
        {
            glInsertEventMarkerEXT(static_cast<GLsizei>(inGroup2Marker.length()),
                                   inGroup2Marker.c_str());

            glDrawArrays(GL_TRIANGLES, 0, 6);

            GLTexture texture;
            glBindTexture(GL_TEXTURE_2D, texture);
            glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 1, 1, 0);

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        glPopGroupMarkerEXT();

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glPushGroupMarkerEXT(static_cast<GLsizei>(drawGroup2.length()), drawGroup2.c_str());
        {
            glDrawArrays(GL_TRIANGLES, 0, 6);

            glInsertEventMarkerEXT(static_cast<GLsizei>(inGroup3Marker.length()),
                                   inGroup3Marker.c_str());

            glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 2, 2, 0);
        }
        glPopGroupMarkerEXT();
    }
    glPopGroupMarkerEXT();
    glInsertEventMarkerEXT(static_cast<GLsizei>(afterDrawMarker.length()), afterDrawMarker.c_str());

    ASSERT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2(DebugMarkerTest);

}  // namespace
