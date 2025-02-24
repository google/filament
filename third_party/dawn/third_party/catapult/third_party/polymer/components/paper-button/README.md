
[![Build status](https://travis-ci.org/PolymerElements/paper-button.svg?branch=master)](https://travis-ci.org/PolymerElements/paper-button)
[![Published on webcomponents.org](https://img.shields.io/badge/webcomponents.org-published-blue.svg)](https://beta.webcomponents.org/element/PolymerElements/paper-button)

##&lt;paper-button&gt;

Material design: [Buttons](https://www.google.com/design/spec/components/buttons.html)

`paper-button` is a button. When the user touches the button, a ripple effect emanates
from the point of contact. It may be flat or raised. A raised button is styled with a
shadow.

Example:

<!---
```
<custom-element-demo>
  <template>
    <script src="../webcomponentsjs/webcomponents-lite.js"></script>
    <link rel="import" href="paper-button.html">
    <link rel="import" href="../paper-styles/color.html">
    <style is="custom-style">
      #container {
        display: flex;
      }
      paper-button {
        font-family: 'Roboto', 'Noto', sans-serif;
        font-weight: normal;
        font-size: 14px;
        -webkit-font-smoothing: antialiased;
      }
      paper-button.pink {
        color: var(--paper-pink-a200);
        --paper-button-ink-color: var(--paper-pink-a200);
      }
      paper-button.pink:hover {
        background-color: var(--paper-pink-100);
      }
      paper-button.indigo {
        background-color: var(--paper-indigo-500);
        color: white;
        --paper-button-raised-keyboard-focus: {
          background-color: var(--paper-pink-a200) !important;
          color: white !important;
        };
      }
      paper-button.indigo:hover {
        background-color: var(--paper-indigo-400);
      }
      paper-button.green {
        background-color: var(--paper-green-500);
        color: white;
      }
      paper-button.green[active] {
        background-color: var(--paper-red-500);
      }
      paper-button.disabled {
        color: white;
      }
    </style>
    <div id="container">
      <next-code-block></next-code-block>
    </div>
  </template>
</custom-element-demo>
```
-->
```html
<paper-button class="pink">link</paper-button>
<paper-button raised class="indigo">raised</paper-button>
<paper-button toggles raised class="green">toggles</paper-button>
<paper-button disabled class="disabled">disabled</paper-button>
```
