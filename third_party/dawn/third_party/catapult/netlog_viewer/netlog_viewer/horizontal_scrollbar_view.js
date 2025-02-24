// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * This view consists of two nested divs.  The outer one has a horizontal
 * scrollbar and the inner one has a height of 1 pixel and a width set to
 * allow an appropriate scroll range.  The view reports scroll events to
 * a callback specified on construction.
 *
 * All this funkiness is necessary because there is no HTML scroll control.
 * TODO(mmenke):  Consider implementing our own scrollbar directly.
 */
var HorizontalScrollbarView = (function() {
  'use strict';

  // We inherit from DivView.
  var superClass = DivView;

  /**
   * @constructor
   */
  function HorizontalScrollbarView(divId, innerDivId, callback) {
    superClass.call(this, divId);
    this.callback_ = callback;
    this.innerDiv_ = $(innerDivId);
    $(divId).onscroll = this.onScroll_.bind(this);

    // The current range and position of the scrollbar.  Because DOM updates
    // are asynchronous, the current state cannot be read directly from the DOM
    // after updating the range.
    this.range_ = 0;
    this.position_ = 0;

    // The DOM updates asynchronously, so sometimes we need a timer to update
    // the current scroll position after resizing the scrollbar.
    this.updatePositionTimerId_ = null;
  }

  HorizontalScrollbarView.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    onResized: function() {
      this.setRange(this.range_);
    },

    show: function(isVisible) {
      superClass.prototype.show.call(this, isVisible);
    },

    /**
     * Sets the range of the scrollbar.  The scrollbar can have a value
     * anywhere from 0 to |range|, inclusive.  The width of the drag area
     * on the scrollbar will generally be based on the width of the scrollbar
     * relative to the size of |range|, so if the scrollbar is about the size
     * of the thing we're scrolling, we get fairly nice behavior.
     *
     * If |range| is less than the original position, |position_| is set to
     * |range|.  Otherwise, it is not modified.
     */
    setRange: function(range) {
      this.range_ = range;
      setNodeWidth(this.innerDiv_, this.getNode().offsetWidth + range);
      if (range < this.position_)
        this.position_ = range;
      this.setPosition(this.position_);
    },

    /**
     * Sets the position of the scrollbar.  |position| must be between 0 and
     * |range_|, inclusive.
     */
    setPosition: function(position) {
      this.position_ = position;
      this.updatePosition_();
    },

    /**
     * Updates the visible position of the scrollbar to be |position_|.
     * On failure, calls itself again after a timeout.  This is needed because
     * setRange does not synchronously update the DOM.
     */
    updatePosition_: function() {
      // Clear the timer if we have one, so we don't have two timers running at
      // once.  This is safe even if we were just called from the timer, in
      // which case clearTimeout will silently fail.
      if (this.updatePositionTimerId_ !== null) {
        window.clearTimeout(this.updatePositionTimerId_);
        this.updatePositionTimerId_ = null;
      }

      this.getNode().scrollLeft = this.position_;
      if (this.getNode().scrollLeft != this.position_) {
        this.updatePositionTimerId_ =
            window.setTimeout(this.updatePosition_.bind(this));
      }
    },

    getRange: function() {
      return this.range_;
    },

    getPosition: function() {
      return this.position_;
    },

    onScroll_: function() {
      // If we're waiting to update the range, ignore messages from the
      // scrollbar.
      if (this.updatePositionTimerId_ !== null)
        return;
      var newPosition = this.getNode().scrollLeft;
      if (newPosition == this.position_)
        return;
      this.position_ = newPosition;
      this.callback_();
    }
  };

  return HorizontalScrollbarView;
})();

