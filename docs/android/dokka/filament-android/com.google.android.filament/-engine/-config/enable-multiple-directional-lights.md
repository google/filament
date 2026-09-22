//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Config](index.md)/[enableMultipleDirectionalLights](enable-multiple-directional-lights.md)

# enableMultipleDirectionalLights

[main]\
open var [enableMultipleDirectionalLights](enable-multiple-directional-lights.md): Boolean

Whether a scene can contain more than one directional light. 

By default, and historically, only the dominant directional light (the one with the highest intensity) of a scene is evaluated. When this is enabled, up to four additional directional lights contribute lighting; they don't cast shadows and don't draw a sun's disk. Scenes with a single directional light are unaffected either way.

#### See also

| |
|---|
| [LightManager](../../-light-manager/index.md) |
