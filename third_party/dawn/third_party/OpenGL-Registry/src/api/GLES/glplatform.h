#ifndef __glplatform_h_
#define __glplatform_h_

/*
** Copyright 2017-2020 The Khronos Group Inc.
** SPDX-License-Identifier: Apache-2.0
*/

/* Platform-specific types and definitions for OpenGL ES 1.X  gl.h
 *
 * Adopters may modify khrplatform.h and this file to suit their platform.
 * Please contribute modifications back to Khronos as pull requests on the
 * public github repository:
 *      https://github.com/KhronosGroup/OpenGL-Registry
 */

#include <KHR/khrplatform.h>

#ifndef GL_API
#define GL_API      KHRONOS_APICALL
#endif

#ifndef GL_APIENTRY
#define GL_APIENTRY KHRONOS_APIENTRY
#endif

#endif /* __glplatform_h_ */
