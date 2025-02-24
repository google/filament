
Getting the polyfill
--------------------

There are three ways to get a copy of the polyfill:

1. Download and use the `web-animations.min.js` file directly from this repository
1. Using npm: Add [`web-animations-js`](https://www.npmjs.com/package/web-animations-js) to your `package.json`
1. Using Bower: Add `web-animations/web-animations-js` to your `bower.json`

Browser support
---------------

The polyfill is supported on modern versions of all major browsers, including:

* Chrome 55+
* Firefox 27+
* IE10+ (including Edge)
* Safari (iOS) 7.1+
* Safari (Mac) 9+

Native fallback
---------------

When the polyfill runs on a browser that implements `Element.animate()` and
`Animation` playback control, it will detect and use the underlying native
features for better performance.

Features
--------

The `web-animations.min.js` polyfill target tracks the Web Animations features
that are supported natively in browsers. These include:

* Element.animate()
* Timing input (easings, duration, fillMode, etc.) for animation effects
* Playback control (play, pause, reverse, currentTime, cancel, onfinish)
* Support for animating CSS properties

Caveat: Prefix handling
-----------------------

The polyfill will automatically detect the correctly prefixed name to use when
writing animated properties back to the platform. Where possible, the polyfill
will only accept unprefixed versions of experimental features. For example:

```js
element.animate({transform: ['none', 'translateX(100px)']}, 1000);
```

will work in all browsers that implement a conforming version of transform, but

```js
element.animate({webkitTransform: ['none', 'translateX(100px)']}, 1000);
```

will not work anywhere.

Process for breaking changes
----------------------------

When we make a potentially breaking change to the polyfill's API
surface (like a rename) we will, where possible, continue supporting the
old version, deprecated, for three months, and ensure that there are
console warnings to indicate that a change is pending. After three
months, the old version of the API surface (e.g. the old version of a
function name) will be removed. *If you see deprecation warnings, you
can't avoid them by not updating*.

