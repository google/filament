// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

class TraceUploader {
  static get BUCKET() { return 'trace-on-tap'; }

  constructor(traceData) {
    if (!traceData) throw new Error('Empty traceData');
    this.onUploadComplete = undefined;
    this.onError = undefined;
    this.xhr_ = undefined;
    this.traceData_ = traceData;
  }

  start() {
    const objName = (new Date()).toISOString() + '-' +
                    TraceUploader.getChromePlatform() + '-' +
                    TraceUploader.getChromeVersion() + '.json.gz';
    const url = 'https://www.googleapis.com/upload/storage/v1/b/' +
                 TraceUploader.BUCKET + '/o?uploadType=media&name=' + objName;
    this.xhr_ = new XMLHttpRequest();
    this.xhr_.addEventListener('load', this.onXHRLoad_.bind(this));
    this.xhr_.addEventListener('error', this.onXHRError_.bind(this));
    this.xhr_.open('POST', url, true);
    this.xhr_.setRequestHeader('Content-Type', 'application/x-gzip');
    this.xhr_.send(this.traceData_);
  }

  onXHRLoad_(evt) {
    if (this.xhr_.readyState === 4 && this.xhr_.status === 200) {
      try {
        const resp = JSON.parse(this.xhr_.response);
        const url = 'https://storage.cloud.google.com/' + TraceUploader.BUCKET +
                     '/' + resp.id.split('/')[1] + '?authuser=1';
        if (this.onUploadComplete !== undefined) {
          this.onUploadComplete(url);
        }
      } catch (e) {
        if (this.onError !== undefined) {
          this.onError('Invalid response: ' + e);
        }
      }
      return;
    }
    if (this.onError !== undefined) {
      this.onError('HTTP ' + this.xhr_.statusText + ': ' +
                   this.xhr_.responseText);
    }
  }

  onXHRError_(evt) {
    if (this.onError !== undefined) {
      this.onError('XHR Error');
    }
  }

  static getChromeVersion() {
    const match = navigator.userAgent.match(/Chrom(?:e|ium)\/([0-9\.]+)/);
    return match ? match[1] : null;
  }

  static getChromePlatform() {
    if (/\Macintosh\b/.test(navigator.userAgent)) {
      return 'Chrome_Mac';
    }
    if (/\bCrOS\b/.test(navigator.userAgent)) {
      return 'Chrome_ChromeOS';
    }
    if (/\bX11\b/.test(navigator.userAgent)) {
      return 'Chrome_Linux';
    }
    if (/\bAndroid\b/.test(navigator.userAgent)) {
      return 'Chrome_Android';
    }
    return 'Chrome';
  }
}
