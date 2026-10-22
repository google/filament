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
| [getIntensity](get-intensity.md) | [main]<br>open fun [getIntensity](get-intensity.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Returns the skybox's intensity in lux, or lumen/m^2. |
| [getLayerMask](get-layer-mask.md) | [main]<br>open fun [getLayerMask](get-layer-mask.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getTexture](get-texture.md) | [main]<br>open fun [getTexture](get-texture.md)(): [Texture](../-texture/index.md) |
| [setColor](set-color.md) | [main]<br>open fun [setColor](set-color.md)(color: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;)<br>open fun [setColor](set-color.md)(colorx: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), colory: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), colorz: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), colorw: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)) |
| [setLayerMask](set-layer-mask.md) | [main]<br>open fun [setLayerMask](set-layer-mask.md)(select: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), values: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Sets bits in a visibility mask. |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [Skybox](index.md) |
