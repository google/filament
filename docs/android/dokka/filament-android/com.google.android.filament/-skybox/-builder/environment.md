//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Skybox](../index.md)/[Builder](index.md)/[environment](environment.md)

# environment

[main]\
open fun [environment](environment.md)(cubemap: [Texture](../../-texture/index.md)): [Skybox.Builder](index.md)

Set the environment map (i.e. the skybox content). 

The `Skybox` is rendered as though it were an infinitely large cube with the camera inside it. This means that the cubemap which is mapped onto the cube's exterior will appear mirrored. This follows the OpenGL conventions.

The `cmgen` tool generates reflection maps by default which are therefore ideal to use as skyboxes.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| cubemap | A cubemap [Texture](../../-texture/index.md) |

#### See also

| |
|---|
| [Texture](../../-texture/index.md) |
