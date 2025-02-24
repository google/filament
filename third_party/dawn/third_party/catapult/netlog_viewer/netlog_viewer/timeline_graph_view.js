// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * A TimelineGraphView displays a timeline graph on a canvas element.
 */
var TimelineGraphView = (function() {
  'use strict';
  // We inherit from DivView.
  var superClass = DivView;

  // Default starting scale factor, in terms of milliseconds per pixel.
  var DEFAULT_SCALE = 1000;

  // Maximum number of labels placed vertically along the sides of the graph.
  var MAX_VERTICAL_LABELS = 6;

  // Vertical spacing between labels and between the graph and labels.
  var LABEL_VERTICAL_SPACING = 4;
  // Horizontal spacing between vertically placed labels and the edges of the
  // graph.
  var LABEL_HORIZONTAL_SPACING = 3;
  // Horizintal spacing between two horitonally placed labels along the bottom
  // of the graph.
  var LABEL_LABEL_HORIZONTAL_SPACING = 25;

  // Length of ticks, in pixels, next to y-axis labels.  The x-axis only has
  // one set of labels, so it can use lines instead.
  var Y_AXIS_TICK_LENGTH = 10;

  // The number of units mouse wheel deltas increase for each tick of the
  // wheel.
  var MOUSE_WHEEL_UNITS_PER_CLICK = 120;

  // Amount we zoom for one vertical tick of the mouse wheel, as a ratio.
  var MOUSE_WHEEL_ZOOM_RATE = 1.25;
  // Amount we scroll for one horizontal tick of the mouse wheel, in pixels.
  var MOUSE_WHEEL_SCROLL_RATE = MOUSE_WHEEL_UNITS_PER_CLICK;
  // Number of pixels to scroll per pixel the mouse is dragged.
  var MOUSE_WHEEL_DRAG_RATE = 3;

  var GRID_COLOR = '#CCC';
  var TEXT_COLOR = '#000';
  var BACKGROUND_COLOR = '#FFF';

  // Which side of the canvas y-axis labels should go on, for a given Graph.
  // TODO(mmenke):  Figure out a reasonable way to handle more than 2 sets
  //                of labels.
  var LabelAlign = {LEFT: 0, RIGHT: 1};

  /**
   * @constructor
   */
  function TimelineGraphView(graphDivId, canvasId, scrollbarId,
                             scrollbarInnerId) {
    // Call superclass's constructor.
    superClass.call(this, graphDivId);

    this.scrollbar_ = new HorizontalScrollbarView(
        scrollbarId, scrollbarInnerId, this.onScroll_.bind(this));

    this.graphDiv_ = $(graphDivId);
    this.canvas_ = $(canvasId);
    this.canvas_.onmousewheel = this.onMouseWheel_.bind(this);
    this.canvas_.onmousedown = this.onMouseDown_.bind(this);
    this.canvas_.onmousemove = this.onMouseMove_.bind(this);
    this.canvas_.onmouseup = this.onMouseUp_.bind(this);
    this.canvas_.onmouseout = this.onMouseUp_.bind(this);

    // Used for click and drag scrolling of graph.  Drag-zooming not supported,
    // for a more stable scrolling experience.
    this.isDragging_ = false;
    this.dragX_ = 0;

    // Set the range and scale of the graph.  Times are in milliseconds since
    // the Unix epoch.

    // All measurements we have must be after this time.
    this.startTime_ = 0;
    // The current rightmost position of the graph is always at most this.
    // We may have some later events.  When actively capturing new events, it's
    // updated on a timer.
    this.endTime_ = 1;

    // Current scale, in terms of milliseconds per pixel.  Each column of
    // pixels represents a point in time |scale_| milliseconds after the
    // previous one.  We only display times that are of the form
    // |startTime_| + K * |scale_| to avoid jittering, and the rightmost
    // pixel that we can display has a time <= |endTime_|.  Non-integer values
    // are allowed.
    this.scale_ = DEFAULT_SCALE;

    this.graphs_ = [];

    // Initialize the scrollbar.
    this.updateScrollbarRange_(true);

    this.initResizePolling_(true);
  }

  // Smallest allowed scaling factor.
  TimelineGraphView.MIN_SCALE = 5;

  TimelineGraphView.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    getPreferredCanvasSize_: function() {
      // The size of the canvas can only be set by using its |width| and
      // |height| properties, which do not take padding into account, so we
      // need to use them ourselves.
      var style = getComputedStyle(this.canvas_);
      var horizontalPadding =
          parseInt(style.paddingRight) + parseInt(style.paddingLeft);
      var verticalPadding =
          parseInt(style.paddingTop) + parseInt(style.paddingBottom);
      var canvasWidth =
          parseInt(this.graphDiv_.clientWidth) - horizontalPadding;
      // For unknown reasons, there's an extra 3 pixels border between the
      // bottom of the canvas and the bottom margin of the enclosing div.
      var canvasHeight =
          parseInt(this.graphDiv_.clientHeight) - verticalPadding - 3;

      // Protect against degenerates.
      if (canvasWidth < 10)
        canvasWidth = 10;
      if (canvasHeight < 10)
        canvasHeight = 10;

      return {
        width: canvasWidth,
        height: canvasHeight
      };
    },

    onResized: function() {
      this.scrollbar_.onResized();

      let preferredSize = this.getPreferredCanvasSize_();

      this.canvas_.width = preferredSize.width;
      this.canvas_.height = preferredSize.height;

      // Use the same font style for the canvas as we use elsewhere.
      // Has to be updated every resize.
      this.canvas_.getContext('2d').font = getComputedStyle(this.canvas_).font;

      this.updateScrollbarRange_(this.graphScrolledToRightEdge_());
      this.repaint();
    },

    show: function(isVisible) {
      superClass.prototype.show.call(this, isVisible);

      this.initResizePolling_(isVisible);

      if (isVisible)
        this.repaint();
    },

    // TODO(eroman): This is a bit silly, but good enough for now. Canvas needs
    // to be sized explicitly, however in this case we want it to occupy all
    // remaining space. Use polling to check when the canvas logical dimensions
    // needs to be updated to match the layout size.
    initResizePolling_: function(active) {
      if (active && !this.resizeTimer_) {
        this.resizeTimer_ = window.setInterval(
            this.checkForResize_.bind(this), 1000);
      } else if (!active && this.resizeTimer_) {
        window.clearInterval(this.resizeTimer_);
        this.resizeTimer_ = null;
      }
    },

    checkForResize_: function() {
      let preferredSize = this.getPreferredCanvasSize_();
      if ((preferredSize.width != this.canvas_.width) ||
          (preferredSize.height != this.canvas_.height)) {
        this.onResized();
      }
    },

    // Returns the total length of the graph, in pixels.
    getLength_: function() {
      var timeRange = this.endTime_ - this.startTime_;
      // Math.floor is used to ignore the last partial area, of length less
      // than |scale_|.
      return Math.floor(timeRange / this.scale_);
    },

    /**
     * Returns true if the graph is scrolled all the way to the right.
     */
    graphScrolledToRightEdge_: function() {
      return this.scrollbar_.getPosition() == this.scrollbar_.getRange();
    },

    /**
     * Update the range of the scrollbar.  If |resetPosition| is true, also
     * sets the slider to point at the rightmost position and triggers a
     * repaint.
     */
    updateScrollbarRange_: function(resetPosition) {
      var scrollbarRange = this.getLength_() - this.canvas_.width;
      if (scrollbarRange < 0)
        scrollbarRange = 0;

      // If we've decreased the range to less than the current scroll position,
      // we need to move the scroll position.
      if (this.scrollbar_.getPosition() > scrollbarRange)
        resetPosition = true;

      this.scrollbar_.setRange(scrollbarRange);
      if (resetPosition) {
        this.scrollbar_.setPosition(scrollbarRange);
        this.repaint();
      }
    },

    /**
     * Sets the date range displayed on the graph, switches to the default
     * scale factor, and moves the scrollbar all the way to the right.
     */
    setDateRange: function(startDate, endDate) {
      this.startTime_ = startDate.getTime();
      this.endTime_ = endDate.getTime();

      // Safety check.
      if (this.endTime_ <= this.startTime_)
        this.startTime_ = this.endTime_ - 1;

      this.scale_ = DEFAULT_SCALE;
      this.updateScrollbarRange_(true);
    },

    /**
     * Updates the end time at the right of the graph to be the current time.
     * Specifically, updates the scrollbar's range, and if the scrollbar is
     * all the way to the right, keeps it all the way to the right.  Otherwise,
     * leaves the view as-is and doesn't redraw anything.
     */
    updateEndDate: function() {
      this.endTime_ = timeutil.getCurrentTime();
      this.updateScrollbarRange_(this.graphScrolledToRightEdge_());
    },

    getStartDate: function() {
      return new Date(this.startTime_);
    },

    /**
     * Scrolls the graph horizontally by the specified amount.
     */
    horizontalScroll_: function(delta) {
      var newPosition = this.scrollbar_.getPosition() + Math.round(delta);
      // Make sure the new position is in the right range.
      if (newPosition < 0) {
        newPosition = 0;
      } else if (newPosition > this.scrollbar_.getRange()) {
        newPosition = this.scrollbar_.getRange();
      }

      if (this.scrollbar_.getPosition() == newPosition)
        return;
      this.scrollbar_.setPosition(newPosition);
      this.onScroll_();
    },

    /**
     * Zooms the graph by the specified amount.
     */
    zoom_: function(ratio) {
      var oldScale = this.scale_;
      this.scale_ *= ratio;
      if (this.scale_ < TimelineGraphView.MIN_SCALE)
        this.scale_ = TimelineGraphView.MIN_SCALE;

      if (this.scale_ == oldScale)
        return;

      // If we were at the end of the range before, remain at the end of the
      // range.
      if (this.graphScrolledToRightEdge_()) {
        this.updateScrollbarRange_(true);
        return;
      }

      // Otherwise, do our best to maintain the old position.  We use the
      // position at the far right of the graph for consistency.
      var oldMaxTime =
          oldScale * (this.scrollbar_.getPosition() + this.canvas_.width);
      var newMaxTime = Math.round(oldMaxTime / this.scale_);
      var newPosition = newMaxTime - this.canvas_.width;

      // Update range and scroll position.
      this.updateScrollbarRange_(false);
      this.horizontalScroll_(newPosition - this.scrollbar_.getPosition());
    },

    onMouseWheel_: function(event) {
      event.preventDefault();
      this.horizontalScroll_(
          MOUSE_WHEEL_SCROLL_RATE * -event.wheelDeltaX /
          MOUSE_WHEEL_UNITS_PER_CLICK);
      this.zoom_(Math.pow(
          MOUSE_WHEEL_ZOOM_RATE,
          -event.wheelDeltaY / MOUSE_WHEEL_UNITS_PER_CLICK));
    },

    onMouseDown_: function(event) {
      event.preventDefault();
      this.isDragging_ = true;
      this.dragX_ = event.clientX;
    },

    onMouseMove_: function(event) {
      if (!this.isDragging_)
        return;
      event.preventDefault();
      this.horizontalScroll_(
          MOUSE_WHEEL_DRAG_RATE * (event.clientX - this.dragX_));
      this.dragX_ = event.clientX;
    },

    onMouseUp_: function(event) {
      this.isDragging_ = false;
    },

    onScroll_: function() {
      this.repaint();
    },

    /**
     * Replaces the current TimelineDataSeries with |dataSeries|.
     */
    setDataSeries: function(dataSeries) {
      // Simplest just to recreate the Graphs.
      this.graphs_ = [];
      this.graphs_[TimelineDataType.BYTES_PER_SECOND] =
          new Graph(TimelineDataType.BYTES_PER_SECOND, LabelAlign.RIGHT);
      this.graphs_[TimelineDataType.SOURCE_COUNT] =
          new Graph(TimelineDataType.SOURCE_COUNT, LabelAlign.LEFT);
      for (var i = 0; i < dataSeries.length; ++i)
        this.graphs_[dataSeries[i].getDataType()].addDataSeries(dataSeries[i]);

      this.repaint();
    },

    /**
     * Draws the graph on |canvas_|.
     */
    repaint: function() {
      this.repaintTimerRunning_ = false;
      if (!this.isVisible())
        return;

      var width = this.canvas_.width;
      var height = this.canvas_.height;
      var context = this.canvas_.getContext('2d');

      // Clear the canvas.
      context.fillStyle = BACKGROUND_COLOR;
      context.fillRect(0, 0, width, height);

      // Try to get font height in pixels.  Needed for layout.
      var fontHeightString = context.font.match(/([0-9]+)px/)[1];
      var fontHeight = parseInt(fontHeightString);

      // Safety check, to avoid drawing anything too ugly.
      if (fontHeightString.length == 0 || fontHeight <= 0 ||
          fontHeight * 4 > height || width < 50) {
        return;
      }

      // Save current transformation matrix so we can restore it later.
      context.save();

      // The center of an HTML canvas pixel is technically at (0.5, 0.5).  This
      // makes near straight lines look bad, due to anti-aliasing.  This
      // translation reduces the problem a little.
      context.translate(0.5, 0.5);

      // Figure out what time values to display.
      var position = this.scrollbar_.getPosition();
      // If the entire time range is being displayed, align the right edge of
      // the graph to the end of the time range.
      if (this.scrollbar_.getRange() == 0)
        position = this.getLength_() - this.canvas_.width;
      var visibleStartTime = this.startTime_ + position * this.scale_;

      // Make space at the bottom of the graph for the time labels, and then
      // draw the labels.
      var textHeight = height;
      height -= fontHeight + LABEL_VERTICAL_SPACING;
      this.drawTimeLabels(context, width, height, textHeight, visibleStartTime);

      // Draw outline of the main graph area.
      context.strokeStyle = GRID_COLOR;
      context.strokeRect(0, 0, width - 1, height - 1);

      // Layout graphs and have them draw their tick marks.
      for (var i = 0; i < this.graphs_.length; ++i) {
        this.graphs_[i].layout(
            width, height, fontHeight, visibleStartTime, this.scale_);
        this.graphs_[i].drawTicks(context);
      }

      // Draw the lines of all graphs, and then draw their labels.
      for (var i = 0; i < this.graphs_.length; ++i)
        this.graphs_[i].drawLines(context);
      for (var i = 0; i < this.graphs_.length; ++i)
        this.graphs_[i].drawLabels(context);

      // Restore original transformation matrix.
      context.restore();
    },

    /**
     * Draw time labels below the graph.  Takes in start time as an argument
     * since it may not be |startTime_|, when we're displaying the entire
     * time range.
     */
    drawTimeLabels: function(context, width, height, textHeight, startTime) {
      // Text for a time string to use in determining how far apart
      // to place text labels.
      var sampleText = (new Date(startTime)).toLocaleTimeString();

      // The desired spacing for text labels.
      var targetSpacing = context.measureText(sampleText).width +
          LABEL_LABEL_HORIZONTAL_SPACING;

      // The allowed time step values between adjacent labels.  Anything much
      // over a couple minutes isn't terribly realistic, given how much memory
      // we use, and how slow a lot of the net-internals code is.
      var timeStepValues = [
        1000,  // 1 second
        1000 * 5, 1000 * 30,
        1000 * 60,  // 1 minute
        1000 * 60 * 5, 1000 * 60 * 30,
        1000 * 60 * 60,  // 1 hour
        1000 * 60 * 60 * 5
      ];

      // Find smallest time step value that gives us at least |targetSpacing|,
      // if any.
      var timeStep = null;
      for (var i = 0; i < timeStepValues.length; ++i) {
        if (timeStepValues[i] / this.scale_ >= targetSpacing) {
          timeStep = timeStepValues[i];
          break;
        }
      }

      // If no such value, give up.
      if (!timeStep)
        return;

      // Find the time for the first label.  This time is a perfect multiple of
      // timeStep because of how UTC times work.
      var time = Math.ceil(startTime / timeStep) * timeStep;

      context.textBaseline = 'bottom';
      context.textAlign = 'center';
      context.fillStyle = TEXT_COLOR;
      context.strokeStyle = GRID_COLOR;

      // Draw labels and vertical grid lines.
      while (true) {
        var x = Math.round((time - startTime) / this.scale_);
        if (x >= width)
          break;
        var text = (new Date(time)).toLocaleTimeString();
        context.fillText(text, x, textHeight);
        context.beginPath();
        context.lineTo(x, 0);
        context.lineTo(x, height);
        context.stroke();
        time += timeStep;
      }
    }
  };

  /**
   * A Graph is responsible for drawing all the TimelineDataSeries that have
   * the same data type.  Graphs are responsible for scaling the values, laying
   * out labels, and drawing both labels and lines for its data series.
   */
  var Graph = (function() {
    /**
     * |dataType| is the DataType that will be shared by all its DataSeries.
     * |labelAlign| is the LabelAlign value indicating whether the labels
     * should be aligned to the right of left of the graph.
     * @constructor
     */
    function Graph(dataType, labelAlign) {
      this.dataType_ = dataType;
      this.dataSeries_ = [];
      this.labelAlign_ = labelAlign;

      // Cached properties of the graph, set in layout.
      this.width_ = 0;
      this.height_ = 0;
      this.fontHeight_ = 0;
      this.startTime_ = 0;
      this.scale_ = 0;

      // At least the highest value in the displayed range of the graph.
      // Used for scaling and setting labels.  Set in layoutLabels.
      this.max_ = 0;

      // Cached text of equally spaced labels.  Set in layoutLabels.
      this.labels_ = [];
    }

    /**
     * A Label is the label at a particular position along the y-axis.
     * @constructor
     */
    function Label(height, text) {
      this.height = height;
      this.text = text;
    }

    Graph.prototype = {
      addDataSeries: function(dataSeries) {
        this.dataSeries_.push(dataSeries);
      },

      /**
       * Returns a list of all the values that should be displayed for a given
       * data series, using the current graph layout.
       */
      getValues: function(dataSeries) {
        if (!dataSeries.isVisible())
          return null;
        return dataSeries.getValues(this.startTime_, this.scale_, this.width_);
      },

      /**
       * Updates the graph's layout.  In particular, both the max value and
       * label positions are updated.  Must be called before calling any of the
       * drawing functions.
       */
      layout: function(width, height, fontHeight, startTime, scale) {
        this.width_ = width;
        this.height_ = height;
        this.fontHeight_ = fontHeight;
        this.startTime_ = startTime;
        this.scale_ = scale;

        // Find largest value.
        var max = 0;
        for (var i = 0; i < this.dataSeries_.length; ++i) {
          var values = this.getValues(this.dataSeries_[i]);
          if (!values)
            continue;
          for (var j = 0; j < values.length; ++j) {
            if (values[j] > max)
              max = values[j];
          }
        }

        this.layoutLabels_(max);
      },

      /**
       * Lays out labels and sets |max_|, taking the time units into
       * consideration.  |maxValue| is the actual maximum value, and
       * |max_| will be set to the value of the largest label, which
       * will be at least |maxValue|.
       */
      layoutLabels_: function(maxValue) {
        if (this.dataType_ != TimelineDataType.BYTES_PER_SECOND) {
          this.layoutLabelsBasic_(maxValue, 0);
          return;
        }

        // Special handling for data rates.

        // Find appropriate units to use.
        var units = ['B/s', 'kB/s', 'MB/s', 'GB/s', 'TB/s', 'PB/s'];
        // Units to use for labels.  0 is bytes, 1 is kilobytes, etc.
        // We start with kilobytes, and work our way up.
        var unit = 1;
        // Update |maxValue| to be in the right units.
        maxValue = maxValue / 1024;
        while (units[unit + 1] && maxValue >= 999) {
          maxValue /= 1024;
          ++unit;
        }

        // Calculate labels.
        this.layoutLabelsBasic_(maxValue, 1);

        // Append units to labels.
        for (var i = 0; i < this.labels_.length; ++i)
          this.labels_[i] += ' ' + units[unit];

        // Convert |max_| back to bytes, so it can be used when scaling values
        // for display.
        this.max_ *= Math.pow(1024, unit);
      },

      /**
       * Same as layoutLabels_, but ignores units.  |maxDecimalDigits| is the
       * maximum number of decimal digits allowed.  The minimum allowed
       * difference between two adjacent labels is 10^-|maxDecimalDigits|.
       */
      layoutLabelsBasic_: function(maxValue, maxDecimalDigits) {
        this.labels_ = [];
        // No labels if |maxValue| is 0.
        if (maxValue == 0) {
          this.max_ = maxValue;
          return;
        }

        // The maximum number of equally spaced labels allowed.  |fontHeight_|
        // is doubled because the top two labels are both drawn in the same
        // gap.
        var minLabelSpacing = 2 * this.fontHeight_ + LABEL_VERTICAL_SPACING;

        // The + 1 is for the top label.
        var maxLabels = 1 + this.height_ / minLabelSpacing;
        if (maxLabels < 2) {
          maxLabels = 2;
        } else if (maxLabels > MAX_VERTICAL_LABELS) {
          maxLabels = MAX_VERTICAL_LABELS;
        }

        // Initial try for step size between conecutive labels.
        var stepSize = Math.pow(10, -maxDecimalDigits);
        // Number of digits to the right of the decimal of |stepSize|.
        // Used for formating label strings.
        var stepSizeDecimalDigits = maxDecimalDigits;

        // Pick a reasonable step size.
        while (true) {
          // If we use a step size of |stepSize| between labels, we'll need:
          //
          // Math.ceil(maxValue / stepSize) + 1
          //
          // labels.  The + 1 is because we need labels at both at 0 and at
          // the top of the graph.

          // Check if we can use steps of size |stepSize|.
          if (Math.ceil(maxValue / stepSize) + 1 <= maxLabels)
            break;
          // Check |stepSize| * 2.
          if (Math.ceil(maxValue / (stepSize * 2)) + 1 <= maxLabels) {
            stepSize *= 2;
            break;
          }
          // Check |stepSize| * 5.
          if (Math.ceil(maxValue / (stepSize * 5)) + 1 <= maxLabels) {
            stepSize *= 5;
            break;
          }
          stepSize *= 10;
          if (stepSizeDecimalDigits > 0)
            --stepSizeDecimalDigits;
        }

        // Set the max so it's an exact multiple of the chosen step size.
        this.max_ = Math.ceil(maxValue / stepSize) * stepSize;

        // Create labels.
        for (var label = this.max_; label >= 0; label -= stepSize)
          this.labels_.push(label.toFixed(stepSizeDecimalDigits));
      },

      /**
       * Draws tick marks for each of the labels in |labels_|.
       */
      drawTicks: function(context) {
        var x1;
        var x2;
        if (this.labelAlign_ == LabelAlign.RIGHT) {
          x1 = this.width_ - 1;
          x2 = this.width_ - 1 - Y_AXIS_TICK_LENGTH;
        } else {
          x1 = 0;
          x2 = Y_AXIS_TICK_LENGTH;
        }

        context.fillStyle = GRID_COLOR;
        context.beginPath();
        for (var i = 1; i < this.labels_.length - 1; ++i) {
          // The rounding is needed to avoid ugly 2-pixel wide anti-aliased
          // lines.
          var y = Math.round(this.height_ * i / (this.labels_.length - 1));
          context.moveTo(x1, y);
          context.lineTo(x2, y);
        }
        context.stroke();
      },

      /**
       * Draws a graph line for each of the data series.
       */
      drawLines: function(context) {
        // Factor by which to scale all values to convert them to a number from
        // 0 to height - 1.
        var scale = 0;
        var bottom = this.height_ - 1;
        if (this.max_)
          scale = bottom / this.max_;

        // Draw in reverse order, so earlier data series are drawn on top of
        // subsequent ones.
        for (var i = this.dataSeries_.length - 1; i >= 0; --i) {
          var values = this.getValues(this.dataSeries_[i]);
          if (!values)
            continue;
          context.strokeStyle = this.dataSeries_[i].getColor();
          context.beginPath();
          for (var x = 0; x < values.length; ++x) {
            // The rounding is needed to avoid ugly 2-pixel wide anti-aliased
            // horizontal lines.
            context.lineTo(x, bottom - Math.round(values[x] * scale));
          }
          context.stroke();
        }
      },

      /**
       * Draw labels in |labels_|.
       */
      drawLabels: function(context) {
        if (this.labels_.length == 0)
          return;
        var x;
        if (this.labelAlign_ == LabelAlign.RIGHT) {
          x = this.width_ - LABEL_HORIZONTAL_SPACING;
        } else {
          // Find the width of the widest label.
          var maxTextWidth = 0;
          for (var i = 0; i < this.labels_.length; ++i) {
            var textWidth = context.measureText(this.labels_[i]).width;
            if (maxTextWidth < textWidth)
              maxTextWidth = textWidth;
          }
          x = maxTextWidth + LABEL_HORIZONTAL_SPACING;
        }

        // Set up the context.
        context.fillStyle = TEXT_COLOR;
        context.textAlign = 'right';

        // Draw top label, which is the only one that appears below its tick
        // mark.
        context.textBaseline = 'top';
        context.fillText(this.labels_[0], x, 0);

        // Draw all the other labels.
        context.textBaseline = 'bottom';
        var step = (this.height_ - 1) / (this.labels_.length - 1);
        for (var i = 1; i < this.labels_.length; ++i)
          context.fillText(this.labels_[i], x, step * i);
      }
    };

    return Graph;
  })();

  return TimelineGraphView;
})();

