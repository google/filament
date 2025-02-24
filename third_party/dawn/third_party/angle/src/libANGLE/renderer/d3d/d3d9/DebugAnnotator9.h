//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DebugAnnotator9.h: D3D9 helpers for adding trace annotations.
//

#ifndef LIBANGLE_RENDERER_D3D_D3D9_DEBUGANNOTATOR9_H_
#define LIBANGLE_RENDERER_D3D_D3D9_DEBUGANNOTATOR9_H_

#include "libANGLE/LoggingAnnotator.h"

namespace rx
{

class DebugAnnotator9 : public angle::LoggingAnnotator
{
  public:
    DebugAnnotator9() {}
    void beginEvent(gl::Context *context,
                    angle::EntryPoint entryPoint,
                    const char *eventName,
                    const char *eventMessage) override;
    void endEvent(gl::Context *context,
                  const char *eventName,
                  angle::EntryPoint entryPoint) override;
    void setMarker(gl::Context *context, const char *markerName) override;
    bool getStatus(const gl::Context *context) override;

  private:
    static constexpr size_t kMaxMessageLength = 256;
    wchar_t mWCharMessage[kMaxMessageLength];
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D9_DEBUGANNOTATOR9_H_
