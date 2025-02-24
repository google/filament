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

import {nativeShadow} from './style-settings.js';
import StyleTransformer from './style-transformer.js';
import {getIsExtends, elementHasBuiltCss, wrap} from './style-util.js';

export let flush = function() {};

/**
 * @param {!Element} element
 * @return {string}
 */
function getClasses(element) {
  if (element.classList && element.classList.value) {
    return element.classList.value;
  } else {
    // NOTE: className is patched to remove scoping classes in ShadyDOM
    // use getAttribute('class') instead, which is unpatched
    return element.getAttribute('class') || '';
  }
}

const scopeRegExp = new RegExp(`${StyleTransformer.SCOPE_NAME}\\s*([^\\s]*)`);

/**
 * @param {!Element} element
 * @return {string}
 */
export function getCurrentScope(element) {
  const match = getClasses(element).match(scopeRegExp);
  if (match) {
    return match[1];
  } else {
    return '';
  }
}

/**
 * @param {!Node} node
 */
export function getOwnerScope(node) {
  const ownerRoot = wrap(node).getRootNode();
  if (ownerRoot === node || ownerRoot === node.ownerDocument) {
    return '';
  }
  const host = /** @type {!ShadowRoot} */(ownerRoot).host;
  if (!host) {
    // this may actually be a document fragment
    return '';
  }
  return getIsExtends(host).is;
}

/**
 * @param {!Element} element
 */
export function ensureCorrectScope(element) {
  const currentScope = getCurrentScope(element);
  const ownerRoot = wrap(element).getRootNode();
  if (ownerRoot === element) {
    return;
  }
  if (currentScope && ownerRoot === element.ownerDocument) {
    // node was scoped, but now is in document
    StyleTransformer.domRemoveScope(element, currentScope);
  } else if (ownerRoot instanceof ShadowRoot) {
    const ownerScope = getOwnerScope(element);
    if (ownerScope !== currentScope) {
      // node was scoped, but not by its current owner
      StyleTransformer.domReplaceScope(element, currentScope, ownerScope);
    }
  }
}

/**
 * @param {!HTMLElement|!HTMLDocument} element
 */
export function ensureCorrectSubtreeScoping(element) {
  // find unscoped subtree nodes
  const unscopedNodes = window['ShadyDOM']['nativeMethods']['querySelectorAll'].call(
    element, `:not(.${StyleTransformer.SCOPE_NAME})`);

  for (let j = 0; j < unscopedNodes.length; j++) {
    // it's possible, during large batch inserts, that nodes that aren't
    // scoped within the current scope were added.
    // To make sure that any unscoped nodes that were inserted in the current batch are correctly styled,
    // query all unscoped nodes and force their style-scope to be applied.
    // This could happen if a sub-element appended an unscoped node in its shadowroot and this function
    // runs on a parent element of the host of that unscoped node:
    // parent-element -> element -> unscoped node
    // Here unscoped node should have the style-scope element, not parent-element.
    const unscopedNode = unscopedNodes[j];
    const scopeForPreviouslyUnscopedNode = getOwnerScope(unscopedNode);
    if (scopeForPreviouslyUnscopedNode) {
      StyleTransformer.element(unscopedNode, scopeForPreviouslyUnscopedNode);
    }
  }
}

/**
 * @param {HTMLElement} el
 * @return {boolean}
 */
function isElementWithBuiltCss(el) {
  if (el.localName === 'style' || el.localName === 'template') {
    return elementHasBuiltCss(el);
  }
  return false;
}

/**
 * @param {Array<MutationRecord|null>|null} mxns
 */
function handler(mxns) {
  for (let x=0; x < mxns.length; x++) {
    let mxn = mxns[x];
    if (mxn.target === document.documentElement ||
      mxn.target === document.head) {
      continue;
    }
    for (let i=0; i < mxn.addedNodes.length; i++) {
      let n = mxn.addedNodes[i];
      if (n.nodeType !== Node.ELEMENT_NODE) {
        continue;
      }
      n = /** @type {HTMLElement} */(n); // eslint-disable-line no-self-assign
      let root = n.getRootNode();
      let currentScope = getCurrentScope(n);
      // node was scoped, but now is in document
      // If this element has built css, we must not remove scoping as this node
      // will be used as a template or style without re - applying scoping as an optimization
      if (currentScope && root === n.ownerDocument && !isElementWithBuiltCss(n)) {
        StyleTransformer.domRemoveScope(n, currentScope);
      } else if (root instanceof ShadowRoot) {
        const newScope = getOwnerScope(n);
        // rescope current node and subtree if necessary
        if (newScope !== currentScope) {
          StyleTransformer.domReplaceScope(n, currentScope, newScope);
        }
        // make sure all the subtree elements are scoped correctly
        ensureCorrectSubtreeScoping(n);
      }
    }
  }
}

// if native Shadow DOM is being used, or ShadyDOM handles dynamic scoiping, do not activate the MutationObserver
if (!nativeShadow && !(window['ShadyDOM'] && window['ShadyDOM']['handlesDynamicScoping'])) {
  let observer = new MutationObserver(handler);
  let start = (node) => {
    observer.observe(node, {childList: true, subtree: true});
  }
  let nativeCustomElements = (window['customElements'] &&
    !window['customElements']['polyfillWrapFlushCallback']);
  // need to start immediately with native custom elements
  // TODO(dfreedm): with polyfilled HTMLImports and native custom elements
  // excessive mutations may be observed; this can be optimized via cooperation
  // with the HTMLImports polyfill.
  if (nativeCustomElements) {
    start(document);
  } else {
    let delayedStart = () => {
      start(document.body);
    }
    // use polyfill timing if it's available
    if (window['HTMLImports']) {
      window['HTMLImports']['whenReady'](delayedStart);
    // otherwise push beyond native imports being ready
    // which requires RAF + readystate interactive.
    } else {
      requestAnimationFrame(function() {
        if (document.readyState === 'loading') {
          let listener = function() {
            delayedStart();
            document.removeEventListener('readystatechange', listener);
          }
          document.addEventListener('readystatechange', listener);
        } else {
          delayedStart();
        }
      });
    }
  }

  flush = function() {
    handler(observer.takeRecords());
  }
}
