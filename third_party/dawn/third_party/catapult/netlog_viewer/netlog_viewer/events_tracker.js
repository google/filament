// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var EventsTracker = (function() {
  'use strict';

  /**
   * This class keeps track of all NetLog events.
   * It receives events from the browser and when loading a log file, and passes
   * them on to all its observers.
   *
   * @constructor
   */
  function EventsTracker() {
    assertFirstConstructorCall(EventsTracker);

    this.capturedEvents_ = [];
    this.observers_ = [];

    // Controls how large |capturedEvents_| can grow.
    this.softLimit_ = Infinity;
    this.hardLimit_ = Infinity;
  }

  cr.addSingletonGetter(EventsTracker);

  EventsTracker.prototype = {
    /**
     * Returns a list of all captured events.
     */
    getAllCapturedEvents: function() {
      return this.capturedEvents_;
    },

    /**
     * Returns the number of events that were captured.
     */
    getNumCapturedEvents: function() {
      return this.capturedEvents_.length;
    },

    /**
     * Deletes all the tracked events, and notifies any observers.
     */
    deleteAllLogEntries: function() {
      timeutil.clearBaseTime();
      this.capturedEvents_ = [];
      for (var i = 0; i < this.observers_.length; ++i)
        this.observers_[i].onAllLogEntriesDeleted();
    },

    /**
     * Adds captured events, and broadcasts them to any observers.
     */
    addLogEntries: function(logEntries) {
      // When reloading a page, it's possible to receive events before
      // Constants.  Discard those events, as they can cause the fake
      // "REQUEST_ALIVE" events for pre-existing requests not be the first
      // events for those requests.
      if (Constants == null)
        return;
      // This can happen when loading logs with no events.
      if (!logEntries.length)
        return;

      if (!timeutil.isBaseTimeSet()) {
        timeutil.setBaseTime(
            timeutil.convertTimeTicksToTime(logEntries[0].time));
      }

      this.capturedEvents_ = this.capturedEvents_.concat(logEntries);
      for (var i = 0; i < this.observers_.length; ++i) {
        this.observers_[i].onReceivedLogEntries(logEntries);
      }

      // Check that we haven't grown too big. If so, toss out older events.
      if (this.getNumCapturedEvents() > this.hardLimit_) {
        var originalEvents = this.capturedEvents_;
        this.deleteAllLogEntries();
        // Delete the oldest events until we reach the soft limit.
        originalEvents.splice(0, originalEvents.length - this.softLimit_);
        this.addLogEntries(originalEvents);
      }
    },

    /**
     * Adds a listener of log entries. |observer| will be called back when new
     * log data arrives or all entries are deleted:
     *
     *   observer.onReceivedLogEntries(entries)
     *   observer.onAllLogEntriesDeleted()
     */
    addLogEntryObserver: function(observer) {
      this.observers_.push(observer);
    },

    /**
     * Set bounds on the maximum number of events that will be tracked. This
     * helps to bound the total amount of memory usage, since otherwise
     * long-running capture sessions can exhaust the renderer's memory and
     * crash.
     *
     * Once |hardLimit| number of events have been captured we do a garbage
     * collection and toss out old events, bringing our count down to
     * |softLimit|.
     *
     * To log observers this will look like all the events got deleted, and
     * then subsequently a bunch of new events were received. In other words, it
     * behaves the same as if the user had simply started logging a bit later
     * in time!
     */
    setLimits: function(softLimit, hardLimit) {
      if (hardLimit != Infinity && softLimit >= hardLimit)
        throw 'hardLimit must be greater than softLimit';

      this.softLimit_ = softLimit;
      this.hardLimit_ = hardLimit;
    }
  };

  return EventsTracker;
})();
