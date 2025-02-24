Reference tests (reftests) for WebGPU canvas presentation.

These render some contents to a canvas using WebGPU, and WPT compares the rendering result with
the "reference" versions (in `ref/`) which render with 2D canvas.

This tests things like:
- The canvas has the correct orientation.
- The canvas renders with the correct transfer function.
- The canvas blends and interpolates in the correct color encoding.

TODO(#918): Test all possible color spaces (once we have more than 1)
TODO(#921): Why is there sometimes a difference of 1 (e.g. 3f vs 40) in canvas_size_different_with_back_buffer_size?
And why does chromium's image_diff show diffs on other pixels that don't seem to have diffs?
TODO(#1093): Test rgba16float values which are out of gamut of the canvas but under SDR luminance.
TODO(#1093): Test rgba16float values which are above SDR luminance.
TODO(#1116): Test canvas scaling.
TODO: Test transferControlToOffscreen, used from {the same,another} thread
