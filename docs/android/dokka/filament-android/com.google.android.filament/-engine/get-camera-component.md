//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[getCameraComponent](get-camera-component.md)

# getCameraComponent

[main]\
open fun [getCameraComponent](get-camera-component.md)(entity: Int): [Camera](../-camera/index.md)

Returns the Camera component of the given entity.

#### Return

A pointer to the Camera component for this entity or nullptr if the entity didn't have a Camera component. The pointer is valid until destroyCameraComponent() is called or the entity itself is destroyed.

#### Parameters

main

| | |
|---|---|
| entity | An entity. |
