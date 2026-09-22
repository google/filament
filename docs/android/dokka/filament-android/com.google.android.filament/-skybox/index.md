//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Skybox](index.md)

# Skybox

open class [Skybox](index.md)

Skybox 

When added to a Scene, the Skybox fills all untouched pixels.

# Creation and destruction

A Skybox object is created using the Skybox::Builder and destroyed by calling Engine::destroy(const Skybox*).

```kotlin

 filament::Engine* engine = filament::Engine::create();

 filament::IndirectLight* skybox = filament::Skybox::Builder()
             .environment(cubemap)
             .build(*engine);

 engine->destroy(skybox);

```

Currently only Texture based sky boxes are supported.

#### See also

| |
|---|
| [Scene](../-scene/index.md) |
| [IndirectLight](../-indirect-light/index.md) |

## Types

| Name | Summary |
|---|---|
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md)<br>Use Builder to construct an Skybox object instance |

## Functions

| Name | Summary |
|---|---|
| [getIntensity](get-intensity.md) | [main]<br>open fun [getIntensity](get-intensity.md)(): Float<br>Returns the skybox's intensity in lux, or lumen/m^2. |
| [getLayerMask](get-layer-mask.md) | [main]<br>open fun [getLayerMask](get-layer-mask.md)(): Int |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): Long |
| [getTexture](get-texture.md) | [main]<br>open fun [getTexture](get-texture.md)(): [Texture](../-texture/index.md) |
| [setColor](set-color.md) | [main]<br>open fun [setColor](set-color.md)(color: Array&lt;Float&gt;)<br>open fun [setColor](set-color.md)(colorx: Float, colory: Float, colorz: Float, colorw: Float) |
| [setLayerMask](set-layer-mask.md) | [main]<br>open fun [setLayerMask](set-layer-mask.md)(select: Int, values: Int)<br>Sets bits in a visibility mask. |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: Long): [Skybox](index.md) |
