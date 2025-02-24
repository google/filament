//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DebugAnnotator11.h: D3D11 helpers for adding trace annotations.
//

#ifndef LIBANGLE_RENDERER_D3D_D3D11_DEBUGANNOTATOR11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_DEBUGANNOTATOR11_H_

#include "libANGLE/LoggingAnnotator.h"

namespace rx
{

// Note: To avoid any race conditions between threads, this class has no private data;
// DebugAnnotatorContext11 will be retrieved from Context11.
class DebugAnnotator11 : public angle::LoggingAnnotator
{
  public:
    DebugAnnotator11();
    ~DebugAnnotator11() override;
    void beginEvent(gl::Context *context,
                    angle::EntryPoint entryPoint,
                    const char *eventName,
                    const char *eventMessage) override;
    void endEvent(gl::Context *context,
                  const char *eventName,
                  angle::EntryPoint entryPoint) override;
    void setMarker(gl::Context *context, const char *markerName) override;
    bool getStatus(const gl::Context *context) override;
};

class DebugAnnotatorContext11
{
  public:
    DebugAnnotatorContext11();
    ~DebugAnnotatorContext11();
    void initialize(ID3D11DeviceContext *context);
    void release();
    void beginEvent(angle::EntryPoint entryPoint, const char *eventName, const char *eventMessage);
    void endEvent(const char *eventName, angle::EntryPoint entryPoint);
    void setMarker(const char *markerName);
    bool getStatus() const;

  private:
    bool loggingEnabledForThisThread() const;

    angle::ComPtr<ID3DUserDefinedAnnotation> mUserDefinedAnnotation;
    static constexpr size_t kMaxMessageLength = 256;
    wchar_t mWCharMessage[kMaxMessageLength];

    // Only log annotations from the thread used to initialize the debug annotator
    uint64_t mAnnotationThread;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_DEBUGANNOTATOR11_H_
