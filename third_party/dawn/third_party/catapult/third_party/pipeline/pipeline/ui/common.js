/*
 * Copyright 2010 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @author: Brett Slatkin (bslatkin@google.com)
 */

// Format a number with a minimum number of digits and a padding character.
function leftPadNumber(number, minSize, paddingChar) {
  var stringified = '' + number;
  if (stringified.length < minSize) {
    for (var i = 0; i < (minSize - stringified.length); ++i) {
      stringified = paddingChar + stringified;
    }
  }
  return stringified;
}


// Convert milliseconds since the epoch to an ISO8601 datestring.
function getIso8601String(timeMs) {
  var time = new Date();
  time.setTime(timeMs);
  return '' +
      time.getUTCFullYear() + '-' +
      leftPadNumber(time.getUTCMonth() + 1, 2, '0') + '-' +
      leftPadNumber(time.getUTCDate(), 2, '0') + 'T' +
      leftPadNumber(time.getUTCHours(), 2, '0') + ':' +
      leftPadNumber(time.getUTCMinutes(), 2, '0') + ':' +
      leftPadNumber(time.getUTCSeconds(), 2, '0') + 'Z';
}


// Get time string for job runtime. Specially handle number of days running as
// a prefix and milliseconds as a suffix. If the runtime is less than one
// minute, use the format "38.123 seconds" instead.
function getElapsedTimeString(startTimestampMs, updatedTimestampMs) {
  var updatedDiff = Math.max(0, updatedTimestampMs - startTimestampMs);
  var updatedDays = Math.floor(updatedDiff / 86400000.0);
  updatedDiff -= (updatedDays * 86400000.0);
  var updatedHours = Math.floor(updatedDiff / 3600000.0);
  updatedDiff -= (updatedHours * 3600000.0);
  var updatedMinutes = Math.floor(updatedDiff / 60000.0);
  updatedDiff -= (updatedMinutes * 60000.0);
  var updatedSeconds = Math.floor(updatedDiff / 1000.0);
  updatedDiff -= (updatedSeconds * 1000.0);
  var updatedMs = Math.floor(updatedDiff / 1.0);

  var updatedString = '';

  if (updatedMinutes > 0) {
    if (updatedDays == 1) {
      updatedString = '1 day, ';
    } else if (updatedDays > 1) {
      updatedString = '' + updatedDays + ' days, ';
    }
    updatedString +=
        leftPadNumber(updatedHours, 2, '0') + ':' +
        leftPadNumber(updatedMinutes, 2, '0') + ':' +
        leftPadNumber(updatedSeconds, 2, '0');
    if (updatedMs > 0) {
      updatedString += '.' + leftPadNumber(updatedMs, 3, '0');
    }
  } else {
    updatedString += updatedSeconds;
    updatedString += '.' + leftPadNumber(updatedMs, 3, '0');
    updatedString += ' seconds';
  }

  return updatedString;
}


// Clears the status butter.
function clearButter() {
  $('#butter').css('display', 'none');
}


// Sets the status butter, optionally indicating if it's an error message.
function setButter(message, error, traceback, asHtml) {
  var butter = $('#butter');
  // Prevent flicker on butter update by hiding it first.
  butter.css('display', 'none');
  if (error) {
    butter.removeClass('info').addClass('error');
  } else {
    butter.removeClass('error').addClass('info');
  }
  butter.children().remove();
  if (asHtml) {
    butter.append($('<div>').html(message));
  } else {
    butter.append($('<div>').text(message));
  }

  function centerButter() {
    butter.css('left', ($(window).width() - $(butter).outerWidth()) / 2);
  }

  if (traceback) {
    var showDetail = $('<a href="">').text('Detail');
    showDetail.click(function(event) {
      $('#butter-detail').toggle();
      centerButter();
      event.preventDefault();
    });
    var butterDetail = $('<pre id="butter-detail">').text(traceback);
    butterDetail.css('display', 'none');

    butter.append(showDetail);
    butter.append(butterDetail);
  }
  centerButter();
  butter.css('display', null);
}


// Given an AJAX error message (which is empty or null on success) and a
// data payload containing JSON, parses the data payload and returns the object.
// Server-side errors and AJAX errors will be brought to the user's attention
// if present in the response object
function getResponseDataJson(error, data) {
  var response = null;
  try {
    response = $.parseJSON(data);
  } catch (e) {
    error = '' + e;
  }
  if (response && response.error_class) {
    error = response.error_class + ': ' + response.error_message;
    setButter('Error -- ' + error, true, response.error_traceback);
  } else if (!response) {
    setButter('Error -- Could not parse response JSON data.', true);
  } else {
    return response;
  }
  return null;
}
