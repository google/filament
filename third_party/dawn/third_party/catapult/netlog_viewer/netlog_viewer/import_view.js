// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * This view displays options for importing data from a log file.
 */
var ImportView = (function() {
  'use strict';

  // We inherit from DivView.
  var superClass = DivView;

  /**
   * @constructor
   */
  function ImportView() {
    assertFirstConstructorCall(ImportView);

    // Call superclass's constructor.
    superClass.call(this, ImportView.MAIN_BOX_ID);

    this.loadedDiv_ = $(ImportView.LOADED_DIV_ID);

    this.loadFileElement_ = $(ImportView.LOAD_LOG_FILE_ID);
    this.loadFileElement_.onchange = this.logFileChanged.bind(this);
    this.loadStatusText_ = $(ImportView.LOAD_STATUS_TEXT_ID);

    var dropTarget = document.body;
    dropTarget.ondragenter = this.onDrag.bind(this);
    dropTarget.ondragover = this.onDrag.bind(this);
    dropTarget.ondrop = this.onDrop.bind(this);
  }

  ImportView.TAB_ID = 'tab-handle-import';
  ImportView.TAB_NAME = 'Import';
  ImportView.TAB_HASH = '#import';

  // IDs for special HTML elements in import_view.html.
  ImportView.MAIN_BOX_ID = 'import-view-tab-content';
  ImportView.LOADED_DIV_ID = 'import-view-loaded-div';
  ImportView.LOAD_LOG_FILE_ID = 'import-view-load-log-file';
  ImportView.LOAD_STATUS_TEXT_ID = 'import-view-load-status-text';

  // IDs for HTML elements pertaining to log dump information.
  ImportView.LOADED_INFO_NUMERIC_DATE_ID = 'import-view-numericDate';
  ImportView.LOADED_INFO_CAPTURE_MODE_ID = 'import-view-capture-mode';
  ImportView.LOADED_INFO_NAME_ID = 'import-view-name';
  ImportView.LOADED_INFO_VERSION_ID = 'import-view-version';
  ImportView.LOADED_INFO_OFFICIAL_ID = 'import-view-official';
  ImportView.LOADED_INFO_CL_ID = 'import-view-cl';
  ImportView.LOADED_INFO_VERSION_MOD_ID = 'import-view-version-mod';
  ImportView.LOADED_INFO_OS_TYPE_ID = 'import-view-os-type';
  ImportView.LOADED_INFO_COMMAND_LINE_ID = 'import-view-command-line';
  ImportView.LOADED_INFO_ACTIVE_FIELD_TRIAL_GROUPS_ID =
      'import-view-activeFieldTrialGroups';
  ImportView.LOADED_INFO_ROW_ADDED_FIELD_TRIAL_GROUPS_ID =
      'import-view-row-addedFieldTrialGroups';
  ImportView.LOADED_INFO_ADDED_FIELD_TRIAL_GROUPS_ID =
      'import-view-addedFieldTrialGroups';
  ImportView.LOADED_INFO_ROW_REMOVED_FIELD_TRIAL_GROUPS_ID =
      'import-view-row-removedFieldTrialGroups';
  ImportView.LOADED_INFO_REMOVED_FIELD_TRIAL_GROUPS_ID =
      'import-view-removedFieldTrialGroups';
  ImportView.LOADED_INFO_ROW_USER_COMMENTS_ID = 'import-view-row-user-comments';
  ImportView.LOADED_INFO_USER_COMMENTS_ID = 'import-view-user-comments';

  cr.addSingletonGetter(ImportView);

  ImportView.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    /**
     * Called when a log file is loaded, after clearing the old log entries and
     * loading the new ones.  Returns true to indicate the view should
     * still be visible.
     */
    onLoadLogFinish(polledData, unused, logDump) {
      $(ImportView.LOADED_INFO_NUMERIC_DATE_ID).textContent =
          timeutil.dateToString(new Date(Constants.clientInfo.numericDate));
      $(ImportView.LOADED_INFO_CAPTURE_MODE_ID).textContent =
          Constants.logCaptureMode || '(Not Recorded)';
      $(ImportView.LOADED_INFO_NAME_ID).textContent =
          Constants.clientInfo.name;
      $(ImportView.LOADED_INFO_VERSION_ID).textContent =
          Constants.clientInfo.version;
      $(ImportView.LOADED_INFO_OFFICIAL_ID).textContent =
          Constants.clientInfo.official;
      $(ImportView.LOADED_INFO_CL_ID).textContent =
          Constants.clientInfo.cl;
      $(ImportView.LOADED_INFO_VERSION_MOD_ID).textContent =
          Constants.clientInfo.version_mod;
      $(ImportView.LOADED_INFO_OS_TYPE_ID).textContent =
          Constants.clientInfo.os_type;
      $(ImportView.LOADED_INFO_COMMAND_LINE_ID).textContent =
          Constants.clientInfo.command_line;
      this.displayTrials_(Constants.activeFieldTrialGroups,
                          polledData.activeFieldTrialGroups);
      this.displayUserComments_(logDump.userComments);
      setNodeDisplay(this.loadedDiv_, true);
      return true;
    },

    /**
     * Display the active Field Trials and Trials activated or deactivated
     * during the capture (if possible).
     */
    displayTrials_(trialsAtStart, trialsAtEnd) {
      if (trialsAtStart && trialsAtStart.constructor !== Array) {
        trialsAtStart = undefined;
      }
      if (trialsAtEnd && trialsAtEnd.constructor !== Array) {
        trialsAtEnd = undefined;
      }

      // Display the Active Trials, preferring the list of Trials Active at the
      // end of the capture, if available. |trialsAtEnd| is absent from netlogs
      // captured before Chrome 88, and logs captured using the command-line
      // --log-net-log command (crbug.com/739487).
      $(ImportView.LOADED_INFO_ACTIVE_FIELD_TRIAL_GROUPS_ID).textContent =
          (trialsAtEnd || trialsAtStart || []).join(' ');

      // Calculate and display the lists of Trials activated or deactivated
      // during the capture.
      const rowAdded =
          $(ImportView.LOADED_INFO_ROW_ADDED_FIELD_TRIAL_GROUPS_ID);
      const rowRemoved =
          $(ImportView.LOADED_INFO_ROW_REMOVED_FIELD_TRIAL_GROUPS_ID);

      // Without Field Trial lists recorded at both the start and the end of the
      // capture, we cannot calculate the diff.
      if (!trialsAtStart || !trialsAtEnd) {
        setNodeDisplay(rowAdded, false);
        setNodeDisplay(rowRemoved, false);
        return;
      }

      // Helper to render the Diff between two lists of Trials.
      function showDiff(listFirst, listSecond, rowDiff, spanDiff) {
        const setSecond = new Set(listSecond);
        const listDiff = listFirst.filter(item => !setSecond.has(item));
        spanDiff.textContent = listDiff.join(' ');
        setNodeDisplay(rowDiff, listDiff.length > 0);
      }

      // Display Field Trials activated during the capture.
      showDiff(trialsAtEnd, trialsAtStart, rowAdded,
               $(ImportView.LOADED_INFO_ADDED_FIELD_TRIAL_GROUPS_ID));
      // Display Field Trials deactivated during the capture.
      showDiff(trialsAtStart, trialsAtEnd, rowRemoved,
               $(ImportView.LOADED_INFO_REMOVED_FIELD_TRIAL_GROUPS_ID));
    },

    /**
     * Display User Comments (if any).
     */
    displayUserComments_(comments) {
      $(ImportView.LOADED_INFO_USER_COMMENTS_ID).textContent = comments || '';
      setNodeDisplay($(ImportView.LOADED_INFO_ROW_USER_COMMENTS_ID),
                     !!comments);
    },

    /**
     * Prevent default browser behavior when a file is dragged over the page to
     * allow our onDrop() handler to handle the drop.
     */
    onDrag(event) {
      if (event.dataTransfer.types.includes('Files')) {
        event.preventDefault();
      }
    },

    /**
     * If a single file is dropped, try to load it as a log file.
     */
    onDrop(event) {
      if (event.dataTransfer.files.length !== 1) {
        return;
      }

      event.preventDefault();

      // Loading a log file may hide the currently active tab.  Switch to the
      // import tab to prevent this.
      document.location.hash = 'import';

      this.loadLogFile(event.dataTransfer.files[0]);
    },

    /**
     * Called when a log file is selected.
     *
     * Gets the log file from the input element and tries to read from it.
     */
    logFileChanged() { this.loadLogFile(this.loadFileElement_.files[0]); },

    /**
     * Attempts to read from the File |logFile|.
     */
    loadLogFile(logFile) {
      if (logFile) {
        this.setLoadFileStatus('Loading log...', true);
        var fileReader = new FileReader();
        fileReader.onerror = this.onLoadLogFileError.bind(this);

        if (logFile.name.toLowerCase().endsWith('.zip')) {
          fileReader.onload = this.onLoadZip.bind(this, logFile);
          fileReader.readAsArrayBuffer(logFile);
        } else {
          fileReader.onload = this.onLoadLogFile.bind(this, logFile);
          fileReader.readAsText(logFile);
        }
      }
    },

    /**
     * Opens a ZIP and finds the first *.json file within, then attempts to
     * decompress the JSON content and parse it as a netlog.
     */
    onLoadZip(logFile, data) {
      const zip = new JSZip();
      zip.loadAsync(data.target.result)
          .then(zip => {
            for (const filename in zip.files) {
              if (!filename.toLowerCase().endsWith('.json')) {
                console.log('Netlog Import skipping: ' + filename);
                continue;
              }

              zip.files[filename].async('string').then(content => {
                var result = LogUtil.loadLogFile(content, logFile.name +
                                                              '::' + filename);
                this.setLoadFileStatus(result, false);
              });
              // We only attempt to parse one log.
              return;
            }
            // If we made it this far, we did not find any JSON.
            throw new Error('The ZIP file did not contain a netlog.json file.');
          })
          .catch(e => {
            console.log('ZIP load failed: ' + e.message);
            this.loadFileElement_.value = null;
            this.setLoadFileStatus(
                'Error: Unable to find json inside the ZIP file.', false);
          });
    },

    /**
     * Displays an error message when unable to read the selected log file.
     * Also clears the file input control, so the same file can be reloaded.
     */
    onLoadLogFileError(event) {
      this.loadFileElement_.value = null;
      this.setLoadFileStatus(
          'Error ' + getKeyWithValue(FileError, event.target.error.code) +
              '.  Unable to read file.',
          false);
    },

    onLoadLogFile(logFile, event) {
      var result = LogUtil.loadLogFile(event.target.result, logFile.name);
      this.setLoadFileStatus(result, false);
    },

    /**
     * Sets the load from file status text, displayed below the load file
     * button, to |text|. Also enables or disables the load buttons based on
     * the value of |isLoading|, which must be true if the load process is still
     * ongoing, and false when the operation has stopped, regardless of success
     * of failure. Also, when loading is done, clears the file input control, so
     * the same file can be reloaded.
     */
    setLoadFileStatus(text, isLoading) {
      this.enableLoadFileElement_(!isLoading);
      this.loadStatusText_.textContent = text;

      if (!isLoading) {
        // Clear the input, so the same file can be reloaded.
        this.loadFileElement_.value = null;
      }

      // Style the log output differently depending on what just happened.
      const pos = text.indexOf('Log loaded.');
      if (isLoading) {
        this.loadStatusText_.className = 'import-view-pending-log';
      } else if (pos == 0) {
        this.loadStatusText_.className = 'import-view-success-log';
      } else if (pos != -1) {
        this.loadStatusText_.className = 'import-view-warning-log';
      } else {
        this.loadStatusText_.className = 'import-view-error-log';
      }
    },

    enableLoadFileElement_(enabled) {
      this.loadFileElement_.disabled = !enabled;
    },
  };

  return ImportView;
})();

