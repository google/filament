// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file provides the ScrollAction object, which scrolls a page
// to the bottom or for a specified distance:
//   1. var action = new __ScrollAction(callback, optDistanceFunc)
//   2. action.start(scroll_options)
'use strict';

(function() {
  const MAX_SCROLL_LENGTH_TIME_MS = 6250;

  function ScrollGestureOptions(opt_options) {
    if (opt_options) {
      this.element_ = opt_options.element;
      this.left_start_ratio_ = opt_options.left_start_ratio;
      this.top_start_ratio_ = opt_options.top_start_ratio;
      this.direction_ = opt_options.direction;
      this.speed_ = opt_options.speed;
      this.gesture_source_type_ = opt_options.gesture_source_type;
      this.vsync_offset_ms_ = opt_options.vsync_offset_ms;
      this.input_event_pattern_ = opt_options.input_event_pattern;
    } else {
      this.element_ = document.scrollingElement || document.body;
      this.left_start_ratio_ = 0.5;
      this.top_start_ratio_ = 0.5;
      this.direction_ = 'down';
      this.speed_ = 800;
      this.gesture_source_type_ = chrome.gpuBenchmarking.DEFAULT_INPUT;
      this.vsync_offset_ms_ = 0.0;
      this.input_event_pattern_ = chrome.gpuBenchmarking.DEFAULT_INPUT_PATTERN;
    }
  }

  function supportedByBrowser() {
    return !!(window.chrome &&
              chrome.gpuBenchmarking &&
              chrome.gpuBenchmarking.smoothScrollBy &&
              chrome.gpuBenchmarking.visualViewportHeight &&
              chrome.gpuBenchmarking.visualViewportWidth);
  }

  // This class scrolls a page from the top to the bottom once.
  //
  // The page is scrolled down by a single scroll gesture.
  function ScrollAction(optCallback, optDistanceFunc) {
    this.beginMeasuringHook = function() {};
    this.endMeasuringHook = function() {};

    this.callback_ = optCallback;
    this.distance_func_ = optDistanceFunc;
  }

  ScrollAction.prototype.isScrollingViewport_ = function() {
    const viewportElement = document.scrollingElement || document.body;
    if (!viewportElement) {
      return false;
    }

    return this.element_ === viewportElement;
  };

  ScrollAction.prototype.getScrollDistanceDown_ = function() {
    let clientHeight;
    let scrollTop;

    // clientHeight and scrollTop are "special" for the scrollingElement.
    if (this.isScrollingViewport_()) {
      if ('visualViewport' in window) {
        clientHeight = window.visualViewport.height;
        scrollTop = window.visualViewport.pageTop;
      } else {
        clientHeight = window.innerHeight;
        scrollTop = window.scrollY;
      }
    } else {
      clientHeight = this.element_.clientHeight;
      scrollTop = this.element_.scrollTop;
    }

    return this.element_.scrollHeight - scrollTop - clientHeight;
  };

  ScrollAction.prototype.getScrollDistanceUp_ = function() {
    if (this.isScrollingViewport_()) {
      if ('visualViewport' in window) {
        return window.visualViewport.pageTop;
      }

      return window.scrollY;
    }
    return this.element_.scrollTop;
  };

  ScrollAction.prototype.getScrollDistanceRight_ = function() {
    let clientWidth;
    let scrollLeft;

    // clientWidth and scrollLeft are "special" for the scrollingElement.
    if (this.isScrollingViewport_()) {
      if ('visualViewport' in window) {
        clientWidth = window.visualViewport.width;
        scrollLeft = window.visualViewport.pageLeft;
      } else {
        clientWidth = window.innerWidth;
        scrollLeft = window.scrollX;
      }
    } else {
      clientWidth = this.element_.clientWidth;
      scrollLeft = this.element_.scrollLeft;
    }

    return this.element_.scrollWidth - scrollLeft - clientWidth;
  };

  ScrollAction.prototype.getScrollDistanceLeft_ = function() {
    if (this.isScrollingViewport_()) {
      if ('visualViewport' in window) {
        return window.visualViewport.pageLeft;
      }

      return window.scrollX;
    }
    return this.element_.scrollLeft;
  };

  // The distance returned is in CSS pixels. i.e. Is not scaled by pinch-zoom.
  ScrollAction.prototype.getScrollDistance_ = function() {
    if (this.distance_func_) {
      return this.distance_func_();
    }

    if (this.options_.direction_ === 'down') {
      return this.getScrollDistanceDown_();
    } else if (this.options_.direction_ === 'up') {
      return this.getScrollDistanceUp_();
    } else if (this.options_.direction_ === 'right') {
      return this.getScrollDistanceRight_();
    } else if (this.options_.direction_ === 'left') {
      return this.getScrollDistanceLeft_();
    } else if (this.options_.direction_ === 'upleft') {
      return Math.min(
          this.getScrollDistanceUp_(),
          this.getScrollDistanceLeft_());
    } else if (this.options_.direction_ === 'upright') {
      return Math.min(
          this.getScrollDistanceUp_(),
          this.getScrollDistanceRight_());
    } else if (this.options_.direction_ === 'downleft') {
      return Math.min(
          this.getScrollDistanceDown_(),
          this.getScrollDistanceLeft_());
    } else if (this.options_.direction_ === 'downright') {
      return Math.min(
          this.getScrollDistanceDown_(),
          this.getScrollDistanceRight_());
    }
  };

  ScrollAction.prototype.start = function(opt_options) {
    this.options_ = new ScrollGestureOptions(opt_options);
    // Assign this.element_ here instead of constructor, because the constructor
    // ensures this method will be called after the document is loaded.
    this.element_ = this.options_.element_;
    requestAnimationFrame(this.startGesture_.bind(this));
  };

  ScrollAction.prototype.startGesture_ = function() {
    this.beginMeasuringHook();

    const maxScrollLengthPixels = (MAX_SCROLL_LENGTH_TIME_MS / 1000) *
        this.options_.speed_;
    const distance =
        Math.min(maxScrollLengthPixels, this.getScrollDistance_()) *
        __GestureCommon_GetPageScaleFactor();
    const speed = this.options_.speed_ * __GestureCommon_GetPageScaleFactor();

    const rect = __GestureCommon_GetBoundingVisibleRect(this.options_.element_);
    const startLeft =
        rect.left + rect.width * this.options_.left_start_ratio_;
    const startTop =
        rect.top + rect.height * this.options_.top_start_ratio_;
    chrome.gpuBenchmarking.smoothScrollBy(
        distance, this.onGestureComplete_.bind(this), startLeft, startTop,
        this.options_.gesture_source_type_, this.options_.direction_, speed,
        true /* precise_scrolling_deltas */, false /* scroll_by_page */,
        true /* cursor_visible */, false /* scroll_by_percentage */,
        '' /* keys_value */, this.options_.vsync_offset_ms_,
        this.options_.input_event_pattern_);
  };

  ScrollAction.prototype.onGestureComplete_ = function() {
    this.endMeasuringHook();

    // We're done.
    if (this.callback_) {
      this.callback_();
    }
  };

  window.__ScrollAction = ScrollAction;
  window.__ScrollAction_SupportedByBrowser = supportedByBrowser;
})();
