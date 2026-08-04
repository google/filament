//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setShadowType](set-shadow-type.md)

# setShadowType

[main]\
open fun [setShadowType](set-shadow-type.md)(type: [View.ShadowType](-shadow-type/index.md))

Sets the shadow mapping technique this View uses. The ShadowType affects all the shadows seen within the View. 

[VSM](-shadow-type/-v-s-m/index.md) imposes a restriction on marking renderables as only shadow receivers (but not casters). To ensure correct shadowing with VSM, all shadow participant renderables should be marked as both receivers and casters. Objects that are guaranteed to not cast shadows on themselves or other objects (such as flat ground planes) can be set to not cast shadows, which might improve shadow quality. 

Warning: This API is still experimental and subject to change.
