//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)

# Camera

open class [Camera](index.md)

Camera represents the eye through which the scene is viewed. 

 A Camera has a position and orientation and controls the projection and exposure parameters. 

# Creation and destruction

 In Filament, Camera is a component that must be associated with an entity. To do so, use [createCamera](../-engine/create-camera.md). A Camera component is destroyed using [destroyCameraComponent](../-engine/destroy-camera-component.md) ()}. ```kotlin
 Camera myCamera = engine.createCamera(myCameraEntity);
 myCamera.setProjection(45, 16.0/9.0, 0.1, 1.0);
 myCamera.lookAt(0, 1.60, 1,
                 0, 0, 0,
                 0, 1, 0);
 engine.destroyCameraComponent(myCameraEntity);

```

# Coordinate system

 The camera coordinate system defines the **view space**. The camera points towards its -z axis and is oriented such that its top side is in the direction of +y, and its right side in the direction of +x. 

 Since the **near** and **far** planes are defined by the distance from the camera, their respective coordinates are -distancenear and -distancefar. 

# Clipping planes

 The camera defines six **clipping planes** which together create a **clipping volume**. The geometry outside this volume is clipped. 

 The clipping volume can either be a box or a frustum depending on which projection is used, respectively [ORTHO](-projection/-o-r-t-h-o/index.md) or [PERSPECTIVE](-projection/-p-e-r-s-p-e-c-t-i-v-e/index.md). The six planes are specified either directly or indirectly using setProjection or [setLensProjection](set-lens-projection.md). 

 The six planes are: 

- left
- right
- bottom
- top
- near
- far

 To increase the depth-buffer precision, the **far** clipping plane is always assumed to be at infinity for rendering. That is, it is not used to clip geometry during rendering. However, it is used during the culling phase (objects entirely behind the **far** plane are culled). 

# Choosing the **near** plane distance

 The **near** plane distance greatly affects the depth-buffer resolution. 

 Example: Precision at 1m, 10m, 100m and 1Km for various near distances assuming a 32-bit float depth-buffer 

 As can be seen in the table above, the depth-buffer precision drops rapidly with the distance to the camera. 

 Make sure to pick the highest **near** plane distance possible. 

# Exposure

 The Camera is also used to set the scene's exposure, just like with a real camera. The lights intensity and the Camera exposure interact to produce the final scene's brightness.

#### See also

| |
|---|
| [View](../-view/index.md) |

## Types

| Name | Summary |
|---|---|
| [Fov](-fov/index.md) | [main]<br>enum [Fov](-fov/index.md)<br>Denotes a field-of-view direction. |
| [Projection](-projection/index.md) | [main]<br>enum [Projection](-projection/index.md)<br>Denotes the projection type used by this camera. |

## Functions

| Name | Summary |
|---|---|
| [getAperture](get-aperture.md) | [main]<br>open fun [getAperture](get-aperture.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Gets the aperture in f-stops |
| [getCullingFar](get-culling-far.md) | [main]<br>open fun [getCullingFar](get-culling-far.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Gets the distance to the far plane |
| [getCullingProjectionMatrix](get-culling-projection-matrix.md) | [main]<br>open fun [getCullingProjectionMatrix](get-culling-projection-matrix.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;<br>Retrieves the camera's culling matrix. |
| [getEntity](get-entity.md) | [main]<br>open fun [getEntity](get-entity.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Gets the entity representing this Camera |
| [getFieldOfViewInDegrees](get-field-of-view-in-degrees.md) | [main]<br>open fun [getFieldOfViewInDegrees](get-field-of-view-in-degrees.md)(direction: [Camera.Fov](-fov/index.md)): [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)<br>Returns the camera's field of view in degrees. |
| [getFocalLength](get-focal-length.md) | [main]<br>open fun [getFocalLength](get-focal-length.md)(): [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)<br>Gets the focal length in meters |
| [getFocusDistance](get-focus-distance.md) | [main]<br>open fun [getFocusDistance](get-focus-distance.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Gets the distance from the camera to the focus plane in world units |
| [getForwardVector](get-forward-vector.md) | [main]<br>open fun [getForwardVector](get-forward-vector.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Retrieves the camera forward unit vector in world space, that is a unit vector that points in the direction the camera is looking at. |
| [getLeftVector](get-left-vector.md) | [main]<br>open fun [getLeftVector](get-left-vector.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Retrieves the camera left unit vector in world space, that is a unit vector that points to the left of the camera. |
| [getModelMatrix](get-model-matrix.md) | [main]<br>open fun [getModelMatrix](get-model-matrix.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;<br>open fun [getModelMatrix](get-model-matrix.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Retrieves the camera's model matrix. |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getNear](get-near.md) | [main]<br>open fun [getNear](get-near.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Gets the distance to the near plane |
| [getPosition](get-position.md) | [main]<br>open fun [getPosition](get-position.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Retrieves the camera position in world space. |
| [getProjectionMatrix](get-projection-matrix.md) | [main]<br>open fun [getProjectionMatrix](get-projection-matrix.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;<br>Retrieves the camera's projection matrix. |
| [getScaling](get-scaling.md) | [main]<br>open fun [getScaling](get-scaling.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;<br>Returns the scaling amount used to scale the projection matrix. |
| [getSensitivity](get-sensitivity.md) | [main]<br>open fun [getSensitivity](get-sensitivity.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Gets the sensitivity in ISO |
| [getShift](get-shift.md) | [main]<br>open fun [getShift](get-shift.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;<br>Returns the shift amount used to translate the projection matrix. |
| [getShutterSpeed](get-shutter-speed.md) | [main]<br>open fun [getShutterSpeed](get-shutter-speed.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Gets the shutter speed in seconds |
| [getUpVector](get-up-vector.md) | [main]<br>open fun [getUpVector](get-up-vector.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Retrieves the camera up unit vector in world space, that is a unit vector that points up with respect to the camera. |
| [getViewMatrix](get-view-matrix.md) | [main]<br>open fun [getViewMatrix](get-view-matrix.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;<br>open fun [getViewMatrix](get-view-matrix.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Retrieves the camera's view matrix. |
| [lookAt](look-at.md) | [main]<br>open fun [lookAt](look-at.md)(eyeX: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), eyeY: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), eyeZ: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), centerX: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), centerY: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), centerZ: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), upX: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), upY: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), upZ: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html))<br>Sets the camera's model matrix. |
| [setCustomEyeProjection](set-custom-eye-projection.md) | [main]<br>open fun [setCustomEyeProjection](set-custom-eye-projection.md)(inProjection: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;, count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), inProjectionForCulling: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;, near: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), far: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html))<br>Sets a custom projection matrix for each eye. |
| [setCustomProjection](set-custom-projection.md) | [main]<br>open fun [setCustomProjection](set-custom-projection.md)(inProjection: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;, near: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), far: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html))<br>open fun [setCustomProjection](set-custom-projection.md)(inProjection: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;, inProjectionForCulling: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;, near: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), far: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html))<br>Sets a custom projection matrix. |
| [setExposure](set-exposure.md) | [main]<br>open fun [setExposure](set-exposure.md)(exposure: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Sets this camera's exposure directly.<br>[main]<br>open fun [setExposure](set-exposure.md)(aperture: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), shutterSpeed: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), sensitivity: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Sets this camera's exposure (default is f/16, 1/125s, 100 ISO) The exposure ultimately controls the scene's brightness, just like with a real camera. |
| [setEyeModelMatrix](set-eye-model-matrix.md) | [main]<br>open fun [setEyeModelMatrix](set-eye-model-matrix.md)(eyeId: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), model: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;)<br>Sets the model matrix for a specific eye. |
| [setFocusDistance](set-focus-distance.md) | [main]<br>open fun [setFocusDistance](set-focus-distance.md)(distance: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Set the camera focus distance in world units |
| [setLensProjection](set-lens-projection.md) | [main]<br>open fun [setLensProjection](set-lens-projection.md)(focalLength: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), aspect: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), near: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), far: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html))<br>Sets the projection matrix from the focal length. |
| [setModelMatrix](set-model-matrix.md) | [main]<br>open fun [setModelMatrix](set-model-matrix.md)(modelMatrix: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;)<br>open fun [setModelMatrix](set-model-matrix.md)(modelMatrix: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;)<br>Sets the camera's model matrix. |
| [setProjection](set-projection.md) | [main]<br>open fun [setProjection](set-projection.md)(fovInDegrees: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), aspect: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), near: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), far: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), direction: [Camera.Fov](-fov/index.md))<br>Sets the projection matrix from the field-of-view.<br>[main]<br>open fun [setProjection](set-projection.md)(projection: [Camera.Projection](-projection/index.md), left: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), right: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), bottom: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), top: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), near: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), far: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html))<br>Sets the projection matrix from a frustum defined by six planes. |
| [setScaling](set-scaling.md) | [main]<br>open fun [~~setScaling~~](set-scaling.md)(inScaling: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;)<br>open fun [setScaling](set-scaling.md)(xscaling: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), yscaling: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html))<br>Sets an additional matrix that scales the projection matrix. |
| [setShift](set-shift.md) | [main]<br>open fun [setShift](set-shift.md)(xshift: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), yshift: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html))<br>Sets an additional matrix that shifts (translates) the projection matrix. |
