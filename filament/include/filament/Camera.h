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

//! \file

#ifndef TNT_FILAMENT_CAMERA_H
#define TNT_FILAMENT_CAMERA_H

#include <filament/FilamentAPI.h>

#include <utils/compiler.h>

#include <math/mathfwd.h>

namespace utils {
class Entity;
} // namespace utils

namespace filament {

/**
 * Camera represents the eye through which the scene is viewed.
 *
 * A Camera has a position and orientation and controls the projection and exposure parameters.
 *
 * Creation and destruction
 * ========================
 *
 * Like all Filament objects, Camera can only be constructed on the heap, however, unlike most
 * Filament objects it doesn't require a builder and can be constructed directly
 * using Engine::createCamera(). At the very least, a projection must be defined
 * using setProjection(). In most case, the camera position also needs to be set.
 *
 * A Camera object is destroyed using Engine::destroy(const Camera*).
 *
 * ~~~~~~~~~~~{.cpp}
 *  filament::Engine* engine = filament::Engine::create();
 *
 *  filament::Camera* myCamera = engine->createCamera();
 *  myCamera->setProjection(45, 16.0/9.0, 0.1, 1.0);
 *  myCamera->lookAt({0, 1.60, 1}, {0, 0, 0});
 *  engine->destroy(myCamera);
 * ~~~~~~~~~~~
 *
 *
 * Coordinate system
 * =================
 *
 * The camera coordinate system defines the *view space*. The camera points towards its -z axis
 * and is oriented such that its top side is in the direction of +y, and its right side in the
 * direction of +x.
 *
 * @note
 * Since the *near* and *far* planes are defined by the distance from the camera,
 * their respective coordinates are -\p distance(near) and -\p distance(far).
 *
 * Clipping planes
 * ===============
 *
 * The camera defines six *clipping planes* which together create a *clipping volume*. The
 * geometry outside this volume is clipped.
 *
 * The clipping volume can either be a box or a frustum depending on which projection is used,
 * respectively Projection.ORTHO or Projection.PERSPECTIVE. The six planes are specified either
 * directly or indirectly using setProjection().
 *
 * The six planes are:
 * - left
 * - right
 * - bottom
 * - top
 * - near
 * - far
 *
 * @note
 * To increase the depth-buffer precision, the *far* clipping plane is always assumed to be at
 * infinity for rendering. That is, it is not used to clip geometry during rendering.
 * However, it is used during the culling phase (objects entirely behind the *far*
 * plane are culled).
 *
 *
 * Choosing the *near* plane distance
 * ==================================
 *
 * The *near* plane distance greatly affects the depth-buffer resolution.
 *
 * Example: Precision at 1m, 10m, 100m and 1Km for various near distances assuming a 32-bit float
 * depth-buffer
 *
 *    near (m)  |   1 m  |   10 m  |  100 m   |  1 Km
 *  -----------:|:------:|:-------:|:--------:|:--------:
 *      0.001   | 7.2e-5 |  0.0043 |  0.4624  |  48.58
 *      0.01    | 6.9e-6 |  0.0001 |  0.0430  |   4.62
 *      0.1     | 3.6e-7 |  7.0e-5 |  0.0072  |   0.43
 *      1.0     |    0   |  3.8e-6 |  0.0007  |   0.07
 *
 *
 *  As can be seen in the table above, the depth-buffer precision drops rapidly with the
 *  distance to the camera.
 * Make sure to pick the highest *near* plane distance possible.
 *
 *
 * Exposure
 * ========
 *
 * The Camera is also used to set the scene's exposure, just like with a real camera. The lights
 * intensity and the Camera exposure interact to produce the final scene's brightness.
 *
 *
 *
 * \see Frustum, View
 */
class UTILS_PUBLIC Camera : public FilamentAPI {
public:
    //! Denotes the projection type used by this camera. \see setProjection
    enum class Projection : int {
        PERSPECTIVE,    //!< perspective projection, objects get smaller as they are farther
        ORTHO           //!< orthonormal projection, preserves distances
    };

    //! Denotes a field-of-view direction. \see setProjection
    enum class Fov : int {
        VERTICAL,       //!< the field-of-view angle is defined on the vertical axis
        HORIZONTAL      //!< the field-of-view angle is defined on the horizontal axis
    };

    /** Sets the projection matrix from a frustum defined by six planes.
     *
     * @param projection    type of #Projection to use.
     *
     * @param left      distance in world units from the camera to the left plane,
     *                  at the near plane.
     *                  Precondition: \p left != \p right.
     *
     * @param right     distance in world units from the camera to the right plane,
     *                  at the near plane.
     *                  Precondition: \p left != \p right.
     *
     * @param bottom    distance in world units from the camera to the bottom plane,
     *                  at the near plane.
     *                  Precondition: \p bottom != \p top.
     *
     * @param top       distance in world units from the camera to the top plane,
     *                  at the near plane.
     *                  Precondition: \p left != \p right.
     *
     * @param near      distance in world units from the camera to the near plane. The near plane's
     *                  position in view space is z = -\p near.
     *                  Precondition: \p near > 0 for PROJECTION::PERSPECTIVE or
     *                                \p near != far for PROJECTION::ORTHO
     *
     * @param far       distance in world units from the camera to the far plane. The far plane's
     *                  position in view space is z = -\p far.
     *                  Precondition: \p far > near for PROJECTION::PERSPECTIVE or
     *                                \p far != near for PROJECTION::ORTHO
     *
     * @attention these parameters are silently modified to meet the preconditions above.
     *
     * @see Projection, Frustum
     */
    void setProjection(Projection projection,
            double left, double right,
            double bottom, double top,
            double near, double far) noexcept;

    /** Sets the projection matrix from the field-of-view.
     *
     * @param fovInDegrees full field-of-view in degrees. 0 < \p fov < 180.
     * @param aspect       aspect ratio \f$ \frac{width}{height} \f$. \p aspect > 0.
     * @param near         distance in world units from the camera to the near plane. \p near > 0.
     * @param far          distance in world units from the camera to the far plane. \p far > \p near.
     * @param direction    direction of the \p fovInDegrees parameter.
     *
     * @see Fov.
     */
    void setProjection(double fovInDegrees, double aspect, double near, double far,
                       Fov direction = Fov::VERTICAL) noexcept;

    /** Sets the projection matrix from the focal length.
     *
     * @param focalLength lens's focal length in millimeters. \p focalLength > 0.
     * @param aspect      aspect ratio \f$ \frac{width}{height} \f$. \p aspect > 0.
     * @param near        distance in world units from the camera to the near plane. \p near > 0.
     * @param far         distance in world units from the camera to the far plane. \p far > \p near.
     */
    void setLensProjection(double focalLength, double aspect, double near, double far) noexcept;

    /** Sets the projection matrix.
     *
     * @param projection  custom projection matrix.
     * @param near        distance in world units from the camera to the near plane. \p near > 0.
     * @param far         distance in world units from the camera to the far plane. \p far > \p near.
     */
    void setCustomProjection(math::mat4 const& projection, double near, double far) noexcept;

    /** Returns the projection matrix used for rendering.
     *
     * The projection matrix used for rendering always has its far plane set to infinity. This
     * it why it may differ from the matrix set through setProjection() or setLensProjection().
     *
     * @return The projection matrix used for rendering
     *
     * @see setProjection, setLensProjection, setCustomProjection, getCullingProjectionMatrix
     */
    const math::mat4& getProjectionMatrix() const noexcept;


    /** Returns the projection matrix used for culling (far plane is finite).
     *
     * @return The projection matrix set by setProjection or setLensProjection.
     *
     * @see setProjection, setLensProjection, getProjectionMatrix
     */
    const math::mat4& getCullingProjectionMatrix() const noexcept;


    //! Returns the frustum's near plane
    float getNear() const noexcept;

    //! Returns the frustum's far plane used for culling
    float getCullingFar() const noexcept;

    /** Sets the camera's view matrix.
     *
     * Helper method to set the camera's entity transform component.
     * It has the same effect as calling:
     *
     * ~~~~~~~~~~~{.cpp}
     *  engine.getTransformManager().setTransform(
     *          engine.getTransformManager().getInstance(camera->getEntity()), view);
     * ~~~~~~~~~~~
     *
     * @param view The camera position and orientation provided as a rigid transform matrix.
     *
     * @note The Camera "looks" towards its -z axis
     *
     * @warning \p view must be a rigid transform
     */
    void setModelMatrix(const math::mat4f& view) noexcept;

    /** Sets the camera's view matrix
     *
     * @param eye       The position of the camera in world space.
     * @param center    The point in world space the camera is looking at.
     * @param up        A unit vector denoting the camera's "up" direction.
     */
    void lookAt(const math::float3& eye,
                const math::float3& center,
                const math::float3& up) noexcept;

    /** Sets the camera's view matrix, assuming up is along the y axis
     *
     * @param eye       The position of the camera in world space.
     * @param center    The point in world space the camera is looking at.
     */
    void lookAt(const math::float3& eye,
                const math::float3& center) noexcept;

    /** Returns the camera's model matrix
     *
     * Helper method to return the camera's entity transform component.
     * It has the same effect as calling:
     *
     * ~~~~~~~~~~~{.cpp}
     *  engine.getTransformManager().getWorldTransform(
     *          engine.getTransformManager().getInstance(camera->getEntity()));
     * ~~~~~~~~~~~
     *
     * @return The camera's pose in world space as a rigid transform. Parent transforms, if any,
     * are taken into account.
     */
    math::mat4f getModelMatrix() const noexcept;

    //! Returns the camera's view matrix (inverse of the model matrix)
    math::mat4f getViewMatrix() const noexcept;

    //! Returns the camera's position in world space
    math::float3 getPosition() const noexcept;

    //! Returns the camera's normalized left vector
    math::float3 getLeftVector() const noexcept;

    //! Returns the camera's normalized up vector
    math::float3 getUpVector() const noexcept;

    //! Returns the camera's forward vector
    math::float3 getForwardVector() const noexcept;

    //! Returns the camera's field of view in degrees
    float getFieldOfViewInDegrees(Fov direction) const noexcept;

    //! Returns a Frustum object in world space
    class Frustum getFrustum() const noexcept;

    //! Returns the entity representing this camera
    utils::Entity getEntity() const noexcept;

    /** Sets this camera's exposure (default is f/16, 1/125s, 100 ISO)
     *
     * The exposure ultimately controls the scene's brightness, just like with a real camera.
     * The default values provide adequate exposure for a camera placed outdoors on a sunny day
     * with the sun at the zenith.
     *
     * @param aperture      Aperture in f-stops, clamped between 0.5 and 64.
     *                      A lower \p aperture value *increases* the exposure, leading to
     *                      a brighter scene. Realistic values are between 0.95 and 32.
     *
     * @param shutterSpeed  Shutter speed in seconds, clamped between 1/25,000 and 60.
     *                      A lower shutter speed increases the exposure. Realistic values are
     *                      between 1/8000 and 30.
     *
     * @param sensitivity   Sensitivity in ISO, clamped between 10 and 204,800.
     *                      A higher \p sensitivity increases the exposure. Realistic values are
     *                      between 50 and 25600.
     *
     * @note
     * With the default parameters, the scene must contain at least one Light of intensity
     * similar to the sun (e.g.: a 100,000 lux directional light).
     *
     * @see LightManager, Exposure
     */
    void setExposure(float aperture, float shutterSpeed, float sensitivity) noexcept;

    /** Sets this camera's exposure directly. Calling this method will set the aperture
     * to 1.0, the shutter speed to 1.2 and the sensitivity will be computed to match
     * the requested exposure (for a desired exposure of 1.0, the sensitivity will be
     * set to 100 ISO).
     *
     * This method is useful when trying to match the lighting of other engines or tools.
     * Many engines/tools use unit-less light intensities, which can be matched by setting
     * the exposure manually. This can be typically achieved by setting the exposure to
     * 1.0.
     */
    void setExposure(float exposure) noexcept {
        setExposure(1.0f, 1.2f, 100.0f * (1.0f / exposure));
    }

    //! returns this camera's aperture in f-stops
    float getAperture() const noexcept;

    //! returns this camera's shutter speed in seconds
    float getShutterSpeed() const noexcept;

    //! returns this camera's sensitivity in ISO
    float getSensitivity() const noexcept;

    /**
     * Returns the inverse of a projection matrix.
     *
     * \param p the projection matrix to inverse
     * \returns the inverse of the projection matrix \p p
     *
     * \warning the projection matrix to invert must have one of the form below:
     * - perspective projection
     *
     *      \f$
     *      \left(
     *      \begin{array}{cccc}
     *      a & 0 & tx & 0 \\
     *      0 & b & ty & 0 \\
     *      0 & 0 & tz & c \\
     *      0 & 0 & -1 & 0 \\
     *      \end{array}
     *      \right)
     *      \f$
     *
     * - orthographic projection
     *
     *      \f$
     *      \left(
     *      \begin{array}{cccc}
     *      a & 0 & 0 & tx \\
     *      0 & b & 0 & ty \\
     *      0 & 0 & c & tz \\
     *      0 & 0 & 0 & 1  \\
     *      \end{array}
     *      \right)
     *      \f$
     */
    static math::mat4 inverseProjection(const math::mat4& p) noexcept;

    /**
     * Returns the inverse of a projection matrix.
     * @see inverseProjection(const math::mat4&)
     */
    static math::mat4f inverseProjection(const math::mat4f& p) noexcept;
};

} // namespace filament

#endif // TNT_FILAMENT_CAMERA_H
