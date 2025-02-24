## Overview

The `iron-flex-layout` component provides simple ways to use [CSS flexible box layout](https://developer.mozilla.org/en-US/docs/Web/Guide/CSS/Flexible_boxes), also known as _flexbox_. This component provides two different ways to use flexbox:

*   Layout classes. The layout class stylesheet provides a simple set of class-based flexbox rules. Layout classes
    let you specify layout properties directly in markup.

*   Custom CSS mixins.  The mixin stylesheet includes custom CSS mixins that can be applied
    inside a CSS rule using the `@apply` function.

Using the classes or CSS mixins is largely a matter of preference. The following sections discuss
how to use the each of the stylesheets.

> <b>Note:</b> Before using either of these stylesheets, it's helpful to be familiar with the basics
of flexbox layout. Chris Coyier's [A Complete Guide to Flexbox](https://css-tricks.com/snippets/css/a-guide-to-flexbox/) is a
good primer.</aside>

### Using layout classes

To use layout classes import the `iron-flex-layout-classes` file. You
must do this in any element that uses any of the `iron-flex-layout` styles.
```html
<link rel="import" href="bower_components/iron-flex-layout/iron-flex-layout-classes.html">
```

Then include the module(s) that you need:
```html
<!-- include classes in the main document -->
<style is="custom-style" include="iron-flex iron-flex-alignment">
```

or:
```html
<!-- import classes in an element -->
<style include="iron-flex iron-flex-alignment">
```

Then simply apply the classes to any element.
```html
<div class="layout horizontal wrap">
```

Many of the layout rules involve combinations of multiple classes (such as `layout horizontal wrap` above),
and will need a combination of modules.
The order in which the classes are specified doesn't matter, so `layout horizontal` and `horizontal layout`
are equivalent.

There are 5 modules available:
- `iron-flex`. Basic flex layouts.
- `iron-flex-reverse`. Reverse flexbox layouts.
- `iron-flex-alignment`.  Main axis, cross axis and self alignment.
- `iron-flex-factors`. All the available flex factors.
- `iron-positioning`. Generic, non-flexbox positioning helpers.

**Example: using classes in the main document**
```html
<head>

  ...

  <link rel="import" href="bower_components/iron-flex-layout/iron-flex-layout-classes.html">

  ...

  <!-- main document -- include the module you need in a custom-style element -->
  <style is="custom-style" include="iron-flex"></style>

</head>
<body>

  <div class="layout horizontal">
    <div>One</div>
    <div>Two</div>
    <div>Three</div>
  </div>

</body>
```
**Example: using classes in a Polymer element**
```html
<link rel="import" href="bower_components/iron-flex-layout/iron-flex-layout-classes.html">

  ...

<dom-module id="mixin-demo">

  <!-- inside an element -- include the module you need in a standard style element -->
  <style include="iron-flex"></style>

  <template>
    <div class="layout horizontal">
      <div>One</div>
      <div>Two</div>
      <div>Three</div>
    </div>
  </template>

  <script>
    Polymer({ is: 'mixin-demo' });
  </script>

</dom-module>
```

It's important to note that unlike the previous layout class stylesheets
(found in `/classes/iron-flex-layout.html`), the new version does not use the `/deep/`
combinator: it does not work across local DOM boundaries,
and the modules must be imported into each scope where they're used.

### Using layout mixins

Custom mixins can be applied inside a Polymer
custom element's stylesheet, **or** inside a `custom-style` stylesheet to apply styles to the
main document. (They cannot be applied in the main document without a `custom-style` stylesheet.)

**Example: using mixins in the main document**
```html
<head>

  ...

  <link rel="import" href="bower_components/iron-flex-layout/iron-flex-layout.html">

  ...

  <!-- main document -- apply mixins in a custom-style element -->
  <style is="custom-style">
    .container {
      @apply(--layout-horizontal);
      @apply(--layout-wrap);
    }
  </style>

</head>
<body>

  <div class="container">
    <div>One</div>
    <div>Two</div>
    <div>Three</div>
  </div>

</body>
```
**Example: using mixins in a Polymer element**
```html
<link rel="import" href="bower_components/iron-flex-layout/iron-flex-layout.html">

  ...

<dom-module id="mixin-demo">

  <!-- inside an element -- apply mixins in a standard style element -->
  <style>
    .container {
      @apply(--layout-horizontal);
      @apply(--layout-wrap);
    }
  </style>

  <template>
    <div class="container">
      <div>One</div>
      <div>Two</div>
      <div>Three</div>
    </div>
  </template>

  <script>
    Polymer({ is: 'mixin-demo' });
  </script>

</dom-module>
```

In general the mixins require a little more code to use, but they can be preferable if you
don't want to use the classes, or if you want to switch layouts based on a media query.

Custom CSS properties and mixins are features provided by the Polymer library.
See [Cross-scope styling](https://www.polymer-project.org/1.0/docs/devguide/styling.html#xscope-styling)
in the Polymer developer guide.

## Horizontal and vertical layout

Create a flex container that lays out its children vertically or horizontally.

Class | Mixin | Result
:-|:-|:-
<code>layout horizontal</code>| <code>--layout-horizontal</code> | Horizontal layout container.
<code>layout vertical</code> | <code>--layout-vertical</code> | Vertical layout container.

The classes listed here are included in the `iron-flex` module of the `iron-flex-layout-classes` file.

**Example: classes**
```html
<div class="layout horizontal">
  <div>One</div>
  <div>Two</div>
  <div>Three</div>
</div>
```

**Example: mixins**
<!--
```
<custom-element-demo>
  <template>
    <script src="../webcomponentsjs/webcomponents-lite.min.js"></script>
    <link rel="import" href="iron-flex-layout.html">
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
<style is="custom-style">
  .container {
    @apply(--layout-horizontal);
  }
</style>

<div class="container">
  <div>One</div>
  <div>Two</div>
  <div>Three</div>
</div>
```

### Flexible children

Children of a flex container can use flex to control their own sizing.

Class | Mixin | Result
:-|:-|:-
<code>flex</code>| <code>--layout-flex</code> | Expand the child to fill available space in the main axis.
<code>flex-<var>ratio</var></code>| <code>--layout-flex-<var>ratio</var></code> | Assign a flex ratio of 1 to 12.
<code>flex-none</code>| <code>--layout-flex-none</code> | Don't flex the child.
<code>flex-auto</code>| <code>--layout-flex-auto</code> | Sets flex `flex-basis` to `auto` and `flex-grow` and `flex-shrink` to 1.

The classes listed here are included in the `iron-flex` module of the `iron-flex-layout-classes` file.

**Example: classes**
```html
<div class="horizontal layout">
  <div>Alpha</div>
  <div class="flex">Beta (flex)</div>
  <div>Gamma</div>
</div>
```

**Example: mixins**
<!--
```
<custom-element-demo>
  <template>
    <script src="../webcomponentsjs/webcomponents-lite.min.js"></script>
    <link rel="import" href="iron-flex-layout.html">
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
<style is="custom-style">
  .container {
    @apply(--layout-horizontal);
  }
  .flexchild {
    @apply(--layout-flex);
  }
</style>

<div class="container">
<div>Alpha</div>
  <div class="flexchild">Beta (flex)</div>
  <div>Gamma</div>
</div>
```

#### Flexible children in vertical layouts

The same rules can be used for children in vertical layouts.

**Example: classes**
```html
<div class="vertical layout" style="height:250px">
  <div>Alpha</div>
  <div class="flex">Beta (flex)</div>
  <div>Gamma</div>
</div>
```

**Example: mixins**
<!--
```
<custom-element-demo>
  <template>
    <script src="../webcomponentsjs/webcomponents-lite.min.js"></script>
    <link rel="import" href="iron-flex-layout.html">
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
<style is="custom-style">
  .container {
    @apply(--layout-vertical);
  }
  .flexchild {
    @apply(--layout-flex);
  }
</style>

<div class="container" style="height: 250px">
  <div>One</div>
  <div class="flexchild">Two</div>
  <div>Three</div>
</div>
```

> **Note**: for vertical layouts, the container needs to have a height for the
children to flex correctly.

#### Flex ratios

Children elements can be told to take up more space by including a "flex ratio"
from 1 to 12. This is equivalent to specifying the CSS `flex-grow` property.

For example, the following examples make "Gamma" 2x larger than "Beta" and "Alpha" 3x larger, use
`flex-2` and `flex-3`, respectively.

The classes listed here are included in the `iron-flex-factors` module of the `iron-flex-layout-classes` file.

**Example: classes**
```html
<div class="horizontal layout demo">
  <div class="flex-3">Alpha</div>
  <div class="flex">Beta</div>
  <div class="flex-2">Gamma</div>
</div>
```

**Example: mixins**
<!--
```
<custom-element-demo>
  <template>
    <script src="../webcomponentsjs/webcomponents-lite.min.js"></script>
    <link rel="import" href="iron-flex-layout.html">
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
<style is="custom-style">
  .container {
    @apply(--layout-horizontal);
  }
  .flexchild {
    @apply(--layout-flex)
  }
  .flex2child {
    @apply(--layout-flex-2);
  }
  .flex3child {
    @apply(--layout-flex-3);
  }
</style>

<div class="container">
  <div class="flex3child">One</div>
  <div class="flexchild">Two</div>
  <div class="flex2child">Three</div>
</div>
```

### Cross-axis alignment

By default, children stretch to fit the cross-axis (e.g. _vertical_ stretching in a _horizontal_ layout).

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
<div class="horizontal layout" style="height: 154px">
  <div>Stretch Fill</div>
</div>
```

Center _across_ the main axis (e.g. _vertical_ centering elements in a _horizontal_ layout)
by adding the `center` class or applying the `--layout-center` mixin.

**Example: classes, cross-axis center**
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
<div class="horizontal layout center" style="height: 154px">
  <div>Center</div>
</div>
```

**Example: mixins, cross-axis center**
<!--
```
<custom-element-demo>
  <template>
    <script src="../webcomponentsjs/webcomponents-lite.min.js"></script>
    <link rel="import" href="iron-flex-layout.html">
    <dom-module id="demo-element">
      <template>
        <style is="custom-style" include="iron-flex iron-flex-alignment"></style>
        <style>
          .container, .layout {
            background-color: #ccc;
            padding: 4px;
            height: 154px;
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
<style is="custom-style">
  .container {
    @apply(--layout-horizontal);
    @apply(--layout-center);
  }
</style>

<div class="container" style="height: 154px">
  <div>Center</div>
</div>
```

You can also position at the top/bottom (or left/right in `vertical` layouts) using the `start` or `end`
classes, or by applying the `--layout-start` or `--layout-end` mixins.


**Example: classes, cross-axis start**
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
<div class="horizontal layout start" style="height: 154px">
  <div>start</div>
</div>
```

**Example: mixins, cross-axis start**
<!--
```
<custom-element-demo>
  <template>
    <script src="../webcomponentsjs/webcomponents-lite.min.js"></script>
    <link rel="import" href="iron-flex-layout.html">
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
<style is="custom-style">
  .container {
    @apply(--layout-horizontal);
    @apply(--layout-start);
  }
</style>

<div class="container" style="height: 154px">
  <div>start</div>
</div>
```

**Example: classes, cross-axis end**
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
<div class="horizontal layout end" style="height: 154px">
  <div>end</div>
</div>
```

**Example: mixins, cross-axis end**
<!--
```
<custom-element-demo>
  <template>
    <script src="../webcomponentsjs/webcomponents-lite.min.js"></script>
    <link rel="import" href="iron-flex-layout.html">
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
<style is="custom-style">
  .container {
    @apply(--layout-horizontal);
    @apply(--layout-end);
  }
</style>

<div class="container" style="height: 154px">
  <div>end</div>
</div>
```

### Justification

Justifying aligns contents along the **main axis**.  Justify the layout
by specifying  one of the following.


Class | Mixin | Result
:-|:-|:-
`start-justified`| <code>--layout-start-justified</code> | Aligns contents at the start of the main axis.
`center-justified` | <code>--layout-center-justified</code> | Centers contents along the main axis.
`end-justified` | <code>--layout-end-justified</code> | Aligns contents to the end of the main axis.
`justified` | <code>--layout-justified</code> | Aligns contents with equal spaces between children.
`around-justified` | <code>--layout-around-justified</code> | Aligns contents with equal spaces arround children.

The classes listed here are included in the `iron-flex-alignment` module of the `iron-flex-layout-classes` file.

**Example: classes, start justified**
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
<div class="horizontal start-justified layout">
  <div>start-justified</div>
</div>
```

**Example: mixins, center justified**
<!--
```
<custom-element-demo>
  <template>
    <script src="../webcomponentsjs/webcomponents-lite.min.js"></script>
    <link rel="import" href="iron-flex-layout.html">
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
<style is="custom-style">
  .container {
    @apply(--layout-horizontal);
    @apply(--layout-center-justified);
  }
</style>

<div class="container">
  <div>center-justified</div>
</div>
```

**Example: classes, end justified**
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
<div class="horizontal end-justified layout">
  <div>end-justified</div>
</div>
```

**Example: mixins, equal space between elements**
<!--
```
<custom-element-demo>
  <template>
    <script src="../webcomponentsjs/webcomponents-lite.min.js"></script>
    <link rel="import" href="iron-flex-layout.html">
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
<style is="custom-style">
  .container {
    @apply(--layout-horizontal);
    @apply(--layout-justified);
  }
</style>

<div class="container">
  <div>justified</div>
  <div>justified</div>
  <div>justified</div>
</div>
```

**Example: classes, equal space around each element**
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
<div class="horizontal around-justified layout">
  <div>around-justified</div>
  <div>around-justified</div>
</div>
```

## Self alignment

Alignment can also be set per-child (instead of using the layout container's rules).

Class | Mixin | Result
:-|:-|:-
`self-start`| <code>--layout-self-start</code> | Aligns the child at the start of the cross-axis.
`self-center` | <code>--layout-self-center</code> | Centers the child along the cross-axis.
`self-end` | <code>--layout-self-end</code> | Aligns the child at the end of the cross-axis.
`self-stretch` | <code>--layout-self-stretch</code> | Stretches the child to fit the cross-axis.

**Example: classes**
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
<div class="horizontal layout" style="height: 120px;">
  <div class="flex self-start">Alpha</div>
  <div class="flex self-center">Beta</div>
  <div class="flex self-end">Gamma</div>
  <div class="flex self-stretch">Delta</div>
</div>
```

**Example: mixins**
<!--
```
<custom-element-demo>
  <template>
    <script src="../webcomponentsjs/webcomponents-lite.min.js"></script>
    <link rel="import" href="iron-flex-layout.html">
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
<style is="custom-style">
  .container {
    @apply(--layout-horizontal);
    @apply(--layout-justified);
    height: 120px;
  }
  .container div {
    @apply(--layout-flex);
  }
  .child1 {
    @apply(--layout-self-start);
  }
  .child2 {
    @apply(--layout-self-center);
  }
  .child3 {
    @apply(--layout-self-end);
  }
  .child4 {
    @apply(--layout-self-stretch);
  }
</style>
<div class="container">
  <div class="child1">Alpha</div>
  <div class="child2">Beta</div>
  <div class="child3">Gamma</div>
  <div class="child4">Delta</div>
</div>
```

> <b>Note:</b> The <code>flex</code> class
(and <code>--layout-flex</code> mixin) shown in these examples is
added for the demo and not required for self-alignment.


## Wrapping

Wrapped layouts can be enabled with the `wrap` class or `--layout-wrap` mixin.

**Example: classes**
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
<div class="horizontal layout wrap" style="width: 220px">
  <div>Alpha</div>
  <div>Beta</div>
  <div>Gamma</div>
  <div>Delta</div>
</div>
```

## Reversed layouts

Layout direction can be mirrored using the following rules:

Class | Mixin | Result
:-|:-|:-
<code>layout horizontal-reverse</code>| <code>--layout-horizontal-reverse</code> | Horizontal layout with children laid out in reverse order (last-to-first).
<code>layout vertical-reverse</code> | <code>--layout-vertical-reverse</code> | Vertical layout with children laid out in reverse order.
<code>layout wrap-reverse</code> | <code>--layout-wrap-reverse</code> | Wrap layout with wrapped rows placed in the reverse order (for example, in a vertical layout, the second row is placed above the first row, instead of below).

The classes listed here are included in the `iron-flex-reverse` module of the `iron-flex-layout-classes` file.

**Example: mixins**
<!--
```
<custom-element-demo>
  <template>
    <script src="../webcomponentsjs/webcomponents-lite.min.js"></script>
    <link rel="import" href="iron-flex-layout.html">
    <dom-module id="demo-element">
      <template>
        <style is="custom-style" include="iron-flex iron-flex-alignment iron-flex-reverse"></style>
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
<style is="custom-style">
  .container {
    @apply(--layout-horizontal-reverse);
  }
</style>

<div class="container">
  <div>Alpha</div>
  <div>Beta</div>
  <div>Gamma</div>
  <div>Delta</div>
</div>
```

## Full bleed &lt;body>

It's common to want the entire `<body>` to fit to the viewport. By themselves, Polymer's layout features on
`<body>` don't achieve the result. You can make `<body>` take up the entire viewport by adding the `fullbleed` class:

```html
<body class="fullbleed vertical layout">
  <div class="flex">Fitting a fullbleed body.</div>
</body>
```

This removes its margins and maximizes its height to the viewport. There is no equivalent mixin, but the same result can
be achieved in CSS very simply:
```css
body {
  margin: 0;
  height: 100vh;
}
```

This class is included in the `iron-positioning` module of the `iron-flex-layout-classes` file.

Note that the `fullbleed` class **only works on the `<body>` tag.** This is the only rule in the
stylesheet that is scoped to a particular tag.


## General purpose rules

Polymer also includes other general purpose rules for basic positioning:

Class | Mixin | Result
:-|:-|:-
`block`| `--layout-block` | Assigns `display: block`
`invisible` | `--layout-invisible` | Assigns `visibility: hidden`
`relative` | `--layout-relative` | Assigns `position: relative`
`fit` | `--layout-fit` | Sets `position: absolute` and sets `top:0;right:0;bottom:0;left:0;` (aka "trbl fitting").

The classes listed here are included in the `iron-positioning` module of the `iron-flex-layout-classes` file.

> <b>Note:</b>When using `fit` layout, the element must have an ancestor with fixed size and `position: relative` layout
to fit inside of.


**Example: classes**
<!--
```
<custom-element-demo>
  <template>
    <script src="../webcomponentsjs/webcomponents-lite.min.js"></script>
    <link rel="import" href="iron-flex-layout-classes.html">
    <dom-module id="demo-element">
      <template>
        <style is="custom-style" include="iron-flex iron-flex-alignment iron-positioning"></style>
        <style>
          :host {
            display: block;
            background: #ccc;
          }
          
          .demo {
            background-color: white;
            margin: 12px;
            padding: 4px;
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
<div class="demo">Before <span>[A Span]</span> After</div>
<div class="demo">Before <span class="block">[A Block Span]</span> After</div>
<div class="demo">Before invisible span <span class="invisible">Not displayed</span> After invisible span</div>
<div class="relative" style="height: 100px;">
  <div class="fit" style="background-color: #000;color: white">Fit</div>
</div>
```
