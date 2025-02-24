// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file provides common functionality for synthetic gesture actions.
'use strict';

(function() {
  // Make sure functions are injected only once.
  if (window.__GestureCommon_GetBoundingVisibleRect) {
    return;
  }

  // Returns the bounding rectangle wrt to the layout viewport.
  function getBoundingRect(el) {
    const clientRect = el.getBoundingClientRect();
    const bound = {
      left: clientRect.left,
      top: clientRect.top,
      width: clientRect.width,
      height: clientRect.height
    };

    let frame = el.ownerDocument.defaultView.frameElement;
    while (frame) {
      const frameBound = frame.getBoundingClientRect();
      // This computation doesn't account for more complex CSS transforms on the
      // frame (e.g. scaling or rotations).
      bound.left += frameBound.left;
      bound.top += frameBound.top;

      frame = frame.ownerDocument.frameElement;
    }
    return bound;
  }

  // Chrome version before M50 doesn't have `pageScaleFactor` function, to run
  // benchmark on them we will need this function to fail back gracefully
  function getPageScaleFactor() {
    const pageScaleFactor = chrome.gpuBenchmarking.pageScaleFactor;
    return pageScaleFactor ? pageScaleFactor.apply(chrome.gpuBenchmarking) : 1;
  }

  // Get scrollable document height.
  function getScrollableHeight() {
    const body = document.body;
    const doc = document.documentElement;

    return Math.max(body.scrollHeight, body.offsetHeight,
        doc.clientHeight, doc.scrollHeight, doc.offsetHeight);
  }

  // Zoom-independent window height. See crbug.com/627123 for more details.
  function getWindowHeight() {
    return getPageScaleFactor() * chrome.gpuBenchmarking.visualViewportHeight();
  }

  // Zoom-independent window width. See crbug.com/627123 for more details.
  function getWindowWidth() {
    return getPageScaleFactor() * chrome.gpuBenchmarking.visualViewportWidth();
  }

  function clamp(min, value, max) {
    return Math.min(Math.max(min, value), max);
  }

  // Returns the bounding rect in the visual viewport's coordinates.
  function getBoundingVisibleRect(el) {
    // Get the element bounding rect in the layout viewport.
    const rect = getBoundingRect(el);

    // Apply the visual viewport transform (i.e. pinch-zoom) to the bounding
    // rect. The viewportX|Y values are in CSS pixels so they don't change
    // with page scale. We first translate so that the viewport offset is
    // at the origin and then we apply the scaling factor.
    const scale = getPageScaleFactor();
    const visualViewportX = chrome.gpuBenchmarking.visualViewportX();
    const visualViewportY = chrome.gpuBenchmarking.visualViewportY();
    rect.top = (rect.top - visualViewportY) * scale;
    rect.left = (rect.left - visualViewportX) * scale;
    rect.width *= scale;
    rect.height *= scale;

    // Get the window dimensions.
    const windowHeight = getWindowHeight();
    const windowWidth = getWindowWidth();

    // Then clip the rect to the screen size.
    rect.top = clamp(0, rect.top, windowHeight);
    rect.left = clamp(0, rect.left, windowWidth);
    rect.height = clamp(0, rect.height, windowHeight - rect.top);
    rect.width = clamp(0, rect.width, windowWidth - rect.left);

    return rect;
  }

  window.__GestureCommon_GetBoundingVisibleRect = getBoundingVisibleRect;
  window.__GestureCommon_GetWindowHeight = getWindowHeight;
  window.__GestureCommon_GetWindowWidth = getWindowWidth;
  window.__GestureCommon_GetPageScaleFactor = getPageScaleFactor;
  window.__GestureCommon_GetScrollableHeight = getScrollableHeight;
})();
