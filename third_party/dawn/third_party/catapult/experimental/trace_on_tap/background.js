// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

class TraceOnTap {
  constructor() {
    this.tracingController_ = undefined;
    this.uiPort_ = undefined;
    this.deflater_ = undefined;
    this.traceStart_ = undefined;
    this.uploader_ = undefined;
    this.lastTraceData_ = undefined;

    chrome.runtime.onConnect.addListener(this.onUiConnect_.bind(this));

    this.tracingController_ = new TracingController(
        3000 /* duration */,
        'disabled-by-default-memory-infra' /* categories */);
    this.tracingController_.onStateChange = this.onTraceStateChange_.bind(this);
    this.tracingController_.onTraceChunk = this.onTraceChunk_.bind(this);
    this.tracingController_.onError = this.onError_.bind(this);
  }

  onUiConnect_(port) {
    this.uiPort_ = port;
    port.onMessage.addListener(this.onUiMessage_.bind(this));
    port.onDisconnect.addListener(this.onUiDisconnected_.bind(this));
  }

  onUiMessage_(msg, sender, sendReponse) {
    switch (msg) {
      case 'start':
        this.deflater_ = new pako.Deflate({gzip: true,
          header: {text: true, hcrc: true}});
        this.tracingController_.start();
        break;

      case 'stop':
        this.tracingController_.stop();
        break;

      case 'abort':
        this.tracingController_.abort();
        break;

      case 'upload':
        this.sendUiCmd_({type: 'status', arg: 'Uploading trace'});
        this.uploader_ = new TraceUploader(this.lastTraceData_);
        this.uploader_.onError = this.onError_.bind(this);
        this.uploader_.onUploadComplete = this.onUploadComplete_.bind(this);
        this.uploader_.start();
        break;
    }
  }

  onUploadComplete_(url) {
    this.sendUiCmd_({type: 'status', arg: 'Trace uploaded (' + url + ')'});
    let bugUrl = 'https://code.google.com/p/chromium/issues/entry?';
    bugUrl += 'labels=Type-Bug,Pri-2,Hotlist=Performance';
    bugUrl += '&summary=TraceOnTap:+[enter+description+here]';
    bugUrl += '&comment=Uploaded+via+go/TraceOnTap';
    bugUrl += '%0A%0A--%0ATrace+URL%3A+' + url;
    chrome.tabs.create({url: bugUrl});
  }

  onTraceStateChange_(state) {
    switch (state) {
      case TracingController.STATE.IDLE:
        this.sendUiCmd_({type: 'status', arg: 'Done'});
        this.sendUiCmd_({type: 'progress.set', arg: 0});
        break;
      case TracingController.STATE.TRACING:
        this.sendUiCmd_({type: 'status', arg: 'Capturing trace'});
        this.sendUiCmd_({type: 'progress.animate',
          arg: this.tracingController_.traceDurationMs});
        break;
      case TracingController.STATE.STOPPING:
        this.sendUiCmd_({type: 'status', arg: 'Finalizing trace'});
        this.sendUiCmd_({type: 'progress.set', arg: 90});
        break;
      case TracingController.STATE.FLUSHING:
        this.sendUiCmd_({type: 'status', arg: 'Compressing trace'});
        this.sendUiCmd_({type: 'progress.set', arg: 95});
        break;
      default:
        this.sendUiCmd_({type: 'status', arg: state});
    }
  }

  onTraceChunk_(data, isLastChunk) {
    this.deflater_.push(data, isLastChunk);
    if (!isLastChunk) return;

    this.lastTraceData_ = this.deflater_.result;
    this.deflater_ = undefined;
    const traceBlob = new Blob([this.lastTraceData_],
                               {type: 'application/x-gzip'});
    const traceUrl = URL.createObjectURL(traceBlob);
    this.sendUiCmd_({
      type: 'result',
      arg: traceUrl,
      size: this.lastTraceData_.length});
  }

  onError_(msg) {
    this.sendUiCmd_({type: 'progress.set', arg: 0});
    this.sendUiCmd_({type: 'error', arg: msg});
  }

  onUiDisconnected_() {
    this.tracingController_.abort();
  }

  sendUiCmd_(args) {
    if (this.uiPort_ === undefined) {
      return;
    }
    try {
      this.uiPort_.postMessage(args);
    } catch (ignored_) { }
  }
}

const traceOnTap_ = new TraceOnTap();
