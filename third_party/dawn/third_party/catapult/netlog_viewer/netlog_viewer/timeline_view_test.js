// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

// Include test fixture.
GEN_INCLUDE(['net_internals_test.js']);

// Anonymous namespace
(function() {
  // Range of time set on a log once loaded used by sanity checks.
  // Used by sanityCheckWithTimeRange.
  let startTime = null;
  let endTime = null;

  function timelineView() {
    return TimelineView.getInstance();
  }

  function graphView() {
    return timelineView().graphView_;
  }

  function scrollbar() {
    return graphView().scrollbar_;
  }

  function canvas() {
    return graphView().canvas_;
  }

  /**
   * A Task that creates a log dump, modifies it so |timeTicks| are all in UTC,
   * clears all events from the log, and then adds two new SOCKET events, which
   * have the specified start and end times.
   *
   * Most of these tests start with this task first.  This gives us a known
   * starting state, and prevents the data from automatically updating.
   *
   * @param {int} startTime Time of the begin event.
   * @param {int} endTime Time of the end event.
   * @extends {NetInternalsTest.Task}
   */
  function LoadLogWithNewEventsTask(startTime, endTime) {
    NetInternalsTest.Task.call(this);
    this.startTime_ = startTime;
    this.endTime_ = endTime;
  }

  LoadLogWithNewEventsTask.prototype = {
    __proto__: NetInternalsTest.Task.prototype,

    /**
     * Starts creating a log dump.
     */
    start() {
      LogUtil.createLogDumpAsync(
          'test', this.onLogDumpCreated.bind(this), true);
    },

    /**
     * Modifies the log dump and loads it.
     */
    onLogDumpCreated(logDumpText) {
      const logDump = JSON.parse(logDumpText);

      logDump.constants.timeTickOffset = 0;
      logDump.events = [];

      const source = new NetInternalsTest.Source(1, EventSourceType.SOCKET);
      logDump.events.push(NetInternalsTest.createBeginEvent(
          source, EventType.SOCKET_ALIVE, this.startTime_, null));
      logDump.events.push(NetInternalsTest.createMatchingEndEvent(
          logDump.events[0], this.endTime_, null));
      logDumpText = JSON.stringify(logDump);

      assertEquals('Log loaded.', LogUtil.loadLogFile(logDumpText));

      endTime = this.endTime_;
      startTime = this.startTime_;
      if (startTime >= endTime) {
        --startTime;
      }

      sanityCheckWithTimeRange(false);

      this.onTaskDone();
    }
  };

  /**
   * Checks certain invariant properties of the TimelineGraphView and the
   * scroll bar.
   */
  function sanityCheck() {
    expectLT(graphView().startTime_, graphView().endTime_);
    expectLE(0, scrollbar().getPosition());
    expectLE(scrollbar().getPosition(), scrollbar().getRange());
  }

  /**
   * Checks what sanityCheck does, but also checks that |startTime| and
   * |endTime| are the same as those used by the graph, as well as whether we
   * have a timer running to update the graph's end time.  To avoid flake, this
   * should only be used synchronously relative to when |startTime| and
   * |endTime| were set, unless |expectUpdateTimer| is false.
   * @param {bool} expectUpdateTimer true if the TimelineView should currently
   *     have an update end time timer running.
   */
  function sanityCheckWithTimeRange(expectUpdateTimer) {
    if (!expectUpdateTimer) {
      expectEquals(null, timelineView().updateIntervalId_);
    } else {
      expectNotEquals(null, timelineView().updateIntervalId_);
    }
    assertNotEquals(startTime, null);
    assertNotEquals(endTime, null);
    expectEquals(startTime, graphView().startTime_);
    expectEquals(endTime, graphView().endTime_);
    sanityCheck(false);
  }

  /**
   * Checks what sanityCheck does, but also checks that |startTime| and
   * |endTime| are the same as those used by the graph.
   */
  function sanityCheckNotUpdating() {
    expectEquals(null, timelineView().updateIntervalId_);
    sanityCheckWithTimeRange();
  }

  /**
   * Simulates mouse wheel movement over the canvas element.
   * @param {number} ticks Number of mouse wheel ticks to simulate.
   */
  function mouseZoom(ticks) {
    const scrollbarStartedAtEnd =
        (scrollbar().getRange() === scrollbar().getPosition());

    const event = new WheelEvent('mousewheel', {deltaX: 0, deltaY: -ticks});
    canvas().dispatchEvent(event);

    // If the scrollbar started at the end of the range, make sure it ends there
    // as well.
    if (scrollbarStartedAtEnd) {
      expectEquals(scrollbar().getRange(), scrollbar().getPosition());
    }

    sanityCheck();
  }

  /**
   * Simulates moving the mouse wheel up.
   * @param {number} ticks Number of mouse wheel ticks to simulate.
   */
  function mouseZoomIn(ticks) {
    assertGT(ticks, 0);
    const oldScale = graphView().scale_;
    const oldRange = scrollbar().getRange();

    mouseZoom(ticks);

    if (oldScale === graphView().scale_) {
      expectEquals(oldScale, TimelineGraphView.MIN_SCALE);
    } else {
      expectLT(graphView().scale_, oldScale);
    }
    expectGE(scrollbar().getRange(), oldRange);
  }

  /**
   * Simulates moving the mouse wheel down.
   * @param {number} ticks Number of mouse wheel ticks to simulate.
   */
  function mouseZoomOut(ticks) {
    assertGT(ticks, 0);
    const oldScale = graphView().scale_;
    const oldRange = scrollbar().getRange();

    mouseZoom(-ticks);

    expectGT(graphView().scale_, oldScale);
    expectLE(scrollbar().getRange(), oldRange);
  }

  /**
   * Simulates zooming all the way with multiple mouse wheel events.
   */
  function mouseZoomAllTheWayIn() {
    expectLT(TimelineGraphView.MIN_SCALE, graphView().scale_);
    while (graphView().scale_ !== TimelineGraphView.MIN_SCALE) {
      mouseZoomIn(8);
    }
    // Verify that zooming in when already at max zoom works.
    mouseZoomIn(1);
  }

  /**
   * A Task that scrolls the scrollbar by manipulating the DOM, and then waits
   * for the scroll to complete.  Has to be a task because onscroll and DOM
   * manipulations both occur asynchronously.
   *
   * Not safe to use when other asynchronously running code may try to
   * manipulate the scrollbar itself, or adjust the length of the scrollbar.
   *
   * @param {int} position Position to scroll to.
   * @extends {NetInternalsTest.Task}
   */
  function MouseScrollTask(position) {
    NetInternalsTest.Task.call(this);
    this.position_ = position;
    // If the scrollbar's |position| and its node's |scrollLeft| values don't
    // currently match, we set this to true and wait for |scrollLeft| to be
    // updated, which will trigger an onscroll event.
    this.waitingToStart_ = false;
  }

  MouseScrollTask.prototype = {
    __proto__: NetInternalsTest.Task.prototype,

    start() {
      this.waitingToStart_ = false;
      // If the scrollbar is already in the correct position, do nothing.
      if (scrollbar().getNode().scrollLeft === this.position_) {
        // We may still have a timer going to adjust the position of the
        // scrollbar to some other value.  If so, this will clear it.
        scrollbar().setPosition(this.position_);
        this.onTaskDone();
        return;
      }

      // Replace the onscroll event handler with our own.
      this.oldOnScroll_ = scrollbar().getNode().onscroll;
      scrollbar().getNode().onscroll = this.onScroll_.bind(this);
      if (scrollbar().getNode().scrollLeft !== scrollbar().getPosition()) {
        this.waitingToStart_ = true;
        return;
      }

      window.setTimeout(this.startScrolling_.bind(this), 0);
    },

    onScroll_(event) {
      // Restore the original onscroll function.
      scrollbar().getNode().onscroll = this.oldOnScroll_;
      // Call the original onscroll function.
      this.oldOnScroll_(event);

      if (this.waitingToStart_) {
        this.start();
        return;
      }

      assertEquals(this.position_, scrollbar().getNode().scrollLeft);
      assertEquals(this.position_, scrollbar().getPosition());

      sanityCheck();
      this.onTaskDone();
    },

    startScrolling_() {
      scrollbar().getNode().scrollLeft = this.position_;
    }
  };

  /**
   * Tests setting and updating range.
   */
  TEST_F('NetInternalsTest', 'netInternalsTimelineViewRange', function() {
    NetInternalsTest.switchToView('timeline');

    // Set startTime/endTime for sanity checks.
    startTime = graphView().startTime_;
    endTime = graphView().endTime_;
    sanityCheckWithTimeRange(true);

    startTime = 0;
    endTime = 10;
    graphView().setDateRange(new Date(startTime), new Date(endTime));
    sanityCheckWithTimeRange(true);

    endTime = (new Date()).getTime();
    graphView().updateEndDate();

    expectGE(graphView().endTime_, endTime);
    sanityCheck();

    testDone();
  });

  /**
   * Tests using the scroll bar.
   */
  TEST_F('NetInternalsTest', 'netInternalsTimelineViewScrollbar', function() {
    // The range we want the graph to have.
    const expectedGraphRange = canvas().width;

    function checkGraphRange() {
      expectEquals(expectedGraphRange, scrollbar().getRange());
    }

    const taskQueue = new NetInternalsTest.TaskQueue(true);
    // Load a log and then switch to the timeline view.  The end time is
    // calculated so that the range is exactly |expectedGraphRange|.
    taskQueue.addTask(new LoadLogWithNewEventsTask(
        55, 55 + graphView().scale_ * (canvas().width + expectedGraphRange)));
    taskQueue.addFunctionTask(
        NetInternalsTest.switchToView.bind(null, 'timeline'));
    taskQueue.addFunctionTask(checkGraphRange);

    taskQueue.addTask(new MouseScrollTask(0));
    taskQueue.addTask(new MouseScrollTask(expectedGraphRange));
    taskQueue.addTask(new MouseScrollTask(1));
    taskQueue.addTask(new MouseScrollTask(expectedGraphRange - 1));

    taskQueue.addFunctionTask(checkGraphRange);
    taskQueue.addFunctionTask(sanityCheckWithTimeRange.bind(null, false));
    taskQueue.run();
  });

  /**
   * Dumps a log file to memory, modifies its events, loads it again, and
   * makes sure the range is correctly set and not automatically updated.
   */
  TEST_F('NetInternalsTest', 'netInternalsTimelineViewLoadLog', function() {
    // After loading the log file, the rest of the test runs synchronously.
    function testBody() {
      NetInternalsTest.switchToView('timeline');
      sanityCheckWithTimeRange(false);

      // Make sure everything's still fine when we switch to another view.
      NetInternalsTest.switchToView('events');
      sanityCheckWithTimeRange(false);
    }

    // Load a log and then run the rest of the test.
    const taskQueue = new NetInternalsTest.TaskQueue(true);
    taskQueue.addTask(new LoadLogWithNewEventsTask(55, 10055));
    taskQueue.addFunctionTask(testBody);
    taskQueue.run();
  });

  /**
   * Zooms out twice, and then zooms in once.
   */
  TEST_F('NetInternalsTest', 'netInternalsTimelineViewZoomOut', function() {
    // After loading the log file, the rest of the test runs synchronously.
    function testBody() {
      NetInternalsTest.switchToView('timeline');
      mouseZoomOut(1);
      mouseZoomOut(1);
      mouseZoomIn(1);
      sanityCheckWithTimeRange(false);
    }

    // Load a log and then run the rest of the test.
    const taskQueue = new NetInternalsTest.TaskQueue(true);
    taskQueue.addTask(new LoadLogWithNewEventsTask(55, 10055));
    taskQueue.addFunctionTask(testBody);
    taskQueue.run();
  });

  /**
   * Zooms in as much as allowed, and zooms out once.
   */
  TEST_F('NetInternalsTest', 'netInternalsTimelineViewZoomIn', function() {
    // After loading the log file, the rest of the test runs synchronously.
    function testBody() {
      NetInternalsTest.switchToView('timeline');
      mouseZoomAllTheWayIn();
      mouseZoomOut(1);
      sanityCheckWithTimeRange(false);
    }

    // Load a log and then run the rest of the test.
    const taskQueue = new NetInternalsTest.TaskQueue(true);
    taskQueue.addTask(new LoadLogWithNewEventsTask(55, 10055));
    taskQueue.addFunctionTask(testBody);
    taskQueue.run();
  });

  /**
   * Tests case of all events having the same time.
   */
  TEST_F('NetInternalsTest', 'netInternalsTimelineViewDegenerate', function() {
    // After loading the log file, the rest of the test runs synchronously.
    function testBody() {
      NetInternalsTest.switchToView('timeline');
      mouseZoomOut(1);
      mouseZoomAllTheWayIn();
      mouseZoomOut(1);
      sanityCheckWithTimeRange(false);
    }

    // Load a log and then run the rest of the test.
    const taskQueue = new NetInternalsTest.TaskQueue(true);
    taskQueue.addTask(new LoadLogWithNewEventsTask(55, 55));
    taskQueue.addFunctionTask(testBody);
    taskQueue.run();
  });

  /**
   * Tests case of having no events.  Runs synchronously.
   */
  TEST_F('NetInternalsTest', 'netInternalsTimelineViewNoEvents', function() {
    // Click the button to clear all the captured events, and then switch to
    // timeline
    $(CaptureView.RESET_BUTTON_ID).click();
    NetInternalsTest.switchToView('timeline');

    // Set startTime/endTime for sanity checks.
    startTime = graphView().startTime_;
    endTime = graphView().endTime_;

    sanityCheckWithTimeRange(true);

    mouseZoomOut(1);
    sanityCheckWithTimeRange(true);

    mouseZoomAllTheWayIn();
    sanityCheckWithTimeRange(true);

    mouseZoomOut(1);
    sanityCheckWithTimeRange(true);

    testDone();
  });
})();  // Anonymous namespace
