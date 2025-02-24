// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';
/* eslint-disable no-console */

// Handles the interaction with the Chrome Debugging Protocol. Allows to start,
// stop and abort tracing and return back the trace JSON as a stream.
// The evolution of the internal state machine, in absence of errors, is the
// following:
// IDLE > ATTACHING > TRACING > STOPPING > FLUSHING > IDLE
//
// STATE_IDLE:      waiting for a start() request.
// STATE_ATTACHING: got start(), attaching to the Chrome Debugging Protocol.
// STATE_TRACING:   attached and started tracing. Will stay in this state for
//                  |traceDurationMs| or until stop() or abort() are called.
// STATE_STOPPING:  timeout expired or stop() called. Ends tracing via CDP
//                  and waits for the events collection callback |onEvent_|.
// STATE_FLUSHING:  tracing ended but still attached to CDP. Reads back the
//                  event stream and passes it to the client via |onTraceChunk|.
class TracingController {
  static get ATTACH_TIMEOUT_MS() { return 3000; }
  static get FLUSH_TIMEOUT_MS() { return 2000000; }
  static get WAIT_FOR_DUMP_TIMEOUT_MS() { return 2000000; }
  static get STATE() {
    return {
      IDLE: 'idle',
      ATTACHING: 'attaching',
      TRACING: 'tracing',
      STOPPING: 'stopping',
      FLUSHING: 'flushing'
    };
  }

  constructor(traceDurationMs, categories) {
    this.traceDurationMs_ = traceDurationMs;
    this.categories_ = categories || '';
    this.onStateChange = undefined;
    this.onTraceChunk = undefined;
    this.onError = undefined;
    this.target_ = undefined;
    this.timer_ = undefined;
    this.events_ = [];
    this.trace_stream_handle_ = undefined;
    this.state_ = TracingController.STATE.IDLE;
    this.memoryDumpInProgress = false;
    this.waitForTraceInProgress = false;
    this.waitForDumpTimer = undefined;
    chrome.debugger.onEvent.addListener(this.onEvent_.bind(this));
    chrome.debugger.onDetach.addListener(this.onDetach_.bind(this));
  }

  start(useDebugFallback = false) {
    if (this.state_ !== TracingController.STATE.IDLE) {
      throw new Error('Cannot start in state ' + this.state_);
    }
    this.events_ = [];
    this.updateState_(TracingController.STATE.ATTACHING);
    this.timer_ = setTimeout(
        this.detach_.bind(this),
        TracingController.ATTACH_TIMEOUT_MS);

    // When using tracing via the debugger protocol, the specific page we attach
    // to is irrelevant (as long is not a worker or another special endpoint).
    // The most reliable choice is attaching to ourselves. The only gotcha is
    // that when debugging this extension, devtools attaches first to the
    // debugger protocol. In such case the onAttach_() method will fail and
    // the code there will fall back creating a background tab and attaching to
    // that one instead.
    if (useDebugFallback) {
      chrome.tabs.create({url: 'about:blank', active: false}, (function(tab) {
        this.target_ = { tabId: tab.id };
        chrome.debugger.attach(this.target_, '1.1', this.onAttach_.bind(this));
      }).bind(this));
      return;
    }
    this.target_ = { extensionId: chrome.runtime.id };
    chrome.debugger.attach(this.target_, '1.1', this.onAttach_.bind(this));
  }

  abort() {
    if (this.state_ === TracingController.STATE.IDLE) {
      return;
    }
    if (this.state_ === TracingController.STATE.TRACING) {
      this.stop();
    }
    this.detach_();
  }

  onWaitForTraceFinished() {
    if (this.state_ !== TracingController.STATE.TRACING) {
      return;
    }
    this.waitForTraceInProgress = false;
    clearTimeout(this.timer_);
    if (!this.memoryDumpInProgress) {
      this.stop();
    }
  }

  onMemoryDumpFinished(success, dumpGuid) {
    if (this.state_ !== TracingController.STATE.TRACING) {
      return;
    }
    this.memoryDumpInProgress = false;
    if (!this.waitForTraceInProgress) {
      this.stop();
    }
  }

  onWaitForDumpTimerFired() {
    if (this.state_ !== TracingController.STATE.TRACING) {
      return;
    }
    this.stop();
  }

  stop() {
    if (this.state_ !== TracingController.STATE.TRACING) {
      return;
    }
    clearTimeout(this.waitForDumpTimer);
    this.waitForDumpTimer = undefined;
    clearTimeout(this.timer_);
    this.timer_ = setTimeout(
        this.detachFromFlushTimeout_.bind(this),
        TracingController.FLUSH_TIMEOUT_MS);
    chrome.debugger.sendCommand(this.target_, 'Tracing.end');
    this.updateState_(TracingController.STATE.STOPPING);
  }

  onAttach_() {
    if (chrome.runtime.lastError !== undefined &&
        chrome.runtime.lastError.message.startsWith('Another debugger')) {
      console.log('Extension being debugged. Reattaching in fallback mode.');
      this.detach_();
      setTimeout(this.start.bind(this, true /* useDebugFallback */), 0);
      return;
    }
    this.checkForErrors_();
    const params =
        {categories: this.categories_, transferMode: 'ReturnAsStream'};
    chrome.debugger.sendCommand(
        this.target_,
        'Tracing.start',
        params,
        this.onTracingStarted_.bind(this));
  }

  onTracingStarted_() {
    this.checkForErrors_();
    clearTimeout(this.timer_);
    this.updateState_(TracingController.STATE.TRACING);

    // Wait for two events before stopping.
    //   1. Grab a single memory dump.
    //   2. Wait traceDurationMs_ to capture other trace events.
    this.memoryDumpInProgress = true;
    this.waitForTraceInProgress = true;
    this.waitForDumpTimer = setTimeout(
        this.onWaitForDumpTimerFired.bind(this),
        TracingController.WAIT_FOR_DUMP_TIMEOUT_MS);
    chrome.debugger.sendCommand(
        this.target_,
        'Tracing.requestMemoryDump',
        this.onMemoryDumpFinished.bind(this));

    this.timer_ = setTimeout(
        this.onWaitForTraceFinished.bind(this),
        this.traceDurationMs_);
  }

  detachFromFlushTimeout_() {
    chrome.extension.getBackgroundPage().console.log('Flush timeout');
    this.detach_();
  }

  detach_() {
    if (this.timer_) {
      clearTimeout(this.timer_);
      this.timer_ = undefined;
    }
    this.updateState_(TracingController.STATE.IDLE);
    chrome.debugger.detach(this.target_);
  }

  // Called if the user dismisses the "Chrome is being debugged" infobar.
  onDetach_() {
    this.updateState_(TracingController.STATE.IDLE);
  }

  onEvent_(source, method, params) {
    switch (method) {
      case 'Tracing.tracingComplete':
        this.updateState_(TracingController.STATE.FLUSHING);
        this.trace_stream_handle_ = params.stream;
        this.readBackNextChunk_();
        break;
    }
  }

  readBackNextChunk_() {
    const args = { handle: this.trace_stream_handle_, size: 128 * 1024 };
    chrome.debugger.sendCommand(this.target_, 'IO.read', args,
        this.onTraceReadback_.bind(this));
  }

  onTraceReadback_(params) {
    // Weird, sometimes |params| is undefined.
    if (params === undefined) {
      return;
    }

    if (this.onTraceChunk !== undefined) {
      this.onTraceChunk(params.data, params.eof);
    }

    if (params.eof) {
      clearTimeout(this.timer_);
      this.detach_();
      return;
    }

    setTimeout(this.readBackNextChunk_.bind(this), 0);
  }

  updateState_(state) {
    this.state_ = state;
    chrome.extension.getBackgroundPage().console.log(
        'TracingController state: ' + state);
    if (this.onStateChange !== undefined) {
      this.onStateChange(state);
    }
  }

  checkForErrors_(errCondition, errMsg) {
    const lastError = chrome.runtime.lastError;
    errCondition = errCondition || (lastError !== undefined);
    if (!errCondition) {
      return;
    }
    errMsg = errMsg || (lastError ? lastError.message : 'unknown error');
    if (this.onError !== undefined) {
      this.onError(errMsg);
    }
    this.detach_();
    throw new Error(errMsg);
  }

  get traceDurationMs() { return this.traceDurationMs_; }
}
