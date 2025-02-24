
Experimental build targets
--------------------------

Most people should use the basic polyfill in `web-animations.min.js`. This
tracks the Web Animations features that are supported natively in browsers.
However, we also provide two additional build targets that contain experimental
features.

### web-animations-next.min.js

Contains all of web-animations.min.js plus features that are still undergoing
discussion or have yet to be implemented natively.

### web-animations-next-lite.min.js

A cut down version of web-animations-next, it removes several lesser used
property handlers and some of the larger and less used features such as matrix
interpolation/decomposition.

Build target comparison
-----------------------

|                        | web-animations | web-animations-next | web-animations-next-lite |
|------------------------|:--------------:|:-------------------:|:------------------------:|
|Size (gzipped)          | 15KB           | 19KB                | 15KB                     |
|Element.animate         | ✔             | ✔                  | ✔                       |
|Timing input (easings, duration, fillMode, etc.) for animation effects| ✔ | ✔ | ✔             | 
|Playback control        | ✔             | ✔                  | ✔                       |
|Support for animating lengths, transforms and opacity| ✔ | ✔ | ✔                       |
|Support for animating other CSS properties| ✔ | ✔            | 🚫                       |
|Matrix fallback for transform animations | ✔ | ✔             | 🚫                       |
|KeyframeEffect constructor   | 🚫             | ✔                  | ✔                       |
|Simple GroupEffects & SequenceEffects           | 🚫             | ✔                  | ✔                       |
|Custom Effects          | 🚫             | ✔                  | ✔                       |
|Timing input (easings, duration, fillMode, etc.) for groups</div>| 🚫 | 🚫\* | 🚫         |
|Additive animation      | 🚫\*           | 🚫\*                | 🚫                       |
|Motion path             | 🚫\*           | 🚫\*                | 🚫                       |
|Modifiable keyframe effect timing| 🚫          | 🚫\*                | 🚫\*                     |
|Modifiable group timing | 🚫             | 🚫\*                | 🚫\*                     |
|Usable inline style\*\* | ✔             | ✔                  | 🚫                       |

\* support is planned for these features.
\*\* see inline style caveat below.

Caveat: Inline style
--------------------

Inline style modification is the mechanism used by the polyfill to animate
properties. Both web-animations and web-animations-next incorporate a module
that emulates a vanilla inline style object, so that style modification from
JavaScript can still work in the presence of animations. However, to keep the
size of web-animations-next-lite as small as possible, the style emulation
module is not included. When using this version of the polyfill, JavaScript
inline style modification will be overwritten by animations.
Due to browser constraints inline style modification is not supported on iOS 7
or Safari 6 (or earlier versions).

