// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var SourceRow = (function() {
  'use strict';

  /**
   * A SourceRow represents the row corresponding to a single SourceEntry
   * displayed by the EventsView.
   *
   * @constructor
   */
  function SourceRow(parentView, sourceEntry) {
    this.parentView_ = parentView;

    this.sourceEntry_ = sourceEntry;
    this.isSelected_ = false;
    this.isMatchedByFilter_ = false;

    // Used to set CSS class for display.  Must only be modified by calling
    // corresponding set functions.
    this.isSelected_ = false;
    this.isMouseOver_ = false;

    // Mirror sourceEntry's values, so we only update the DOM when necessary.
    this.isError_ = sourceEntry.isError();
    this.isInactive_ = sourceEntry.isInactive();
    this.description_ = sourceEntry.getDescription();

    this.createRow_();
    this.onSourceUpdated();
  }

  SourceRow.prototype = {
    createRow_: function() {
      // Create a row.
      var tr = addNode(this.parentView_.tableBody_, 'tr');
      tr._id = this.getSourceId();
      tr.style.display = 'none';
      this.row_ = tr;

      var selectionCol = addNode(tr, 'td');
      var checkbox = addNode(selectionCol, 'input');
      checkbox.title = this.getSourceId();
      selectionCol.style.borderLeft = '0';
      checkbox.type = 'checkbox';

      var idCell = addNode(tr, 'td');
      idCell.style.textAlign = 'right';

      var typeCell = addNode(tr, 'td');
      var descriptionCell = addNode(tr, 'td');
      this.descriptionCell_ = descriptionCell;

      // Connect listeners.
      checkbox.onchange = this.onCheckboxToggled_.bind(this);

      var onclick = this.onClicked_.bind(this);
      idCell.onclick = onclick;
      typeCell.onclick = onclick;
      descriptionCell.onclick = onclick;

      tr.onmouseover = this.onMouseover_.bind(this);
      tr.onmouseout = this.onMouseout_.bind(this);

      // Set the cell values to match this source's data.
      addTextNode(idCell, this.getSourceId());
      var sourceTypeString = this.sourceEntry_.getSourceTypeString();
      addTextNode(typeCell, sourceTypeString);
      this.updateDescription_();

      // Add a CSS classname specific to this source type (so CSS can specify
      // different stylings for different types).
      var sourceTypeClass = sourceTypeString.toLowerCase().replace(/_/g, '-');
      this.row_.classList.add('source-' + sourceTypeClass);

      this.updateClass_();
    },

    onSourceUpdated: function() {
      if (this.sourceEntry_.isInactive() != this.isInactive_ ||
          this.sourceEntry_.isError() != this.isError_) {
        this.updateClass_();
      }

      if (this.description_ != this.sourceEntry_.getDescription())
        this.updateDescription_();

      // Update filters.
      var matchesFilter = this.parentView_.currentFilter_(this.sourceEntry_);
      this.setIsMatchedByFilter(matchesFilter);
    },

    /**
     * Changes |row_|'s class based on currently set flags.  Clears any previous
     * class set by this method.  This method is needed so that some styles
     * override others.
     */
    updateClass_: function() {
      this.isInactive_ = this.sourceEntry_.isInactive();
      this.isError_ = this.sourceEntry_.isError();

      // Each element of this list contains a property of |this| and the
      // corresponding class name to set if that property is true.  Entries
      // earlier in the list take precedence.
      var propertyNames = [
        ['isSelected_', 'selected'],
        ['isMouseOver_', 'mouseover'],
        ['isError_', 'error'],
        ['isInactive_', 'inactive'],
      ];

      // Loop through |propertyNames| in order, checking if each property
      // is true.  For the first such property found, if any, add the
      // corresponding class to the SourceEntry's row.  Remove classes
      // that correspond to any other property.
      var noStyleSet = true;
      for (var i = 0; i < propertyNames.length; ++i) {
        var setStyle = noStyleSet && this[propertyNames[i][0]];
        if (setStyle) {
          this.row_.classList.add(propertyNames[i][1]);
          noStyleSet = false;
        } else {
          this.row_.classList.remove(propertyNames[i][1]);
        }
      }
    },

    getSourceEntry: function() {
      return this.sourceEntry_;
    },

    setIsMatchedByFilter: function(isMatchedByFilter) {
      if (this.isMatchedByFilter() == isMatchedByFilter)
        return;  // No change.

      this.isMatchedByFilter_ = isMatchedByFilter;

      this.setFilterStyles(isMatchedByFilter);

      if (isMatchedByFilter) {
        this.parentView_.incrementPostfilterCount(1);
      } else {
        this.parentView_.incrementPostfilterCount(-1);
        // If we are filtering an entry away, make sure it is no longer
        // part of the selection.
        this.setSelected(false);
      }
    },

    isMatchedByFilter: function() {
      return this.isMatchedByFilter_;
    },

    setFilterStyles: function(isMatchedByFilter) {
      // Hide rows which have been filtered away.
      if (isMatchedByFilter) {
        this.row_.style.display = '';
      } else {
        this.row_.style.display = 'none';
      }
    },

    isSelected: function() {
      return this.isSelected_;
    },

    setSelected: function(isSelected) {
      if (isSelected == this.isSelected())
        return;

      this.isSelected_ = isSelected;

      this.setSelectedStyles(isSelected);
      this.parentView_.modifySelectionArray(this.getSourceId(), isSelected);
      this.parentView_.onSelectionChanged();
    },

    setSelectedStyles: function(isSelected) {
      this.isSelected_ = isSelected;
      this.getSelectionCheckbox().checked = isSelected;
      this.updateClass_();
    },

    setMouseoverStyle: function(isMouseOver) {
      this.isMouseOver_ = isMouseOver;
      this.updateClass_();
    },

    onClicked_: function() {
      this.parentView_.clearSelection();
      this.setSelected(true);
      if (this.isSelected())
        this.parentView_.scrollToSourceId(this.getSourceId());
    },

    onMouseover_: function() {
      this.setMouseoverStyle(true);
    },

    onMouseout_: function() {
      this.setMouseoverStyle(false);
    },

    updateDescription_: function() {
      this.description_ = this.sourceEntry_.getDescription();
      this.descriptionCell_.innerHTML = '';
      addTextNode(this.descriptionCell_, this.description_);
    },

    onCheckboxToggled_: function() {
      this.setSelected(this.getSelectionCheckbox().checked);
      if (this.isSelected())
        this.parentView_.scrollToSourceId(this.getSourceId());
    },

    getSelectionCheckbox: function() {
      return this.row_.childNodes[0].firstChild;
    },

    getSourceId: function() {
      return this.sourceEntry_.getSourceId();
    },

    /**
     * Returns source ID of the entry whose row is currently above this one's.
     * Returns null if no such node exists.
     */
    getPreviousNodeSourceId: function() {
      var prevNode = this.row_.previousSibling;
      if (prevNode == null)
        return null;
      return prevNode._id;
    },

    /**
     * Returns source ID of the entry whose row is currently below this one's.
     * Returns null if no such node exists.
     */
    getNextNodeSourceId: function() {
      var nextNode = this.row_.nextSibling;
      if (nextNode == null)
        return null;
      return nextNode._id;
    },

    /**
     * Moves current object's row before |entry|'s row.
     */
    moveBefore: function(entry) {
      this.row_.parentNode.insertBefore(this.row_, entry.row_);
    },

    /**
     * Moves current object's row after |entry|'s row.
     */
    moveAfter: function(entry) {
      this.row_.parentNode.insertBefore(this.row_, entry.row_.nextSibling);
    }
  };

  return SourceRow;
})();

