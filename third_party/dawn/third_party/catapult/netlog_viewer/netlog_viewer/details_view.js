// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var DetailsView = (function() {
  'use strict';

  // We inherit from DivView.
  var superClass = DivView;

  /**
   * The DetailsView displays the "log" view. This class keeps track of the
   * selected SourceEntries, and repaints when they change.
   *
   * @constructor
   */
  function DetailsView(boxId) {
    superClass.call(this, boxId);
    this.sourceEntries_ = [];
    // Map of source IDs to their corresponding DIVs.
    this.sourceIdToDivMap_ = {};
    // True when there's an asychronous repaint outstanding.
    this.outstandingRepaint_ = false;
    // ID of source entry we should jump to after the oustanding repaint.
    // 0 if none, or there's no such repaint.
    this.outstandingScrollToId_ = 0;
  }

  // The delay between updates to repaint.
  var REPAINT_TIMEOUT_MS = 50;

  DetailsView.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    setData: function(sourceEntries) {
      // Make a copy of the array (in case the caller mutates it), and sort it
      // by the source ID.
      this.sourceEntries_ = createSortedCopy_(sourceEntries);

      // Repaint the view.
      if (this.isVisible() && !this.outstandingRepaint_) {
        this.outstandingRepaint_ = true;
        window.setTimeout(this.repaint.bind(this), REPAINT_TIMEOUT_MS);
      }
    },

    repaint: function() {
      this.outstandingRepaint_ = false;
      this.sourceIdToDivMap_ = {};
      this.getNode().innerHTML = '';

      var node = this.getNode();

      for (var i = 0; i < this.sourceEntries_.length; ++i) {
        if (i != 0)
          addNode(node, 'hr');

        var sourceEntry = this.sourceEntries_[i];
        var div = addNode(node, 'div');
        div.className = 'log-source-entry';

        var p = addNode(div, 'p');
        addNodeWithText(
            p, 'h4',
            sourceEntry.getSourceId() + ': ' +
                sourceEntry.getSourceTypeString());

        if (sourceEntry.getDescription())
          addNodeWithText(p, 'h4', sourceEntry.getDescription());

        const startDate =
            timeutil.convertTimeTicksToDate(sourceEntry.getStartTicks());
        var startTimeDiv = addNodeWithText(p, 'div', 'Start Time: ');
        timeutil.addNodeWithDate(startTimeDiv, startDate);

        sourceEntry.printAsText(div);

        this.sourceIdToDivMap_[sourceEntry.getSourceId()] = div;
      }

      if (this.outstandingScrollToId_) {
        this.scrollToSourceId(this.outstandingScrollToId_);
        this.outstandingScrollToId_ = 0;
      }
    },

    show: function(isVisible) {
      superClass.prototype.show.call(this, isVisible);
      if (isVisible) {
        this.repaint();
      } else {
        this.getNode().innerHTML = '';
      }
    },

    /**
     * Scrolls to the source indicated by |sourceId|, if displayed.  If a
     * repaint is outstanding, waits for it to complete before scrolling.
     */
    scrollToSourceId: function(sourceId) {
      if (this.outstandingRepaint_) {
        this.outstandingScrollToId_ = sourceId;
        return;
      }
      var div = this.sourceIdToDivMap_[sourceId];
      if (div)
        div.scrollIntoView();
    }
  };

  function createSortedCopy_(origArray) {
    var sortedArray = origArray.slice(0);
    sortedArray.sort(function(a, b) {
      return a.getSourceId() - b.getSourceId();
    });
    return sortedArray;
  }

  return DetailsView;
})();

