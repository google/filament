/*
 * Copyright (C) 2019 The Android Open Source Project
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

//! \file

#ifndef TNT_FILAMENT_BACKEND_PRESENT_CALLABLE
#define TNT_FILAMENT_BACKEND_PRESENT_CALLABLE

#include <utils/compiler.h>

namespace filament {
namespace backend {

/**
 * A PresentCallable is a callable object that, when called, schedules a frame for presentation on
 * a SwapChain.
 *
 * Typically, Filament's backend is responsible scheduling a frame's presentation. However, there
 * are certain cases where the application might want to control when a frame is scheduled for
 * presentation.
 *
 * For example, on iOS, UIKit elements can be synchronized to 3D content by scheduling a present
 * within a CATransation:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * void myFrameFinishedCallback(PresentCallable presentCallable, void* user) {
 *     [CATransaction begin];
 *     // Update other UI elements...
 *     presentCallable();
 *     [CATransaction commit];
 * }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * To obtain a PresentCallable, pass a backend::FrameFinishedCallback to the beginFrame() function.
 * The callback is called with a PresentCallable object and optional user data:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * if (renderer->beginFrame(swapChain, myFrameFinishedCallback, nullptr)) {
 *     renderer->render(view);
 *     renderer->endFrame();
 * }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * @remark Only Filament's Metal backend supports PresentCallables and frame callbacks. Other
 * backends ignore the callback (which will never be called) and proceed normally.
 *
 * @remark The backend::FrameFinishedCallback is called on an arbitrary thread.
 *
 * Applications *must* call each PresentCallable they receive. Each PresentCallable represents a
 * frame that is waiting to be presented. If an application fails to call a PresentCallable, a
 * memory leak could occur. To "cancel" the presentation of a frame, pass false to the
 * PresentCallable, which will cancel the presentation of the frame and release associated memory.
 *
 * @see Renderer, SwapChain, Renderer.beginFrame
 */
class UTILS_PUBLIC PresentCallable {
public:

    using PresentFn = void(*)(bool presentFrame, void* user);

    PresentCallable(PresentFn fn, void* user) noexcept;
    ~PresentCallable() noexcept = default;
    PresentCallable(const PresentCallable& rhs) = default;
    PresentCallable& operator=(const PresentCallable& rhs) = default;

    /**
     * Call this PresentCallable, scheduling the associated frame for presentation. Pass false for
     * presentFrame to effectively "cancel" the presentation of the frame.
     *
     * @param presentFrame if false, will not present the frame but releases associated memory
     */
    void operator()(bool presentFrame = true) noexcept;

private:

    PresentFn mPresentFn;
    void* mUser = nullptr;

};

/**
 * FrameFinishedCallback is a callback function that notifies an application when Filament has
 * finished processing a frame and that frame is ready to be scheduled for presentation.
 *
 * beginFrame() takes an optional FrameFinishedCallback. If the callback is provided, then that
 * frame will *not* automatically be scheduled for presentation. Instead, the application must call
 * the given PresentCallable.
 *
 * @remark The backend::FrameFinishedCallback is called on an arbitrary thread.
 *
 * @see PresentCallable, beginFrame()
 */
using FrameFinishedCallback = void(*)(PresentCallable callable, void* user);

} // namespace backend
} // namespace filament

#endif // TNT_FILAMENT_BACKEND_PRESENT_FRAME_CALLABLE
