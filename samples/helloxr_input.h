#ifndef HELLOXR_INPUT_H
#define HELLOXR_INPUT_H

#include "helloxr_features.h"

#include <stdint.h>

namespace helloxr {

class ControllerInput {
public:
    static constexpr uint32_t HAND_COUNT = 2;

    bool initialize(XrInstance instance, XrSession session);
    void update(XrTime displayTime, XrSpace appSpace);
    void terminate() noexcept;

    bool getGripPose(uint32_t hand, XrPosef* pose) const noexcept;
    bool getAimPose(uint32_t hand, XrPosef* pose) const noexcept;
    float getTriggerValue(uint32_t hand) const noexcept;

private:
    struct HandState {
        XrPath path = XR_NULL_PATH;
        XrSpace gripSpace = XR_NULL_HANDLE;
        XrSpace aimSpace = XR_NULL_HANDLE;
        XrPosef gripPose = {};
        XrPosef aimPose = {};
        float triggerValue = 0.0f;
        bool gripValid = false;
        bool aimValid = false;
    };

    XrInstance mInstance = XR_NULL_HANDLE;
    XrSession mSession = XR_NULL_HANDLE;
    XrActionSet mActionSet = XR_NULL_HANDLE;
    XrAction mGripAction = XR_NULL_HANDLE;
    XrAction mAimAction = XR_NULL_HANDLE;
    XrAction mTriggerAction = XR_NULL_HANDLE;
    HandState mHands[HAND_COUNT];
};

} // namespace helloxr

#endif // HELLOXR_INPUT_H