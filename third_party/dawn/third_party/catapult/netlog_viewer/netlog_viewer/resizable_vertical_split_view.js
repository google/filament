// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * This view implements a vertically split display with a draggable divider.
 *
 *                  <<-- sizer -->>
 *
 *  +----------------------++----------------+
 *  |                      ||                |
 *  |                      ||                |
 *  |                      ||                |
 *  |                      ||                |
 *  |       leftView       ||   rightView    |
 *  |                      ||                |
 *  |                      ||                |
 *  |                      ||                |
 *  |                      ||                |
 *  |                      ||                |
 *  +----------------------++----------------+
 *
 * @param {!View} leftView The widget to position on the left.
 * @param {!View} rightView The widget to position on the right.
 * @param {!DivView} sizerView The widget that will serve as draggable divider.
 */
var ResizableVerticalSplitView = (function() {
  'use strict';

  // Minimum width to size panels to, in pixels.
  var MIN_PANEL_WIDTH = 50;

  // We inherit from View.
  var superClass = View;

  /**
   * @constructor
   */
  function ResizableVerticalSplitView(leftView, rightView, sizerView) {
    // Call superclass's constructor.
    superClass.call(this);

    this.leftView_ = leftView;
    this.rightView_ = rightView;
    this.sizerView_ = sizerView;

    this.mouseDragging_ = false;
    this.touchDragging_ = false;

    // Setup the "sizer" so it can be dragged left/right to reposition the
    // vertical split.  The start event must occur within the sizer's node,
    // but subsequent events may occur anywhere.
    var node = sizerView.getNode();
    node.addEventListener('mousedown', this.onMouseDragSizerStart_.bind(this));
    window.addEventListener('mousemove', this.onMouseDragSizer_.bind(this));
    window.addEventListener('mouseup', this.onMouseDragSizerEnd_.bind(this));

    node.addEventListener('touchstart', this.onTouchDragSizerStart_.bind(this));
    window.addEventListener('touchmove', this.onTouchDragSizer_.bind(this));
    window.addEventListener('touchend', this.onTouchDragSizerEnd_.bind(this));
    window.addEventListener(
        'touchcancel', this.onTouchDragSizerEnd_.bind(this));
  }

  ResizableVerticalSplitView.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    show: function(isVisible) {
      superClass.prototype.show.call(this, isVisible);
      this.leftView_.show(isVisible);
      this.sizerView_.show(isVisible);
      this.rightView_.show(isVisible);
    },

    /**
     * Called once the sizer is clicked on. Starts moving the sizer in response
     * to future mouse movement.
     */
    onMouseDragSizerStart_: function(event) {
      this.mouseDragging_ = true;
      event.preventDefault();
    },

    /**
     * Called when the mouse has moved.
     */
    onMouseDragSizer_: function(event) {
      if (!this.mouseDragging_)
        return;
      // If dragging has started, move the sizer.
      this.onDragSizer_(event.pageX);
      event.preventDefault();
    },

    /**
     * Called once the mouse has been released.
     */
    onMouseDragSizerEnd_: function(event) {
      if (!this.mouseDragging_)
        return;
      // Dragging is over.
      this.mouseDragging_ = false;
      event.preventDefault();
    },

    /**
     * Called when the user touches the sizer.  Starts moving the sizer in
     * response to future touch events.
     */
    onTouchDragSizerStart_: function(event) {
      this.touchDragging_ = true;
      event.preventDefault();
    },

    /**
     * Called when the mouse has moved after dragging started.
     */
    onTouchDragSizer_: function(event) {
      if (!this.touchDragging_)
        return;
      // If dragging has started, move the sizer.
      this.onDragSizer_(event.touches[0].pageX);
      event.preventDefault();
    },

    /**
     * Called once the user stops touching the screen.
     */
    onTouchDragSizerEnd_: function(event) {
      if (!this.touchDragging_)
        return;
      // Dragging is over.
      this.touchDragging_ = false;
      event.preventDefault();
    },

    /**
     * Common code used for both mouse and touch dragging.
     */
    onDragSizer_: function(pageX) {
      // Convert the mouse coordinate to a coordinate relative to leftView_.
      let newWidth = (pageX - this.leftView_.getNode().offsetLeft);

      // Avoid shrinking the left box too much.
      newWidth = Math.max(newWidth, MIN_PANEL_WIDTH);

      // TODO(eroman): Prevent the rightView from becoming smaller than MIN_PANEL_WIDTH.

      // Resize the left view.
      this.leftView_.getNode().style.width = newWidth + 'px';
    },
  };

  return ResizableVerticalSplitView;
})();

