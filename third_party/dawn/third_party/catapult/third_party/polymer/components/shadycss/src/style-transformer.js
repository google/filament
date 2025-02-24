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

import {StyleNode} from './css-parse.js'; // eslint-disable-line no-unused-vars
import * as StyleUtil from './style-util.js';
import {nativeShadow} from './style-settings.js';

/* Transforms ShadowDOM styling into ShadyDOM styling

* scoping:

  * elements in scope get scoping selector class="x-foo-scope"
  * selectors re-written as follows:

    div button -> div.x-foo-scope button.x-foo-scope

* :host -> scopeName

* :host(...) -> scopeName...

* ::slotted(...) -> scopeName > ...

* ...:dir(ltr|rtl) -> [dir="ltr|rtl"] ..., ...[dir="ltr|rtl"]

* :host(:dir[rtl]) -> scopeName:dir(rtl) -> [dir="rtl"] scopeName, scopeName[dir="rtl"]

*/
const SCOPE_NAME = 'style-scope';

class StyleTransformer {
  get SCOPE_NAME() {
    return SCOPE_NAME;
  }
  /**
   * Given a node and scope name, add a scoping class to each node
   * in the tree. This facilitates transforming css into scoped rules.
   * @param {!Node} node
   * @param {string} scope
   * @param {boolean=} shouldRemoveScope
   * @deprecated
   */
  dom(node, scope, shouldRemoveScope) {
    const fn = (node) => {
      this.element(node, scope || '', shouldRemoveScope);
    };
    this._transformDom(node, fn);
  }

  /**
   * Given a node and scope name, add a scoping class to each node in the tree.
   * @param {!Node} node
   * @param {string} scope
   */
  domAddScope(node, scope) {
    const fn = (node) => {
      this.element(node, scope || '');
    };
    this._transformDom(node, fn);
  }

  /**
   * @param {!Node} startNode
   * @param {!function(!Node)} transformer
   */
  _transformDom(startNode, transformer) {
    if (startNode.nodeType === Node.ELEMENT_NODE) {
      transformer(startNode)
    }
    let c$;
    if (startNode.localName === 'template') {
      const template = /** @type {!HTMLTemplateElement} */ (startNode);
      // In case the template is in svg context, fall back to the node
      // since it won't be an HTMLTemplateElement with a .content property
      c$ = (template.content || template._content || template).childNodes;
    } else {
      c$ = /** @type {!ParentNode} */ (startNode).children ||
          startNode.childNodes;
    }
    if (c$) {
      for (let i = 0; i < c$.length; i++) {
        this._transformDom(c$[i], transformer);
      }
    }
  }

  /**
   * @param {?} element
   * @param {?} scope
   * @param {?=} shouldRemoveScope
   */
  element(element, scope, shouldRemoveScope) {
    // note: if using classes, we add both the general 'style-scope' class
    // as well as the specific scope. This enables easy filtering of all
    // `style-scope` elements
    if (scope) {
      // note: svg on IE does not have classList so fallback to class
      if (element.classList) {
        if (shouldRemoveScope) {
          element.classList.remove(SCOPE_NAME);
          element.classList.remove(scope);
        } else {
          element.classList.add(SCOPE_NAME);
          element.classList.add(scope);
        }
      } else if (element.getAttribute) {
        let c = element.getAttribute(CLASS);
        if (shouldRemoveScope) {
          if (c) {
            let newValue = c.replace(SCOPE_NAME, '').replace(scope, '');
            StyleUtil.setElementClassRaw(element, newValue);
          }
        } else {
          let newValue = (c ? c + ' ' : '') + SCOPE_NAME + ' ' + scope;
          StyleUtil.setElementClassRaw(element, newValue);
        }
      }
    }
  }

  /**
   * Given a node, replace the scoping class to each subnode in the tree.
   * @param {!Node} node
   * @param {string} oldScope
   * @param {string} newScope
   */
  domReplaceScope(node, oldScope, newScope) {
    const fn = (node) => {
      this.element(node, oldScope, true);
      this.element(node, newScope);
    };
    this._transformDom(node, fn);
  }
  /**
   * Given a node, remove the scoping class to each subnode in the tree.
   * @param {!Node} node
   * @param {string} oldScope
   */
  domRemoveScope(node, oldScope) {
    const fn = (node) => {
      this.element(node, oldScope || '', true);
    };
    this._transformDom(node, fn);
  }

  /**
   * @param {?} element
   * @param {?} styleRules
   * @param {?=} callback
   * @param {string=} cssBuild
   * @param {string=} cssText
   * @return {string}
   */
  elementStyles(element, styleRules, callback, cssBuild = '', cssText = '') {
    // no need to shim selectors if settings.useNativeShadow, also
    // a shady css build will already have transformed selectors
    // NOTE: This method may be called as part of static or property shimming.
    // When there is a targeted build it will not be called for static shimming,
    // but when the property shim is used it is called and should opt out of
    // static shimming work when a proper build exists.
    if (cssText === '') {
      if (nativeShadow || cssBuild === 'shady') {
        cssText = StyleUtil.toCssText(styleRules, callback);
      } else {
        let {is, typeExtension} = StyleUtil.getIsExtends(element);
        cssText = this.css(styleRules, is, typeExtension, callback) + '\n\n';
      }
    }
    return cssText.trim();
  }

  // Given a string of cssText and a scoping string (scope), returns
  // a string of scoped css where each selector is transformed to include
  // a class created from the scope. ShadowDOM selectors are also transformed
  // (e.g. :host) to use the scoping selector.
  css(rules, scope, ext, callback) {
    let hostScope = this._calcHostScope(scope, ext);
    scope = this._calcElementScope(scope);
    let self = this;
    return StyleUtil.toCssText(rules, function(/** StyleNode */rule) {
      if (!rule.isScoped) {
        self.rule(rule, scope, hostScope);
        rule.isScoped = true;
      }
      if (callback) {
        callback(rule, scope, hostScope);
      }
    });
  }

  _calcElementScope(scope) {
    if (scope) {
      return CSS_CLASS_PREFIX + scope;
    } else {
      return '';
    }
  }

  _calcHostScope(scope, ext) {
    return ext ? `[is=${scope}]` : scope;
  }

  rule(rule, scope, hostScope) {
    this._transformRule(rule, this._transformComplexSelector,
      scope, hostScope);
  }

  /**
   * transforms a css rule to a scoped rule.
   *
   * @param {StyleNode} rule
   * @param {Function} transformer
   * @param {string=} scope
   * @param {string=} hostScope
   */
  _transformRule(rule, transformer, scope, hostScope) {
    // NOTE: save transformedSelector for subsequent matching of elements
    // against selectors (e.g. when calculating style properties)
    rule['selector'] = rule.transformedSelector =
      this._transformRuleCss(rule, transformer, scope, hostScope);
  }

  /**
   * @param {StyleNode} rule
   * @param {Function} transformer
   * @param {string=} scope
   * @param {string=} hostScope
   */
  _transformRuleCss(rule, transformer, scope, hostScope) {
    let p$ = StyleUtil.splitSelectorList(rule['selector']);
    // we want to skip transformation of rules that appear in keyframes,
    // because they are keyframe selectors, not element selectors.
    if (!StyleUtil.isKeyframesSelector(rule)) {
      for (let i=0, l=p$.length, p; (i<l) && (p=p$[i]); i++) {
        p$[i] = transformer.call(this, p, scope, hostScope);
      }
    }
    return p$.filter((part) => Boolean(part)).join(COMPLEX_SELECTOR_SEP);
  }

  /**
   * @param {string} selector
   * @return {string}
   */
  _twiddleNthPlus(selector) {
    return selector.replace(NTH, (m, type, inside) => {
      if (inside.indexOf('+') > -1) {
        inside = inside.replace(/\+/g, '___');
      } else if (inside.indexOf('___') > -1) {
        inside = inside.replace(/___/g, '+');
      }
      return `:${type}(${inside})`;
    });
  }

  /**
   * Preserve `:matches()` selectors by replacing them with MATCHES_REPLACMENT
   * and returning an array of `:matches()` selectors.
   * Use `_replacesMatchesPseudo` to replace the `:matches()` parts
   *
   * @param {string} selector
   * @return {{selector: string, matches: !Array<string>}}
   */
  _preserveMatchesPseudo(selector) {
    /** @type {!Array<string>} */
    const matches = [];
    let match;
    while ((match = selector.match(MATCHES))) {
      const start = match.index;
      const end = StyleUtil.findMatchingParen(selector, start);
      if (end === -1) {
        throw new Error(`${match.input} selector missing ')'`)
      }
      const part = selector.slice(start, end + 1);
      selector = selector.replace(part, MATCHES_REPLACEMENT);
      matches.push(part);
    }
    return {selector, matches};
  }

  /**
   * Replace MATCHES_REPLACMENT character with the given set of `:matches()`
   * selectors.
   *
   * @param {string} selector
   * @param {!Array<string>} matches
   * @return {string}
   */
  _replaceMatchesPseudo(selector, matches) {
    const parts = selector.split(MATCHES_REPLACEMENT);
    return matches.reduce((acc, cur, idx) => acc + cur + parts[idx + 1], parts[0]);
  }

/**
 * @param {string} selector
 * @param {string} scope
 * @param {string=} hostScope
 */
  _transformComplexSelector(selector, scope, hostScope) {
    let stop = false;
    selector = selector.trim();
    // Remove spaces inside of selectors like `:nth-of-type` because it confuses SIMPLE_SELECTOR_SEP
    let isNth = NTH.test(selector);
    if (isNth) {
      selector = selector.replace(NTH, (m, type, inner) => `:${type}(${inner.replace(/\s/g, '')})`)
      selector = this._twiddleNthPlus(selector);
    }
    // Preserve selectors like `:-webkit-any` so that SIMPLE_SELECTOR_SEP does
    // not get confused by spaces inside the pseudo selector
    const isMatches = MATCHES.test(selector);
    /** @type {!Array<string>} */
    let matches;
    if (isMatches) {
      ({selector, matches} = this._preserveMatchesPseudo(selector));
    }
    selector = selector.replace(SLOTTED_START, `${HOST} $1`);
    selector = selector.replace(SIMPLE_SELECTOR_SEP, (m, c, s) => {
      if (!stop) {
        let info = this._transformCompoundSelector(s, c, scope, hostScope);
        stop = stop || info.stop;
        c = info.combinator;
        s = info.value;
      }
      return c + s;
    });
    // replace `:matches()` selectors
    if (isMatches) {
      selector = this._replaceMatchesPseudo(selector, matches);
    }
    if (isNth) {
      selector = this._twiddleNthPlus(selector);
    }
    return selector;
  }

  _transformCompoundSelector(selector, combinator, scope, hostScope) {
    // replace :host with host scoping class
    let slottedIndex = selector.indexOf(SLOTTED);
    if (selector.indexOf(HOST) >= 0) {
      selector = this._transformHostSelector(selector, hostScope);
    // replace other selectors with scoping class
    } else if (slottedIndex !== 0) {
      selector = scope ? this._transformSimpleSelector(selector, scope) :
        selector;
    }
    // mark ::slotted() scope jump to replace with descendant selector + arg
    // also ignore left-side combinator
    let slotted = false;
    if (slottedIndex >= 0) {
      combinator = '';
      slotted = true;
    }
    // process scope jumping selectors up to the scope jump and then stop
    let stop;
    if (slotted) {
      stop = true;
      if (slotted) {
        // .zonk ::slotted(.foo) -> .zonk.scope > .foo
        selector = selector.replace(SLOTTED_PAREN, (m, paren) => ` > ${paren}`);
      }
    }
    selector = selector.replace(DIR_PAREN, (m, before, dir) =>
      `[dir="${dir}"] ${before}, ${before}[dir="${dir}"]`);
    return {value: selector, combinator, stop};
  }

  _transformSimpleSelector(selector, scope) {
    const attributes = selector.split(/(\[.+?\])/);

    const output = [];
    for (let i = 0; i < attributes.length; i++) {
      // Do not attempt to transform any attribute selector content
      if ((i % 2) === 1) {
        output.push(attributes[i]);
      } else {
        const part = attributes[i];

        if (!(part === '' && i === attributes.length - 1)) {
          let p$ = part.split(PSEUDO_PREFIX);
          p$[0] += scope;
          output.push(p$.join(PSEUDO_PREFIX));
        }
      }
    }

    return output.join('');
  }

  // :host(...) -> scopeName...
  _transformHostSelector(selector, hostScope) {
    let m = selector.match(HOST_PAREN);
    let paren = m && m[2].trim() || '';
    if (paren) {
      if (!paren[0].match(SIMPLE_SELECTOR_PREFIX)) {
        // paren starts with a type selector
        let typeSelector = paren.split(SIMPLE_SELECTOR_PREFIX)[0];
        // if the type selector is our hostScope then avoid pre-pending it
        if (typeSelector === hostScope) {
          return paren;
        // otherwise, this selector should not match in this scope so
        // output a bogus selector.
        } else {
          return SELECTOR_NO_MATCH;
        }
      } else {
        // make sure to do a replace here to catch selectors like:
        // `:host(.foo)::before`
        return selector.replace(HOST_PAREN, function(m, host, paren) {
          return hostScope + paren;
        });
      }
    // if no paren, do a straight :host replacement.
    // TODO(sorvell): this should not strictly be necessary but
    // it's needed to maintain support for `:host[foo]` type selectors
    // which have been improperly used under Shady DOM. This should be
    // deprecated.
    } else {
      return selector.replace(HOST, hostScope);
    }
  }

  /**
   * @param {StyleNode} rule
   */
  documentRule(rule) {
    // reset selector in case this is redone.
    rule['selector'] = rule['parsedSelector'];
    this.normalizeRootSelector(rule);
    this._transformRule(rule, this._transformDocumentSelector);
  }

  /**
   * @param {StyleNode} rule
   */
  normalizeRootSelector(rule) {
    if (rule['selector'] === ROOT) {
      rule['selector'] = 'html';
    }
  }

/**
 * @param {string} selector
 */
  _transformDocumentSelector(selector) {
    if (selector.match(HOST)) {
      // remove ':host' type selectors in document rules
      return '';
    } else if (selector.match(SLOTTED)) {
      return this._transformComplexSelector(selector, SCOPE_DOC_SELECTOR)
    } else {
      return this._transformSimpleSelector(selector.trim(), SCOPE_DOC_SELECTOR);
    }
  }
}

const NTH = /:(nth[-\w]+)\(([^)]+)\)/;
const SCOPE_DOC_SELECTOR = `:not(.${SCOPE_NAME})`;
const COMPLEX_SELECTOR_SEP = ',';
const SIMPLE_SELECTOR_SEP = /(^|[\s>+~]+)((?:\[.+?\]|[^\s>+~=[])+)/g;
const SIMPLE_SELECTOR_PREFIX = /[[.:#*]/;
const HOST = ':host';
const ROOT = ':root';
const SLOTTED = '::slotted';
const SLOTTED_START = new RegExp(`^(${SLOTTED})`);
// NOTE: this supports 1 nested () pair for things like
// :host(:not([selected]), more general support requires
// parsing which seems like overkill
const HOST_PAREN = /(:host)(?:\(((?:\([^)(]*\)|[^)(]*)+?)\))/;
// similar to HOST_PAREN
const SLOTTED_PAREN = /(?:::slotted)(?:\(((?:\([^)(]*\)|[^)(]*)+?)\))/;
const DIR_PAREN = /(.*):dir\((?:(ltr|rtl))\)/;
const CSS_CLASS_PREFIX = '.';
const PSEUDO_PREFIX = ':';
const CLASS = 'class';
const SELECTOR_NO_MATCH = 'should_not_match';
const MATCHES = /:(?:matches|any|-(?:webkit|moz)-any)/;
const MATCHES_REPLACEMENT = '\u{e000}';

export default new StyleTransformer()
