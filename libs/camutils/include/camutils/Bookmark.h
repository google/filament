/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef CAMUTILS_BOOKMARK_H
#define CAMUTILS_BOOKMARK_H

#include <camutils/compiler.h>

#include <math/vec2.h>
#include <math/vec3.h>

namespace filament {
namespace camutils {

template <typename FLOAT> class FreeFlightManipulator;
template <typename FLOAT> class FreeFlight2Manipulator;
template <typename FLOAT> class OrbitManipulator;
template <typename FLOAT> class MapManipulator;
template <typename FLOAT> class Manipulator;

// FREE_FLIGHT_2 enables PAN and ZOOM through gestures
// FREE_FLIGHT offers PAN and ZOOM through keys
enum class Mode { ORBIT, MAP, FREE_FLIGHT, FREE_FLIGHT_2 };

/**
 * Opaque memento to a viewing position and orientation (e.g. the "home" camera position).
 *
 * This little struct is meant to be passed around by value and can be used to track camera
 * animation between waypoints. In map mode this implements Van Wijk interpolation.
 *
 * @see Manipulator::getCurrentBookmark, Manipulator::jumpToBookmark
 */
template <typename FLOAT>
struct CAMUTILS_PUBLIC Bookmark {
    /**
     * Interpolates between two bookmarks. The t argument must be between 0 and 1 (inclusive), and
     * the two endpoints must have the same mode (ORBIT or MAP).
     */
    static Bookmark<FLOAT> interpolate(Bookmark<FLOAT> a, Bookmark<FLOAT> b, double t);

    /**
     * Recommends a duration for animation between two MAP endpoints. The return value is a unitless
     * multiplier.
     */
    static double duration(Bookmark<FLOAT> a, Bookmark<FLOAT> b);

private:
    struct MapParams {
        FLOAT extent;
        filament::math::vec2<FLOAT> center;
    };
    struct OrbitParams {
        FLOAT phi;
        FLOAT theta;
        FLOAT distance;
        filament::math::vec3<FLOAT> pivot;
    };
    struct FlightParams {
        FLOAT pitch;
        FLOAT yaw;
        filament::math::vec3<FLOAT> position;
    };
    Mode mode;
    MapParams map;
    OrbitParams orbit;
    FlightParams flight;
    friend class FreeFlightManipulator<FLOAT>;
    friend class FreeFlight2Manipulator<FLOAT>;
    friend class OrbitManipulator<FLOAT>;
    friend class MapManipulator<FLOAT>;
};

} // namespace camutils
} // namespace filament

#endif // CAMUTILS_BOOKMARK_H
