# Trace-Viewer Coordinate Systems.

## Coordinate Systems

To represent browser content in trace-viewer we need to draw boxes and
pictures created in one browser in the DOM of another browser window.
How does a pixel in the output relate to a pixel in the original browser view?

### Scaling

The snapshot view lives in a quad-stack-viewer DOM element. This is area of
pixels in trace-viewer, for example 685x342 px.

The quad-stack-viewer contains a view-container with a CSS transform. The
transform will zoom (CSS scale), pan (CSS translateX, translateY),
orient (CSS rotateX, rotateY) its contents, a canvas.  Common scale factors
will be 0.1 - 2.0. The transformation is controlled by user inputs.

Internally the canvas has the _world_ coordinates.

The _world_ coordinates completely enclose the boxes we may draw, plus some
padding so the edges of boxes do not sit against the edge of the world. For
example, padding space of .75 times the minimum of width and height may be
added. Since the original browser has a few thousand pixels, the padded world
may be 5-6000 pixels on a side.

The _world_ coordinates are scaled by several factors:
 * _quad_stack_scale_ adjusts the size of the canvas (eg 0.5).
 * _devicePixelRatio_ adjusts for high-res devices (eg 1 or 2),
 * _ui.RASTER_SCALE_, adjusts the size of the canvas. (eg 0.75)

*Do we still need RASTER_SCALE?*

### Translation (origins)

The quad-stack-viewer DOM element is positioned by CSS at some offset in the
document. All of our origins are relative to the top left corner of the
quad-stack-viewer.

The CSS transforms move us from the DOM coordinate system to the world system.
*What is the origin of the canvas in the DOM coordinate system
when the final size of the canvas is less than the element?*

The _deviceViewportRect_ is the visible browser window in _world_ coordinates.
Typically it will be at X,Y = 0,0. Thus the _world_ origin will be eg
-0.75\*3000px , -0.75\*2500px, due to the world padding.
