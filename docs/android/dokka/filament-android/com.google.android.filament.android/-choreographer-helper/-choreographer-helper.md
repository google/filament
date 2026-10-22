//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[ChoreographerHelper](index.md)/[ChoreographerHelper](-choreographer-helper.md)

# ChoreographerHelper

[main]\
constructor()

Initializes a new `ChoreographerHelper` in Inheritance Mode. 

Subclasses must override [onFrame](on-frame.md) or [onFrame](on-frame.md) to handle frame rendering.

[main]\
constructor(callback: [ChoreographerHelper.Callback](-callback/index.md))

Initializes a new `ChoreographerHelper` in Composition Mode with a client callback.

#### Parameters

main

| | |
|---|---|
| callback | A non-null [Callback](-callback/index.md) instance to invoke on every frame. |
