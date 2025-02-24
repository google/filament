// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * Different data types that each require their own labelled axis.
 */
var TimelineDataType = {SOURCE_COUNT: 0, BYTES_PER_SECOND: 1};

/**
 * A TimelineDataSeries collects an ordered series of (time, value) pairs,
 * and converts them to graph points.  It also keeps track of its color and
 * current visibility state.  DataSeries are solely responsible for tracking
 * data, and do not send notifications on state changes.
 *
 * Abstract class, doesn't implement onReceivedLogEntry.
 */
var TimelineDataSeries = (function() {
  'use strict';

  /**
   * @constructor
   */
  function TimelineDataSeries(dataType) {
    // List of DataPoints in chronological order.
    this.dataPoints_ = [];

    // Data type of the DataSeries.  This is used to scale all values with
    // the same units in the same way.
    this.dataType_ = dataType;
    // Default color.  Should always be overridden prior to display.
    this.color_ = 'red';
    // Whether or not the data series should be drawn.
    this.isVisible_ = false;

    this.cacheStartTime_ = null;
    this.cacheStepSize_ = 0;
    this.cacheValues_ = [];
  }

  TimelineDataSeries.prototype = {
    /**
     * Adds a DataPoint to |this| with the specified time and value.
     * DataPoints are assumed to be received in chronological order.
     */
    addPoint: function(timeTicks, value) {
      var time = timeutil.convertTimeTicksToDate(timeTicks).getTime();
      this.dataPoints_.push(new DataPoint(time, value));
    },

    isVisible: function() {
      return this.isVisible_;
    },

    show: function(isVisible) {
      this.isVisible_ = isVisible;
    },

    getColor: function() {
      return this.color_;
    },

    setColor: function(color) {
      this.color_ = color;
    },

    getDataType: function() {
      return this.dataType_;
    },

    /**
     * Returns a list containing the values of the data series at |count|
     * points, starting at |startTime|, and |stepSize| milliseconds apart.
     * Caches values, so showing/hiding individual data series is fast, and
     * derived data series can be efficiently computed, if we add any.
     */
    getValues: function(startTime, stepSize, count) {
      // Use cached values, if we can.
      if (this.cacheStartTime_ == startTime &&
          this.cacheStepSize_ == stepSize &&
          this.cacheValues_.length == count) {
        return this.cacheValues_;
      }

      // Do all the work.
      this.cacheValues_ = this.getValuesInternal_(startTime, stepSize, count);
      this.cacheStartTime_ = startTime;
      this.cacheStepSize_ = stepSize;

      return this.cacheValues_;
    },

    /**
     * Does all the work of getValues when we can't use cached data.
     *
     * The default implementation just uses the |value| of the most recently
     * seen DataPoint before each time, but other DataSeries may use some
     * form of interpolation.
     * TODO(mmenke):  Consider returning the maximum value over each interval
     *                to create graphs more stable with respect to zooming.
     */
    getValuesInternal_: function(startTime, stepSize, count) {
      var values = [];
      var nextPoint = 0;
      var currentValue = 0;
      var time = startTime;
      for (var i = 0; i < count; ++i) {
        while (nextPoint < this.dataPoints_.length &&
               this.dataPoints_[nextPoint].time < time) {
          currentValue = this.dataPoints_[nextPoint].value;
          ++nextPoint;
        }
        values[i] = currentValue;
        time += stepSize;
      }
      return values;
    }
  };

  /**
   * A single point in a data series.  Each point has a time, in the form of
   * milliseconds since the Unix epoch, and a numeric value.
   * @constructor
   */
  function DataPoint(time, value) {
    this.time = time;
    this.value = value;
  }

  return TimelineDataSeries;
})();

/**
 * Tracks how many sources of the given type have seen a begin
 * event of type |eventType| more recently than an end event.
 */
var SourceCountDataSeries = (function() {
  'use strict';

  var superClass = TimelineDataSeries;

  /**
   * @constructor
   */
  function SourceCountDataSeries(sourceType, eventType) {
    superClass.call(this, TimelineDataType.SOURCE_COUNT);
    this.sourceType_ = sourceType;
    this.eventType_ = eventType;

    // Map of sources for which we've seen a begin event more recently than an
    // end event.  Each such source has a value of "true".  All others are
    // undefined.
    this.activeSources_ = {};
    // Number of entries in |activeSources_|.
    this.activeCount_ = 0;
  }

  SourceCountDataSeries.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    onReceivedLogEntry: function(entry) {
      if (entry.source.type != this.sourceType_ ||
          entry.type != this.eventType_) {
        return;
      }

      if (entry.phase == EventPhase.PHASE_BEGIN) {
        this.onBeginEvent(entry.source.id, entry.time);
        return;
      }
      if (entry.phase == EventPhase.PHASE_END)
        this.onEndEvent(entry.source.id, entry.time);
    },

    /**
     * Called when the source with the specified id begins doing whatever we
     * care about.  If it's not already an active source, we add it to the map
     * and add a data point.
     */
    onBeginEvent: function(id, time) {
      if (this.activeSources_[id])
        return;
      this.activeSources_[id] = true;
      ++this.activeCount_;
      this.addPoint(time, this.activeCount_);
    },

    /**
     * Called when the source with the specified id stops doing whatever we
     * care about.  If it's an active source, we remove it from the map and add
     * a data point.
     */
    onEndEvent: function(id, time) {
      if (!this.activeSources_[id])
        return;
      delete this.activeSources_[id];
      --this.activeCount_;
      this.addPoint(time, this.activeCount_);
    }
  };

  return SourceCountDataSeries;
})();

/**
 * Tracks the number of sockets currently in use.  Needs special handling of
 * SSL sockets, so can't just use a normal SourceCountDataSeries.
 */
var SocketsInUseDataSeries = (function() {
  'use strict';

  var superClass = SourceCountDataSeries;

  /**
   * @constructor
   */
  function SocketsInUseDataSeries() {
    superClass.call(this, EventSourceType.SOCKET, EventType.SOCKET_IN_USE);
  }

  SocketsInUseDataSeries.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    onReceivedLogEntry: function(entry) {
      // SSL sockets have two nested SOCKET_IN_USE events.  This is needed to
      // mark SSL sockets as unused after SSL negotiation.
      if (entry.type == EventType.SSL_CONNECT &&
          entry.phase == EventPhase.PHASE_END) {
        this.onEndEvent(entry.source.id, entry.time);
        return;
      }
      superClass.prototype.onReceivedLogEntry.call(this, entry);
    }
  };

  return SocketsInUseDataSeries;
})();

/**
 * Tracks approximate data rate using individual data transfer events.
 * Abstract class, doesn't implement onReceivedLogEntry.
 */
var TransferRateDataSeries = (function() {
  'use strict';

  var superClass = TimelineDataSeries;

  /**
   * @constructor
   */
  function TransferRateDataSeries() {
    superClass.call(this, TimelineDataType.BYTES_PER_SECOND);
  }

  TransferRateDataSeries.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    /**
     * Returns the average data rate over each interval, only taking into
     * account transfers that occurred within each interval.
     * TODO(mmenke): Do something better.
     */
    getValuesInternal_: function(startTime, stepSize, count) {
      // Find the first DataPoint after |startTime| - |stepSize|.
      var nextPoint = 0;
      while (nextPoint < this.dataPoints_.length &&
             this.dataPoints_[nextPoint].time < startTime - stepSize) {
        ++nextPoint;
      }

      var values = [];
      var time = startTime;
      for (var i = 0; i < count; ++i) {
        // Calculate total bytes transferred from |time| - |stepSize|
        // to |time|.  We look at the transfers before |time| to give
        // us generally non-varying values for a given time.
        var transferred = 0;
        while (nextPoint < this.dataPoints_.length &&
               this.dataPoints_[nextPoint].time < time) {
          transferred += this.dataPoints_[nextPoint].value;
          ++nextPoint;
        }
        // Calculate bytes per second.
        values[i] = 1000 * transferred / stepSize;
        time += stepSize;
      }
      return values;
    }
  };

  return TransferRateDataSeries;
})();

/**
 * Tracks TCP and UDP transfer rate.
 */
var NetworkTransferRateDataSeries = (function() {
  'use strict';

  var superClass = TransferRateDataSeries;

  /**
   * |tcpEvent| and |udpEvent| are the event types for data transfers using
   * TCP and UDP, respectively.
   * @constructor
   */
  function NetworkTransferRateDataSeries(tcpEvent, udpEvent) {
    superClass.call(this);
    this.tcpEvent_ = tcpEvent;
    this.udpEvent_ = udpEvent;
  }

  NetworkTransferRateDataSeries.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    onReceivedLogEntry: function(entry) {
      if (entry.type != this.tcpEvent_ && entry.type != this.udpEvent_)
        return;
      this.addPoint(entry.time, entry.params.byte_count);
    },
  };

  return NetworkTransferRateDataSeries;
})();

/**
 * Tracks disk cache read or write rate.  Doesn't include clearing, opening,
 * or dooming entries, as they don't have clear size values.
 */
var DiskCacheTransferRateDataSeries = (function() {
  'use strict';

  var superClass = TransferRateDataSeries;

  /**
   * @constructor
   */
  function DiskCacheTransferRateDataSeries(eventType) {
    superClass.call(this);
    this.eventType_ = eventType;
  }

  DiskCacheTransferRateDataSeries.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    onReceivedLogEntry: function(entry) {
      if (entry.source.type != EventSourceType.DISK_CACHE_ENTRY ||
          entry.type != this.eventType_ ||
          entry.phase != EventPhase.PHASE_END) {
        return;
      }
      // The disk cache has a lot of 0-length writes, when truncating entries.
      // Ignore those.
      if (entry.params.bytes_copied != 0)
        this.addPoint(entry.time, entry.params.bytes_copied);
    }
  };

  return DiskCacheTransferRateDataSeries;
})();

