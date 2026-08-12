#include "helloxr_input.h"

#include <cstring>
#include <string>

namespace helloxr {

namespace {

constexpr char const* HAND_PATHS[ControllerInput::HAND_COUNT] = {
    "/user/hand/left",
    "/user/hand/right",
};

} // anonymous namespace

bool ControllerInput::initialize(XrInstance instance, XrSession session) {
    mInstance = instance;
    mSession = session;

    XrActionSetCreateInfo setInfo = { XR_TYPE_ACTION_SET_CREATE_INFO };
    strcpy(setInfo.actionSetName, "helloxr_input");
    strcpy(setInfo.localizedActionSetName, "helloxr input");
    if (XR_FAILED(xrCreateActionSet(instance, &setInfo, &mActionSet))) {
        XRLOG("controller input: xrCreateActionSet failed");
        return false;
    }

    XrPath handPaths[HAND_COUNT] = {};
    for (uint32_t hand = 0; hand < HAND_COUNT; ++hand) {
        xrStringToPath(instance, HAND_PATHS[hand], &handPaths[hand]);
        mHands[hand].path = handPaths[hand];
    }

    auto const createAction = [&](char const* name, char const* localizedName,
                                      XrActionType type, XrAction* action) {
        XrActionCreateInfo info = { XR_TYPE_ACTION_CREATE_INFO };
        strcpy(info.actionName, name);
        strcpy(info.localizedActionName, localizedName);
        info.actionType = type;
        info.countSubactionPaths = HAND_COUNT;
        info.subactionPaths = handPaths;
        return XR_SUCCEEDED(xrCreateAction(mActionSet, &info, action));
    };
    if (!createAction("grip_pose", "Grip pose", XR_ACTION_TYPE_POSE_INPUT, &mGripAction) ||
            !createAction("aim_pose", "Aim pose", XR_ACTION_TYPE_POSE_INPUT, &mAimAction) ||
            !createAction("trigger", "Trigger", XR_ACTION_TYPE_FLOAT_INPUT, &mTriggerAction)) {
        XRLOG("controller input: xrCreateAction failed");
        return false;
    }

    XrPath profile = XR_NULL_PATH;
    xrStringToPath(instance, "/interaction_profiles/oculus/touch_controller", &profile);
    XrActionSuggestedBinding bindings[HAND_COUNT * 3] = {};
    for (uint32_t hand = 0; hand < HAND_COUNT; ++hand) {
        XrPath grip = XR_NULL_PATH;
        XrPath aim = XR_NULL_PATH;
        XrPath trigger = XR_NULL_PATH;
        std::string const prefix = HAND_PATHS[hand];
        xrStringToPath(instance, (prefix + "/input/grip/pose").c_str(), &grip);
        xrStringToPath(instance, (prefix + "/input/aim/pose").c_str(), &aim);
        xrStringToPath(instance, (prefix + "/input/trigger/value").c_str(), &trigger);
        bindings[hand * 3 + 0] = { mGripAction, grip };
        bindings[hand * 3 + 1] = { mAimAction, aim };
        bindings[hand * 3 + 2] = { mTriggerAction, trigger };
    }
    XrInteractionProfileSuggestedBinding suggested = {
        XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING
    };
    suggested.interactionProfile = profile;
    suggested.countSuggestedBindings = HAND_COUNT * 3;
    suggested.suggestedBindings = bindings;
    if (XR_FAILED(xrSuggestInteractionProfileBindings(instance, &suggested))) {
        XRLOG("controller input: xrSuggestInteractionProfileBindings failed");
        return false;
    }

    XrSessionActionSetsAttachInfo attachInfo = { XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
    attachInfo.countActionSets = 1;
    attachInfo.actionSets = &mActionSet;
    if (XR_FAILED(xrAttachSessionActionSets(session, &attachInfo))) {
        XRLOG("controller input: xrAttachSessionActionSets failed");
        return false;
    }

    for (uint32_t hand = 0; hand < HAND_COUNT; ++hand) {
        auto const createSpace = [&](XrAction action, XrSpace* space) {
            XrActionSpaceCreateInfo info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
            info.action = action;
            info.subactionPath = handPaths[hand];
            info.poseInActionSpace.orientation.w = 1.0f;
            return XR_SUCCEEDED(xrCreateActionSpace(session, &info, space));
        };
        if (!createSpace(mGripAction, &mHands[hand].gripSpace) ||
                !createSpace(mAimAction, &mHands[hand].aimSpace)) {
            XRLOG("controller input: xrCreateActionSpace failed for %s", HAND_PATHS[hand]);
            return false;
        }
    }
    return true;
}

void ControllerInput::update(XrTime displayTime, XrSpace appSpace) {
    XrActiveActionSet activeActionSet = { mActionSet, XR_NULL_PATH };
    XrActionsSyncInfo syncInfo = { XR_TYPE_ACTIONS_SYNC_INFO };
    syncInfo.countActiveActionSets = 1;
    syncInfo.activeActionSets = &activeActionSet;
    if (XR_FAILED(xrSyncActions(mSession, &syncInfo))) {
        for (HandState& hand: mHands) {
            hand.gripValid = false;
            hand.aimValid = false;
            hand.triggerValue = 0.0f;
        }
        return;
    }

    constexpr XrSpaceLocationFlags REQUIRED = XR_SPACE_LOCATION_POSITION_VALID_BIT |
                                              XR_SPACE_LOCATION_ORIENTATION_VALID_BIT;
    for (uint32_t hand = 0; hand < HAND_COUNT; ++hand) {
        HandState& state = mHands[hand];
        auto const locate = [&](XrSpace space, XrPosef* pose) {
            XrSpaceLocation location = { XR_TYPE_SPACE_LOCATION };
            bool const valid = XR_SUCCEEDED(
                                       xrLocateSpace(space, appSpace, displayTime, &location)) &&
                               (location.locationFlags & REQUIRED) == REQUIRED;
            if (valid) {
                *pose = location.pose;
            }
            return valid;
        };
        state.gripValid = locate(state.gripSpace, &state.gripPose);
        state.aimValid = locate(state.aimSpace, &state.aimPose);

        XrActionStateGetInfo getInfo = { XR_TYPE_ACTION_STATE_GET_INFO };
        getInfo.action = mTriggerAction;
        getInfo.subactionPath = state.path;
        XrActionStateFloat trigger = { XR_TYPE_ACTION_STATE_FLOAT };
        state.triggerValue = XR_SUCCEEDED(xrGetActionStateFloat(mSession, &getInfo, &trigger)) &&
                                     trigger.isActive
                                   ? trigger.currentState
                                   : 0.0f;
    }
}

void ControllerInput::terminate() noexcept {
    for (HandState& hand: mHands) {
        if (hand.gripSpace != XR_NULL_HANDLE) {
            xrDestroySpace(hand.gripSpace);
        }
        if (hand.aimSpace != XR_NULL_HANDLE) {
            xrDestroySpace(hand.aimSpace);
        }
        hand = {};
    }
    for (XrAction* action: { &mGripAction, &mAimAction, &mTriggerAction }) {
        if (*action != XR_NULL_HANDLE) {
            xrDestroyAction(*action);
            *action = XR_NULL_HANDLE;
        }
    }
    if (mActionSet != XR_NULL_HANDLE) {
        xrDestroyActionSet(mActionSet);
        mActionSet = XR_NULL_HANDLE;
    }
}

bool ControllerInput::getGripPose(uint32_t hand, XrPosef* pose) const noexcept {
    if (hand >= HAND_COUNT || !mHands[hand].gripValid) {
        return false;
    }
    *pose = mHands[hand].gripPose;
    return true;
}

bool ControllerInput::getAimPose(uint32_t hand, XrPosef* pose) const noexcept {
    if (hand >= HAND_COUNT || !mHands[hand].aimValid) {
        return false;
    }
    *pose = mHands[hand].aimPose;
    return true;
}

float ControllerInput::getTriggerValue(uint32_t hand) const noexcept {
    return hand < HAND_COUNT ? mHands[hand].triggerValue : 0.0f;
}

} // namespace helloxr