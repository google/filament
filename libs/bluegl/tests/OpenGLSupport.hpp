/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef __BLUE_GL_OPENGL_SUPPORT_HPP__
#define __BLUE_GL_OPENGL_SUPPORT_HPP__

namespace bluegl {
namespace gl {

typedef void* OpenGLContext;

/**
 * Creates a new OpenGL context.
 *
 * @return an OpenGLContext object that represents the newly created
 *         OpenGL context or nullptr if the context cannot be created
 */
OpenGLContext createOpenGLContext();

/**
 * Makes the specified context the current OpenGL context on the
 * current thread.
 *
 * @param context the OpenGLContext to become current
 */
void setCurrentOpenGLContext(OpenGLContext context);

/**
 * Destroys the specified OpenGL context.
 *
 * @param context the OpenGLContext to destroy
 */
void destroyOpenGLContext(OpenGLContext context);

}; // namespace gl
}; // namespace bluegl

#endif // __BLUE_GL_OPENGL_SUPPORT_HPP__
