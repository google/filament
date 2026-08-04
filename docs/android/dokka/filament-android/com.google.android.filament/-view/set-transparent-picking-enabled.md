//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setTransparentPickingEnabled](set-transparent-picking-enabled.md)

# setTransparentPickingEnabled

[main]\
open fun [setTransparentPickingEnabled](set-transparent-picking-enabled.md)(enabled: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html))

Enables or disables transparent picking. Disabled by default. When transparent picking is enabled, View::pick() will pick from both transparent and opaque renderables. When disabled, View::pick() will only pick from opaque renderables. 

 Transparent picking will create an extra pass for rendering depth from both transparent and opaque renderables. 

#### Parameters

main

| | |
|---|---|
| enabled | true enables transparent picking, false disables it. |
