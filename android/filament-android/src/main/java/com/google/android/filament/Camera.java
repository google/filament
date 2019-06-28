/*
 * Copyright (C) 2017 The Android Open Source Project
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

package com.google.android.filament;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.Size;

/**
 * Camera represents the eye through which the scene is viewed.
 * <p>
 * A Camera has a position and orientation and controls the projection and exposure parameters.
 *
 * <h1><u>Creation and destruction</u></h1>
 *
 * Unlike most Filament objects, Camera doesn't require a builder and can be constructed directly
 * using {@link Engine#createCamera}. At the very least, a projection must be defined
 * using {@link #setProjection}. In most case, the camera position also needs to be set.
 * <p>
 * A Camera object is destroyed using {@link Engine#destroyCamera}.
 *
 * <pre>
 *  Engine engine = Engine.create();
 *
 *  Camera myCamera = engine.createCamera();
 *  myCamera.setProjection(45, 16.0/9.0, 0.1, 1.0);
 *  myCamera.lookAt(0, 1.60, 1,
 *                  0, 0, 0,
 *                  0, 1, 0);
 *  engine.destroyCamera(myCamera);
 * </pre>
 *
 *
 * <h1><u>Coordinate system</u></h1>
 *
 * The camera coordinate system defines the <b>view space</b>. The camera points towards its -z axis
 * and is oriented such that its top side is in the direction of +y, and its right side in the
 * direction of +x.
 * <p>
 * Since the <b>near</b> and <b>far</b> planes are defined by the distance from the camera,
 * their respective coordinates are -distance<sub>near</sub> and -distance<sub>far</sub>.
 *
 * <h1><u>Clipping planes</u></h1>
 *
 * The camera defines six <b>clipping planes</b> which together create a <b>clipping volume</b>. The
 * geometry outside this volume is clipped.
 * <p>
 * The clipping volume can either be a box or a frustum depending on which projection is used,
 * respectively {@link Projection#ORTHO ORTHO} or {@link Projection#PERSPECTIVE PERSPECTIVE}.
 * The six planes are specified either directly or indirectly using  {@link #setProjection} or
 * {@link #setLensProjection}.
 * <p>
 * The six planes are:
 * <ul>
 * <li> left    </li>
 * <li> right   </li>
 * <li> bottom  </li>
 * <li> top     </li>
 * <li> near    </li>
 * <li> far     </li>
 * </ul>
 * <p>
 *
 * To increase the depth-buffer precision, the <b>far</b> clipping plane is always assumed to be at
 * infinity for rendering. That is, it is not used to clip geometry during rendering.
 * However, it is used during the culling phase (objects entirely behind the <b>far</b>
 * plane are culled).
 *
 * <h1><u>Choosing the <b>near</b> plane distance</u></h1>
 *
 * The <b>near</b> plane distance greatly affects the depth-buffer resolution.
 * <p>
 *
 * Example: Precision at 1m, 10m, 100m and 1Km for various near distances assuming a 32-bit float
 * depth-buffer
 *
 * <center>
 * <table border="1">
 *     <tr>
 *         <th> near (m) </th><th> 1 m </th><th> 10 m </th><th> 100 m</th><th> 1 Km </th>
 *     </tr>
 *     <tr>
 *         <td>0.001</td><td>7.2e-5</td><td>0.0043</td><td>0.4624</td><td>48.58</td>
 *     </tr>
 *     <tr>
 *         <td>0.01</td><td>6.9e-6</td><td>0.0001</td><td>0.0430</td><td>4.62</td>
 *     </tr>
 *     <tr>
 *         <td>0.1</td><td>3.6e-7</td><td>7.0e-5</td><td>0.0072</td><td>0.43</td>
 *     </tr>
 *     <tr>
 *         <td>1.0</td><td>0</td><td>3.8e-6</td><td>0.0007</td><td>0.07</td>
 *     </tr>
 * </table>
 * </center>
 * <p>
 *
 * As can be seen in the table above, the depth-buffer precision drops rapidly with the
 * distance to the camera.
 * <p>
 * Make sure to pick the highest <b>near</b> plane distance possible.
 *
 *
 * <h1><u>Exposure</u></h1>
 *
 * The Camera is also used to set the scene's exposure, just like with a real camera. The lights
 * intensity and the Camera exposure interact to produce the final scene's brightness.
 *
 * @see View
 */
public class Camera {
    private long mNativeObject;

    /**
     * Denotes the projection type used by this camera.
     * @see #setProjection
     */
    public enum Projection {
        /** Perspective projection, objects get smaller as they are farther.  */
        PERSPECTIVE,
        /** Orthonormal projection, preserves distances. */
        ORTHO
    }

    /**
     * Denotes a field-of-view direction.
     * @see #setProjection
     */
    public enum Fov {
        /** The field-of-view angle is defined on the vertical axis. */
        VERTICAL,
        /** The field-of-view angle is defined on the horizontal axis. */
        HORIZONTAL
    }

    Camera(long nativeCamera) {
        mNativeObject = nativeCamera;
    }

    /**
     * Sets the projection matrix from a frustum defined by six planes.
     *
     * @param projection    type of projection to use
     *
     * @param left          distance in world units from the camera to the left plane,
     *                      at the near plane. Precondition: left != right.
     *
     * @param right         distance in world units from the camera to the right plane,
     *                      at the near plane. Precondition: left != right.
     *
     * @param bottom        distance in world units from the camera to the bottom plane,
     *                      at the near plane. Precondition: bottom != top.
     *
     * @param top           distance in world units from the camera to the top plane,
     *                      at the near plane. Precondition: left != right.
     *
     * @param near          distance in world units from the camera to the near plane.
     *                      The near plane's position in view space is z = -near.
     *                      Precondition: near > 0 for {@link Projection#PERSPECTIVE} or
     *                                    near != far for {@link Projection#ORTHO}.
     *
     * @param far           distance in world units from the camera to the far plane.
     *                      The far plane's position in view space is z = -far.
     *                      Precondition: far > near for {@link Projection#PERSPECTIVE} or
     *                                    far != near for {@link Projection#ORTHO}.
     *
     * <p>
     * These parameters are silently modified to meet the preconditions above.
     *
     * @see Projection
     */
    public void setProjection(@NonNull Projection projection, double left, double right,
            double bottom, double top, double near, double far) {
        nSetProjection(getNativeObject(), projection.ordinal(), left, right, bottom, top, near, far);
    }

    /**
     *
     * @param fovInDegrees
     * @param aspect
     * @param near
     * @param far
     * @param direction
     */
    public void setProjection(double fovInDegrees, double aspect, double near, double far,
            @NonNull Fov direction) {
        nSetProjectionFov(getNativeObject(), fovInDegrees, aspect, near, far, direction.ordinal());
    }

    /**
     *
     * @param focalLength
     * @param near
     * @param far
     */
    public void setLensProjection(double focalLength, double near, double far) {
        nSetLensProjection(getNativeObject(), focalLength, near, far);
    }

    /**
     *
     * @param inMatrix
     * @param near
     * @param far
     */
    public void setCustomProjection(@NonNull @Size(min = 16) double inMatrix[],
            double near, double far) {
        Asserts.assertMat4dIn(inMatrix);
        nSetCustomProjection(getNativeObject(), inMatrix, near, far);
    }

    /**
     *
     * @param in
     */
    public void setModelMatrix(@NonNull @Size(min = 16) float in[]) {
        Asserts.assertMat4fIn(in);
        nSetModelMatrix(getNativeObject(), in);
    }

    /**
     *
     * @param eyeX
     * @param eyeY
     * @param eyeZ
     * @param centerX
     * @param centerY
     * @param centerZ
     * @param upX
     * @param upY
     * @param upZ
     */
    public void lookAt(double eyeX, double eyeY, double eyeZ,
            double centerX, double centerY, double centerZ, double upX, double upY, double upZ) {
        nLookAt(getNativeObject(), eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ);
    }

    /**
     *
     * @return Distance to the near plane.
     */
    public float getNear() {
        return nGetNear(getNativeObject());
    }

    /**
     *
     * @return Distance to the far plane.
     */
    public float getCullingFar() {
        return nGetCullingFar(getNativeObject());
    }

    /**
     * Retrieves the camera's projection matrix.
     * @param out A 16-float array where the projection matrix will be stored, or null in which
     *            case a new array is allocated.
     * @return A 16-float array containing the camera's projection as a column-major matrix.
     */
    @NonNull @Size(min = 16)
    public double[] getProjectionMatrix(@Nullable @Size(min = 16) double out[]) {
        out = Asserts.assertMat4d(out);
        nGetProjectionMatrix(getNativeObject(), out);
        return out;
    }

    /**
     * Retrieves the camera's model matrix. The model matrix encodes the camera position and
     * orientation, or pose.
     * @param out A 16-float array where the model matrix will be stored, or null in which
     *            case a new array is allocated.
     * @return A 16-float array containing the camera's pose as a column-major matrix.
     */
    @NonNull @Size(min = 16)
    public float[] getModelMatrix(@Nullable @Size(min = 16) float out[]) {
        out = Asserts.assertMat4f(out);
        nGetModelMatrix(getNativeObject(), out);
        return out;
    }

    /**
     * Retrieves the camera's view matrix. The view matrix is the inverse of the model matrix.
     *
     * @param out A 16-float array where the model view will be stored, or null in which
     *            case a new array is allocated.
     * @return A 16-float array containing the camera's view as a column-major matrix.
     */
    @NonNull @Size(min = 16)
    public float[] getViewMatrix(@Nullable @Size(min = 16) float out[]) {
        out = Asserts.assertMat4f(out);
        nGetViewMatrix(getNativeObject(), out);
        return out;
    }

    /**
     * Retrieves the camera position in world space.
     * @param out A 3-float array where the position will be stored, or null in which case a new
     *            array is allocated.
     * @return A 3-float array containing the camera's position in world units.
     */
    @NonNull @Size(min = 3)
    public float[] getPosition(@Nullable @Size(min = 3) float out[]) {
        out = Asserts.assertFloat3(out);
        nGetPosition(getNativeObject(), out);
        return out;
    }

    /**
     * Retrieves the camera left unit vector in world space, that is a unit vector that points to
     * the left of the camera.
     * @param out A 3-float array where the left vector will be stored, or null in which case a new
     *            array is allocated.
     * @return A 3-float array containing the camera's left vector in world units.
     */
    @NonNull @Size(min = 3)
    public float[] getLeftVector(@Nullable @Size(min = 3) float out[]) {
        out = Asserts.assertFloat3(out);
        nGetLeftVector(getNativeObject(), out);
        return out;
    }

    /**
     * Retrieves the camera up unit vector in world space, that is a unit vector that points up with
     * respect to the camera.
     * @param out A 3-float array where the up vector will be stored, or null in which case a new
     *            array is allocated.
     * @return A 3-float array containing the camera's up vector in world units.
     */
    @NonNull @Size(min = 3)
    public float[] getUpVector(@Nullable @Size(min = 3) float out[]) {
        out = Asserts.assertFloat3(out);
        nGetUpVector(getNativeObject(), out);
        return out;
    }

    /**
     * Retrieves the camera forward unit vector in world space, that is a unit vector that points
     * in the direction the camera is looking at.
     * @param out A 3-float array where the forward vector will be stored, or null in which case a
     *           new  array is allocated.
     * @return A 3-float array containing the camera's forward vector in world units.
     */
    @NonNull @Size(min = 3)
    public float[] getForwardVector(@Nullable @Size(min = 3) float out[]) {
        out = Asserts.assertFloat3(out);
        nGetForwardVector(getNativeObject(), out);
        return out;
    }

    /**
     *
     * @param aperture
     * @param shutterSpeed
     * @param sensitivity
     */
    public void setExposure(float aperture, float shutterSpeed, float sensitivity) {
        nSetExposure(getNativeObject(), aperture, shutterSpeed, sensitivity);
    }

    /**
     *
     * @return
     */
    public float getAperture() {
        return nGetAperture(getNativeObject());
    }

    /**
     *
     * @return
     */
    public float getShutterSpeed() {
        return nGetShutterSpeed(getNativeObject());
    }

    /**
     *
     * @return
     */
    public float getSensitivity() {
        return nGetSensitivity(getNativeObject());
    }

    long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed Camera");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native void nSetProjection(long nativeCamera, int projection, double left, double right, double bottom, double top, double near, double far);
    private static native void nSetProjectionFov(long nativeCamera, double fovInDegrees, double aspect, double near, double far, int fov);
    private static native void nSetLensProjection(long nativeCamera, double focalLength, double near, double far);
    private static native void nSetCustomProjection(long nativeCamera, double[] inMatrix, double near, double far);
    private static native void nSetModelMatrix(long nativeCamera, float[] in);
    private static native void nLookAt(long nativeCamera, double eyeX, double eyeY, double eyeZ, double centerX, double centerY, double centerZ, double upX, double upY, double upZ);
    private static native float nGetNear(long nativeCamera);
    private static native float nGetCullingFar(long nativeCamera);
    private static native void nGetProjectionMatrix(long nativeCamera, double[] out);
    private static native void nGetModelMatrix(long nativeCamera, float[] out);
    private static native void nGetViewMatrix(long nativeCamera, float[] out);
    private static native void nGetPosition(long nativeCamera, float[] out);
    private static native void nGetLeftVector(long nativeCamera, float[] out);
    private static native void nGetUpVector(long nativeCamera, float[] out);
    private static native void nGetForwardVector(long nativeCamera, float[] out);
    private static native void nSetExposure(long nativeCamera, float aperture, float shutterSpeed, float sensitivity);
    private static native float nGetAperture(long nativeCamera);
    private static native float nGetShutterSpeed(long nativeCamera);
    private static native float nGetSensitivity(long nativeCamera);
}
