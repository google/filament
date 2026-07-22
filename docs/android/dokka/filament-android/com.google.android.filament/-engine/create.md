//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[create](create.md)

# create

[main]\
open fun [create](create.md)(): [Engine](index.md)

Creates an instance of Engine using the default [Backend](-backend/index.md)

 This method is one of the few thread-safe methods.

#### Return

A newly created `Engine`, or `null` if the GPU driver couldn't be initialized, for instance if it doesn't support the right version of OpenGL or OpenGL ES.

#### Throws

| | |
|---|---|
| [IllegalStateException](https://developer.android.com/reference/kotlin/java/lang/IllegalStateException.html) | can be thrown if there isn't enough memory to allocate the command buffer. |

[main]\
open fun [create](create.md)(backend: [Engine.Backend](-backend/index.md)): [Engine](index.md)

Creates an instance of Engine using the specified [Backend](-backend/index.md)

 This method is one of the few thread-safe methods.

#### Return

A newly created `Engine`, or `null` if the GPU driver couldn't be initialized, for instance if it doesn't support the right version of OpenGL or OpenGL ES.

#### Parameters

main

| | |
|---|---|
| backend | driver backend to use |

#### Throws

| | |
|---|---|
| [IllegalStateException](https://developer.android.com/reference/kotlin/java/lang/IllegalStateException.html) | can be thrown if there isn't enough memory to allocate the command buffer. |

[main]\
open fun [create](create.md)(sharedContext: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html)): [Engine](index.md)

Creates an instance of Engine using the [OPENGL](-backend/-o-p-e-n-g-l/index.md) and a shared OpenGL context. 

 This method is one of the few thread-safe methods.

#### Return

A newly created `Engine`, or `null` if the GPU driver couldn't be initialized, for instance if it doesn't support the right version of OpenGL or OpenGL ES.

#### Parameters

main

| | |
|---|---|
| sharedContext | A platform-dependant OpenGL context used as a shared context when creating filament's internal context. On Android this parameter **must be** an instance of android.opengl.EGLContext. |

#### Throws

| | |
|---|---|
| [IllegalStateException](https://developer.android.com/reference/kotlin/java/lang/IllegalStateException.html) | can be thrown if there isn't enough memory to allocate the command buffer. |
