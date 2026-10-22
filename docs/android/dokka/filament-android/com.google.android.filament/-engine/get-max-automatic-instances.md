//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[getMaxAutomaticInstances](get-max-automatic-instances.md)

# getMaxAutomaticInstances

[main]\
open fun [getMaxAutomaticInstances](get-max-automatic-instances.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)

Queries the maximum number of GPU instances that Filament creates when automatic instancing is enabled. 

This value is also the limit for the number of transforms that can be stored in an InstanceBuffer. This value may depend on the device and platform, but will remain constant during the lifetime of this Engine.

This value does not apply when using the instances(size_t) method on RenderableManager::Builder.

#### Return

the number of max automatic instances

#### See also

| |
|---|
| [setAutomaticInstancingEnabled](set-automatic-instancing-enabled.md) |
| [RenderableManager.Builder](../-renderable-manager/-builder/instances.md) |
