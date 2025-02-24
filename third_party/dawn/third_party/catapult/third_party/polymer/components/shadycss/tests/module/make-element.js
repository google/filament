/**
@license
Copyright (c) 2016 The Polymer Project Authors. All rights reserved.
This code may only be used under the BSD style license found at http://polymer.github.io/LICENSE.txt
The complete set of authors may be found at http://polymer.github.io/AUTHORS.txt
The complete set of contributors may be found at http://polymer.github.io/CONTRIBUTORS.txt
Code distributed by Google as part of the polymer project is also
subject to an additional IP rights grant found at http://polymer.github.io/PATENTS.txt
*/

/*
A simple webcomponents helper
*/
'use strict';

window.makeElement = (name, connectedCallback) => {
  let template = document.querySelector(`template#${name}`);
  if (template && window.ShadyCSS) {
    window.ShadyCSS.prepareTemplate(template, name);
  }
  window.customElements.define(name, class extends window.HTMLElement {
    connectedCallback() {
      window.ShadyCSS && window.ShadyCSS.styleElement(this);
      if (!this.shadowRoot) {
        this.attachShadow({mode: 'open'});
        if (template) {
          this.shadowRoot.appendChild(template.content.cloneNode(true));
        }
      }
      if (connectedCallback) {
        connectedCallback.call(this);
      }
    }
  });
};
