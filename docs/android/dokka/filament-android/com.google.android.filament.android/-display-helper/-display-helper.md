//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[DisplayHelper](index.md)/[DisplayHelper](-display-helper.md)

# DisplayHelper

[main]\
constructor(context: Context)

Creates a DisplayHelper which helps managing a Display. The Display to manage is specified with [attach](attach.md)

#### Parameters

main

| | |
|---|---|
| context | a Context to used to retrieve the DisplayManager |

[main]\
constructor(context: Context, handler: Handler)

Creates a DisplayHelper which helps manage a Display and provides a Handler where callbacks can execute filament code. Use this method if filament is executing on another thread.

#### Parameters

main

| | |
|---|---|
| context | a Context to used to retrieve teh DisplayManager |
| handler | a Handler used to run callbacks accessing filament |
