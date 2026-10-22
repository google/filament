# iOS `hello-ar` Sample

The `hello-ar` sample shows how to use ARKit and Filament to render an AR experience. Tapping the
screen positions an object in front of the camera.

The app sets up an `ARSession` and implements the `session:didUpdateFrame:` method to receive
frames. It uploads each frame to a Filament texture using Filament's `Texture::setExternalImage`
method. The camera feed is rendered on a full-screen, unlit triangle. See camera_feed.mat for the
material used to render the feed, and FullScreenTriangle.cpp for the camera triangle setup.

View and projection matrices received from ARKit are sent to Filament with the helpers inside of
`MatrixHelpers.h` handling the matrix conversions.

The app is locked to `UIInterfaceOrientationLandscapeRight` (landscape, home button on the
right-hand side). This is done for simplicity as the camera transformation matrix, stored in
`frame.camera.transform`, assumes this orientation.

## iOS External Images

On iOS, external images are `CVPixelBufferRef`s. ARKit provides the app with a single
`CVPixelBufferRef` each frame, held inside `frame.capturedImage`. After calling `setExternalImage`,
Filament takes ownership of the image, releasing it only when it can guarantee all frames using the
image have finished rendering on the GPU. This means that upon calling `setExternalImage`, an image
will not be released until (at the earliest) another call to `setExternalImage`. This allows
external images to persist for multiple frames.

To explicitly release any external image that Filament has retained, either destroy the texture or
call `setExternalImage(nullptr)`. Filament ensures that the image is not released until all GPU work
using it has finished.

