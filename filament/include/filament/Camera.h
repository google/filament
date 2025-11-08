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
#include <math/vec2.h>
#include <math/vec4.h>
#include <math/mat4.h>

#include <math.h>

#include <stdint.h>
#include <stddef.h>

namespace utils {
class Entity;
} // namespace utils

namespace filament {

/**
 * Camera represents the eye(s) through which the scene is viewed.
 *
 * A Camera has a position and orientation and controls the projection and exposure parameters.
 *
 * For stereoscopic rendering, a Camera maintains two separate "eyes": Eye 0 and Eye 1. These are
 * arbitrary and don't necessarily need to correspond to "left" and "right".
 *
 * Creation and destruction
 * ========================
 *
 * In Filament, Camera is a component that must be associated with an entity. To do so,
 * use Engine::createCamera(Entity). A Camera component is destroyed using
 * Engine::destroyCameraComponent(Entity).
 *
 * ~~~~~~~~~~~{.cpp}
 *  filament::Engine* engine = filament::Engine::create();
 *
 *  utils::Entity myCameraEntity = utils::EntityManager::get().create();
 *  filament::Camera* myCamera = engine->createCamera(myCameraEntity);
 *  myCamera->setProjection(45, 16.0/9.0, 0.1, 1.0);
 *  myCamera->lookAt({0, 1.60, 1}, {0, 0, 0});
 *  engine->destroyCameraComponent(myCameraEntity);
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
 * depth-buffer:
 *
 *    near (m)  |   1 m  |   10 m  |  100 m   |  1 Km
 *  -----------:|:------:|:-------:|:--------:|:--------:
 *      0.001   | 7.2e-5 |  0.0043 |  0.4624  |  48.58
 *      0.01    | 6.9e-6 |  0.0001 |  0.0430  |   4.62
 *      0.1     | 3.6e-7 |  7.0e-5 |  0.0072  |   0.43
 *      1.0     |    0   |  3.8e-6 |  0.0007  |   0.07
 *
 *  As can be seen in the table above, the depth-buffer precision drops rapidly with the
 *  distance to the camera.
 *
 * Make sure to pick the highest *near* plane distance possible.
 *
 * On Vulkan and Metal platforms (or OpenGL platforms supporting either EXT_clip_control or
 * ARB_clip_control extensions), the depth-buffer precision is much less dependent on the *near*
 * plane value:
 *
 *    near (m)  |   1 m  |   10 m  |  100 m   |  1 Km
 *  -----------:|:------:|:-------:|:--------:|:--------:
 *      0.001   | 1.2e-7 |  9.5e-7 |  7.6e-6  |  6.1e-5
 *      0.01    | 1.2e-7 |  9.5e-7 |  7.6e-6  |  6.1e-5
 *      0.1     | 5.9e-8 |  9.5e-7 |  1.5e-5  |  1.2e-4
 *      1.0     |    0   |  9.5e-7 |  7.6e-6  |  1.8e-4
 *
 *
 * Choosing the *far* plane distance
 * =================================
 *
 * The far plane distance is always set internally to infinity for rendering, however it is used for
 * culling and shadowing calculations. It is important to keep a reasonable ratio between
 * the near and far plane distances. Typically a ratio in the range 1:100 to 1:100000 is
 * commanded. Larger values may causes rendering artifacts or trigger assertions in debug builds.
 *
 *
 * Exposure
 * ========
 *
 * The Camera is also used to set the scene's exposure, just like with a real camera. The lights
 * intensity and the Camera exposure interact to produce the final scene's brightness.
 *
 *
 * Stereoscopic rendering
 * ======================
 *
 * The Camera's transform (as set by setModelMatrix or via TransformManager) defines a "head" space,
 * which typically corresponds to the location of the viewer's head. Each eye's transform is set
 * relative to this head space by setEyeModelMatrix.
 *
 * Each eye also maintains its own projection matrix. These can be set with setCustomEyeProjection.
 * Care must be taken to correctly set the projectionForCulling matrix, as well as its corresponding
 * near and far values. The projectionForCulling matrix must define a frustum (in head space) that
 * bounds the frustums of both eyes. Alternatively, culling may be disabled with
 * View::setFrustumCullingEnabled.
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

    /** Returns the projection matrix from the field-of-view.
     *
     * @param fovInDegrees full field-of-view in degrees. 0 < \p fov < 180.
     * @param aspect       aspect ratio \f$ \frac{width}{height} \f$. \p aspect > 0.
     * @param near         distance in world units from the camera to the near plane. \p near > 0.
     * @param far          distance in world units from the camera to the far plane. \p far > \p near.
     * @param direction    direction of the \p fovInDegrees parameter.
     *
     * @see Fov.
     */
    static math::mat4 projection(Fov direction, double fovInDegrees,
            double aspect, double near, double far = INFINITY);

    /** Returns the projection matrix from the focal length.
     *
     * @param focalLengthInMillimeters lens's focal length in millimeters. \p focalLength > 0.
     * @param aspect      aspect ratio \f$ \frac{width}{height} \f$. \p aspect > 0.
     * @param near        distance in world units from the camera to the near plane. \p near > 0.
     * @param far         distance in world units from the camera to the far plane. \p far > \p near.
     */
    static math::mat4 projection(double focalLengthInMillimeters,
            double aspect, double near, double far = INFINITY);


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
     * @see Projection, Frustum
     */
    void setProjection(Projection projection,
            double left, double right,
            double bottom, double top,
            double near, double far);


    /** Utility to set the projection matrix from the field-of-view.
     *
     * @param fovInDegrees full field-of-view in degrees. 0 < \p fov < 180.
     * @param aspect    aspect ratio \f$ \frac{width}{height} \f$. \p aspect > 0.
     * @param near      distance in world units from the camera to the near plane. \p near > 0.
     * @param far       distance in world units from the camera to the far plane. \p far > \p near.
     * @param direction direction of the \p fovInDegrees parameter.
     *
     * @see Fov.
     */
    void setProjection(double fovInDegrees, double aspect, double near, double far,
                       Fov direction = Fov::VERTICAL);

    /** Utility to set the projection matrix from the focal length.
     *
     * @param focalLengthInMillimeters lens's focal length in millimeters. \p focalLength > 0.
     * @param aspect    aspect ratio \f$ \frac{width}{height} \f$. \p aspect > 0.
     * @param near      distance in world units from the camera to the near plane. \p near > 0.
     * @param far       distance in world units from the camera to the far plane. \p far > \p near.
     */
    void setLensProjection(double focalLengthInMillimeters,
            double aspect, double near, double far);


    /** Sets a custom projection matrix.
     *
     * The projection matrix must define an NDC system that must match the OpenGL convention,
     * that is all 3 axis are mapped to [-1, 1].
     *
     * @param projection  custom projection matrix used for rendering and culling
     * @param near      distance in world units from the camera to the near plane.
     * @param far       distance in world units from the camera to the far plane. \p far != \p near.
     */
    void setCustomProjection(math::mat4 const& projection, double near, double far) noexcept;

    /** Sets the projection matrix.
     *
     * The projection matrices must define an NDC system that must match the OpenGL convention,
     * that is all 3 axis are mapped to [-1, 1].
     *
     * @param projection  custom projection matrix used for rendering
     * @param projectionForCulling  custom projection matrix used for culling
     * @param near      distance in world units from the camera to the near plane.
     * @param far       distance in world units from the camera to the far plane. \p far != \p near.
     */
    void setCustomProjection(math::mat4 const& projection, math::mat4 const& projectionForCulling,
            double near, double far) noexcept;

    /** Sets a custom projection matrix for each eye.
     *
     * The projectionForCulling, near, and far parameters establish a "culling frustum" which must
     * encompass anything any eye can see. All projection matrices must be set simultaneously. The
     * number of stereoscopic eyes is controlled by the stereoscopicEyeCount setting inside of
     * Engine::Config.
     *
     * @param projection an array of projection matrices, only the first config.stereoscopicEyeCount
     *                   are read
     * @param count size of the projection matrix array to set, must be
     *              >= config.stereoscopicEyeCount
     * @param projectionForCulling custom projection matrix for culling, must encompass both eyes
     * @param near distance in world units from the camera to the culling near plane. \p near > 0.
     * @param far distance in world units from the camera to the culling far plane. \p far > \p
     * near.
     * @see setCustomProjection
     * @see Engine::Config::stereoscopicEyeCount
     */
    void setCustomEyeProjection(math::mat4 const* UTILS_NONNULL projection, size_t count,
            math::mat4 const& projectionForCulling, double near, double far);

    /** Sets an additional matrix that scales the projection matrix.
     *
     * This is useful to adjust the aspect ratio of the camera independent from its projection.
     * First, pass an aspect of 1.0 to setProjection. Then set the scaling with the desired aspect
     * ratio:
     *
     *     const double aspect = width / height;
     *
     *     // with Fov::HORIZONTAL passed to setProjection:
     *     camera->setScaling(double4 {1.0, aspect});
     *
     *     // with Fov::VERTICAL passed to setProjection:
     *     camera->setScaling(double4 {1.0 / aspect, 1.0});
     *
     *
     * By default, this is an identity matrix.
     *
     * @param scaling     diagonal of the 2x2 scaling matrix to be applied after the projection matrix.
     *
     * @see setProjection, setLensProjection, setCustomProjection
     */
    void setScaling(math::double2 scaling) noexcept;

    /**
     * Sets an additional matrix that shifts the projection matrix.
     * By default, this is an identity matrix.
     *
     * @param shift     x and y translation added to the projection matrix, specified in NDC
     *                  coordinates, that is, if the translation must be specified in pixels,
     *                  shift must be scaled by 1.0 / { viewport.width, viewport.height }.
     *
     * @see setProjection, setLensProjection, setCustomProjection
     */
    void setShift(math::double2 shift) noexcept;

    /** Returns the scaling amount used to scale the projection matrix.
     *
     * @return the diagonal of the scaling matrix applied after the projection matrix.
     *
     * @see setScaling
     */
    math::double4 getScaling() const noexcept;

    /** Returns the shift amount used to translate the projection matrix.
     *
     * @return the 2D translation x and y offsets applied after the projection matrix.
     *
     * @see setShift
     */
    math::double2 getShift() const noexcept;

    /** Returns the projection matrix used for rendering.
     *
     * The projection matrix used for rendering always has its far plane set to infinity. This
     * is why it may differ from the matrix set through setProjection() or setLensProjection().
     *
     * @param eyeId the index of the eye to return the projection matrix for, must be
     *              < config.stereoscopicEyeCount
     * @return The projection matrix used for rendering
     *
     * @see setProjection, setLensProjection, setCustomProjection, getCullingProjectionMatrix,
     * setCustomEyeProjection
     */
    math::mat4 getProjectionMatrix(uint8_t eyeId = 0) const;


    /** Returns the projection matrix used for culling (far plane is finite).
     *
     * @return The projection matrix set by setProjection or setLensProjection.
     *
     * @see setProjection, setLensProjection, getProjectionMatrix
     */
    math::mat4 getCullingProjectionMatrix() const noexcept;


    //! Returns the frustum's near plane
    double getNear() const noexcept;

    //! Returns the frustum's far plane used for culling
    double getCullingFar() const noexcept;

    /** Sets the camera's model matrix.
     *
     * Helper method to set the camera's entity transform component.
     * It has the same effect as calling:
     *
     * ~~~~~~~~~~~{.cpp}
     *  engine.getTransformManager().setTransform(
     *          engine.getTransformManager().getInstance(camera->getEntity()), model);
     * ~~~~~~~~~~~
     *
     * @param modelMatrix The camera position and orientation provided as a rigid transform matrix.
     *
     * @note The Camera "looks" towards its -z axis
     *
     * @warning \p model must be a rigid transform
     */
    void setModelMatrix(const math::mat4& modelMatrix) noexcept;
    void setModelMatrix(const math::mat4f& modelMatrix) noexcept; //!< @overload

    /** Set the position of an eye relative to this Camera (head).
     *
     * By default, both eyes' model matrices are identity matrices.
     *
     * For example, to position Eye 0 3cm leftwards and Eye 1 3cm rightwards:
     * ~~~~~~~~~~~{.cpp}
     * const mat4 leftEye  = mat4::translation(double3{-0.03, 0.0, 0.0});
     * const mat4 rightEye = mat4::translation(double3{ 0.03, 0.0, 0.0});
     * camera.setEyeModelMatrix(0, leftEye);
     * camera.setEyeModelMatrix(1, rightEye);
     * ~~~~~~~~~~~
     *
     * This method is not intended to be called every frame. Instead, to update the position of the
     * head, use Camera::setModelMatrix.
     *
     * @param eyeId the index of the eye to set, must be < config.stereoscopicEyeCount
     * @param model the model matrix for an individual eye
     */
    void setEyeModelMatrix(uint8_t eyeId, math::mat4 const& model);

    /** Sets the camera's model matrix
     *
     * @param eye       The position of the camera in world space.
     * @param center    The point in world space the camera is looking at.
     * @param up        A unit vector denoting the camera's "up" direction.
     */
    void lookAt(math::double3 const& eye,
                math::double3 const& center,
                math::double3 const& up = math::double3{0, 1, 0}) noexcept;

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
    math::mat4 getModelMatrix() const noexcept;

    //! Returns the camera's view matrix (inverse of the model matrix)
    math::mat4 getViewMatrix() const noexcept;

    //! Returns the camera's position in world space
    math::double3 getPosition() const noexcept;

    //! Returns the camera's normalized left vector
    math::float3 getLeftVector() const noexcept;

    //! Returns the camera's normalized up vector
    math::float3 getUpVector() const noexcept;

    //! Returns the camera's forward vector
    math::float3 getForwardVector() const noexcept;

    //! Returns the camera's field of view in degrees
    float getFieldOfViewInDegrees(Fov direction) const noexcept;

    //! Returns the camera's culling Frustum in world space
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

    /** Returns the focal length in meters [m] for a 35mm camera.
     * Eye 0's projection matrix is used to compute the focal length.
     */
    double getFocalLength() const noexcept;

    /**
     * Sets the camera focus distance. This is used by the Depth-of-field PostProcessing effect.
     * @param distance Distance from the camera to the plane of focus in world units.
     *                 Must be positive and larger than the near clipping plane.
     */
    void setFocusDistance(float distance) noexcept;

    //! Returns the focus distance in world units
    float getFocusDistance() const noexcept;

    /**
     * Returns the inverse of a projection matrix.
     *
     * \param p the projection matrix to inverse
     * \returns the inverse of the projection matrix \p p
     */
    static math::mat4 inverseProjection(const math::mat4& p) noexcept;

    /**
     * Returns the inverse of a projection matrix.
     * @see inverseProjection(const math::mat4&)
     */
    static math::mat4f inverseProjection(const math::mat4f& p) noexcept;

    /**
     * Helper to compute the effective focal length taking into account the focus distance
     *
     * @param focalLength       focal length in any unit (e.g. [m] or [mm])
     * @param focusDistance     focus distance in same unit as focalLength
     * @return                  the effective focal length in same unit as focalLength
     */
    static double computeEffectiveFocalLength(double focalLength, double focusDistance) noexcept;

    /**
     * Helper to compute the effective field-of-view taking into account the focus distance
     *
     * @param fovInDegrees      full field of view in degrees
     * @param focusDistance     focus distance in meters [m]
     * @return                  effective full field of view in degrees
     */
    static double computeEffectiveFov(double fovInDegrees, double focusDistance) noexcept;

protected:
    // prevent heap allocation
    ~Camera() = default;
};

} // namespace filament

#endif // TNT_FILAMENT_CAMERA_H
