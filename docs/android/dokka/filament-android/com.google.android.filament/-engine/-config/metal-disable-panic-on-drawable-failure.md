//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Config](index.md)/[metalDisablePanicOnDrawableFailure](metal-disable-panic-on-drawable-failure.md)

# metalDisablePanicOnDrawableFailure

[main]\
open var [metalDisablePanicOnDrawableFailure](metal-disable-panic-on-drawable-failure.md): Boolean

The action to take if a Drawable cannot be acquired. 

Each frame rendered requires a CAMetalDrawable texture, which is presented on-screen at the completion of each frame. These are limited and provided round-robin style by the system.
