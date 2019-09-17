/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_DRIVER_OPENGLBLITTER_H
#define TNT_FILAMENT_DRIVER_OPENGLBLITTER_H

#include <utils/compiler.h>
#include "gl_headers.h"

namespace filament {

class OpenGLContext;

class OpenGLBlitter {
public:
    explicit OpenGLBlitter(OpenGLContext& context) noexcept : mContext(context) {}

    void init() noexcept;
    void terminate() noexcept;

    void blit(GLuint srcTextureExternal, GLuint dstTexture2d, GLuint w, GLuint h) noexcept;

    class State {
        bool mHasState = false;
        GLint activeTexture, sampler, texture, framebuffer, array, vertexAttrib, program;
        GLboolean stencilTest, scissorTest, cullFace;
        GLint viewport[4], writeMask[4];
        void save() noexcept;
        void restore() noexcept;
    public:
        void setup() noexcept {
            if (UTILS_UNLIKELY(!mHasState)) {
                save();
            }
        }
        ~State() noexcept {
            if (UTILS_UNLIKELY(mHasState)) {
                restore();
            }
        }
    };

private:
    UTILS_UNUSED OpenGLContext& mContext;
    GLuint mSampler{};
    GLuint mVertexShader{};
    GLuint mFragmentShader{};
    GLuint mProgram{};
    GLuint mFBO{};
};

} // namespace filament

#endif // TNT_FILAMENT_DRIVER_OPENGLBLITTER_H
