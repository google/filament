/*
 * Copyright (C) 2021 The Android Open Source Project
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

#ifndef TNT_FILAMENT_PIDCONTROLLER_H
#define TNT_FILAMENT_PIDCONTROLLER_H

#include <math/scalar.h>

#include <limits>

namespace filament {

class PIDController {
public:
    PIDController() noexcept = default;

    void setStandardGains(float Kp, float Ti, float Td) noexcept {
        mKp = Kp;
        mKi = Kp / Ti;
        mKd = Kp * Td;
    }

    void setParallelGains(float Kp, float Ki, float Kd) noexcept {
        mKp = Kp;
        mKi = Ki;
        mKd = Kd;
    }

    // output is kept steady in the dead band
    void setOutputDeadBand(float low, float high) noexcept {
        mDeadBandLow = low;
        mDeadBandHigh = high;
    }

    // integral bounds to prevent windup
    void setIntegralLimits(float low, float high) noexcept {
        mIntegralLimitLow = low;
        mIntegralLimitHigh = high;
    }

    // output bounds
    void setOutputLimits(float low, float high) noexcept {
        mOutputLimitLow = low;
        mOutputLimitHigh = high;
    }

    // disable integral term to prevent windup
    void setIntegralInhibitionEnabled(bool enabled) noexcept {
        mIntegralInhibition = enabled ? 0.0f : 1.0f;
    }

    // update PID output
    float update(float measure, float target, float dt) const noexcept {
        // compute error
        const float error = target - measure;

        // compute error integration
        float integral = mIntegral + error * mIntegralInhibition * dt;

        // compute derivative
        const float derivative = (error - mLastError) / dt;

        // prevent integral windup
        integral = math::clamp(integral, mIntegralLimitLow, mIntegralLimitHigh);

        // PID controller output
        float out = mKp * error + mKi * integral + mKd * derivative;

        // Apply dead band
        if (out > mDeadBandLow && out < mDeadBandHigh) {
            out = 0.0f;
        }

        // Apply output limits
        out = math::clamp(out, mOutputLimitLow, mOutputLimitHigh);

        // save state for next round
        mIntegral = integral;
        mLastError = error;
        mDerivative = derivative;

        return out;
    }

    float getError() const noexcept {
        return mLastError;
    }

    float getIntegral() const noexcept {
        return mIntegral;
    }

    float getDerivative() const noexcept {
        return mDerivative;
    }

private:
    float mKp = 0.1f;
    float mKi = 0.0f;
    float mKd = 0.0f;
    float mIntegralInhibition = 1.0f;
    float mIntegralLimitLow = -std::numeric_limits<float>::infinity();
    float mIntegralLimitHigh = std::numeric_limits<float>::infinity();
    float mOutputLimitLow = -std::numeric_limits<float>::infinity();
    float mOutputLimitHigh = std::numeric_limits<float>::infinity();
    float mDeadBandLow = 0.0f;
    float mDeadBandHigh = 0.0f;
    mutable float mLastError = 0.0f;
    mutable float mIntegral = 0.0f;
    mutable float mDerivative = 0.0f;
};

} // namespace filament

#endif // TNT_FILAMENT_PIDCONTROLLER_H
