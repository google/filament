//[filament-android](../../../../index.md)/[com.google.android.filament.android](../../index.md)/[ChoreographerHelper](../index.md)/[Callback](index.md)

# Callback

[main]\
interface [Callback](index.md)

Callback interface for receiving frame synchronization events in Composition Mode.

## Functions

| Name | Summary |
|---|---|
| [onFrame](on-frame.md) | [main]<br>abstract fun [onFrame](on-frame.md)(frameTimeNanos: Long)<br>Called when a new frame should be rendered.<br>[main]<br>open fun [onFrame](on-frame.md)(frameTimeNanos: Long, frameData: Any)<br>Called when a new frame should be rendered, providing optional payload telemetry. |
