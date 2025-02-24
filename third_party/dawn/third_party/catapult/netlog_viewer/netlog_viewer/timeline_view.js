// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * TimelineView displays a zoomable and scrollable graph of a number of values
 * over time.  The TimelineView class itself is responsible primarily for
 * updating the TimelineDataSeries its GraphView displays.
 */
var TimelineView = (function() {
  'use strict';

  // We inherit from DivView.
  var superClass = DivView;

  /**
   * @constructor
   */
  function TimelineView() {
    assertFirstConstructorCall(TimelineView);

    // TODO(eroman): Shouldn't be naming a node ID.
    superClass.call(this, 'timeline-view-host');

    this.graphView_ = new TimelineGraphView(
        TimelineView.GRAPH_DIV_ID, TimelineView.GRAPH_CANVAS_ID,
        TimelineView.SCROLLBAR_DIV_ID, TimelineView.SCROLLBAR_INNER_DIV_ID);

    // Call superclass's constructor.

    var selectionView = new DivView(TimelineView.SELECTION_DIV_ID);

    $(TimelineView.SELECTION_TOGGLE_ID).onclick =
        this.toggleSelectionDiv_.bind(this);

    // Interval id returned by window.setInterval for update timer.
    this.updateIntervalId_ = null;

    // List of DataSeries.  These are shared with the TimelineGraphView.  The
    // TimelineView updates their state, the TimelineGraphView reads their
    // state and draws them.
    this.dataSeries_ = [];

    // DataSeries depend on some of the global constants, so they're only
    // created once constants have been received.  We also use this message to
    // recreate DataSeries when log files are being loaded.
    g_browser.addConstantsObserver(this);

    // We observe new log entries to determine the range of the graph, and pass
    // them on to each DataSource.  We initialize the graph range to initially
    // include all events, but after that, we only update it to be the current
    // time on a timer.
    EventsTracker.getInstance().addLogEntryObserver(this);
    this.graphRangeInitialized_ = false;
  }

  TimelineView.TAB_ID = 'tab-handle-timeline';
  TimelineView.TAB_NAME = 'Timeline';
  TimelineView.TAB_HASH = '#timeline';

  // IDs for special HTML elements in timeline_view.html
  TimelineView.GRAPH_DIV_ID = 'timeline-view-graph-div';
  TimelineView.GRAPH_CANVAS_ID = 'timeline-view-graph-canvas';
  TimelineView.SELECTION_DIV_ID = 'timeline-view-selection-div';
  TimelineView.SELECTION_TOGGLE_ID = 'timeline-view-selection-toggle';
  TimelineView.SELECTION_UL_ID = 'timeline-view-selection-ul';
  TimelineView.SCROLLBAR_DIV_ID = 'timeline-view-scrollbar-div';
  TimelineView.SCROLLBAR_INNER_DIV_ID = 'timeline-view-scrollbar-inner-div';

  TimelineView.OPEN_SOCKETS_ID = 'timeline-view-open-sockets';
  TimelineView.IN_USE_SOCKETS_ID = 'timeline-view-in-use-sockets';
  TimelineView.URL_REQUESTS_ID = 'timeline-view-url-requests';
  TimelineView.DNS_JOBS_ID = 'timeline-view-dns-jobs';
  TimelineView.BYTES_RECEIVED_ID = 'timeline-view-bytes-received';
  TimelineView.BYTES_SENT_ID = 'timeline-view-bytes-sent';
  TimelineView.DISK_CACHE_BYTES_READ_ID = 'timeline-view-disk-cache-bytes-read';
  TimelineView.DISK_CACHE_BYTES_WRITTEN_ID =
      'timeline-view-disk-cache-bytes-written';

  // Class used for hiding the colored squares next to the labels for the
  // lines.
  TimelineView.HIDDEN_CLASS = 'timeline-view-hidden';

  cr.addSingletonGetter(TimelineView);

  // Frequency with which we increase update the end date to be the current
  // time, when actively capturing events.
  var UPDATE_INTERVAL_MS = 2000;

  TimelineView.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    show: function(isVisible) {
      superClass.prototype.show.call(this, isVisible);

      this.graphView_.show(isVisible);

      // Live capture is no longer supported, so don't need to constantly
      // update the graph's range.
      this.setUpdateEndDateInterval_(0);
    },

    /**
     * Starts calling the GraphView's updateEndDate function every |intervalMs|
     * milliseconds.  If |intervalMs| is 0, stops calling the function.
     */
    setUpdateEndDateInterval_: function(intervalMs) {
      if (this.updateIntervalId_ !== null) {
        window.clearInterval(this.updateIntervalId_);
        this.updateIntervalId_ = null;
      }
      if (intervalMs > 0) {
        this.updateIntervalId_ =
            window.setInterval(this.updateEndDate_.bind(this), intervalMs);
      }
    },

    /**
     * Updates the end date of graph to be the current time, unless the
     * BrowserBridge is disabled.
     */
    updateEndDate_: function() {
      // If we loaded a log file or capturing data was stopped, stop the timer.
      if (g_browser.isDisabled()) {
        this.setUpdateEndDateInterval_(0);
        return;
      }
      this.graphView_.updateEndDate();
    },

    onLoadLogFinish: function(data) {
      this.setUpdateEndDateInterval_(0);
      return true;
    },

    /**
     * Updates the visibility state of |dataSeries| to correspond to the
     * current checked state of |checkBox|.  Also updates the class of
     * |listItem| based on the new visibility state.
     */
    updateDataSeriesVisibility_: function(dataSeries, listItem, checkBox) {
      dataSeries.show(checkBox.checked);
      if (checkBox.checked)
        listItem.classList.remove(TimelineView.HIDDEN_CLASS);
      else
        listItem.classList.add(TimelineView.HIDDEN_CLASS);
    },

    dataSeriesClicked_: function(dataSeries, listItem, checkBox) {
      this.updateDataSeriesVisibility_(dataSeries, listItem, checkBox);
      this.graphView_.repaint();
    },

    /**
     * Adds the specified DataSeries to |dataSeries_|, and hooks up
     * |listItemId|'s checkbox and color to correspond to the current state
     * of the given DataSeries.
     */
    addDataSeries_: function(dataSeries, listItemId) {
      this.dataSeries_.push(dataSeries);
      var listItem = $(listItemId);
      var checkBox = $(listItemId).querySelector('input');

      // Make sure |listItem| is visible, and then use its color for the
      // DataSource.
      listItem.classList.remove(TimelineView.HIDDEN_CLASS);
      dataSeries.setColor(getComputedStyle(listItem).color);

      this.updateDataSeriesVisibility_(dataSeries, listItem, checkBox);
      checkBox.onclick =
          this.dataSeriesClicked_.bind(this, dataSeries, listItem, checkBox);
    },

    /**
     * Recreate all DataSeries.  Global constants must have been set before
     * this is called.
     */
    createDataSeries_: function() {
      this.graphRangeInitialized_ = false;
      this.dataSeries_ = [];

      this.addDataSeries_(
          new SourceCountDataSeries(
              EventSourceType.SOCKET, EventType.SOCKET_ALIVE),
          TimelineView.OPEN_SOCKETS_ID);

      this.addDataSeries_(
          new SocketsInUseDataSeries(), TimelineView.IN_USE_SOCKETS_ID);

      this.addDataSeries_(
          new SourceCountDataSeries(
              EventSourceType.URL_REQUEST, EventType.REQUEST_ALIVE),
          TimelineView.URL_REQUESTS_ID);

      this.addDataSeries_(
          new SourceCountDataSeries(
              EventSourceType.HOST_RESOLVER_IMPL_JOB,
              EventType.HOST_RESOLVER_IMPL_JOB),
          TimelineView.DNS_JOBS_ID);

      this.addDataSeries_(
          new NetworkTransferRateDataSeries(
              EventType.SOCKET_BYTES_RECEIVED, EventType.UDP_BYTES_RECEIVED),
          TimelineView.BYTES_RECEIVED_ID);

      this.addDataSeries_(
          new NetworkTransferRateDataSeries(
              EventType.SOCKET_BYTES_SENT, EventType.UDP_BYTES_SENT),
          TimelineView.BYTES_SENT_ID);

      this.addDataSeries_(
          new DiskCacheTransferRateDataSeries(EventType.ENTRY_READ_DATA),
          TimelineView.DISK_CACHE_BYTES_READ_ID);

      this.addDataSeries_(
          new DiskCacheTransferRateDataSeries(EventType.ENTRY_WRITE_DATA),
          TimelineView.DISK_CACHE_BYTES_WRITTEN_ID);

      this.graphView_.setDataSeries(this.dataSeries_);
    },

    /**
     * When we receive the constants, create or recreate the DataSeries.
     */
    onReceivedConstants: function(constants) {
      this.createDataSeries_();
    },

    /**
     * When all log entries are deleted, recreate the DataSeries.
     */
    onAllLogEntriesDeleted: function() {
      this.graphRangeInitialized_ = false;
      this.createDataSeries_();
    },

    onReceivedLogEntries: function(entries) {
      // Pass each entry to every DataSeries, one at a time.  Not having each
      // data series get data directly from the EventsTracker saves us from
      // having very un-Javascript-like destructors for when we load new,
      // constants and slightly simplifies DataSeries objects.
      for (var entry = 0; entry < entries.length; ++entry) {
        for (var i = 0; i < this.dataSeries_.length; ++i)
          this.dataSeries_[i].onReceivedLogEntry(entries[entry]);
      }

      // If this is the first non-empty set of entries we've received, or we're
      // viewing a loaded log file, we will need to update the date range.
      if (this.graphRangeInitialized_ && !MainView.isViewingLoadedLog())
        return;
      if (entries.length == 0)
        return;

      // Update the date range.
      var startDate;
      if (!this.graphRangeInitialized_) {
        startDate = timeutil.convertTimeTicksToDate(entries[0].time);
      } else {
        startDate = this.graphView_.getStartDate();
      }
      var endDate =
          timeutil.convertTimeTicksToDate(entries[entries.length - 1].time);
      this.graphView_.setDateRange(startDate, endDate);
      this.graphRangeInitialized_ = true;
    },

    toggleSelectionDiv_: function() {
      var toggle = $(TimelineView.SELECTION_TOGGLE_ID);
      var shouldCollapse = toggle.className == 'timeline-view-rotateleft';

      setNodeDisplay($(TimelineView.SELECTION_UL_ID), !shouldCollapse);
      toggle.className = shouldCollapse ? 'timeline-view-rotateright' :
                                          'timeline-view-rotateleft';

      // Notify the graph view that it may need to update the canvas size. This
      // isn's strictly necessary since that view will also poll to see when a
      // nresize has happened, but this makes the update happen right away.
      //
      // TODO(eroman): Shouldn't be calling a private method on graphView_.
      this.graphView_.checkForResize_();
    }
  };

  return TimelineView;
})();

