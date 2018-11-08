/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <cmath>
#include <iostream>
#include <limits>
#include <stdint.h>

struct Projection {

    enum NdcCoords {
        GL_NDC, DX_NDC
    };

    Projection(float near, float far, NdcCoords coords) {
        if (coords == GL_NDC) {
            if (std::isinf(far)) {
                A = -1;
                B = -2.0f * near;
            } else if (std::isinf(near)) {
                A = 1;
                B = 2.0f * far;
            } else {
                A = (far + near) / (near - far);            // lim(far->inf) = -1
                B = (2.0f * far * near) / (near - far);     // lim(far->inf) = -2*near
            }
        } else {
            if (std::isinf(far)) {
                A = -1;
                B = -near;
            } else if (std::isinf(near)) {
                A = 0;
                B = far;
            } else {
                A = far / (near - far);             // lim(far->inf) = -1
                B = (far * near) / (near - far);    // lim(far->inf) = -near
            }
        }
    }

    float cc(float z) const {
        return A*z + B;
    }

    float ndc(float z) const {
        return cc(z)/-z;
    }

    float A;
    float B;
};


float world_to_window(float z, Projection const& proj, float nearVal, float farVal) {
    float ndc = proj.ndc(z);
    float wc = ndc * ((farVal - nearVal) * 0.5f) + (farVal + nearVal) * 0.5f;
    return wc;
}

float window_to_world(double wc, Projection const& proj, float nearVal, float farVal) {
    float A = proj.A;
    float B = proj.B;
    double ndc = (wc - (farVal + nearVal) * 0.5) / ((farVal - nearVal) * 0.5);
    double z = -B / (ndc + A);
    return float(z);
}

static const float s_values[] = { 0.01f, 0.1f, 1.0f, 10.0f, 100.0f, 1000.0f, 10000.0f, 100000.0f };

void precisionForDistance(Projection const& proj, float nearVal, float farVal, float z) {
    float wc = world_to_window(-z, proj, nearVal, farVal);
    float wcn = std::nextafter(wc, farVal);
    float zn = window_to_world(wcn, proj, nearVal, farVal);

    float integer_wcn = (uint32_t(wc * 0x1000000U) + 1)/float(0x1000000U);
    float integer_zn = window_to_world(integer_wcn, proj, nearVal, farVal);

    printf("%8g:\t%8g\t%g\n", -z, std::abs(zn + z), std::abs(integer_zn + z));
}

void depthBufferPrecision(Projection proj, float nearVal, float farVal) {
    for (float z : s_values) {
        precisionForDistance(proj, nearVal, farVal, z);
    }
}

int main() {
    static constexpr float inf = std::numeric_limits<float>::infinity();

    const float near = 0.1f;

    std::cout << "GL [-1, 1]" << std::endl;
    depthBufferPrecision(Projection(near, inf, Projection::GL_NDC), 0, 1);

    std::cout << "GL [1, -1] (reversed)" << std::endl;
    depthBufferPrecision(Projection(inf, near, Projection::GL_NDC), 0, 1);

    std::cout << "DX [0, 1]" << std::endl;
    depthBufferPrecision(Projection(near, inf, Projection::DX_NDC), -1, 1);

    std::cout << "DX [1, 0] (reversed)" << std::endl;
    depthBufferPrecision(Projection(inf, near, Projection::DX_NDC), -1, 1);

    return 0;
}
