//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[IndexBuffer](index.md)/[isCreationComplete](is-creation-complete.md)

# isCreationComplete

[main]\
open fun [isCreationComplete](is-creation-complete.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

This non-blocking method checks if the resource has finished creation *successfully*. 

If the resource creation was initiated asynchronously, it will return true only after all related asynchronous tasks are complete, and only if none of them was canceled. If the resource was created normally without using async method, it will always return true.

A canceled asynchronous creation never populates the resource, so this method keeps returning false for it. The object itself remains valid and must still be destroyed as usual.

#### Return

Whether the resource is created and usable.

#### See also

| |
|---|
| com.google.android.filament.IndexBuffer.Builder |
