//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Backend](index.md)

# Backend

[main]\
enum [Backend](index.md)

Selects which driver a particular Engine should use.

## Entries

| | |
|---|---|
| [DEFAULT](-d-e-f-a-u-l-t/index.md) | [main]<br>[DEFAULT](-d-e-f-a-u-l-t/index.md)<br>Automatically selects an appropriate driver for the platform. |
| [OPENGL](-o-p-e-n-g-l/index.md) | [main]<br>[OPENGL](-o-p-e-n-g-l/index.md)<br>Selects the OpenGL/ES driver (default on Android) |
| [VULKAN](-v-u-l-k-a-n/index.md) | [main]<br>[VULKAN](-v-u-l-k-a-n/index.md)<br>Selects the Vulkan driver if the platform supports it (default on Linux/Windows) |
| [METAL](-m-e-t-a-l/index.md) | [main]<br>[METAL](-m-e-t-a-l/index.md)<br>Selects the Metal driver if the platform supports it (default on MacOS/iOS). |
| [WEBGPU](-w-e-b-g-p-u/index.md) | [main]<br>[WEBGPU](-w-e-b-g-p-u/index.md)<br>Selects the Webgpu driver if the platform supports webgpu. |
| [NOOP](-n-o-o-p/index.md) | [main]<br>[NOOP](-n-o-o-p/index.md)<br>Selects the no-op driver for testing purposes. |

## Functions

| Name | Summary |
|---|---|
| [toFilamentNative](to-filament-native.md) | [main]<br>open fun [toFilamentNative](to-filament-native.md)(): Int |
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Engine.Backend](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): Array&lt;[Engine.Backend](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
