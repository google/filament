// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * The status view at the top of the page when viewing a loaded dump file.
 */
var LoadedStatusView = (function() {
  'use strict';

  // We inherit from DivView.
  var superClass = DivView;

  function LoadedStatusView() {
    superClass.call(this, LoadedStatusView.MAIN_BOX_ID);
  }

  LoadedStatusView.MAIN_BOX_ID = 'loaded-status-view';
  LoadedStatusView.DUMP_FILE_NAME_ID = 'loaded-status-view-dump-file-name';

  LoadedStatusView.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    setFileName: function(fileName) {
      $(LoadedStatusView.DUMP_FILE_NAME_ID).innerText = fileName;
      document.title = fileName + ' - NetLog Viewer';
    }
  };

  return LoadedStatusView;
})();
