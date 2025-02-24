// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file provides the PinchAction object, which zooms into or out of a
// page by a given scale factor:
//   1. var action = new __PinchAction(callback)
//   2. action.start(pinch_options)
'use strict';

(function() {
  function PinchGestureOptions(opt_options) {
    if (opt_options) {
      // The center of the pinch gesture, expressed as a ratio of the
      // application viewport (i.e. unaffected by pinch-zoom and scrolling).
      this.left_anchor_ratio_ = opt_options.left_anchor_ratio;
      this.top_anchor_ratio_ = opt_options.top_anchor_ratio;
      this.scale_factor_ = opt_options.scale_factor;
      // Speed of the pinch gesture in pixels per second. This is the speed of
      // the relative motion between the two touch points.
      this.speed_ = opt_options.speed;
      this.vsync_offset_ms_ = opt_options.vsync_offset_ms;
      this.input_event_pattern_ = opt_options.input_event_pattern;
    } else {
      this.left_anchor_ratio_ = 0.5;
      this.top_anchor_ratio_ = 0.5;
      this.scale_factor_ = 2.0;
      this.speed_ = 800;
      this.vsync_offset_ms_ = 0.0;
      this.input_event_pattern_ = chrome.gpuBenchmarking.DEFAULT_INPUT_PATTERN;
    }
  }

  function supportedByBrowser() {
    return !!(window.chrome &&
              chrome.gpuBenchmarking &&
              chrome.gpuBenchmarking.pinchBy &&
              chrome.gpuBenchmarking.visualViewportHeight &&
              chrome.gpuBenchmarking.visualViewportWidth);
  }

  // This class zooms into or out of a page, given a number of pixels for
  // the synthetic pinch gesture to cover.
  function PinchAction(opt_callback) {
    this.beginMeasuringHook = function() {};
    this.endMeasuringHook = function() {};

    this.callback_ = opt_callback;
  }

  PinchAction.prototype.start = function(opt_options) {
    this.options_ = new PinchGestureOptions(opt_options);

    requestAnimationFrame(this.startPass_.bind(this));
  };

  PinchAction.prototype.startPass_ = function() {
    this.beginMeasuringHook();

    const anchorLeft =
          __GestureCommon_GetWindowWidth() *
          this.options_.left_anchor_ratio_;
    const anchorTop =
          __GestureCommon_GetWindowHeight() *
          this.options_.top_anchor_ratio_;

    const kDefaultInput = 0;

    chrome.gpuBenchmarking.pinchBy(
        this.options_.scale_factor_, anchorLeft, anchorTop,
        this.onGestureComplete_.bind(this), this.options_.speed_,
        chrome.gpuBenchmarking.DEFAULT_INPUT, this.options_.vsync_offset_ms_,
        this.options_.input_event_pattern_);
  };

  PinchAction.prototype.onGestureComplete_ = function() {
    this.endMeasuringHook();

    if (this.callback_) {
      this.callback_();
    }
  };

  window.__PinchAction = PinchAction;
  window.__PinchAction_SupportedByBrowser = supportedByBrowser;
})();
