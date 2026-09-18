//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setColorGrading](set-color-grading.md)

# setColorGrading

[main]\
open fun [setColorGrading](set-color-grading.md)(colorGrading: [ColorGrading](../-color-grading/index.md))

Sets this View's color grading transforms. 

There is no reference-counting. Make sure to dissociate a ColorGrading from all Views before destroying it.

#### Parameters

main

| | |
|---|---|
| colorGrading | Associate the specified ColorGrading to this View. A ColorGrading can be associated to several View instances. `colorGrading` can be nullptr to dissociate the currently set ColorGrading from this View. Doing so will revert to the use of the default color grading transforms. The View doesn't take ownership of the ColorGrading pointer (which acts as a reference). |
