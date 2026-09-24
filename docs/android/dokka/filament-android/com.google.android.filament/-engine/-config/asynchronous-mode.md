//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Config](index.md)/[asynchronousMode](asynchronous-mode.md)

# asynchronousMode

[main]\
open var [asynchronousMode](asynchronous-mode.md): [Engine.AsynchronousMode](../-asynchronous-mode/index.md)

Asynchronous mode for the engine. 

Defines how asynchronous operations are handled. Note that selecting a non-NONE mode does not guarantee asynchronous methods are supported, as the underlying backend or the feature flag may override this configuration. Always validate availability via Engine::isAsynchronousModeEnabled() before invoking asynchronous methods.
