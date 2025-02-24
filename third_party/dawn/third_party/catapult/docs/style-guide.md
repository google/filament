# Catapult Style guide

## Base style guide

Unless stated below, we follow the conventions listed in the [Chromium style
guide](https://www.chromium.org/developers/coding-style) and [Google JavaScript
style guide](http://google.github.io/styleguide/javascriptguide.xml).

## JavaScript

### Files
File names `should_look_like_this.html`.

Keep to one concept per file, always. In practice, this usually means one
component or class per file, but can lead to multiple if they’re small and
closely related. If you can, group utility functions into a static class to
clarify their relationship, e.g. `base/statistics.html`.

```html
<!-- tracing/model/point.html -->
<script>
‘use strict’;

tr.exportTo(‘tr.model’, function() {
  function Point() {}

  return {
    Point: Point
  };
});
</script>
```

The exception to this rule is when there are multiple small, related classes or
methods. In this case, a file may export multiple symbols:

```html
<!-- tracing/base/dom_helpers.html -->
<script>
‘use strict’;

tr.exportTo(‘tr.ui.b’, function() {
  function createSpan() { // … }
  function createDiv() { // … }
  function isElementAttached(element) { // … }

  return {
    createSpan: createSpan,
    createDiv: createDiv,
    isElementAttached: isElementAttached
  };
});
</script>
```

Any tests for a file should be in a file with the same name as the
implementation file, but with a trailing `_test`.

```sh
touch tracing/model/access_point.html
touch tracing/model/access_point_test.html
```

### Namespacing and element names

All symbols that exist in the global namespace should be exported using the
`exportTo` method.

Exported package names show the file’s location relative to the root `tracing/`
directory. These package names are abbreviated, usually with a 1 or 2 letter
abbreviation - just enough to resolve naming conflicts. All files in the same
directory should share the same package.

```html
<!-- tracing/extras/chrome/cc/input_latency_async_slice.html →
tr.exportTo(‘tr.e.cc’, function() {
   // ...
});
```

Polymer element names should use the convention
`hyphenated-package-name-element-name`.

```html
<!-- tracing/ui/analysis/counter_sample_sub_view.html -->
<dom-module id='tr-ui-a-counter-sample-sub-view'>
  ...
</dom-module>
```

### Classes and objects

Classes should expose public fields only if those fields represent a part of the
class’s public interface.

All fields should be initialized in the constructor. Fields with no reasonable
default value should be initialized to undefined.

Do not set defaults via the prototype chain.

```javascript
function Line() {
  // Good
  this.yIntercept_ = undefined;
}

Line.prototype = {
  // Bad
  xIntercept_: undefined,


  set slope(slope) {
    // Bad: this.slope_ wasn’t initialized in the constructor.
    this.slope_ = slope;
  },

  set yIntercept() {
    // Good
    return this.yIntercept_;
  }
};
```

### Blocks

From the [Blocks section of the airbnb style
guide](https://github.com/airbnb/javascript#blocks):
Use braces with all multi-line blocks.

```javascript
// bad
if (test)
  return false;

// good
if (test) return false;

// good
if (test) {
  return false;
}

// bad
function foo() { return false; }

// good
function bar() {
  return false;
}
```

If you're using multi-line blocks with `if` and `else`, put `else` on the same
line as your `if` block's closing brace.

```javascript
// bad
if (test) {
  thing1();
  thing2();
}
else {
  thing3();
}

// good
if (test) {
  thing1();
  thing2();
} else {
  thing3();
}
```

### Variables

Use `const` and `let` instead of `var` in all new files and functions. Prefer `const` over `let` when a variable can only refer to a single value throughout its lifetime.

```javascript
// bad
function() {
  let hello = '  hello  ';
  return hello.trim();
}

// good
function() {
  const hello = '  hello  ';
  return hello.trim();
}
```

### Polymer elements
The `<script>` block for the Polymer element can go either inside or outside of
the element’s definition. Generally, the block outside is placed outside when
the script is sufficiently complex that having 2 fewer spaces of indentation
would make it more readable.

```html
<dom-module id="tr-bar">
  <template><div></div></template>
   <script>
     // Can go here...
   </script>
</dom-module>

<script>
'use strict';
(function(){   // Use this if you need to define constants scoped to that element.
Polymer('tr-bar', {
  // ... or here.
});
})();
</script>
```

Style sheets should be inline rather than in external .css files.

```html
<dom-module id="tr-bar">
  <style>
  #content {
    display: flex;
  }
  </style>
  <template><div id=”content”></div></template>
</dom-module>
```

### `undefined` and `null`
Prefer use of `undefined` over `null`.

```javascript
function Line() {
  // Good
  this.yIntercept_ = undefined;
  // Bad
  this.slope = null;
}
```

### Tests
UI element tests that make sure that an element is instantiable should have
names that start with “`instantiate`”. These tests should, as a general rule,
should not make assertions.

Assertions should specify the actual value before the expected value.

```javascript
assert.strictEqual(value.get(), 42);
assert.isBelow(value.get(), 42);
assert.isAbove(value.get(), 42);
assert.lengthOf(value.get(), 42);
```

```python
self.assertEqual(value.Get(), 42)
self.assertLess(value.Get(), 42)
```

### ECMAScript 2015 (ES6) features

**Use of ES6 features is prohibited unless explicitly approved in the table below.** However, we're currently working to allow them.

| Feature                                                                                                                                     | Status                                                                          |
|---------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------|
| [Arrows](https://github.com/lukehoban/es6features#arrows)                                                                                   | [Approved](https://github.com/catapult-project/catapult/issues/2165)            |
| [Classes](https://github.com/lukehoban/es6features#classes)                                                                                 | [Approved](https://github.com/catapult-project/catapult/issues/2176)            |
| [Enhanced object literals](https://github.com/lukehoban/es6features#enhanced-object-literals)                                               | Approved                                                                        |
| [Template strings](https://github.com/lukehoban/es6features#template-strings)                                                               | Approved                                                                        |
| [Destructuring](https://github.com/lukehoban/es6features#destructuring)                                                                     | Approved                                                                        |
| [Default, rest, and spread](https://github.com/lukehoban/es6features#default--rest--spread)                                                 | Approved                                                                        |
| [`let` and `const`](https://github.com/lukehoban/es6features#let--const)                                                                    | Approved and required for new code                                              |
| [Iterators and `for...of`](https://github.com/lukehoban/es6features#iterators--forof)                                                       | Approved                                                                        |
| [Generators](https://github.com/lukehoban/es6features#generators)                                                                           | Approved                                                                        |
| [Unicode](https://github.com/lukehoban/es6features#unicode)                                                                                 | To be discussed                                                                 |
| [Modules](https://github.com/lukehoban/es6features#modules)                                                                                 | To be discussed                                                                 |
| [Module loaders](https://github.com/lukehoban/es6features#module-loaders)                                                                   | To be discussed                                                                 |
| [`Map`, `Set`, `WeakMap`, and `WeakSet`](https://github.com/lukehoban/es6features#map--set--weakmap--weakset)                               | Approved                                                                        |
| [Proxies](https://github.com/lukehoban/es6features#proxies)                                                                                 | To be discussed                                                                 |
| [Symbols](https://github.com/lukehoban/es6features#symbols)                                                                                 | To be discussed                                                                 |
| [Subclassable Built-ins](https://github.com/lukehoban/es6features#subclassable-built-ins)                                                   | Approved                                                                        |
| [Promises](https://github.com/lukehoban/es6features#promises)                                                                               | Approved                                                                        |
| [`Math`, `Number`, `String`, `Array`, and `Object` APIs](https://github.com/lukehoban/es6features#math--number--string--array--object-apis) | To be discussed                                                                 |
| [Binary and octal literals](https://github.com/lukehoban/es6features#binary-and-octal-literals)                                             | To be discussed                                                                 |
| [Reflect API](https://github.com/lukehoban/es6features#reflect-api)                                                                         | To be discussed                                                                 |

### ECMAScript 2016 (ES7) features

**Use of ES7 features is prohibited unless explicitly approved in the table below.** However, we're currently working to allow them.

| Feature                  | Status          |
|--------------------------|-----------------|
| [Array.prototype.includes](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/includes) | [Approved](https://github.com/catapult-project/catapult/issues/3424) |
| [Exponentiation operator](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/Arithmetic_Operators#Exponentiation_(**))  | To be discussed |

### ECMAScript 2017 (ES8) features

**Use of ES8 features is prohibited unless explicitly approved in the table below.** Generally, ES8 features are still experimental and liable to change and therefore not fit for use in Catapult. However, in a few rare cases, features may be stable enough for use.

| Feature                  | Status          |
|--------------------------|-----------------|
| [Object.entries](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object/entries) and [Object.values](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object/values) | Approved |
| [async/await](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Statements/async_function) | Approved |

### Possible feature statuses
  - **Approved**: this feature is approved for general use.
  - **Testing in progress**: there's agreement that we should use this feature, but we still need to make sure that it's safe. "Testing in progress" statuses should link to a Catapult bug thread tracking the testing.
  - **Discussion in progress**: there's not yet agreement that we should use this feature. "Discussion in progress" statuses should link to a Catapult bug thread about whether the feature should be used.
  - **To be discussed**: this feature hasn't been discussed yet.

Use of an ES6 features shouldn't be considered until that feature is [supported](https://kangax.github.io/compat-table/es6/) in both Chrome stable and [our current version of D8](/third_party/vinn/third_party/v8/README.chromium).

If you see that Catapult’s version of D8 is behind Chrome stable's, use
[this script](/third_party/vinn/bin/update_v8) to update it.

## Avoid defensive programming (and document it when you can't)

Don't silently handle unexpected conditions. When such conditions occur, you
should:

  1. Emit a clear warning and continue if the error is non-catastrophic
  2. Fail loudly if the error is catastrophic

If fixing the problem is hard but a simple workaround is possible, then using
the workaround is OK so long as:

  1. An issue is created to track the problem.
  2. The defensive patch is wrapped in a `// TODO` linking to that issue.
  3. The todo and defensive patch are removed after the problem is fixed.
