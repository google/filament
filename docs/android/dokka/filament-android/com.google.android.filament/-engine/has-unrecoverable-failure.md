//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[hasUnrecoverableFailure](has-unrecoverable-failure.md)

# hasUnrecoverableFailure

[main]\
open fun [hasUnrecoverableFailure](has-unrecoverable-failure.md)(): Boolean

Returns whether the engine has encountered an unrecoverable failure. 

If this returns true, the engine is in an unrecoverable state and further calls to rendering methods will fail or be ignored. Apps can use this to check for fatal errors instead of relying on exceptions.

#### Return

true if an unrecoverable failure has occurred, false otherwise.
