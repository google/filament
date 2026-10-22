//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[ChoreographerHelper](index.md)

# ChoreographerHelper

[main]\
open class [ChoreographerHelper](index.md)

`ChoreographerHelper` is a utility class that encapsulates Android's Choreographer callbacks to properly schedule rendering loops and feed desired presentation timestamps and deadlines to Filament's [Renderer](../../com.google.android.filament/-renderer/index.md). 

It automatically utilizes Android 13+ (API 33) `VsyncCallback` and `FrameTimeline` APIs when available, gracefully falling back to traditional `FrameCallback` APIs on older Android versions. 

This class can be used in two distinct architectural patterns: **Inheritance Mode** and **Composition Mode**. 

### Usage Example 1: Inheritance Mode (Recommended for Custom Handlers)

```kotlin

public class MyFrameHandler extends ChoreographerHelper {
    
    public void onFrame(long frameTimeNanos, @Nullable Object frameData) {
        // Optional: Configure Filament FrameData or Presentation Timestamps
        if (Build.VERSION.SDK_INT >= 33 && frameData instanceof Choreographer.FrameData) {
            modelViewer.render((Choreographer.FrameData) frameData);
        } else {
            modelViewer.render(frameTimeNanos);
        }
    }
}

MyFrameHandler handler = new MyFrameHandler();
handler.post();

```

### Usage Example 2: Composition Mode (Recommended for Anonymous Callbacks)

```kotlin

ChoreographerHelper helper = new ChoreographerHelper(new ChoreographerHelper.Callback() {
    
    public void onFrame(long frameTimeNanos) {
        // Render frame...
    }
});

// Optional: Let the helper automatically apply frame timeline targets to your Renderer
helper.setRenderer(myRenderer);
helper.post();

```

## Constructors

| | |
|---|---|
| [ChoreographerHelper](-choreographer-helper.md) | [main]<br>constructor()<br>Initializes a new `ChoreographerHelper` in Inheritance Mode.<br>constructor(callback: [ChoreographerHelper.Callback](-callback/index.md))<br>Initializes a new `ChoreographerHelper` in Composition Mode with a client callback. |

## Types

| Name | Summary |
|---|---|
| [Callback](-callback/index.md) | [main]<br>interface [Callback](-callback/index.md)<br>Callback interface for receiving frame synchronization events in Composition Mode. |

## Functions

| Name | Summary |
|---|---|
| [onFrame](on-frame.md) | [main]<br>open fun [onFrame](on-frame.md)(frameTimeNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))<br>Base callback invoked when a new frame should be rendered.<br>[main]<br>open fun [onFrame](on-frame.md)(frameTimeNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), frameData: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html))<br>Main callback invoked when a new frame should be rendered, providing optional payload telemetry. |
| [post](post.md) | [main]<br>open fun [post](post.md)()<br>Posts a callback to Android's Choreographer to schedule the next frame. |
| [remove](remove.md) | [main]<br>open fun [remove](remove.md)()<br>Cancels any pending frame synchronization callbacks, stopping the scheduling loop. |
| [setRenderer](set-renderer.md) | [main]<br>open fun [setRenderer](set-renderer.md)(renderer: [Renderer](../../com.google.android.filament/-renderer/index.md))<br>Attaches an optional Filament [Renderer](../../com.google.android.filament/-renderer/index.md) to be automatically paced by this helper. |
