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

import {applyStylePlaceHolder} from './style-util.js';
import {nativeShadow} from './style-settings.js';

/** @type {!Object<string, !Node>} */
const placeholderMap = {};

/**
 * @param {string} elementName
 * @return {Node}
 */
export function getStylePlaceholder(elementName) {
  return placeholderMap[elementName] || null;
}

/**
 * @param {string} elementName
 */
export function ensureStylePlaceholder(elementName) {
  if (!placeholderMap[elementName]) {
    placeholderMap[elementName] = applyStylePlaceHolder(elementName);
  }
}

/**
 * @const {CustomElementRegistry}
 */
const ce = window['customElements'];
if (ce && !nativeShadow) {
  /**
   * @const {function(this:CustomElementRegistry, string,function(new:HTMLElement),{extends: string}=)}
   */
  const origDefine = ce['define'];
  /**
   * @param {string} name
   * @param {function(new:HTMLElement)} clazz
   * @param {{extends: string}=} options
   */
  const wrappedDefine = (name, clazz, options) => {
    ensureStylePlaceholder(name);
    origDefine.call(/** @type {!CustomElementRegistry} */(ce), name, clazz, options);
  };
  ce['define'] = wrappedDefine;
}