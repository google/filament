/**
@license
Copyright (c) 2017 The Polymer Project Authors. All rights reserved.
This code may only be used under the BSD style license found at http://polymer.github.io/LICENSE.txt
The complete set of authors may be found at http://polymer.github.io/AUTHORS.txt
The complete set of contributors may be found at http://polymer.github.io/CONTRIBUTORS.txt
Code distributed by Google as part of the polymer project is also
subject to an additional IP rights grant found at http://polymer.github.io/PATENTS.txt
*/

'use strict';

import ScopingShim from '../src/scoping-shim.js';
import {nativeCssVariables, nativeShadow, cssBuild} from '../src/style-settings.js';

/** @const {ScopingShim} */
const scopingShim = new ScopingShim();

let ApplyShim, CustomStyleInterface;

if (window['ShadyCSS']) {
  ApplyShim = window['ShadyCSS']['ApplyShim'];
  CustomStyleInterface = window['ShadyCSS']['CustomStyleInterface'];
}

window.ShadyCSS = {
  ScopingShim: scopingShim,
  /**
   * @param {!HTMLTemplateElement} template
   * @param {string} elementName
   * @param {string=} elementExtends
   */
  prepareTemplate(template, elementName, elementExtends) {
    scopingShim.flushCustomStyles();
    scopingShim.prepareTemplate(template, elementName, elementExtends)
  },

  /**
   * @param {!HTMLTemplateElement} template
   * @param {string} elementName
   */
  prepareTemplateDom(template, elementName) {
    scopingShim.prepareTemplateDom(template, elementName);
  },

  /**
   * @param {!HTMLTemplateElement} template
   * @param {string} elementName
   * @param {string=} elementExtends
   */
  prepareTemplateStyles(template, elementName, elementExtends) {
    scopingShim.flushCustomStyles();
    scopingShim.prepareTemplateStyles(template, elementName, elementExtends)
  },
  /**
   * @param {!HTMLElement} element
   * @param {Object=} properties
   */
  styleSubtree(element, properties) {
    scopingShim.flushCustomStyles();
    scopingShim.styleSubtree(element, properties);
  },

  /**
   * @param {!HTMLElement} element
   */
  styleElement(element) {
    scopingShim.flushCustomStyles();
    scopingShim.styleElement(element);
  },

  /**
   * @param {Object=} properties
   */
  styleDocument(properties) {
    scopingShim.flushCustomStyles();
    scopingShim.styleDocument(properties);
  },

  flushCustomStyles() {
    scopingShim.flushCustomStyles();
  },

  /**
   * @param {Element} element
   * @param {string} property
   * @return {string}
   */
  getComputedStyleValue(element, property) {
    return scopingShim.getComputedStyleValue(element, property);
  },

  nativeCss: nativeCssVariables,

  nativeShadow: nativeShadow,

  cssBuild: cssBuild
};

if (ApplyShim) {
  window.ShadyCSS.ApplyShim = ApplyShim;
}

if (CustomStyleInterface) {
  window.ShadyCSS.CustomStyleInterface = CustomStyleInterface;
}