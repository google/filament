//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[setCustomEyeProjection](set-custom-eye-projection.md)

# setCustomEyeProjection

[main]\
open fun [setCustomEyeProjection](set-custom-eye-projection.md)(projection: Array&lt;Double&gt;, projectionForCulling: Array&lt;Double&gt;, near: Double, far: Double)

Sets a custom projection matrix for each eye. 

The projectionForCulling, near, and far parameters establish a &quot;culling frustum&quot; which must encompass anything any eye can see. All projection matrices must be set simultaneously. The number of stereoscopic eyes is controlled by the stereoscopicEyeCount setting inside of Engine::Config.

#### Parameters

main

| | |
|---|---|
| projection | an array of projection matrices, only the first config.stereoscopicEyeCount are read |
| projectionForCulling | custom projection matrix for culling, must encompass both eyes |
| near | distance in world units from the camera to the culling near plane. `near`>0. |
| far | distance in world units from the camera to the culling far plane. `far`>`near`. |

#### See also

| |
|---|
| setCustomProjection |
| [Engine.Config](../-engine/-config/stereoscopic-eye-count.md) |
