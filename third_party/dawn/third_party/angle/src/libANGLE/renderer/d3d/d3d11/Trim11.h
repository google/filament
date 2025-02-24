//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Trim11.h: Trim support utility class.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_TRIM11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_TRIM11_H_

#include "common/angleutils.h"
#include "libANGLE/Error.h"
#include "libANGLE/angletypes.h"

#if defined(ANGLE_ENABLE_WINDOWS_UWP)
#    include <EventToken.h>
#endif

namespace rx
{
class Renderer11;

class Trim11 : angle::NonCopyable
{
  public:
    explicit Trim11(Renderer11 *renderer);
    ~Trim11();

  private:
    Renderer11 *mRenderer;
#if defined(ANGLE_ENABLE_WINDOWS_UWP)
    EventRegistrationToken mApplicationSuspendedEventToken;
#endif

    void trim();
    bool registerForRendererTrimRequest();
    void unregisterForRendererTrimRequest();
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_TRIM11_H_
