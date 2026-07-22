//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Skybox](index.md)

# Skybox

open class [Skybox](index.md)

Skybox 

When added to a [Scene](../-scene/index.md), the `Skybox` fills all untouched pixels.

# Creation and destruction

 A `Skybox` object is created using the [Skybox.Builder](-builder/index.md) and destroyed by calling [destroySkybox](../-engine/destroy-skybox.md).```kotlin
 Engine engine = Engine.create();

 Scene scene = engine.createScene();

 Skybox skybox = new Skybox.Builder()
             .environment(cubemap)
             .build(engine);

 scene.setSkybox(skybox);

```
 Currently only [Texture](../-texture/index.md) based sky boxes are supported.

#### See also

| |
|---|
| [Scene](../-scene/index.md) |
| [IndirectLight](../-indirect-light/index.md) |

## Constructors

| | |
|---|---|
| [Skybox](-skybox.md) | [main]<br>constructor(nativeSkybox: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |

## Types

| Name | Summary |
|---|---|
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md)<br>Use `Builder` to construct a `Skybox` object instance. |

## Functions

| Name | Summary |
|---|---|
| [getIntensity](get-intensity.md) | [main]<br>open fun [getIntensity](get-intensity.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Returns the `Skybox`'s intensity in *lux*, or *lumen/m^2*. |
| [getLayerMask](get-layer-mask.md) | [main]<br>open fun [getLayerMask](get-layer-mask.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getTexture](get-texture.md) | [main]<br>open fun [getTexture](get-texture.md)(): [Texture](../-texture/index.md) |
| [setColor](set-color.md) | [main]<br>open fun [setColor](set-color.md)(color: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;)<br>open fun [setColor](set-color.md)(r: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), g: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), b: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), a: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Mutates the `Skybox`'s constant color. |
| [setLayerMask](set-layer-mask.md) | [main]<br>open fun [setLayerMask](set-layer-mask.md)(select: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), values: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Sets bits in a visibility mask. |
