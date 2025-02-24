/**
@license
Copyright (c) 2016 The Polymer Project Authors. All rights reserved.
This code may only be used under the BSD style license found at http://polymer.github.io/LICENSE.txt
The complete set of authors may be found at http://polymer.github.io/AUTHORS.txt
The complete set of contributors may be found at http://polymer.github.io/CONTRIBUTORS.txt
Code distributed by Google as part of the polymer project is also
subject to an additional IP rights grant found at http://polymer.github.io/PATENTS.txt
*/

'use strict';

const ShadyCSS = window.ShadyCSS;

window.registerSVGElement = () => {
  const LOCAL_NAME = 'svg-in-shadow';
  const TEMPLATE = document.querySelector(`template#${LOCAL_NAME}`);
  ShadyCSS.prepareTemplate(TEMPLATE, LOCAL_NAME);

  class SVGInShadow extends window.HTMLElement {
    connectedCallback() {
      ShadyCSS.styleElement(this);
      this.attachShadow({mode: 'open'});
      this.shadowRoot.appendChild(document.importNode(TEMPLATE.content, true));
    }

    get svg() {
      return this.shadowRoot.querySelector('svg');
    }

    addCircle() {
      const circle = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
      const x = 10 + Math.floor(80 * Math.random());
      const y = 10 + Math.floor(80 * Math.random());
      circle.setAttribute('cx', String(x));
      circle.setAttribute('cy', String(y));
      circle.setAttribute('r', '10');
      this.svg.appendChild(circle);
      return circle;
    }
  }
  window.customElements.define(LOCAL_NAME, SVGInShadow);
};