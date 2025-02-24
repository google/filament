/* Copyright 2018 The Chromium Authors. All rights reserved.
   Use of this source code is governed by a BSD-style license that can be
   found in the LICENSE file.

   TODO Remove this file after Polymer#1976 is fixed:
   https://github.com/Polymer/polymer/issues/1976
   https://github.com/garryyao/polymer-svg-template
*/
'use strict';
window.PolymerSvgTemplate = (nameOrContent, doc) => {
  const ua = window.navigator.userAgent;

  // owner document of this import module
  if (!doc) doc = document.currentScript.ownerDocument;
  const ns = doc.body.namespaceURI;

  walkTemplate((typeof nameOrContent === 'string') ?
      Polymer.DomModule.import(name, 'template').content : nameOrContent);

  function upgradeTemplate(el) {
    const attribs = el.attributes;
    const tmpl = el.ownerDocument.createElement('template');
    el.parentNode.insertBefore(tmpl, el);
    let count = attribs.length;
    while (count-- > 0) {
      const attrib = attribs[count];
      tmpl.setAttribute(attrib.name, attrib.value);
      el.removeAttribute(attrib.name);
    }
    el.parentNode.removeChild(el);
    const content = tmpl.content;
    let child;
    while (child = el.firstChild) {
      content.appendChild(child);
    }
    return tmpl;
  }

  function walkTemplate(root) {
    const treeWalker = doc.createTreeWalker(
        root, NodeFilter.SHOW_ELEMENT);
    while (treeWalker.nextNode()) {
      let node = treeWalker.currentNode;
      if (node.localName === 'svg') {
        walkTemplate(node);
      } else if (node.localName === 'template' &&
                 !node.hasAttribute('preserve-content') &&
                 node.namespaceURI !== ns) {
        node = upgradeTemplate(node);
        walkTemplate(node.content);
        treeWalker.currentNode = node;
      }
    }
  }
};
