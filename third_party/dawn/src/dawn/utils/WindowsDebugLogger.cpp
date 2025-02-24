// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <array>
#include <thread>

#include "dawn/utils/PlatformDebugLogger.h"

#include "dawn/common/Assert.h"
#include "dawn/common/windows_with_undefs.h"

namespace dawn::utils {

class WindowsDebugLogger : public PlatformDebugLogger {
  public:
    WindowsDebugLogger() : PlatformDebugLogger() {
        if (IsDebuggerPresent()) {
            // This condition is true when running inside Visual Studio or some other debugger.
            // Messages are already printed there so we don't need to do anything.
            return;
        }

        mShouldExitHandle = CreateEventA(nullptr, TRUE, FALSE, nullptr);
        DAWN_ASSERT(mShouldExitHandle != nullptr);

        mThread = std::thread(
            [](HANDLE shouldExit) {
                // https://blogs.msdn.microsoft.com/reiley/2011/07/29/a-debugging-approach-to-outputdebugstring/
                // for the layout of this struct.
                struct {
                    DWORD process_id;
                    char data[4096 - sizeof(DWORD)];
                }* dbWinBuffer = nullptr;

                HANDLE file = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                                                 sizeof(*dbWinBuffer), "DBWIN_BUFFER");
                DAWN_ASSERT(file != nullptr);
                DAWN_ASSERT(file != INVALID_HANDLE_VALUE);

                dbWinBuffer = static_cast<decltype(dbWinBuffer)>(
                    MapViewOfFile(file, SECTION_MAP_READ, 0, 0, 0));
                DAWN_ASSERT(dbWinBuffer != nullptr);

                HANDLE dbWinBufferReady = CreateEventA(nullptr, FALSE, FALSE, "DBWIN_BUFFER_READY");
                DAWN_ASSERT(dbWinBufferReady != nullptr);

                HANDLE dbWinDataReady = CreateEventA(nullptr, FALSE, FALSE, "DBWIN_DATA_READY");
                DAWN_ASSERT(dbWinDataReady != nullptr);

                std::array<HANDLE, 2> waitHandles = {shouldExit, dbWinDataReady};
                while (true) {
                    SetEvent(dbWinBufferReady);
                    DWORD wait = WaitForMultipleObjects(waitHandles.size(), waitHandles.data(),
                                                        FALSE, INFINITE);
                    if (wait == WAIT_OBJECT_0) {
                        break;
                    }
                    DAWN_ASSERT(wait == WAIT_OBJECT_0 + 1);
                    fprintf(stderr, "%.*s\n", static_cast<int>(sizeof(dbWinBuffer->data)),
                            dbWinBuffer->data);
                    fflush(stderr);
                }

                CloseHandle(dbWinDataReady);
                CloseHandle(dbWinBufferReady);
                UnmapViewOfFile(dbWinBuffer);
                CloseHandle(file);
            },
            mShouldExitHandle);
    }

    ~WindowsDebugLogger() override {
        if (IsDebuggerPresent()) {
            // This condition is true when running inside Visual Studio or some other debugger.
            // Messages are already printed there so we don't need to do anything.
            return;
        }

        if (mShouldExitHandle != nullptr) {
            BOOL result = SetEvent(mShouldExitHandle);
            DAWN_ASSERT(result != 0);
            CloseHandle(mShouldExitHandle);
        }

        if (mThread.joinable()) {
            mThread.join();
        }
    }

  private:
    std::thread mThread;
    HANDLE mShouldExitHandle = INVALID_HANDLE_VALUE;
};

PlatformDebugLogger* CreatePlatformDebugLogger() {
    return new WindowsDebugLogger();
}

}  // namespace dawn::utils
