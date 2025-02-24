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

/** @type {!Set<string>} */
const styleTextSet = new Set();

export const scopingAttribute = 'shady-unscoped';

/**
 * Add a specifically-marked style to the document directly, and only one copy of that style.
 *
 * @param {!HTMLStyleElement} style
 * @return {undefined}
 */
export function processUnscopedStyle(style) {
  const text = style.textContent;
  if (!styleTextSet.has(text)) {
    styleTextSet.add(text);
    const newStyle = style.cloneNode(true);
    document.head.appendChild(newStyle);
  }
}

/**
 * Check if a style is supposed to be unscoped
 * @param {!HTMLStyleElement} style
 * @return {boolean} true if the style has the unscoping attribute
 */
export function isUnscopedStyle(style) {
  return style.hasAttribute(scopingAttribute);
}