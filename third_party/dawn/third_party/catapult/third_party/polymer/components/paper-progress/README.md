[![Build status](https://travis-ci.org/PolymerElements/paper-progress.svg?branch=master)](https://travis-ci.org/PolymerElements/paper-progress)

##&lt;paper-progress&gt;

Material design: [Progress & activity](https://www.google.com/design/spec/components/progress-activity.html)

<!---
```
<custom-element-demo>
  <template>
    <script src="../webcomponentsjs/webcomponents-lite.js"></script>
    <link rel="import" href="../paper-styles/color.html">
    <link rel="import" href="paper-progress.html">
    <style is="custom-style">
      paper-progress {
        display: block;
        width: 100%;
        margin: 20px 0;
      }
      paper-progress.slow {
        --paper-progress-indeterminate-cycle-duration: 5s;
      }
      paper-progress.blue {
        --paper-progress-active-color: var(--paper-light-blue-500);
        --paper-progress-secondary-color: var(--paper-light-blue-100);
      }
      paper-progress.red {
        --paper-progress-active-color: var(--paper-red-500);
        --paper-progress-secondary-color: var(--paper-red-100);
      }
    </style>
    <next-code-block></next-code-block>
  </template>
</custom-element-demo>
```
-->
```html
<paper-progress indeterminate class="blue"></paper-progress>
<paper-progress indeterminate class="slow red"></paper-progress>
<paper-progress value="40" secondary-progress="80"></paper-progress>
```

The progress bars are for situations where the percentage completed can be
determined. They give users a quick sense of how much longer an operation
will take.

There is also a secondary progress which is useful for displaying intermediate
progress, such as the buffer level during a streaming playback progress bar.

