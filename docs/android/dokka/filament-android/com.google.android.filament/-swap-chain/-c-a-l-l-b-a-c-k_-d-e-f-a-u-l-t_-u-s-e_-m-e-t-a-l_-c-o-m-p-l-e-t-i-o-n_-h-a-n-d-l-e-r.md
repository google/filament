//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SwapChain](index.md)/[CALLBACK_DEFAULT_USE_METAL_COMPLETION_HANDLER](-c-a-l-l-b-a-c-k_-d-e-f-a-u-l-t_-u-s-e_-m-e-t-a-l_-c-o-m-p-l-e-t-i-o-n_-h-a-n-d-l-e-r.md)

# CALLBACK_DEFAULT_USE_METAL_COMPLETION_HANDLER

[main]\
val [CALLBACK_DEFAULT_USE_METAL_COMPLETION_HANDLER](-c-a-l-l-b-a-c-k_-d-e-f-a-u-l-t_-u-s-e_-m-e-t-a-l_-c-o-m-p-l-e-t-i-o-n_-h-a-n-d-l-e-r.md): Long = 1

If this flag is passed to setFrameScheduledCallback, then the behavior of the default CallbackHandler (when nullptr is passed as the handler argument) is altered to call the callback on the Metal completion handler thread (as opposed to the main Filament thread). 

This flag also instructs the Metal backend to release the associated CAMetalDrawable on the completion handler thread.

This flag has no effect if a custom CallbackHandler is passed or on backends other than Metal.

#### See also

| |
|---|
| setFrameScheduledCallback |
