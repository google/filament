//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Config](index.md)/[stereoscopicType](stereoscopic-type.md)

# stereoscopicType

[main]\
open var [stereoscopicType](stereoscopic-type.md): [Engine.StereoscopicType](../-stereoscopic-type/index.md)

The type of technique for stereoscopic rendering. This setting determines the algorithm used when stereoscopic rendering is enabled. This decision applies to the entire Engine for the lifetime of the Engine. E.g., multiple Views created from the Engine must use the same stereoscopic type. Each view can enable stereoscopic rendering via the StereoscopicOptions::enable flag.

#### See also

| |
|---|
| [View](../../-view/set-stereoscopic-options.md) |
