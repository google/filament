//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CaptureReplay: Template for replaying a frame capture with ANGLE.

#include "SampleApplication.h"

#include <functional>

#include "util/capture/frame_capture_test_utils.h"

class CaptureReplaySample : public SampleApplication
{
  public:
    CaptureReplaySample(int argc, char **argv, const angle::TraceInfo &traceInfo)
        : SampleApplication("CaptureReplaySample",
                            argc,
                            argv,
                            ClientType::ES3_0,
                            traceInfo.drawSurfaceWidth,
                            traceInfo.drawSurfaceHeight),
          mTraceInfo(traceInfo)
    {}

    bool initialize() override
    {
        mTraceLibrary.reset(new angle::TraceLibrary("capture_replay_sample_trace"));
        assert(mTraceLibrary->valid());

        std::stringstream binaryPathStream;
        binaryPathStream << angle::GetExecutableDirectory() << angle::GetPathSeparator()
                         << ANGLE_CAPTURE_REPLAY_SAMPLE_DATA_DIR;
        mTraceLibrary->setBinaryDataDir(binaryPathStream.str().c_str());
        mTraceLibrary->setupReplay();
        return true;
    }

    void destroy() override { mTraceLibrary->finishReplay(); }

    void draw() override
    {
        // Compute the current frame, looping from frameStart to frameEnd.
        uint32_t frame = mTraceInfo.frameStart +
                         (mCurrentFrame % ((mTraceInfo.frameEnd - mTraceInfo.frameStart) + 1));
        if (mPreviousFrame > frame)
        {
            mTraceLibrary->resetReplay();
        }
        mTraceLibrary->replayFrame(frame);
        mPreviousFrame = frame;
        mCurrentFrame++;
    }

  private:
    uint32_t mCurrentFrame  = 0;
    uint32_t mPreviousFrame = 0;
    const angle::TraceInfo mTraceInfo;
    std::unique_ptr<angle::TraceLibrary> mTraceLibrary;
};

int main(int argc, char **argv)
{
    std::string exeDir = angle::GetExecutableDirectory();

    std::stringstream traceJsonPathStream;
    traceJsonPathStream << exeDir << angle::GetPathSeparator()
                        << ANGLE_CAPTURE_REPLAY_SAMPLE_DATA_DIR << angle::GetPathSeparator()
                        << "angle_capture.json";

    std::string traceJsonPath = traceJsonPathStream.str();

    angle::TraceInfo traceInfo = {};
    if (!angle::LoadTraceInfoFromJSON("capture_replay_sample_trace", traceJsonPath, &traceInfo))
    {
        std::cout << "Unable to load trace data: " << traceJsonPath << "\n";
        return 1;
    }

    CaptureReplaySample app(argc, argv, traceInfo);
    return app.run();
}
