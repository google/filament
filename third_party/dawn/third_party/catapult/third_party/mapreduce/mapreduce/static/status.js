/*
 * Copyright 2010 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/********* Common functions *********/

// Sets the status butter, optionally indicating if it's an error message.
function setButter(message, error) {
  var butter = getButterBar();
  // Prevent flicker on butter update by hiding it first.
  butter.hide();
  if (error) {
    butter.removeClass('info').addClass('error').text(message);
  } else {
    butter.removeClass('error').addClass('info').text(message);
  }
  butter.show();
  $(document).scrollTop(0);
}

// Hides the butter bar.
function hideButter() {
  getButterBar().hide();
}

// Fetches the butter bar dom element.
function getButterBar() {
  return $('#butter');
}


// Renders a value with a collapsable twisty.
function renderCollapsableValue(value, container) {
  var stringValue = $.toJSON(value);
  var SPLIT_LENGTH = 200;
  if (stringValue.length < SPLIT_LENGTH) {
    container.append($('<span>').text(stringValue));
    return;
  }

  var startValue = stringValue.substr(0, SPLIT_LENGTH);
  var endValue = stringValue.substr(SPLIT_LENGTH);

  // Split the end value with <wbr> tags so it looks nice; forced
  // word wrapping never works right.
  var moreSpan = $('<span class="value-disclosure-more">');
  moreSpan.hide();
  for (var i = 0; i < endValue.length; i += SPLIT_LENGTH) {
    moreSpan.append(endValue.substr(i, SPLIT_LENGTH));
    moreSpan.append('<wbr/>');
  }
  var betweenMoreText = '...(' + endValue.length + ' more) ';
  var betweenSpan = $('<span class="value-disclosure-between">')
      .text(betweenMoreText);
  var toggle = $('<a class="value-disclosure-toggle">')
      .text('Expand')
      .attr('href', '');
  toggle.click(function(e) {
      e.preventDefault();
      if (moreSpan.is(':hidden')) {
        betweenSpan.text(' ');
        toggle.text('Collapse');
      } else {
        betweenSpan.text(betweenMoreText);
        toggle.text('Expand');
      }
      moreSpan.toggle();
  });
  container.append($('<span>').text(startValue));
  container.append(moreSpan);
  container.append(betweenSpan);
  container.append(toggle);
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
  } else if (!response) {
    error = 'Could not parse response JSON data.';
  }
  if (error) {
    setButter(error, true);
    return null;
  }
  return response;
}

// Retrieve the list of configs.
function listConfigs(resultFunc) {
  $.ajax({
    type: 'GET',
    url: 'command/list_configs',
    dataType: 'text',
    error: function(request, textStatus) {
      getResponseDataJson(textStatus);
    },
    success: function(data, textStatus, request) {
      var response = getResponseDataJson(null, data);
      if (response) {
        resultFunc(response.configs);
      }
    }
  });
}

// Return the list of job records and notifies the user the content
// is being fetched.
function listJobs(cursor, resultFunc) {
  // If the user is paging then they scrolled down so let's
  // help them by scrolling the window back to the top.
  var jumpToTop = !!cursor;
  cursor = cursor ? cursor : '';
  setButter('Loading');
  $.ajax({
    type: 'GET',
    url: 'command/list_jobs?cursor=' + cursor,
    dataType: 'text',
    error: function(request, textStatus) {
      getResponseDataJson(textStatus);
    },
    success: function(data, textStatus, request) {
      var response = getResponseDataJson(null, data);
      if (response) {
        resultFunc(response.jobs, response.cursor);
        if (jumpToTop) {
          window.scrollTo(0, 0);
        }
        hideButter();  // Hide the loading message.
      }
    }
  });
}

// Cleans up a job with the given name and ID, updates butter with status.
function cleanUpJob(name, mapreduce_id) {
  if (!confirm('Clean up job "' + name +
               '" with ID "' + mapreduce_id + '"?')) {
    return;
  }

  $.ajax({
    async: false,
    type: 'POST',
    url: 'command/cleanup_job',
    data: {'mapreduce_id': mapreduce_id},
    dataType: 'text',
    error: function(request, textStatus) {
      getResponseDataJson(textStatus);
    },
    success: function(data, textStatus, request) {
      var response = getResponseDataJson(null, data);
      if (response) {
        setButter(response.status);
        if (!response.status.error) {
          $('#row-' + mapreduce_id).remove();
        }
      }
    }
  });
}

// Aborts the job with the given ID, updates butter with status.
function abortJob(name, mapreduce_id) {
  if (!confirm('Abort job "' + name + '" with ID "' + mapreduce_id + '"?')) {
    return;
  }

  $.ajax({
    async: false,
    type: 'POST',
    url: 'command/abort_job',
    data: {'mapreduce_id': mapreduce_id},
    dataType: 'text',
    error: function(request, textStatus) {
      getResponseDataJson(textStatus);
    },
    success: function(data, textStatus, request) {
      var response = getResponseDataJson(null, data);
      if (response) {
        setButter(response.status);
      }
    }
  });
}

// Retrieve the detail for a job.
function getJobDetail(jobId, resultFunc) {
  $.ajax({
    type: 'GET',
    url: 'command/get_job_detail',
    dataType: 'text',
    data: {'mapreduce_id': jobId},
    statusCode: {
      404: function() {
        setButter('job ' + jobId + ' was not found.', true);
      }
    },
    error: function(request, textStatus) {
      getResponseDataJson(textStatus);
    },
    success: function(data, textStatus, request) {
      var response = getResponseDataJson(null, data);
      if (response) {
        resultFunc(jobId, response);
      }
    }
  });
}

// Turns a key into a nicely scrubbed parameter name.
function getNiceParamKey(key) {
  // TODO: Figure out if we want to do this at all.
  return key;
}

// Returns an array of the keys of an object in sorted order.
function getSortedKeys(obj) {
  var keys = [];
  $.each(obj, function(key, value) {
    keys.push(key);
  });
  keys.sort();
  return keys;
}

// Convert milliseconds since the epoch to an ISO8601 datestring.
// Consider using new Date().toISOString() instead (J.S 1.8+)
function getIso8601String(timestamp_ms) {
  var time = new Date();
  time.setTime(timestamp_ms);
  return '' +
      time.getUTCFullYear() + '-' +
      leftPadNumber(time.getUTCMonth() + 1, 2, '0') + '-' +
      leftPadNumber(time.getUTCDate(), 2, '0') + 'T' +
      leftPadNumber(time.getUTCHours(), 2, '0') + ':' +
      leftPadNumber(time.getUTCMinutes(), 2, '0') + ':' +
      leftPadNumber(time.getUTCSeconds(), 2, '0') + 'Z';
}

function leftPadNumber(number, minSize, paddingChar) {
  var stringified = '' + number;
  if (stringified.length < minSize) {
    for (var i = 0; i < (minSize - stringified.length); ++i) {
      stringified = paddingChar + stringified;
    }
  }
  return stringified;
}

// Get locale time string for time portion of job runtime. Specially
// handle number of days running as a prefix.
function getElapsedTimeString(start_timestamp_ms, updated_timestamp_ms) {
  var updatedDiff = updated_timestamp_ms - start_timestamp_ms;
  var updatedDays = Math.floor(updatedDiff / 86400000.0);
  updatedDiff -= (updatedDays * 86400000.0);
  var updatedHours = Math.floor(updatedDiff / 3600000.0);
  updatedDiff -= (updatedHours * 3600000.0);
  var updatedMinutes = Math.floor(updatedDiff / 60000.0);
  updatedDiff -= (updatedMinutes * 60000.0);
  var updatedSeconds = Math.floor(updatedDiff / 1000.0);

  var updatedString = '';
  if (updatedDays == 1) {
    updatedString = '1 day, ';
  } else if (updatedDays > 1) {
    updatedString = '' + updatedDays + ' days, ';
  }
  updatedString +=
      leftPadNumber(updatedHours, 2, '0') + ':' +
      leftPadNumber(updatedMinutes, 2, '0') + ':' +
      leftPadNumber(updatedSeconds, 2, '0');

  return updatedString;
}

// Retrieves the mapreduce_id from the query string.
function getJobId() {
  var jobId = $.url().param('mapreduce_id');
  return jobId == null ? '' : jobId;
}

/********* Specific to overview status page *********/

//////// Running jobs overview.
function initJobOverview(jobs, cursor) {
  // Empty body.
  var body = $('#running-list > tbody');
  body.empty();

  if (!jobs || (jobs && jobs.length == 0)) {
    $('<td colspan="8">').text('No job records found.').appendTo(body);
    return;
  }

  // Show header.
  $('#running-list > thead').show();

  // Populate the table.
  $.each(jobs, function(index, job) {
    var row = $('<tr id="row-' + job.mapreduce_id + '">');

    var status = (job.active ? 'running' : job.result_status) || 'unknown';
    row.append($('<td class="status-text">').text(status));

    $('<td>').append(
      $('<a>')
        .attr('href', 'detail?mapreduce_id=' + job.mapreduce_id)
        .text('Detail')).appendTo(row);

    row.append($('<td>').text(job.mapreduce_id))
      .append($('<td>').text(job.name));

    var activity = '' + job.active_shards + ' / ' + job.shards + ' shards';
    row.append($('<td>').text(activity))

    row.append($('<td>').text(getIso8601String(job.start_timestamp_ms)));

    row.append($('<td>').text(getElapsedTimeString(
        job.start_timestamp_ms, job.updated_timestamp_ms)));

    // Controller links for abort, cleanup, etc.
    if (job.active) {
      var control = $('<a href="">').text('Abort')
        .click(function(event) {
          abortJob(job.name, job.mapreduce_id);
          event.stopPropagation();
          return false;
        });
      row.append($('<td>').append(control));
    } else {
      var control = $('<a href="">').text('Cleanup')
        .click(function(event) {
          cleanUpJob(job.name, job.mapreduce_id);
          event.stopPropagation();
          return false;
        });
      row.append($('<td>').append(control));
    }
    row.appendTo(body);
  });

  // Set up the next/first page links.
  $('#running-first-page')
    .show()
    .unbind('click')
    .click(function() {
    listJobs(null, initJobOverview);
    return false;
  });
  $('#running-next-page').unbind('click');
  if (cursor) {
    $('#running-next-page')
      .show()
      .click(function() {
        listJobs(cursor, initJobOverview);
        return false;
      });
  } else {
    $('#running-next-page').hide();
  }
  $('#running-list > tfoot').show();
}

//////// Launching jobs.

// TODO(user): new job parameters shouldn't be hidden by default.
var FIXED_JOB_PARAMS = [
    'name', 'mapper_input_reader', 'mapper_handler', 'mapper_params_validator',
    'mapper_output_writer'
];

var EDITABLE_JOB_PARAMS = ['shard_count', 'processing_rate', 'queue_name'];

function getJobForm(name) {
  return $('form.run-job > input[name="name"][value="' + name + '"]').parent();
}

function showRunJobConfig(name) {
  var matchedForm = null;
  $.each($('form.run-job'), function(index, jobForm) {
    if ($(jobForm).find('input[name="name"]').val() == name) {
      matchedForm = jobForm;
    } else {
      $(jobForm).hide();
    }
  });
  $(matchedForm).show();
}

function runJobDone(name, error, data) {
  var jobForm = getJobForm(name);
  var response = getResponseDataJson(error, data);
  if (response) {
    setButter('Successfully started job "' + response['mapreduce_id'] + '"');
    listJobs(null, initJobOverview);
  }
  jobForm.find('input[type="submit"]').disabled = false;
}

function runJob(name) {
  var jobForm = getJobForm(name);
  jobForm.find('input[type="submit"]').disabled = true;
  $.ajax({
    type: 'POST',
    url: 'command/start_job',
    data: jobForm.serialize(),
    dataType: 'text',
    error: function(request, textStatus) {
      runJobDone(name, textStatus);
    },
    success: function(data, textStatus, request) {
      runJobDone(name, null, data);
    }
  });
}

function initJobLaunching(configs) {
  $('#launch-control').empty();
  if (!configs || (configs && configs.length == 0)) {
    $('#launch-control').append('No job configurations found.');
    return;
  }

  // Set up job config forms.
  $.each(configs, function(index, config) {
    var jobForm = $('<form class="run-job">')
      .submit(function() {
        runJob(config.name);
        return false;
      })
      .hide()
      .appendTo('#launch-container');

    // Fixed job config values.
    $.each(FIXED_JOB_PARAMS, function(unused, key) {
      var value = config[key];
      if (!value) return;
      if (key != 'name') {
        // Name is up in the page title so doesn't need to be shown again.
        $('<p class="job-static-param">')
          .append($('<span class="param-key">').text(getNiceParamKey(key)))
          .append($('<span>').text(': '))
          .append($('<span class="param-value">').text(value))
          .appendTo(jobForm);
      }
      $('<input type="hidden">')
        .attr('name', key)
        .attr('value', value)
        .appendTo(jobForm);
    });

    // Add parameter values to the job form.
    function addParameters(params, prefix) {
      if (!params) {
        return;
      }

      var sortedParams = getSortedKeys(params);
      $.each(sortedParams, function(index, key) {
        var value = params[key];
        var paramId = 'job-' + prefix + key + '-param';
        var paramP = $('<p class="editable-input">');

        // Deal with the case in which the value is an object rather than
        // just the default value string.
        var prettyKey = key;
        if (value && value['human_name']) {
          prettyKey = value['human_name'];
        }

        if (value && value['default_value']) {
          value = value['default_value'];
        }

        $('<label>')
          .attr('for', paramId)
          .text(prettyKey)
          .appendTo(paramP);
        $('<span>').text(': ').appendTo(paramP);
        $('<input type="text">')
          .attr('id', paramId)
          .attr('name', prefix + key)
          .attr('value', value)
          .appendTo(paramP);
        paramP.appendTo(jobForm);
      });
    }

    addParameters(config.params, 'params.');
    addParameters(config.mapper_params, 'mapper_params.');

    $('<input type="submit">')
      .attr('value', 'Run')
      .appendTo(jobForm);
  });

  // Setup job name drop-down.
  var jobSelector = $('<select>')
      .change(function(event) {
        showRunJobConfig($(event.target).val());
      })
      .appendTo('#launch-control');
  $.each(configs, function(index, config) {
    $('<option>')
      .attr('name', config.name)
      .text(config.name)
      .appendTo(jobSelector);
  });
  showRunJobConfig(jobSelector.val());
}

//////// Status page entry point.
function initStatus() {
  listConfigs(initJobLaunching);
  listJobs(null, initJobOverview);
}

/********* Specific to detail status page *********/

//////// Job detail.
function refreshJobDetail(jobId, detail) {
  // Overview parameters.
  var jobParams = $('#detail-params');
  jobParams.empty();

  var status = (detail.active ? 'running' : detail.result_status) || 'unknown';
  $('<li class="status-text">').text(status).appendTo(jobParams);

  $('<li>')
    .append($('<span class="param-key">').text('Elapsed time'))
    .append($('<span>').text(': '))
    .append($('<span class="param-value">').text(getElapsedTimeString(
          detail.start_timestamp_ms, detail.updated_timestamp_ms)))
    .appendTo(jobParams);
  $('<li>')
    .append($('<span class="param-key">').text('Start time'))
    .append($('<span>').text(': '))
    .append($('<span class="param-value">').text(getIso8601String(
          detail.start_timestamp_ms)))
    .appendTo(jobParams);

  $.each(FIXED_JOB_PARAMS, function(index, key) {
    // Skip some parameters or those with no values.
    if (key == 'name') return;
    var value = detail[key];
    if (!value) return;

    $('<li>')
      .append($('<span class="param-key">').text(getNiceParamKey(key)))
      .append($('<span>').text(': '))
      .append($('<span class="param-value">').text('' + value))
      .appendTo(jobParams);
  });

  // User-supplied parameters.
  if (detail.mapper_spec.mapper_params) {
    var sortedKeys = getSortedKeys(detail.mapper_spec.mapper_params);
    $.each(sortedKeys, function(index, key) {
      var value = detail.mapper_spec.mapper_params[key];
      var valueSpan = $('<span class="param-value">');
      renderCollapsableValue(value, valueSpan);
      $('<li>')
        .append($('<span class="user-param-key">').text(key))
        .append($('<span>').text(': '))
        .append(valueSpan)
        .appendTo(jobParams);
    });
  }

  // Graph image.
  var detailGraph = $('#detail-graph');
  detailGraph.empty();
  var chartTitle = 'Processed items per shard';
  if (detail.chart_data) {
    var data = new google.visualization.DataTable();
    data.addColumn('string', 'Shard');
    data.addColumn('number', 'Count');
    var shards = detail.chart_data.length;
    for (var i = 0; i < shards; i++) {
      data.addRow([i.toString(), detail.chart_data[i]]);
    }
    var log2Shards = Math.log(shards) / Math.log(2);
    var chartWidth = Math.max(Math.max(300, shards * 2), 100 * log2Shards);
    var chartHeight = 200;
    var options = {
        legend: 'none',
        bar: {
            groupWidth: '100%'
        },
        vAxis: {
          minValue: 0
        },
        title: chartTitle,
        chartArea: {
          width: chartWidth,
          height: chartHeight
        },
        width: 80 + chartWidth,
        height: 80 + chartHeight
    };
    var chart = new google.visualization.ColumnChart(detailGraph[0]);
    chart.draw(data, options);
  } else {
    $('<div>').text(chartTitle).appendTo(detailGraph);
    $('<img>')
      .attr('src', detail.chart_url)
      .attr('width', detail.chart_width || 300)
      .attr('height', 200)
      .appendTo(detailGraph);
  }

  // Aggregated counters.
  var aggregatedCounters = $('#aggregated-counters');
  aggregatedCounters.empty();
  var runtimeMs = detail.updated_timestamp_ms - detail.start_timestamp_ms;
  var sortedCounters = getSortedKeys(detail.counters);
  $.each(sortedCounters, function(index, key) {
    var value = detail.counters[key];
    // Round to 2 decimal places.
    var avgRate = Math.round(100.0 * value / (runtimeMs / 1000.0)) / 100.0;
    $('<li>')
      .append($('<span class="param-key">').html(getNiceParamKey(key)))
      .append($('<span>').text(': '))
      .append($('<span class="param-value">').html(value))
      .append($('<span>').text(' '))
      .append($('<span class="param-aux">').text('(' + avgRate + '/sec avg.)'))
      .appendTo(aggregatedCounters);
  });

  // Set up the mapper detail.
  var mapperBody = $('#mapper-shard-status');
  mapperBody.empty();

  $.each(detail.shards, function(index, shard) {
    var row = $('<tr>');

    row.append($('<td>').text(shard.shard_number));

    var status = (shard.active ? 'running' : shard.result_status) || 'unknown';
    row.append($('<td>').text(status));

    // TODO: Set colgroup width for shard description.
    row.append($('<td>').text(shard.shard_description));

    row.append($('<td>').text(shard.last_work_item || 'Unknown'));

    row.append($('<td>').text(getElapsedTimeString(
        detail.start_timestamp_ms, shard.updated_timestamp_ms)));

    row.appendTo(mapperBody);
  });
}

function initJobDetail(jobId, detail) {
  // Set titles.
  var title = 'Status for "' + detail.name + '"-- Job #' + jobId;
  $('head > title').text(title);
  $('#detail-page-title').text(detail.name);
  $('#detail-page-undertext').text('Job #' + jobId);

  // Set control buttons.
  if (self != top) {
    $('#overview-link').hide();
  }
  if (detail.active) {
    var control = $('<a href="">')
      .text('Abort Job')
      .click(function(event) {
        abortJob(detail.name, jobId);
        event.stopPropagation();
        return false;
      });
    $('#job-control').append(control);
  } else {
    var control = $('<a href="">')
      .text('Cleanup Job')
      .click(function(event) {
        cleanUpJob(detail.name, jobId);
        event.stopPropagation();
        return false;
      });
    $('#job-control').append(control);
  }

  refreshJobDetail(jobId, detail);
}

//////// Detail page entry point.
function initDetail() {
  var jobId = getJobId();
  if (!jobId) {
    setButter('Could not find job ID in query string.', true);
    return;
  }
  getJobDetail(jobId, initJobDetail);
}
