// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var timeutil = (function() {
  'use strict';

  /**
   * Offset needed to convert event times to Date objects.
   * Updated whenever constants are loaded.
   */
  var timeTickOffset = 0;

  /**
   * The time of the first observed event.  Used for more friendly time display.
   */
  var baseTime = 0;

  /**
   * Sets the offset used to convert tick counts to dates.
   */
  function setTimeTickOffset(offset) {
    // Note that the subtraction by 0 is to cast to a number (probably a float
    // since the numbers are big).
    timeTickOffset = offset - 0;
  }

  /**
   * The browser gives us times in terms of "time ticks" in milliseconds.
   * This function converts the tick count to a Javascript "time", which is
   * the UTC time in milliseconds.
   *
   * @param {string} timeTicks A time represented in "time ticks".
   * @return {number} The Javascript time that |timeTicks| represents.
   */
  function convertTimeTicksToTime(timeTicks) {
    return timeTickOffset + (timeTicks - 0);
  }

  /**
   * The browser gives us times in terms of "time ticks" in milliseconds.
   * This function converts the tick count to a Date() object.
   *
   * @param {string} timeTicks A time represented in "time ticks".
   * @return {Date} The time that |timeTicks| represents.
   */
  function convertTimeTicksToDate(timeTicks) {
    return new Date(convertTimeTicksToTime(timeTicks));
  }

  /**
   * Returns the current time.
   *
   * @return {number} Milliseconds since the Unix epoch.
   */
  function getCurrentTime() {
    return Date.now();
  }

  /**
   * Returns the curent time in time ticks.
   *
   * @return {number} Current time, in TimeTicks.
   */
  function getCurrentTimeTicks() {
    return getCurrentTime() - timeTickOffset;
  }

  /**
   * Sets the base time more friendly display.
   *
   * @param {string} firstEventTime The time of the first event, as a Javascript
   *     numeric time.  Other times can be displayed relative to this time.
   */
  function setBaseTime(firstEventTime) {
    baseTime = firstEventTime;
  }

  /**
   * Sets the base time more friendly display.
   *
   * @return {number} Time set by setBaseTime, or 0 if no time has been set.
   */
  function getBaseTime() {
    return baseTime;
  }

  /**
   * Clears the base time, so isBaseTimeSet() returns 0.
   */
  function clearBaseTime() {
    baseTime = 0;
  }

  /**
   * Returns true if the base time has been initialized.
   *
   * @return {bool} True if the base time is set.
   */
  function isBaseTimeSet() {
    return baseTime != 0;
  }

  /**
   * Takes in a "time ticks" and returns it as a time since the base time, in
   * milliseconds.
   *
   * @param {string} timeTicks A time represented in "time ticks".
   * @return {number} Milliseconds since the base time.
   */
  function convertTimeTicksToRelativeTime(timeTicks) {
    return convertTimeTicksToTime(timeTicks) - baseTime;
  }

  /**
   * Adds an HTML representation of |date| to |parentNode|.
   *
   * @param {DomNode} parentNode The node that will contain the new node.
   * @param {Date} date The date to be displayed.
   * @return {DomNode} The new node containing the date/time.
   */
  function addNodeWithDate(parentNode, date) {
    var span = addNodeWithText(parentNode, 'span', dateToString(date));
    span.title = 't=' + date.getTime();
    return span;
  }

  /**
   * Returns a string representation of |date|.
   *
   * @param {Date} date The date to be represented.
   * @return {string} A string representation of |date|.
   */
  function dateToString(date) {
    var dateStr = date.getFullYear() + '-' + zeroPad_(date.getMonth() + 1, 2) +
        '-' + zeroPad_(date.getDate(), 2);

    var timeStr = zeroPad_(date.getHours(), 2) + ':' +
        zeroPad_(date.getMinutes(), 2) + ':' + zeroPad_(date.getSeconds(), 2) +
        '.' + zeroPad_(date.getMilliseconds(), 3);

    return dateStr + ' ' + timeStr;
  }

  /**
   * Prefixes enough zeros to |num| so that it has length |len|.
   * @param {number} num The number to be padded.
   * @param {number} len The desired length of the returned string.
   * @return {string} The zero-padded representation of |num|.
   */
  function zeroPad_(num, len) {
    var str = num + '';
    while (str.length < len)
      str = '0' + str;
    return str;
  }

  return {
    setTimeTickOffset: setTimeTickOffset,
    convertTimeTicksToTime: convertTimeTicksToTime,
    convertTimeTicksToDate: convertTimeTicksToDate,
    getCurrentTime: getCurrentTime,
    getCurrentTimeTicks: getCurrentTimeTicks,
    setBaseTime: setBaseTime,
    getBaseTime: getBaseTime,
    clearBaseTime: clearBaseTime,
    isBaseTimeSet: isBaseTimeSet,
    convertTimeTicksToRelativeTime: convertTimeTicksToRelativeTime,
    addNodeWithDate: addNodeWithDate,
    dateToString: dateToString,
  };
})();
