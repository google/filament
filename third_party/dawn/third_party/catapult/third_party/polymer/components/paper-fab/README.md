[![Build status](https://travis-ci.org/PolymerElements/paper-fab.svg?branch=master)](https://travis-ci.org/PolymerElements/paper-fab)

##&lt;paper-fab&gt;

Material design: [Floating Action Button](https://www.google.com/design/spec/components/buttons-floating-action-button.html)

`paper-fab` is a floating action button. It contains an image placed in the center and
comes in two sizes: regular size and a smaller size by applying the attribute `mini`. When
the user touches the button, a ripple effect emanates from the center of the button.

You may import `iron-icons` to use with this element, or provide a URL to a custom icon.
See `iron-iconset` for more information about how to use a custom icon set.

<!---
```
<custom-element-demo>
  <template>
    <script src="../webcomponentsjs/webcomponents-lite.js"></script>
    <link rel="import" href="paper-fab.html">
    <link rel="import" href="../iron-icons/iron-icons.html">
    <style is="custom-style">
      paper-fab {
        display: inline-block;
        margin: 8px;
      }
      
      paper-fab[mini] {
        --paper-fab-background: #FF5722;
      }
      
      paper-fab[label] {
        font-size: 20px;
        --paper-fab-background: #2196F3;
      }
      
      .container {
        display: flex;
        align-items: center;
      }
    </style>
    <div class="container">
      <next-code-block></next-code-block>
    </div>
  </template>
</custom-element-demo>
```
-->
```html
<paper-fab icon="favorite"></paper-fab>
<paper-fab mini icon="reply"></paper-fab>
<paper-fab label="😻"></paper-fab>
```

