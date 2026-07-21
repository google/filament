//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setStereoscopicOptions](set-stereoscopic-options.md)

# setStereoscopicOptions

[main]\
open fun [setStereoscopicOptions](set-stereoscopic-options.md)(options: [View.StereoscopicOptions](-stereoscopic-options/index.md))

Sets the stereoscopic rendering options for this view. 

 Currently, only one type of stereoscopic rendering is supported: side-by-side. Side-by-side stereo rendering splits the viewport into two halves: a left and right half. Eye 0 will render to the left half, while Eye 1 will render into the right half. 

 Currently, the following features are not supported with stereoscopic rendering: - post-processing - shadowing - punctual lights 

 Stereo rendering depends on device and platform support. To check if stereo rendering is supported, use isStereoSupported. If stereo rendering is not supported, then the stereoscopic options have no effect. 

#### Parameters

main

| | |
|---|---|
| options | The stereoscopic options to use on this view |

#### See also

| |
|---|
| [getStereoscopicOptions](get-stereoscopic-options.md) |
