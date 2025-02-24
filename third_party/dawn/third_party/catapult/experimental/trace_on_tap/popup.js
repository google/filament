// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

class Popup {
  constructor() {
    this.errorUi_ = document.getElementById('error');
    this.statusUi_ = document.getElementById('trace_status');
    this.traceSizeUi_ = document.getElementById('trace_size');
    this.actionsUi_ = document.getElementById('tracing_complete');
    this.downloadLink_ = document.getElementById('trace_download');
    this.uploadLink_ = document.getElementById('trace_upload');
    this.uploadLink_.addEventListener('click', this.uploadTrace_.bind(this));
    this.stopBtn_ = document.getElementById('stop_btn');
    this.stopBtn_.addEventListener('click', this.stopCapture_.bind(this));
    document.documentElement.style.cursor = 'default';
    this.port_ = chrome.runtime.connect();
    this.port_.onMessage.addListener(this.onMessage_.bind(this));
    this.progressUi_ = document.getElementById('tracing_in_progress');
    this.progressUi_.style.display = 'block';
    this.autoUpload_ = document.getElementById('autoupload');
    this.autoUpload_.checked = Boolean(localStorage.getItem('autoupload'));
    this.autoUpload_.addEventListener(
        'change', this.onAutoUploadChange_.bind(this));
  }

  captureTrace() {
    this.port_.postMessage('start');
  }

  stopCapture_() {
    this.port_.postMessage('stop');
  }

  uploadTrace_() {
    this.uploadLink_.innerHTML = 'Uploading ...';
    document.documentElement.style.cursor = 'wait';
    this.port_.postMessage('upload');
  }

  onMessage_(msg, sender, sendReponse) {
    switch (msg.type) {
      case 'progress.animate':
        this.progressAnimation_ = true;
        this.animateProgress_(Date.now(), msg.arg);
        break;
      case 'progress.set':
        this.progressAnimation_ = false;
        this.setTraceProgress_(msg.arg);
        break;
      case 'status':
        this.statusUi_.innerText = msg.arg;
        break;
      case 'error':
        this.errorUi_.innerText = msg.arg;
        this.errorUi_.style.display = 'block';
        break;
      case 'result':
        this.progressUi_.style.display = 'none';
        this.actionsUi_.style.display = 'block';
        this.downloadLink_.href = msg.arg;
        this.downloadLink_.download = 'trace.json.gz';
        this.traceSizeUi_.innerText = ~~(msg.size / 10000) / 100;
        if (this.autoUpload_.checked) {
          this.uploadTrace_();
        }
        break;
    }
  }

  setTraceProgress_(perc) {
    const gradient = ('linear-gradient(to right, #8BC34A ' + perc +
                      '%, #fff ' + perc + '%)');
    this.statusUi_.style.backgroundImage = gradient;
  }

  animateProgress_(startTime, duration) {
    if (!this.progressAnimation_) {
      return;
    }
    const elapsedMs = Date.now() - startTime;
    const progress = elapsedMs / duration;
    this.setTraceProgress_(~~(90 * progress));
    if (elapsedMs < duration) {
      requestAnimationFrame(
          this.animateProgress_.bind(this, startTime, duration));
    } else {
      this.progressAnimation_ = false;
    }
  }

  onAutoUploadChange_() {
    if (this.autoUpload_.checked) {
      localStorage.setItem('autoupload', 1);
    } else {
      localStorage.removeItem('autoupload');
    }
  }
}

let popup_ = undefined;
window.addEventListener('load', function() {
  popup_ = popup_ || new Popup();
  popup_.captureTrace();
});
