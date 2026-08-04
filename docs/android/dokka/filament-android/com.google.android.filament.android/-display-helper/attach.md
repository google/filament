//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[DisplayHelper](index.md)/[attach](attach.md)

# attach

[main]\
open fun [attach](attach.md)(renderer: [Renderer](../../com.google.android.filament/-renderer/index.md), display: Display)

Sets the filament [Renderer](../../com.google.android.filament/-renderer/index.md) associated to the Display, from this point on, [Renderer.DisplayInfo](../../com.google.android.filament/-renderer/-display-info/index.md) will be automatically updated when the Display properties change. This is typically called from [onNativeWindowChanged](../-ui-helper/-renderer-callback/on-native-window-changed.md).

#### Parameters

main

| | |
|---|---|
| renderer | a filament [Renderer](../../com.google.android.filament/-renderer/index.md) instance |
| display | a Display to be associated with the [Renderer](../../com.google.android.filament/-renderer/index.md) |
