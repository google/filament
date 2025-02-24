
What is Web Animations?
-----------------------

A new JavaScript API for driving animated content on the web. By unifying
the animation features of SVG and CSS, Web Animations unlocks features
previously only usable declaratively, and exposes powerful, high-performance
animation capabilities to developers.

What is in this repository?
---------------------------

A JavaScript implementation of the Web Animations API that provides Web
Animation features in browsers that do not support it natively. The polyfill
falls back to the native implementation when one is available.

Quick start
-----------

Here's a simple example of an animation that fades and scales a `<div>`.  
[Try it as a live demo.](http://jsbin.com/yageyezabo/edit?html,js,output)

```html
<!-- Include the polyfill -->
<script src="web-animations.min.js"></script>

<!-- Set up a target to animate -->
<div class="pulse" style="width: 150px;">Hello world!</div>

<!-- Animate! -->
<script>
    var elem = document.querySelector('.pulse');
    var animation = elem.animate({
        opacity: [0.5, 1],
        transform: ['scale(0.5)', 'scale(1)'],
    }, {
        direction: 'alternate',
        duration: 500,
        iterations: Infinity,
    });
</script>
```

Documentation
-------------

* [Codelab tutorial](https://github.com/web-animations/web-animations-codelabs)
* [Examples of usage](/docs/examples.md)
* [Live demos](https://web-animations.github.io/web-animations-demos)
* [MDN reference](https://developer.mozilla.org/en-US/docs/Web/API/Element/animate)
* [W3C specification](http://w3c.github.io/web-animations/)

We love feedback!
-----------------

* For feedback on the API and the specification:
    * File an issue on GitHub: <https://github.com/w3c/web-animations/issues/new>
    * Alternatively, send an email to <public-fx@w3.org> with subject line
"[web-animations] ... message topic ..."
([archives](http://lists.w3.org/Archives/Public/public-fx/)).

* For issues with the polyfill, report them on GitHub:
<https://github.com/web-animations/web-animations-js/issues/new>.

Keep up-to-date
---------------

Breaking polyfill changes will be announced on this low-volume mailing list:
[web-animations-changes@googlegroups.com](https://groups.google.com/forum/#!forum/web-animations-changes).

More info
---------

* [Technical details about the polyfill](/docs/support.md)
    * [Browser support](/docs/support.md#browser-support)
    * [Fallback to native](/docs/support.md#native-fallback)
    * [Feature list](/docs/support.md#features)
    * [Feature deprecation and removal processes](/docs/support.md#process-for-breaking-changes)
* To test experimental API features, try one of the
  [experimental targets](/docs/experimental.md)
