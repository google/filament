//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[destroy](destroy.md)

# destroy

[main]\
open fun [destroy](destroy.md)()

Destroy the `Engine` instance and all associated resources. 

 This method is one of the few thread-safe methods. 

[destroy](destroy.md) should be called last and after all other resources have been destroyed, it ensures all filament resources are freed. 

`Destroy` performs the following tasks: 

Destroy all internal software and hardware resources.Free all user allocated resources that are not already destroyed and logs a warning. 

This indicates a &quot;leak&quot; in the user's code.

Terminate the rendering engine's thread.```kotlin
Engine engine = Engine.create();
engine.destroy();

```
