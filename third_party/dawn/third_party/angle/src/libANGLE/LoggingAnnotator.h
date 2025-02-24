//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// LoggingAnnotator.h: DebugAnnotator implementing logging
//

#ifndef LIBANGLE_LOGGINGANNOTATOR_H_
#define LIBANGLE_LOGGINGANNOTATOR_H_

#include "common/debug.h"

namespace gl
{
class Context;
}  // namespace gl

namespace angle
{

class LoggingAnnotator : public gl::DebugAnnotator
{
  public:
    LoggingAnnotator() {}
    ~LoggingAnnotator() override {}
    void beginEvent(gl::Context *context,
                    EntryPoint entryPoint,
                    const char *eventName,
                    const char *eventMessage) override;
    void endEvent(gl::Context *context, const char *eventName, EntryPoint entryPoint) override;
    void setMarker(gl::Context *context, const char *markerName) override;
    bool getStatus(const gl::Context *context) override;
    void logMessage(const gl::LogMessage &msg) const override;
};

}  // namespace angle

#endif  // LIBANGLE_LOGGINGANNOTATOR_H_
