//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[getMaterialTime](get-material-time.md)

# getMaterialTime

[main]\
open fun [getMaterialTime](get-material-time.md)(): [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)

Returns the material time in seconds evaluated for the current frame. 

This value is constant for all views rendered during a frame. When available, this time is projected forward to the predicted presentation time on the display; otherwise, it evaluates at the vsync time of beginFrame(). The epoch is set with setMaterialTimeEpoch().

In materials, this value can be queried using `vec4 getUserTime()`. The value returned is a highp vec4 encoded as follows:

time.x = (float)Renderer.getMaterialTime(); time.y = Renderer.getMaterialTime() - time.x;

It follows that the following invariants are true:

(double)time.x + (double)time.y == Renderer.getMaterialTime() time.x == (float)Renderer.getMaterialTime()

This &quot;float-float&quot; encoding allows the shader code to perform high precision (i.e. double) time calculations when needed despite the lack of double precision in the shader (e.g. using Dekker's algorithms). For example, to compute (double)time * vertex in the material, use the following construct:

vec3 result = time.x  *vertex + time.y*  vertex;

Most of the time, high precision computations are not required, but be aware that the precision of time.x rapidly diminishes as time passes:

time | precision --------+----------

1. 7s | us 4h39 | ms 77h | 1/60s In other words, it is only possible to get microsecond accuracy for about 16s or millisecond accuracy for just under 5h. This problem can be mitigated by calling setMaterialTimeEpoch(), or using high precision time as described above.

#### Return

The time in seconds since setMaterialTimeEpoch() was last called.

#### See also

| |
|---|
| [setMaterialTimeEpoch](set-material-time-epoch.md) |
