// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * EventsView displays a filtered list of all events sharing a source, and
 * a details pane for the selected sources.
 *
 *  +----------------------++----------------+
 *  |      filter box      ||                |
 *  +----------------------+|                |
 *  |                      ||                |
 *  |                      ||                |
 *  |                      ||                |
 *  |                      ||                |
 *  |     source list      ||    details     |
 *  |                      ||    view        |
 *  |                      ||                |
 *  |                      ||                |
 *  |                      ||                |
 *  |                      ||                |
 *  |                      ||                |
 *  |                      ||                |
 *  +----------------------++----------------+
 */
var EventsView = (function() {
  'use strict';

  // How soon after updating the filter list the counter should be updated.
  var REPAINT_FILTER_COUNTER_TIMEOUT_MS = 0;

  // We inherit from DivView.
  var superClass = DivView;

  /*
   * @constructor
   */
  function EventsView() {
    assertFirstConstructorCall(EventsView);

    // Call superclass's constructor.
    // TODO(eroman): Not host element.
    superClass.call(this, EventsView.LEFT_PANE_ID);

    // Initialize the sub-views.
    var leftPane = new DivView(EventsView.LEFT_PANE_ID);

    this.detailsView_ = new DetailsView(EventsView.DETAILS_LOG_BOX_ID);

    this.splitterView_ = new ResizableVerticalSplitView(
        leftPane, this.detailsView_, new DivView(EventsView.SIZER_ID));

    SourceTracker.getInstance().addSourceEntryObserver(this);

    this.tableBody_ = $(EventsView.TBODY_ID);

    this.filterInput_ = $(EventsView.FILTER_INPUT_ID);
    this.filterCount_ = $(EventsView.FILTER_COUNT_ID);

    this.filterInput_.addEventListener(
        'search', this.onFilterTextChanged_.bind(this), true);

    $(EventsView.SELECT_ALL_ID)
        .addEventListener('click', this.selectAll_.bind(this), true);

    $(EventsView.SORT_BY_ID_ID)
        .addEventListener('click', this.sortById_.bind(this), true);

    $(EventsView.SORT_BY_SOURCE_TYPE_ID)
        .addEventListener('click', this.sortBySourceType_.bind(this), true);

    $(EventsView.SORT_BY_DESCRIPTION_ID)
        .addEventListener('click', this.sortByDescription_.bind(this), true);

    new MouseOverHelp(
        EventsView.FILTER_HELP_ID, EventsView.FILTER_HELP_HOVER_ID);

    // Sets sort order and filter.
    this.setFilter_('');

    this.initializeSourceList_();
  }

  EventsView.TAB_ID = 'tab-handle-events';
  EventsView.TAB_NAME = 'Events';
  EventsView.TAB_HASH = '#events';

  // IDs for special HTML elements in events_view.html
  EventsView.TBODY_ID = 'events-view-source-list-tbody';
  EventsView.FILTER_INPUT_ID = 'events-view-filter-input';
  EventsView.FILTER_COUNT_ID = 'events-view-filter-count';
  EventsView.FILTER_HELP_ID = 'events-view-filter-help';
  EventsView.FILTER_HELP_HOVER_ID = 'events-view-filter-help-hover';
  EventsView.SELECT_ALL_ID = 'events-view-select-all';
  EventsView.SORT_BY_ID_ID = 'events-view-sort-by-id';
  EventsView.SORT_BY_SOURCE_TYPE_ID = 'events-view-sort-by-source';
  EventsView.SORT_BY_DESCRIPTION_ID = 'events-view-sort-by-description';
  EventsView.DETAILS_LOG_BOX_ID = 'events-view-details-log-box';
  EventsView.LEFT_PANE_ID = 'events-view-left-pane';
  EventsView.TOPBAR_ID = 'events-view-filter-box';
  EventsView.LIST_BOX_ID = 'events-view-source-list';
  EventsView.SIZER_ID = 'events-view-splitter-box';

  cr.addSingletonGetter(EventsView);

  EventsView.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    /**
     * Initializes the list of source entries.  If source entries are already,
     * being displayed, removes them all in the process.
     */
    initializeSourceList_: function() {
      this.currentSelectedRows_ = [];
      this.sourceIdToRowMap_ = {};
      this.tableBody_.innerHTML = '';
      this.numPrefilter_ = 0;
      this.numPostfilter_ = 0;
      this.invalidateFilterCounter_();
      this.invalidateDetailsView_();
    },

    show: function(isVisible) {
      superClass.prototype.show.call(this, isVisible);
      this.splitterView_.show(isVisible);
    },

    getFilterText_: function() {
      return this.filterInput_.value;
    },

    setFilterText_: function(filterText) {
      this.filterInput_.value = filterText;
      this.onFilterTextChanged_();
    },

    onFilterTextChanged_: function() {
      this.setFilter_(this.getFilterText_());
    },

    /**
     * Updates text in the details view when time display mode is toggled.
     */
    onUseRelativeTimesChanged: function() {
      this.invalidateDetailsView_();
    },

    comparisonFuncWithReversing_: function(a, b) {
      var result = this.comparisonFunction_(a, b);
      if (this.doSortBackwards_)
        result *= -1;
      return result;
    },

    sort_: function() {
      var sourceEntries = [];
      for (var id in this.sourceIdToRowMap_) {
        sourceEntries.push(this.sourceIdToRowMap_[id].getSourceEntry());
      }
      sourceEntries.sort(this.comparisonFuncWithReversing_.bind(this));

      // Reposition source rows from back to front.
      for (var i = sourceEntries.length - 2; i >= 0; --i) {
        var sourceRow = this.sourceIdToRowMap_[sourceEntries[i].getSourceId()];
        var nextSourceId = sourceEntries[i + 1].getSourceId();
        if (sourceRow.getNextNodeSourceId() != nextSourceId) {
          var nextSourceRow = this.sourceIdToRowMap_[nextSourceId];
          sourceRow.moveBefore(nextSourceRow);
        }
      }
    },

    setFilter_: function(filterText) {
      var lastComparisonFunction = this.comparisonFunction_;
      var lastDoSortBackwards = this.doSortBackwards_;

      var filterParser = new SourceFilterParser(filterText);
      this.currentFilter_ = filterParser.filter;

      this.pickSortFunction_(filterParser.sort);

      if (lastComparisonFunction != this.comparisonFunction_ ||
          lastDoSortBackwards != this.doSortBackwards_) {
        this.sort_();
      }

      // Iterate through all of the rows and see if they match the filter.
      for (var id in this.sourceIdToRowMap_) {
        var entry = this.sourceIdToRowMap_[id];
        entry.setIsMatchedByFilter(this.currentFilter_(entry.getSourceEntry()));
      }
    },

    /**
     * Given a "sort" object with "method" and "backwards" keys, looks up and
     * sets |comparisonFunction_| and |doSortBackwards_|.  If the ID does not
     * correspond to a sort function, defaults to sorting by ID.
     */
    pickSortFunction_: function(sort) {
      this.doSortBackwards_ = sort.backwards;
      this.comparisonFunction_ = COMPARISON_FUNCTION_TABLE[sort.method];
      if (!this.comparisonFunction_) {
        this.doSortBackwards_ = false;
        this.comparisonFunction_ = compareSourceStartTime_;
      }
    },

    /**
     * Repositions |sourceRow|'s in the table using an insertion sort.
     * Significantly faster than sorting the entire table again, when only
     * one entry has changed.
     */
    insertionSort_: function(sourceRow) {
      // SourceRow that should be after |sourceRow|, if it needs
      // to be moved earlier in the list.
      var sourceRowAfter = sourceRow;
      while (true) {
        var prevSourceId = sourceRowAfter.getPreviousNodeSourceId();
        if (prevSourceId == null)
          break;
        var prevSourceRow = this.sourceIdToRowMap_[prevSourceId];
        if (this.comparisonFuncWithReversing_(
                sourceRow.getSourceEntry(), prevSourceRow.getSourceEntry()) >=
            0) {
          break;
        }
        sourceRowAfter = prevSourceRow;
      }
      if (sourceRowAfter != sourceRow) {
        sourceRow.moveBefore(sourceRowAfter);
        return;
      }

      var sourceRowBefore = sourceRow;
      while (true) {
        var nextSourceId = sourceRowBefore.getNextNodeSourceId();
        if (nextSourceId == null)
          break;
        var nextSourceRow = this.sourceIdToRowMap_[nextSourceId];
        if (this.comparisonFuncWithReversing_(
                sourceRow.getSourceEntry(), nextSourceRow.getSourceEntry()) <=
            0) {
          break;
        }
        sourceRowBefore = nextSourceRow;
      }
      if (sourceRowBefore != sourceRow)
        sourceRow.moveAfter(sourceRowBefore);
    },

    /**
     * Called whenever SourceEntries are updated with new log entries.  Updates
     * the corresponding table rows, sort order, and the details view as needed.
     */
    onSourceEntriesUpdated: function(sourceEntries) {
      var isUpdatedSourceSelected = false;
      var numNewSourceEntries = 0;

      for (var i = 0; i < sourceEntries.length; ++i) {
        var sourceEntry = sourceEntries[i];

        // Lookup the row.
        var sourceRow = this.sourceIdToRowMap_[sourceEntry.getSourceId()];

        if (!sourceRow) {
          sourceRow = new SourceRow(this, sourceEntry);
          this.sourceIdToRowMap_[sourceEntry.getSourceId()] = sourceRow;
          ++numNewSourceEntries;
        } else {
          sourceRow.onSourceUpdated();
        }

        if (sourceRow.isSelected())
          isUpdatedSourceSelected = true;

        // TODO(mmenke): Fix sorting when sorting by duration.
        //               Duration continuously increases for all entries that
        //               are still active.  This can result in incorrect
        //               sorting, until sort_ is called.
        this.insertionSort_(sourceRow);
      }

      if (isUpdatedSourceSelected)
        this.invalidateDetailsView_();
      if (numNewSourceEntries)
        this.incrementPrefilterCount(numNewSourceEntries);
    },

    /**
     * Returns the SourceRow with the specified ID, if there is one.
     * Otherwise, returns undefined.
     */
    getSourceRow: function(id) {
      return this.sourceIdToRowMap_[id];
    },

    /**
     * Called whenever all log events are deleted.
     */
    onAllSourceEntriesDeleted: function() {
      this.initializeSourceList_();
    },

    /**
     * Called when either a log file is loaded, after clearing the old entries,
     * but before getting any new ones.
     */
    onLoadLogStart: function() {},

    onLoadLogFinish: function(data) {
      return true;
    },

    incrementPrefilterCount: function(offset) {
      this.numPrefilter_ += offset;
      this.invalidateFilterCounter_();
    },

    incrementPostfilterCount: function(offset) {
      this.numPostfilter_ += offset;
      this.invalidateFilterCounter_();
    },

    onSelectionChanged: function() {
      this.invalidateDetailsView_();
    },

    clearSelection: function() {
      var prevSelection = this.currentSelectedRows_;
      this.currentSelectedRows_ = [];

      // Unselect everything that is currently selected.
      for (var i = 0; i < prevSelection.length; ++i) {
        prevSelection[i].setSelected(false);
      }

      this.onSelectionChanged();
    },

    selectAll_: function(event) {
      for (var id in this.sourceIdToRowMap_) {
        var sourceRow = this.sourceIdToRowMap_[id];
        if (sourceRow.isMatchedByFilter()) {
          sourceRow.setSelected(true);
        }
      }
      event.preventDefault();
    },

    unselectAll_: function() {
      var entries = this.currentSelectedRows_.slice(0);
      for (var i = 0; i < entries.length; ++i) {
        entries[i].setSelected(false);
      }
    },

    /**
     * If |params| includes a query, replaces the current filter and unselects.
     * all items.  If it includes a selection, tries to select the relevant
     * item.
     */
    setParameters: function(params) {
      if (params.q) {
        this.unselectAll_();
        this.setFilterText_(params.q);
      }

      if (params.s) {
        var sourceRow = this.sourceIdToRowMap_[params.s];
        if (sourceRow) {
          sourceRow.setSelected(true);
          this.scrollToSourceId(params.s);
        }
      }
    },

    /**
     * Scrolls to the source indicated by |sourceId|, if displayed.
     */
    scrollToSourceId: function(sourceId) {
      this.detailsView_.scrollToSourceId(sourceId);
    },

    /**
     * If already using the specified sort method, flips direction.  Otherwise,
     * removes pre-existing sort parameter before adding the new one.
     */
    toggleSortMethod_: function(sortMethod) {
      // Get old filter text and remove old sort directives, if any.
      var filterParser = new SourceFilterParser(this.getFilterText_());
      var filterText = filterParser.filterTextWithoutSort;

      filterText = 'sort:' + sortMethod + ' ' + filterText;

      // If already using specified sortMethod, sort backwards.
      if (!this.doSortBackwards_ &&
          COMPARISON_FUNCTION_TABLE[sortMethod] == this.comparisonFunction_) {
        filterText = '-' + filterText;
      }

      this.setFilterText_(filterText.trim());
    },

    sortById_: function(event) {
      this.toggleSortMethod_('id');
    },

    sortBySourceType_: function(event) {
      this.toggleSortMethod_('source');
    },

    sortByDescription_: function(event) {
      this.toggleSortMethod_('desc');
    },

    /**
     * Modifies the map of selected rows to include/exclude the one with
     * |sourceId|, if present.  Does not modify checkboxes or the LogView.
     * Should only be called by a SourceRow in response to its selection
     * state changing.
     */
    modifySelectionArray: function(sourceId, addToSelection) {
      var sourceRow = this.sourceIdToRowMap_[sourceId];
      if (!sourceRow)
        return;
      // Find the index for |sourceEntry| in the current selection list.
      var index = -1;
      for (var i = 0; i < this.currentSelectedRows_.length; ++i) {
        if (this.currentSelectedRows_[i] == sourceRow) {
          index = i;
          break;
        }
      }

      if (index != -1 && !addToSelection) {
        // Remove from the selection.
        this.currentSelectedRows_.splice(index, 1);
      }

      if (index == -1 && addToSelection) {
        this.currentSelectedRows_.push(sourceRow);
      }
    },

    getSelectedSourceEntries_: function() {
      var sourceEntries = [];
      for (var i = 0; i < this.currentSelectedRows_.length; ++i) {
        sourceEntries.push(this.currentSelectedRows_[i].getSourceEntry());
      }
      return sourceEntries;
    },

    invalidateDetailsView_: function() {
      this.detailsView_.setData(this.getSelectedSourceEntries_());
    },

    invalidateFilterCounter_: function() {
      if (!this.outstandingRepaintFilterCounter_) {
        this.outstandingRepaintFilterCounter_ = true;
        window.setTimeout(
            this.repaintFilterCounter_.bind(this),
            REPAINT_FILTER_COUNTER_TIMEOUT_MS);
      }
    },

    repaintFilterCounter_: function() {
      this.outstandingRepaintFilterCounter_ = false;
      this.filterCount_.innerHTML = '';
      addTextNode(
          this.filterCount_, this.numPostfilter_ + ' of ' + this.numPrefilter_);
    }
  };  // end of prototype.

  // ------------------------------------------------------------------------
  // Helper code for comparisons
  // ------------------------------------------------------------------------

  var COMPARISON_FUNCTION_TABLE = {
    // sort: and -sort: are allowed
    '': compareSourceStartTime_,
    'active': compareActive_,
    'desc': compareDescription_,
    'description': compareDescription_,
    'duration': compareDuration_,
    'id': compareSourceStartTime_,
    'source': compareSourceType_,
    'type': compareSourceType_
  };

  /**
   * Sorts active entries first.  If both entries are inactive, puts the one
   * that was active most recently first.  If both are active, uses source
   * start time or source ID, which puts longer lived events at the top, and
   * behaves better than using duration or time of first event.
   */
  function compareActive_(source1, source2) {
    if (!source1.isInactive() && source2.isInactive())
      return -1;
    if (source1.isInactive() && !source2.isInactive())
      return 1;
    if (source1.isInactive()) {
      var deltaEndTime = source1.getEndTicks() - source2.getEndTicks();
      if (deltaEndTime != 0) {
        // The one that ended most recently (Highest end time) should be sorted
        // first.
        return -deltaEndTime;
      }
      // If both ended at the same time, then odds are they were related events,
      // started one after another, so sort in the opposite order of their
      // source start time or source IDs to get a more intuitive ordering.
      return -compareSourceStartTime_(source1, source2);
    }
    return compareSourceStartTime_(source1, source2);
  }

  function compareDescription_(source1, source2) {
    var source1Text = source1.getDescription().toLowerCase();
    var source2Text = source2.getDescription().toLowerCase();
    var compareResult = source1Text.localeCompare(source2Text);
    if (compareResult != 0)
      return compareResult;
    return compareSourceStartTime_(source1, source2);
  }

  function compareDuration_(source1, source2) {
    var durationDifference = source2.getDuration() - source1.getDuration();
    if (durationDifference)
      return durationDifference;
    return compareSourceStartTime_(source1, source2);
  }

  function compareSourceId_(source1, source2) {
    return source1.getSourceId() - source2.getSourceId();
  }

  function compareSourceType_(source1, source2) {
    var source1Text = source1.getSourceTypeString();
    var source2Text = source2.getSourceTypeString();
    var compareResult = source1Text.localeCompare(source2Text);
    if (compareResult != 0)
      return compareResult;
    return compareSourceStartTime_(source1, source2);
  }

  /**
   * If the log contains source start_time fields, and the sources have
   * differing start times, sort by that.
   * Otherwise, sort by source id. Older logs that don't contain source start
   * time also won't contain source id partitioning, so sorting by source id is
   * equivalent to sorting by source start time.  If the start_times are the
   * same, the source ID will indicate the true order the sources were created,
   * if they were created in the same process.
   */
  function compareSourceStartTime_(source1, source2) {
    if (source1.hasSourceStartTime() && source2.hasSourceStartTime()) {
      var startDifference = source1.getStartTicks() - source2.getStartTicks();
      if (startDifference) {
        return startDifference;
      }
    }
    return compareSourceId_(source1, source2);
  }

  return EventsView;
})();

