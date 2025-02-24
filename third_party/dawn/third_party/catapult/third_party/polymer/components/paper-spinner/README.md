[![Build status](https://travis-ci.org/PolymerElements/paper-spinner.svg?branch=master)](https://travis-ci.org/PolymerElements/paper-spinner)

##&lt;paper-spinner&gt;

Material design: [Progress & activity](https://www.google.com/design/spec/components/progress-activity.html)

Element providing a multiple color material design circular spinner.

<!---
```
<custom-element-demo>
  <template>
    <script src="../webcomponentsjs/webcomponents-lite.js"></script>
    <link rel="import" href="paper-spinner.html">
    <link rel="import" href="paper-spinner-lite.html">
    <style is="custom-style">
      paper-spinner, paper-spinner-lite {
        margin: 8px 8px 8px 0;
      }
      paper-spinner-lite.orange {
        --paper-spinner-color: var(--google-yellow-500);
      }
      paper-spinner-lite.green {
        --paper-spinner-color: var(--google-green-500);
      }
      paper-spinner-lite.thin {
        --paper-spinner-stroke-width: 1px;
      }
      paper-spinner-lite.thick {
        --paper-spinner-stroke-width: 6px;
      }
      #container {
        display: flex;
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
<paper-spinner active>...</paper-spinner>
<paper-spinner-lite active class="orange"></paper-spinner-lite>
<paper-spinner-lite active class="green"></paper-spinner-lite>
<paper-spinner-lite active class="thin"></paper-spinner-lite>
<paper-spinner-lite active class="thick"></paper-spinner-lite>
```
