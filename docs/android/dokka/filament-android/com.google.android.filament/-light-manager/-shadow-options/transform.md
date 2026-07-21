//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowOptions](index.md)/[transform](transform.md)

# transform

[main]\
open var [transform](transform.md): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Transforms the shadow direction. Must be a unit quaternion. The default is identity. Ignored if the light type isn't directional. For artistic use. Use with caution. The quaternion is stored as the imaginary part in the first 3 elements and the real part in the last element of the transform array.
