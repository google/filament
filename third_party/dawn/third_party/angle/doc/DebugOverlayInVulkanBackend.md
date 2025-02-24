# Debug Overlay in ANGLE's Vulkan Backend

## Motivation

A complex application has frequently changing performance characteristics due to
both a varying number of objects to draw and different effects that need to be
applied to them. When characterizing the performance of an application, it can
be easy to miss scenes which need optimization, especially if they are
ephemeral.

A debug overlay that shows on-the-fly statistics from the running application
can greatly aid the developer in finding where the bottlenecks are and which
scenes need further investigation and profiling.

ANGLE's Vulkan debug overlay implements this. The initial implementation
includes a few pieces of information for demonstration purposes. Here's the
glmark2 *terrain* scene with these overlay items enabled:

![glmark2 terrain scene](img/VangleDebugOverlay.png)

This is a screenshot of a debug build, hence the low FPS. The command graph size
widget no longer applies to current ANGLE code.

## Implementation

Overlay items are of two fundamental types:

* Text items: A single line of text with small or large font.
* Graph items: A bar graph of data. These each have a Text item attached
  that is automatically rendered with the graph item.

Built on these, various overlay item types are defined that gather statistics.
Five such types are defined with one item per type as example:

* **Count**: An item that counts something. **VulkanValidationMessageCount**
  is an overlay item of this type that shows the number of validation messages
  received from the validation layers.
* **Text**: A generic text widget. **VulkanLastValidationMessage** is an overlay
  item of this type that shows the last validation message.
* **PerSecond**: A value that gets reset every second automatically. **FPS** is
  an overlay item of this type that simply gets incremented on every `swap()`.
* **RunningGraph**: A graph of the last N values. **VulkanRenderPassCount** is an
  overlay of this type. This counter reports the number of RenderPasses rendered
  in each vkQueueSubmit call.
* **RunningHistogram**: A histogram of last N values. Input values are in the
  [0, 1] range and they are ranked to N buckets for histogram calculation.
  **VulkanSecondaryCommandBufferPoolWaste** is an overlay item of this type.
  On `vkQueueSubmit()`, the memory waste from command buffer pool allocations
  is recorded in the histogram.

Overlay font is placed in [libANGLE/overlay/](../src/libANGLE/overlay/) which
[gen_overlay_fonts.py](../src/libANGLE/gen_overlay_fonts.py) processes to create
an array of rasterized font data, which is used at runtime to create the font
image (an image with one layer per character, and one mip level per font size).

The overlay widget layout is defined in
[overlay_widgets.json](../src/libANGLE/overlay_widgets.json)
which [gen_overlay_widgets.py](../src/libANGLE/gen_overlay_widgets.py)
processes to generate an array of widgets, each of its respective type,
and sets their properties, such as color and bounding box.
The json file allows widgets to align against other widgets as well as against
the framebuffer edges. The following is a part of this file:

```json
{
    "name": "VulkanValidationMessageCount",
    "type": "Count",
    "color": [255, 0, 0, 255],
    "coords": [10, "VulkanLastValidationMessage.top.adjacent"],
    "font": "small",
    "length": 25
},
{
    "name": "VulkanSecondaryCommandBufferPoolWaste",
    "type": "RunningHistogram(50)",
    "color": [255, 200, 75, 200],
    "coords": [-50, 100],
    "bar_width": 6,
    "height": 100,
    "description": {
        "color": [255, 200, 75, 255],
        "coords": ["VulkanSecondaryCommandBufferPoolWaste.left.align",
                   "VulkanSecondaryCommandBufferPoolWaste.top.adjacent"],
        "font": "small",
        "length": 40
    }
}
```

Negative coordinates in this file indicate alignment to the right/bottom of the
framebuffer. `OtherItem.edge.mode` lets an item be aligned with another.
If `mode` is `align`, the item has the same origin as `OtherItem` and expands
in the same direction. If `adjacent`, the item expands in the opposite
direction.

The UI is rendered in two passes, one draw call for all graph widgets and
another draw call for all text widgets. The vertex shader in these draw calls
generates 4 vertices for each instance (one instance per widget) based on the
widget bounding box. The fragment shader renders font or a graph based on widget
data. This is done once per frame on `present()`, and the result is blended into
the swapchain image.

To build ANGLE with overlay capability, `angle_enable_overlay = true` must be
placed in `args.gn`.

Currently, to enable overlay items an environment variable is used. For example:

On Desktop:

```commandline
$ export ANGLE_OVERLAY=FPS:Vulkan*PipelineCache*
$ ./hello_triangle --use-angle=vulkan
```

On Android:

```
$ adb shell setprop debug.angle.overlay FPS:Vulkan*PipelineCache*
$ ./hello_triangle --use-angle=vulkan
```

## Future Work

Possible future work:

* On Android, add settings in developer options and enable items based on those.
* Spawn a small server in ANGLE and write an application that sends
  enable/disable commands remotely.
* Move the Overlay rendering functionality to the front-end to benefit all
  backends.
* Add more overlay widgets.
* Implement automatic widget layout to remove the need to specify positions in
  the overlay widgets JSON.
