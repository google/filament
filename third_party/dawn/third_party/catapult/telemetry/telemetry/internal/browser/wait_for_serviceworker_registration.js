// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

/**
 * @fileoverview This file provides a JavaScript helper function that
 * determines whether service worker registration has completed.
 * TODO(crbug.com/768701): Move the ServiceWorker state operations to
 * InspectorServiceWorker by implementing _OnNotification().
 */
(function() {
  // Make executing this code idempotent.
  if (window.__telemetry_getServiceWorkerState) {
    return;
  }

  // These variables are used for checking service worker state.
  let isServiceWorkerAccessable = true;
  let isServiceWorkerRegistered = false;
  let isServiceWorkerActivated = false;

  window.__telemetry_getServiceWorkerState = () => {
    // These returned string should exactly match strings used in
    // ServiceWorkerState, in web_contents.py.
    if (isServiceWorkerActivated) { return 'activated'; }
    if (isServiceWorkerRegistered) { return 'installing'; }
    return 'not registered';
  };

  // Check if service worker is accessible.
  try {
    if (!navigator.serviceWorker) {
      isServiceWorkerAccessable = false;
    }
  } catch (e) {
    // Ignore the exception here. We will frequently meet it because
    // navigator.serviceWorker is not accessible when loading about:blank etc.
    isServiceWorkerAccessable = false;
  }

  if (!isServiceWorkerAccessable) { return; }

  // Patch navigator.serviceWorker.register().
  navigator.serviceWorker.originalRegister =
      navigator.serviceWorker.register;
  navigator.serviceWorker.register = (name, options = {}) => {
    isServiceWorkerRegistered = true;
    return navigator.serviceWorker.originalRegister(name, options).then(
        (registration) => {
          let serviceworker = null;
          // Service worker always have .active or .waiting or .installing.
          if (registration.active) {
            if (registration.active.state === 'activated') {
              isServiceWorkerActivated = true;
              return registration;
            }
            serviceworker = registration.active;
          } else if (registration.waiting) {
            serviceworker = registration.waiting;
          } else {
            serviceworker = registration.installing;
          }
          serviceworker.addEventListener('statechange', (event) => {
            if (serviceworker.state === 'activated') {
              isServiceWorkerActivated = true;
            } else if (serviceworker.state === 'redundant') {
              isServiceWorkerRegistered = false;
            }
          });
          return registration;
        },
        (error) => {
          isServiceWorkerRegistered = false;
          return Promise.reject(error);
        });
  };
})();
