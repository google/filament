//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[clearFrameHistory](clear-frame-history.md)

# clearFrameHistory

[main]\
open fun [clearFrameHistory](clear-frame-history.md)(engine: [Engine](../-engine/index.md))

When certain temporal features are used (e.g.: TAA or Screen-space reflections), the view keeps a history of previous frame renders associated with the Renderer the view was last used with. When switching Renderer, it may be necessary to clear that history by calling this method. Similarly, if the whole content of the screen change, like when a cut-scene starts, clearing the history might be needed to avoid artifacts due to the previous frame being very different.
