//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[castShadows](cast-shadows.md)

# castShadows

[main]\
open fun [castShadows](cast-shadows.md)(enabled: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [RenderableManager.Builder](index.md)

Controls if this renderable casts shadows, false by default. If the View's shadow type is set to [VSM](../../-view/-shadow-type/-v-s-m/index.md), castShadows should only be disabled if either is true: 

- 
   [setReceiveShadows](../set-receive-shadows.md) is also disabled
- the object is guaranteed to not cast shadows on itself or other objects (for example, a ground plane)
