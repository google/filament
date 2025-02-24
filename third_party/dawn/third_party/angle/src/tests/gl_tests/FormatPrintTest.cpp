//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FormatPrintTest:
//   Prints all format support info
//

#include "common/gl_enum_utils.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/formatutils.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_instantiate.h"
#include "util/EGLWindow.h"

using namespace angle;

namespace
{

class FormatPrintTest : public ANGLETest<>
{};

// This test enumerates all sized and unsized GL formats and prints out support information
// This test omits unsupported formats
// The output is csv parseable and has a header and a new line.
// Each row consists of:
// (InternalFormat,Type,texturable,filterable,textureAttachmentSupported,renderBufferSupported)
TEST_P(FormatPrintTest, PrintAllSupportedFormats)
{
    // Hack the angle!
    egl::Display *display   = static_cast<egl::Display *>(getEGLWindow()->getDisplay());
    gl::ContextID contextID = {
        static_cast<GLuint>(reinterpret_cast<uintptr_t>(getEGLWindow()->getContext()))};
    gl::Context *context                                 = display->getContext(contextID);
    const gl::InternalFormatInfoMap &allSupportedFormats = gl::GetInternalFormatMap();

    std::cout << std::endl
              << "InternalFormat,Type,Texturable,Filterable,Texture attachment,Renderbuffer"
              << std::endl
              << std::endl;

    for (const auto &internalFormat : allSupportedFormats)
    {
        for (const auto &typeFormatPair : internalFormat.second)
        {
            bool textureSupport = typeFormatPair.second.textureSupport(context->getClientVersion(),
                                                                       context->getExtensions());
            bool filterSupport  = typeFormatPair.second.filterSupport(context->getClientVersion(),
                                                                      context->getExtensions());
            bool textureAttachmentSupport = typeFormatPair.second.textureAttachmentSupport(
                context->getClientVersion(), context->getExtensions());
            bool renderbufferSupport = typeFormatPair.second.renderbufferSupport(
                context->getClientVersion(), context->getExtensions());

            // Skip if not supported
            // A format is not supported if the only feature bit enabled is "filterSupport"
            if (!(textureSupport || textureAttachmentSupport || renderbufferSupport))
            {
                continue;
            }

            // Lookup enum strings from enum
            std::stringstream resultStringStream;
            gl::OutputGLenumString(resultStringStream, gl::GLESEnum::InternalFormat,
                                   internalFormat.first);
            resultStringStream << ",";
            gl::OutputGLenumString(resultStringStream, gl::GLESEnum::PixelType,
                                   typeFormatPair.first);
            resultStringStream << ",";

            // able to be sampled from, see GLSL sampler variables
            if (textureSupport)
            {
                resultStringStream << "texturable";
            }
            resultStringStream << ",";

            // able to be linearly filtered (GL_LINEAR)
            if (filterSupport)
            {
                resultStringStream << "filterable";
            }
            resultStringStream << ",";

            // a texture with this can be used for glFramebufferTexture2D
            if (textureAttachmentSupport)
            {
                resultStringStream << "textureAttachmentSupported";
            }
            resultStringStream << ",";

            // usable with glFramebufferRenderbuffer, glRenderbufferStorage,
            // glNamedRenderbufferStorage
            if (renderbufferSupport)
            {
                resultStringStream << "renderbufferSupported";
            }

            std::cout << resultStringStream.str() << std::endl;
        }
    }
}

ANGLE_INSTANTIATE_TEST(FormatPrintTest, ES2_VULKAN(), ES3_VULKAN());
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(FormatPrintTest);

}  // anonymous namespace
