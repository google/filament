[![Build status](https://travis-ci.org/PolymerElements/paper-input.svg?branch=master)](https://travis-ci.org/PolymerElements/paper-input)
[![Published on webcomponents.org](https://img.shields.io/badge/webcomponents.org-published-blue.svg)](https://beta.webcomponents.org/element/PolymerElements/paper-input)

## &lt;paper-input&gt;

Material design: [Text fields](https://www.google.com/design/spec/components/text-fields.html)

`<paper-input>` is a single-line text field with Material Design styling.

<!---
```
<custom-element-demo>
  <template>
    <script src="../webcomponentsjs/webcomponents-lite.js"></script>
    <link rel="import" href="paper-input.html">
    <link rel="import" href="../iron-icons/iron-icons.html">
    <style>
      paper-input {
        max-width: 400px;
        margin: auto;
      }
      iron-icon, div[suffix] {
        color: hsl(0, 0%, 50%);
        margin-right: 12px;
      }
    </style>
    <next-code-block></next-code-block>
  </template>
</custom-element-demo>
```
-->
```html
<paper-input always-float-label label="Floating label"></paper-input>
<paper-input label="username">
  <iron-icon icon="mail" prefix></iron-icon>
  <div suffix>@email.com</div>
</paper-input>
```
