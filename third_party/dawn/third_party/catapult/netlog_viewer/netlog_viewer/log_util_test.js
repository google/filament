// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview This file contains tests for creating and loading log files.
 * It also tests the stop capturing button, since it both creates and then loads
 * a log dump.
 */

// Include test fixture.
GEN_INCLUDE(['net_internals_test.js']);

// Anonymous namespace
(function() {

/**
 * A Task that creates a log dump and then loads it.
 * @param {string} userComments User comments to use for the generated log.
 * @extends {NetInternalsTest.Task}
 */
function CreateAndLoadLogTask(userComments) {
  this.userComments_ = userComments;
  NetInternalsTest.Task.call(this);
  // Not strictly necessary, but since we complete from a call to the observer,
  // seems reasonable to complete asynchronously.
  this.setCompleteAsync(true);
}

CreateAndLoadLogTask.prototype = {
  __proto__: NetInternalsTest.Task.prototype,

  /**
   * Starts creating the log dump.
   */
  start: function() {
    LogUtil.createLogDumpAsync(
        this.userComments_, this.onLogDumpCreated.bind(this), true);
  },

  /**
   * Callback passed to |createLogDumpAsync|.  Tries to load the dumped log
   * text.  Checks the status bar and user comments after loading the log.
   * @param {string} logDumpText Log dump, as a string.
   */
  onLogDumpCreated: function(logDumpText) {
    expectEquals('Log loaded.', LogUtil.loadLogFile(logDumpText, 'log.txt'));

    NetInternalsTest.expectStatusViewNodeVisible(LoadedStatusView.MAIN_BOX_ID);

    // Make sure the DIV on the import tab containing the comments is visible
    // before checking the displayed text.
    expectTrue(NetInternalsTest.nodeIsVisible($(ImportView.LOADED_DIV_ID)));
    expectEquals(
        this.userComments_,
        $(ImportView.LOADED_INFO_USER_COMMENTS_ID).innerText);

    this.onTaskDone();
  }
};

/**
 * A Task that waits until constants are received, completing asynchronously
 * once they are.
 * @extends {NetInternalsTest.Task}
 */
function WaitForConstantsTask() {
  NetInternalsTest.Task.call(this);
  this.setCompleteAsync(true);
}

WaitForConstantsTask.prototype = {
  __proto__: NetInternalsTest.Task.prototype,

  /**
   * Starts watching for constants.
   */
  start: function() {
    g_browser.addConstantsObserver(this);
  },

  /**
   * Resumes the test once we receive constants.
   */
  onReceivedConstants: function() {
    if (!this.isDone())
      this.onTaskDone();
  }
};

/**
 * A Task that creates a log dump in the browser process, waits to receive it
 * via IPC, and and then loads it as a string.
 * @param {integer} truncate The number of bytes to truncate from the end of
 *     the string, if any, to simulate a truncated log due to crash, or
 *     quitting without properly shutting down the log writer.
 * @extends {NetInternalsTest.Task}
 */
function GetNetLogFileContentsAndLoadLogTask(truncate) {
  NetInternalsTest.Task.call(this);
  this.setCompleteAsync(true);
  this.truncate_ = truncate;
}

GetNetLogFileContentsAndLoadLogTask.prototype = {
  __proto__: NetInternalsTest.Task.prototype,

  /**
   * Sets |NetInternals.callback|, and sends the request to the browser process.
   */
  start: function() {
    NetInternalsTest.setCallback(this.onLogReceived_.bind(this));
    chrome.send('getNetLogFileContents');
  },

  /**
   * Loads the received log and completes the Task.
   * @param {string} logDumpText Log received from the browser process.
   */
  onLogReceived_: function(logDumpText) {
    assertEquals('string', typeof logDumpText);

    var expectedResult = 'Log loaded.';

    if (this.truncate_) {
      expectedResult =
          'Log file truncated.  Events may be missing.\n' + expectedResult;
    }

    logDumpText = logDumpText.substring(0, logDumpText.length - this.truncate_);
    expectEquals(expectedResult, LogUtil.loadLogFile(logDumpText, 'log.txt'));

    NetInternalsTest.expectStatusViewNodeVisible(LoadedStatusView.MAIN_BOX_ID);

    // Make sure the DIV on the import tab containing the comments is visible
    // before checking the displayed text.
    expectTrue(NetInternalsTest.nodeIsVisible($(ImportView.LOADED_DIV_ID)));
    expectEquals('', $(ImportView.LOADED_INFO_USER_COMMENTS_ID).innerText);

    this.onTaskDone();
  }
};

/**
 * Checks the visibility of each view after loading a freshly created log dump.
 * Also checks that the BrowserBridge is disabled.
 */
function checkViewsAfterLogLoaded() {
  var tabVisibilityState = {
    capture: false,
    import: true,
    proxy: true,
    events: true,
    timeline: true,
    dns: true,
    sockets: true,
    http2: true,
    quic: true,
    reporting: true,
    'alt-svc': true,
    httpCache: true,
    modules: true,
    prerender: true,
    bandwidth: true,
  };
  NetInternalsTest.checkTabLinkVisibility(tabVisibilityState, false);
}

/**
 * Checks the visibility of each view after loading a log dump created by the
 * browser. Also checks that the BrowserBridge is disabled.
 */
function checkViewsAfterNetLogFileLoaded() {
  var tabVisibilityState = {
    capture: false,
    import: true,
    proxy: false,
    events: true,
    timeline: true,
    dns: false,
    sockets: false,
    http2: false,
    quic: false,
    reporting: false,
    'alt-svc': false,
    httpCache: false,
    modules: false,
    prerender: false,
  };
  NetInternalsTest.checkTabLinkVisibility(tabVisibilityState, false);
}

/**
 * Checks the currently active view.
 * @param {string} id ID of the view that should be active.
 */
function checkActiveView(id) {
  expectEquals(id, NetInternalsTest.getActiveTabId());
}

/**
 * Exports a log dump to a string and loads it.  Makes sure no errors occur,
 * and checks visibility of tabs aftwards.  Does not actually save the log to a
 * file.
 * TODO(mmenke):  Add some checks for the import view.
 */
TEST_F('NetInternalsTest', 'netInternalsLogUtilExportImport', function() {
  NetInternalsTest.expectStatusViewNodeVisible(CaptureStatusView.MAIN_BOX_ID);

  var taskQueue = new NetInternalsTest.TaskQueue(true);
  taskQueue.addTask(new CreateAndLoadLogTask('Detailed explanation.'));
  taskQueue.addFunctionTask(checkViewsAfterLogLoaded);
  taskQueue.run(true);
});

/**
 * Attempts to load a NetLog created by the browser. The log contents are
 * passed to Javascript via an IPC rather than drag and drop.
 */
TEST_F('NetInternalsTest', 'netInternalsLogUtilImportNetLogFile', function() {
  var taskQueue = new NetInternalsTest.TaskQueue(true);
  taskQueue.addTask(new GetNetLogFileContentsAndLoadLogTask(0));
  taskQueue.addFunctionTask(checkViewsAfterNetLogFileLoaded);
  taskQueue.run(true);
});

/**
 * Same as above, but it truncates the log to simulate the case of a crash when
 * creating a log.
 */
TEST_F(
    'NetInternalsTest', 'netInternalsLogUtilImportNetLogFileTruncated',
    function() {
      var taskQueue = new NetInternalsTest.TaskQueue(true);
      taskQueue.addTask(new GetNetLogFileContentsAndLoadLogTask(20));
      taskQueue.addFunctionTask(checkViewsAfterNetLogFileLoaded);
      taskQueue.run(true);
    });

/**
 * Checks pressing the stop capturing button.
 */
TEST_F('NetInternalsTest', 'netInternalsLogUtilStopCapturing', function() {
  var taskQueue = new NetInternalsTest.TaskQueue(true);
  // Switching to stop capturing mode will load a log dump, which will update
  // the constants.
  taskQueue.addTask(new WaitForConstantsTask());

  taskQueue.addFunctionTask(NetInternalsTest.expectStatusViewNodeVisible.bind(
      null, HaltedStatusView.MAIN_BOX_ID));
  taskQueue.addFunctionTask(checkViewsAfterLogLoaded);
  taskQueue.addFunctionTask(checkActiveView.bind(null, EventsView.TAB_ID));
  taskQueue.run();

  // Simulate a click on the stop capturing button
  $(CaptureView.STOP_BUTTON_ID).click();
});

})();  // Anonymous namespace
