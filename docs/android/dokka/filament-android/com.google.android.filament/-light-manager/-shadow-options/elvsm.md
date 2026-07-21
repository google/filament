//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowOptions](index.md)/[elvsm](elvsm.md)

# elvsm

[main]\
open var [elvsm](elvsm.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

When elvsm is set to true, &quot;Exponential Layered VSM without Layers&quot; are used. It is an improvement to the default EVSM which suffers important light leaks. Enabling ELVSM for a single shadowmap doubles the memory usage of all shadow maps. ELVSM is mostly useful when large blurs are used.
