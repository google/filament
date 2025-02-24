[![Build status](https://travis-ci.org/PolymerElements/iron-flex-layout.svg?branch=master)](https://travis-ci.org/PolymerElements/iron-flex-layout)
[![Published on webcomponents.org](https://img.shields.io/badge/webcomponents.org-published-blue.svg)](https://beta.webcomponents.org/element/PolymerElements/iron-flex-layout)

## &lt;iron-flex-layout&gt;

The `<iron-flex-layout>` component provides simple ways to use
[CSS flexible box layout](https://developer.mozilla.org/en-US/docs/Web/Guide/CSS/Flexible_boxes),
also known as flexbox. This component provides two different ways to use flexbox:

1. [Layout classes](https://github.com/PolymerElements/iron-flex-layout/tree/master/iron-flex-layout-classes.html).
The layout class stylesheet provides a simple set of class-based flexbox rules, that
let you specify layout properties directly in markup. You must include this file
in every element that needs to use them.

Sample use:

<!--
```
<custom-element-demo>
  <template>
    <script src="../webcomponentsjs/webcomponents-lite.min.js"></script>
    <link rel="import" href="iron-flex-layout-classes.html">
    <dom-module id="demo-element">
      <template>
        <style is="custom-style" include="iron-flex iron-flex-alignment"></style>
        <style>
          .container, .layout {
            background-color: #ccc;
            padding: 4px;
          }

          .container div, .layout div {
            background-color: white;
            padding: 12px;
            margin: 4px;
          }
        </style>
        <next-code-block></next-code-block>
      </template>
      <script>Polymer({is: "demo-element"});</script>
    </dom-module>
    <demo-element></demo-element>
  </template>
</custom-element-demo>
```
-->
```html
<div class="layout horizontal layout-start" style="height: 154px">
  <div>cross axis start alignment</div>
</div>
```

1. [Custom CSS mixins](https://github.com/PolymerElements/iron-flex-layout/blob/master/iron-flex-layout.html).
The mixin stylesheet includes custom CSS mixins that can be applied inside a CSS rule using the `@apply` function.



Please note that the old [/deep/ layout classes](https://github.com/PolymerElements/iron-flex-layout/tree/master/classes)
are deprecated, and should not be used. To continue using layout properties
directly in markup, please switch to using the new `dom-module`-based
[layout classes](https://github.com/PolymerElements/iron-flex-layout/tree/master/iron-flex-layout-classes.html).
Please note that the new version does not use `/deep/`, and therefore requires you
to import the `dom-modules` in every element that needs to use them.

A complete [guide](https://elements.polymer-project.org/guides/flex-layout) to `<iron-flex-layout>` is available.


