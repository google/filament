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

// Global variables.
var AUTO_REFRESH = true;
var ROOT_PIPELINE_ID = null;
var STATUS_MAP = null;
var LANG = null;


// Adjusts the height/width of the embedded status console iframe.
function adjustStatusConsole() {
  var statusConsole = $('#status-console');
  var detail = $('#detail');
  var sidebar = $('#sidebar');
  var control = $('#control');

  // NOTE: 16 px here is the height of the resize grip in most browsers.
  // Need to specify this explicitly because some browsers (eg, Firefox)
  // cause the overflow scrollbars to bounce around the page randomly when
  // they accidentally overlap the resize grip.
  if (statusConsole.css('display') == 'none') {
    var paddingAndMargin = detail.outerHeight() - detail.height() + 16;
    detail.css('max-height', (sidebar.outerHeight() - paddingAndMargin) + 'px');
  } else {
    detail.css('max-height', '200px');
    statusConsole.width(
        $(window).width() - sidebar.outerWidth());
    statusConsole.height(
        $(window).height() - (statusConsole.offset().top + 16));
  }
}


// Gets the ID of the pipeline info in the left-nav.
function getTreePipelineElementId(value) {
  if (value.indexOf('#item-pipeline-') == 0) {
    return value;
  } else {
    return '#item-pipeline-' + value;
  }
}


// Scrolls to element of the pipeline in the tree.
function scrollTreeToPipeline(pipelineIdOrElement) {
  var element = pipelineIdOrElement;
  if (!(pipelineIdOrElement instanceof jQuery)) {
    element = $(getTreePipelineElementId(pipelineIdOrElement));
  }
  $('#sidebar').scrollTop(element.attr('offsetTop'));
  $('#sidebar').scrollLeft(element.attr('offsetLeft'));
}


// Opens all pipelines down to the target one if not already expanded and
// scroll that pipeline into view.
function expandTreeToPipeline(pipelineId) {
  if (pipelineId == null) {
    return;
  }
  var elementId = getTreePipelineElementId(pipelineId);
  var parents = $(elementId).parents('.expandable');
  if (parents.size() > 0) {
    // The toggle function will scroll to highlight the pipeline.
    parents.children('.hitarea').click();
  } else {
    // No children, so just scroll.
    scrollTreeToPipeline(pipelineId);
  }
}


// Handles when the user toggles a leaf of the tree.
function handleTreeToggle(index, element) {
  var parentItem = $(element).parent();
  var collapsing = parentItem.hasClass('expandable');
  if (collapsing) {
  } else {
    // When expanded be sure the pipeline and its children are showing.
    scrollTreeToPipeline(parentItem);
  }
}


// Counts the number of total and active children for the given pipeline.
// Will include the supplied pipeline in the totals.
function countChildren(pipelineId) {
  var current = STATUS_MAP.pipelines[pipelineId];
  if (!current) {
    return [0, 0];
  }
  var total = 1;
  var done = 0;
  if (current.status == 'done') {
    done += 1;
  }
  for (var i = 0, n = current.children.length; i < n; i++) {
    var parts = countChildren(current.children[i]);
    total += parts[0];
    done += parts[1];
  }
  return [total, done];
}


// Create the readable name for the pipeline name.
function prettyName(name, sidebar) {
  var adjustedName = name;
  if (sidebar) {
    var adjustedName = name;
    var parts = name.split('.');
    if (parts.length > 0) {
      adjustedName = parts[parts.length - 1];
    }
  }
  return adjustedName.replace(/\./, '.<wbr>');
}


// Constructs the info div for a stage.
function constructStageNode(pipelineId, infoMap, sidebar) {
  if (!infoMap) {
    return;
  }
  var containerDiv = $('<div class="status-box">');
  containerDiv.addClass('status-' + infoMap.status);

  var detailDiv = $('<div class="detail-link">');
  if (sidebar) {
    detailDiv.append($('<div class="selected-indicator">').html('&#x2023;'));
  }

  var detailLink = $('<a>');
  detailLink.attr('href', '#pipeline-' + pipelineId);
  detailLink.attr('title', 'ID #' + pipelineId);
  detailLink.attr('id', 'link-pipeline-' + pipelineId);
  detailLink.html(prettyName(infoMap.classPath, sidebar));
  detailDiv.append(detailLink);
  containerDiv.append(detailDiv);

  // ID of the pipeline
  if (!sidebar) {
    var pipelineIdDiv = $('<div class="status-pipeline-id">');
    pipelineIdDiv.text('ID #' + pipelineId);
    containerDiv.append(pipelineIdDiv);
  }

  // Broad status category.
  var statusTitleDiv = $('<div class="status-title">');
  if (!sidebar) {
    statusTitleDiv.append($('<span>').text('Status: '));
  }
  statusTitleDiv.append($('<span>').text(infoMap.status));
  containerDiv.append(statusTitleDiv);

  // Determine timing information based on state.
  var statusTimeLabel = null;
  var statusTimeMs = null;
  var statusRuntimeDiv = null;

  if (infoMap.status == 'done') {
    statusRuntimeDiv = $('<div class="status-runtime">');

    var statusTimeSpan = $('<span class="status-time-label">');
    statusTimeSpan.text('Run time: ');
    statusRuntimeDiv.append(statusTimeSpan);

    var runtimeSpan = $('<span class="status-runtime-value">');
    runtimeSpan.text(getElapsedTimeString(
        infoMap.startTimeMs, infoMap.endTimeMs));
    statusRuntimeDiv.append(runtimeSpan);

    statusTimeLabel = 'Complete';
    statusTimeMs = infoMap.endTimeMs;
  } else if (infoMap.status == 'run') {
    statusTimeLabel = 'Started';
    statusTimeMs = infoMap.startTimeMs;
  } else if (infoMap.status == 'retry') {
    statusTimeLabel = 'Will run';
    statusTimeMs = infoMap.startTimeMs;
  } else if (infoMap.status == 'finalizing') {
    statusTimeLabel = 'Complete';
    statusTimeMs = infoMap.endTimeMs;
  } else if (infoMap.status == 'aborted' ||
             infoMap.status == 'canceled') {
    statusTimeLabel = 'Aborted';
    statusTimeMs = infoMap.endTimeMs;
  } else if (infoMap.status == 'waiting') {
    // Do nothing.
  }

  // Last abort message, if any.
  if (infoMap.abortMessage) {
    var abortMessageDiv = $('<div class="status-message abort">');
    abortMessageDiv.append($('<span>').text('Abort Message: '));
    abortMessageDiv.append(
        $('<span class="status-message-text">').text(infoMap.abortMessage));
    containerDiv.append(abortMessageDiv);
  }

  // Last error message that caused a retry, if any.
  if (infoMap.lastRetryMessage) {
    var errorMessageDiv = $('<div class="status-message error">');
    errorMessageDiv.append($('<span>').text('Retry Message: '));
    errorMessageDiv.append(
        $('<span class="status-message-text">').text(infoMap.lastRetryMessage));
    containerDiv.append(errorMessageDiv);
  }

  // User-supplied status message.
  if (infoMap.statusMessage) {
    var statusMessageDiv = $('<div class="status-message normal">');
    statusMessageDiv.append($('<span>').text('Message: '));
    statusMessageDiv.append(
        $('<span class="status-message-text">').text(infoMap.statusMessage));
    containerDiv.append(statusMessageDiv);
  }

  // Completed children count.
  if (infoMap.status == 'run' || infoMap.status == 'done') {
    var counts = countChildren(pipelineId);
    var totalChildren = counts[0];
    var doneChildren = counts[1];
    // Do not count ourselves
    totalChildren--;
    if (infoMap.status == 'done') {
      doneChildren--;
    }
    if (totalChildren > 0 && doneChildren < totalChildren) {
      var doneChildrenDiv = $('<div class="active-children">');
      doneChildrenDiv.append($('<span>').text('Children: '));
      var countText = '' + doneChildren + ' / ' + totalChildren + ' done';
      doneChildrenDiv.append($('<span>').text(countText));
      containerDiv.append(doneChildrenDiv);
    }
  }

  // Number of attempts, if more than one.
  if (infoMap.currentAttempt > 1) {
    var attemptDiv = $('<div class="status-attempt">');
    var attemptTitle = 'Attempt: ';
    if (infoMap.status == 'retry') {
      attemptTitle = 'Next Attempt: ';
    } else if (infoMap.status == 'done') {
      attemptTitle = 'Attempts: ';
    }
    attemptDiv.append($('<span>').text(attemptTitle));
    var attemptText = '' + infoMap.currentAttempt + ' / ' +
        infoMap.maxAttempts + '';
    attemptDiv.append($('<span>').text(attemptText));
    containerDiv.append(attemptDiv);
  }

  // Runtime if present.
  if (statusRuntimeDiv) {
    containerDiv.append(statusRuntimeDiv);
  }

  // Next retry time, complete time, start time.
  if (statusTimeLabel && statusTimeMs) {
    var statusTimeDiv = $('<div class="status-time">');

    var statusTimeSpan = $('<span class="status-time-label">');
    statusTimeSpan.text(statusTimeLabel + ': ');
    statusTimeDiv.append(statusTimeSpan);

    var sinceSpan = $('<abbr class="timeago status-time-since">');
    var isoDate = getIso8601String(statusTimeMs);
    sinceSpan.attr('title', isoDate);
    sinceSpan.text(isoDate);
    sinceSpan.timeago();
    statusTimeDiv.append(sinceSpan);

    containerDiv.append(statusTimeDiv);
  }

  // User-supplied status links.
  var linksDiv = $('<div class="status-links">');
  if (!sidebar) {
    linksDiv.append($('<span>').text('Links: '));
  }
  var foundLinks = 0;
  if (infoMap.statusConsoleUrl) {
    var link = $('<a class="status-console">');
    link.attr('href', infoMap.statusConsoleUrl);
    link.text('Console');
    link.click(function(event) {
      selectPipeline(pipelineId);
      event.preventDefault();
    });
    linksDiv.append(link);
    foundLinks++;
  }
  if (infoMap.statusLinks) {
    $.each(infoMap.statusLinks, function(key, value) {
      var link = $('<a>');
      link.attr('href', value);
      link.text(key);
      link.click(function(event) {
        selectPipeline(pipelineId, key);
        event.preventDefault();
      });
      linksDiv.append(link);
      foundLinks++;
    });
  }
  if (foundLinks > 0) {
    containerDiv.append(linksDiv);
  }

  // Retry parameters.
  if (!sidebar) {
    var retryParamsDiv = $('<div class="status-retry-params">');
    retryParamsDiv.append(
        $('<div class="retry-params-title">').text('Retry parameters'));

    var backoffSecondsDiv = $('<div class="retry-param">');
    $('<span>').text('Backoff seconds: ').appendTo(backoffSecondsDiv);
    $('<span>')
        .text(infoMap.backoffSeconds)
        .appendTo(backoffSecondsDiv);
    retryParamsDiv.append(backoffSecondsDiv);

    var backoffFactorDiv = $('<div class="retry-param">');
    $('<span>').text('Backoff factor: ').appendTo(backoffFactorDiv);
    $('<span>')
        .text(infoMap.backoffFactor)
        .appendTo(backoffFactorDiv);
    retryParamsDiv.append(backoffFactorDiv);

    containerDiv.append(retryParamsDiv);
  }

  function renderCollapsableValue(value, container) {
    var stringValue = $.toJSON(value);
    var SPLIT_LENGTH = 200;
    if (stringValue.length < SPLIT_LENGTH) {
      container.append($('<span>').text(stringValue));
      return;
    }

    var startValue = stringValue.substr(0, SPLIT_LENGTH);
    var endValue = stringValue.substr(SPLIT_LENGTH);

    // Split the end value with <wbr> tags so it looks nice; force
    // word wrapping never works right.
    var moreSpan = $('<span class="value-disclosure-more">');
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
        if (moreSpan.css('display') == 'none') {
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

  // Slot rendering
  function renderSlot(slotKey) {
    var filledMessage = null;
    var slot = STATUS_MAP.slots[slotKey];
    var slotDetailDiv = $('<div class="slot-detail">');
    if (!slot) {
      var keyAbbr = $('<abbr>');
      keyAbbr.attr('title', slotKey);
      keyAbbr.text('Pending slot');
      slotDetailDiv.append(keyAbbr);
      return slotDetailDiv;
    }

    if (slot.status == 'filled') {
      var valueDiv = $('<span class="slot-value-container">');
      valueDiv.append($('<span>').text('Value: '));
      var valueContainer = $('<span class="slot-value">');
      renderCollapsableValue(slot.value, valueContainer);
      valueDiv.append(valueContainer);
      slotDetailDiv.append(valueDiv);

      var filledDiv = $('<div class="slot-filled">');
      filledDiv.append($('<span>').text('Filled: '));
      var isoDate = getIso8601String(slot.fillTimeMs);
      filledDiv.append(
          $('<abbr class="timeago">')
              .attr('title', isoDate)
              .text(isoDate)
              .timeago());
      slotDetailDiv.append(filledDiv);

      filledMessage = 'Filled by';
    } else {
      filledMessage = 'Waiting for';
    }

    var filledMessageDiv = $('<div class="slot-message">');
    filledMessageDiv.append(
        $('<span>').text(filledMessage + ': '));
    var otherPipeline = STATUS_MAP.pipelines[slot.fillerPipelineId];
    if (otherPipeline) {
      var fillerLink = $('<a class="slot-filler">');
      fillerLink
          .attr('title', 'ID #' + slot.fillerPipelineId)
          .attr('href', '#pipeline-' + slot.fillerPipelineId)
          .text(otherPipeline.classPath);
      fillerLink.click(function(event) {
          selectPipeline(slot.fillerPipelineId);
          event.preventDefault();
      });
      filledMessageDiv.append(fillerLink);
    } else {
      filledMessageDiv.append(
          $('<span class="status-pipeline-id">')
              .text('ID #' + slot.fillerPipelineId));
    }
    slotDetailDiv.append(filledMessageDiv);
    return slotDetailDiv;
  }

  // Argument/ouptut rendering
  function renderParam(key, valueDict) {
    var paramDiv = $('<div class="status-param">');

    var nameDiv = $('<span class="status-param-name">');
    nameDiv.text(key + ':');
    paramDiv.append(nameDiv);

    if (valueDict.type == 'slot' && STATUS_MAP.slots) {
      paramDiv.append(renderSlot(valueDict.slotKey));
    } else {
      var valueDiv = $('<span class="status-param-value">');
      renderCollapsableValue(valueDict.value, valueDiv);
      paramDiv.append(valueDiv);
    }

    return paramDiv;
  }

  if (!sidebar && (
      !$.isEmptyObject(infoMap.kwargs) || infoMap.args.length > 0)) {
    var paramDiv = $('<div class="param-container">');
    paramDiv.append(
        $('<div class="param-container-title">')
            .text('Parameters'));

    // Positional arguments
    $.each(infoMap.args, function(index, valueDict) {
      paramDiv.append(renderParam(index, valueDict));
    });

    // Keyword arguments in alphabetical order
    var keywordNames = [];
    $.each(infoMap.kwargs, function(key, value) {
      keywordNames.push(key);
    });
    keywordNames.sort();
    $.each(keywordNames, function(index, key) {
      paramDiv.append(renderParam(key, infoMap.kwargs[key]));
    });

    containerDiv.append(paramDiv);
  }

  // Outputs in alphabetical order, but default first
  if (!sidebar) {
    var outputContinerDiv = $('<div class="outputs-container">');
    outputContinerDiv.append(
        $('<div class="outputs-container-title">')
            .text('Outputs'));

    var outputNames = [];
    $.each(infoMap.outputs, function(key, value) {
      if (key != 'default') {
        outputNames.push(key);
      }
    });
    outputNames.sort();
    outputNames.unshift('default');

    $.each(outputNames, function(index, key) {
      outputContinerDiv.append(renderParam(
            key, {'type': 'slot', 'slotKey': infoMap.outputs[key]}));
    });

    containerDiv.append(outputContinerDiv);
  }

  // Related pipelines
  function renderRelated(relatedList, relatedTitle, classPrefix) {
    var relatedDiv = $('<div>');
    relatedDiv.addClass(classPrefix + '-container');
    relatedTitleDiv = $('<div>');
    relatedTitleDiv.addClass(classPrefix + '-container-title');
    relatedTitleDiv.text(relatedTitle);
    relatedDiv.append(relatedTitleDiv);

    $.each(relatedList, function(index, relatedPipelineId) {
      var relatedInfoMap = STATUS_MAP.pipelines[relatedPipelineId];
      if (relatedInfoMap) {
        var relatedLink = $('<a>');
        relatedLink
            .addClass(classPrefix + '-link')
            .attr('title', 'ID #' + relatedPipelineId)
            .attr('href', '#pipeline-' + relatedPipelineId)
            .text(relatedInfoMap.classPath);
        relatedLink.click(function(event) {
            selectPipeline(relatedPipelineId);
            event.preventDefault();
        });
        relatedDiv.append(relatedLink);
      } else {
        var relatedIdDiv = $('<div>');
        relatedIdDiv
            .addClass(classPrefix + '-pipeline-id')
            .text('ID #' + relatedPipelineId);
        relatedDiv.append(relatedIdDiv);
      }
    });

    return relatedDiv;
  }

  // Run after
  if (!sidebar && infoMap.afterSlotKeys.length > 0) {
    var foundPipelineIds = [];
    $.each(infoMap.afterSlotKeys, function(index, slotKey) {
      if (STATUS_MAP.slots[slotKey]) {
        var slotDict = STATUS_MAP.slots[slotKey];
        if (slotDict.fillerPipelineId) {
          foundPipelineIds.push(slotDict.fillerPipelineId);
        }
      }
    });
    containerDiv.append(
        renderRelated(foundPipelineIds, 'Run after', 'run-after'));
  }

  // Spawned children
  if (!sidebar && infoMap.children.length > 0) {
    containerDiv.append(
        renderRelated(infoMap.children, 'Children', 'child'));
  }

  return containerDiv;
}


// Recursively creates the sidebar. Use null nextPipelineId to create from root.
function generateSidebar(statusMap, nextPipelineId, rootElement) {
  var currentElement = null;

  if (nextPipelineId) {
    currentElement = $('<li>');
    // Value should match return of getTreePipelineElementId
    currentElement.attr('id', 'item-pipeline-' + nextPipelineId);
  } else {
    currentElement = rootElement;
    nextPipelineId = statusMap.rootPipelineId;
  }

  var parentInfoMap = statusMap.pipelines[nextPipelineId];
  currentElement.append(
      constructStageNode(nextPipelineId, parentInfoMap, true));

  if (statusMap.pipelines[nextPipelineId]) {
    var children = statusMap.pipelines[nextPipelineId].children;
    if (children.length > 0) {
      var treeElement = null;
      if (rootElement) {
        treeElement =
            $('<ul id="pipeline-tree" class="treeview-black treeview">');
      } else {
        treeElement = $('<ul>');
      }

      $.each(children, function(index, childPipelineId) {
        var childElement = generateSidebar(statusMap, childPipelineId);
        treeElement.append(childElement);
      });
      currentElement.append(treeElement);
    }
  }
  return currentElement;
}


function selectPipeline(pipelineId, linkName) {
  if (linkName) {
    location.hash = '#pipeline-' + pipelineId + ';' + linkName;
  } else {
    location.hash = '#pipeline-' + pipelineId;
  }
}


// Depth-first search for active pipeline.
function findActivePipeline(pipelineId, isRoot) {
  var infoMap = STATUS_MAP.pipelines[pipelineId];
  if (!infoMap) {
    return null;
  }

  // This is an active leaf node.
  if (infoMap.children.length == 0 && infoMap.status != 'done') {
    return pipelineId;
  }

  // Sort children by start time only.
  var children = infoMap.children.slice(0);
  children.sort(function(a, b) {
    var infoMapA = STATUS_MAP.pipelines[a];
    var infoMapB = STATUS_MAP.pipelines[b];
    if (!infoMapA || !infoMapB) {
      return 0;
    }
    if (infoMapA.startTimeMs && infoMapB.startTimeMs) {
      return infoMapA.startTimeMs - infoMapB.startTimeMs;
    } else {
      return 0;
    }
  });

  for (var i = 0; i < children.length; ++i) {
    var foundPipelineId = findActivePipeline(children[i], false);
    if (foundPipelineId != null) {
      return foundPipelineId;
    }
  }

  return null;
}


function getSelectedPipelineId() {
  var prefix = '#pipeline-';
  var pieces = location.hash.split(';', 2);
  if (pieces[0].indexOf(prefix) == 0) {
    return pieces[0].substr(prefix.length);
  }
  return null;
}


/* Event handlers */
function handleHashChange() {
  var prefix = '#pipeline-';
  var hash = location.hash;
  var pieces = hash.split(';', 2);
  var pipelineId = null;

  if (pieces[0].indexOf(prefix) == 0) {
    pipelineId = pieces[0].substr(prefix.length);
  } else {
    // Bad hash, just show the root pipeline.
    location.hash = '';
    return;
  }

  if (!pipelineId) {
    // No hash means show the root pipeline.
    pipelineId = STATUS_MAP.rootPipelineId;
  }
  var rootMap = STATUS_MAP.pipelines[STATUS_MAP.rootPipelineId];
  var infoMap = STATUS_MAP.pipelines[pipelineId];
  if (!rootMap || !infoMap) {
    // Hash not found.
    return;
  }

  // Clear any selection styling.
  $('.selected-link').removeClass('selected-link');

  if (pieces[1]) {
    // Show a specific status link.
    var statusLink = $(getTreePipelineElementId(pipelineId))
        .find('.status-links>a:contains("' + pieces[1] + '")');
    if (statusLink.size() > 0) {
      var selectedLink = $(statusLink[0]);
      selectedLink.addClass('selected-link');
      $('#status-console').attr('src', selectedLink.attr('href'));
      $('#status-console').show();
    } else {
      // No console link for this pipeline; ignore it.
      $('#status-console').hide();
    }
  } else {
    // Show the console link.
    var consoleLink = $(getTreePipelineElementId(pipelineId))
        .find('a.status-console');
    if (consoleLink.size() > 0) {
      var selectedLink = $(consoleLink[0]);
      selectedLink.addClass('selected-link');
      $('#status-console').attr('src', selectedLink.attr('href'));
      $('#status-console').show();
    } else {
      // No console link for this pipeline; ignore it.
      $('#status-console').hide();
    }
  }

  // Mark the pipeline as selected.
  var selected = $('#link-pipeline-' + pipelineId);
  selected.addClass('selected-link');
  selected.parents('.status-box').addClass('selected-link');

  // Title is always the info for the root pipeline, to make it easier to
  // track across multiple tabs.
  document.title = rootMap.classPath + ' - ID #' + STATUS_MAP.rootPipelineId;

  // Update the detail status frame.
  var stageNode = constructStageNode(pipelineId, infoMap, false);
  $('#overview').remove();
  stageNode.attr('id', 'overview');
  $('#detail').append(stageNode);

  // Make sure everything is the right size.
  adjustStatusConsole();
}


function handleAutoRefreshClick(event) {
  var loc = window.location;
  var newSearch = null;
  if (!AUTO_REFRESH && event.target.checked) {
    newSearch = '?root=' + ROOT_PIPELINE_ID;
  } else if (AUTO_REFRESH && !event.target.checked) {
    newSearch = '?root=' + ROOT_PIPELINE_ID + '&auto=false';
  }

  if (newSearch != null) {
    loc.replace(
        loc.protocol + '//' + loc.host + loc.pathname +
        newSearch + loc.hash);
  }
}


function handleRefreshClick(event) {
  var loc = window.location;
  if (AUTO_REFRESH) {
    newSearch = '?root=' + ROOT_PIPELINE_ID;
  } else {
    newSearch = '?root=' + ROOT_PIPELINE_ID + '&auto=false';
  }
  loc.href = loc.protocol + '//' + loc.host + loc.pathname + newSearch;
  return false;
}

function handleDeleteClick(event) {
  var ajaxRequest = {
    type: 'GET',
    url: 'rpc/delete?root_pipeline_id=' + ROOT_PIPELINE_ID,
    dataType: 'text',
    error: function(request, textStatus) {
      if (request.status == 404) {
        setButter('Pipeline is already deleted');
      } else {
        setButter('Delete request failed: ' + textStatus);
      }
      window.setTimeout(function() {
        clearButter();
      }, 5000);
    },
    success: function(data, textStatus, request) {
      setButter('Delete request was sent');
      window.setTimeout(function() {
        window.location.href = 'list';
      }, 5000);
    }
  };
  $.ajax(jQuery.extend({}, ajaxRequest));
}

function handleAbortClick(event) {
  var ajaxRequest = {
    type: 'GET',
    url: 'rpc/abort?root_pipeline_id=' + ROOT_PIPELINE_ID,
    dataType: 'text',
    error: function(request, textStatus) {
      setButter('Abort request failed: ' + textStatus);
      window.setTimeout(function() {
        clearButter();
      }, 5000);
    },
    success: function(data, textStatus, request) {
      setButter('Abort request was sent');
      window.setTimeout(function() {
        clearButter();
        window.location.reload();
      }, 5000);
    }
  };
  if (confirm('Are you sure you want to abort the pipeline', 'Abort')) {
    $.ajax(jQuery.extend({}, ajaxRequest));
  }
}

/* Initialization. */
function initStatus() {
  if (window.location.search.length > 0 &&
      window.location.search[0] == '?') {
    var query = window.location.search.substr(1);
    var pieces = query.split('&');
    $.each(pieces, function(index, param) {
      var mapping = param.split('=');
      if (mapping.length != 2) {
        return;
      }
      if (mapping[0] == 'auto' && mapping[1] == 'false') {
        AUTO_REFRESH = false;
      } else if (mapping[0] == 'root') {
        ROOT_PIPELINE_ID = mapping[1];
        if (ROOT_PIPELINE_ID.match(/^pipeline-/)) {
          ROOT_PIPELINE_ID = ROOT_PIPELINE_ID.substring(9);
        }
      }
    });
  }

  if (!Boolean(ROOT_PIPELINE_ID)) {
    setButter('Missing root param' +
        '. For a job list click <a href="list">here</a>.',
        true, null, true);
    return;
  }

  var loadingMsg = 'Loading... #' + ROOT_PIPELINE_ID;
  var attempts = 1;
  var ajaxRequest = {
    type: 'GET',
    url: 'rpc/tree?root_pipeline_id=' + ROOT_PIPELINE_ID,
    dataType: 'text',
    error: function(request, textStatus) {
      if (request.status == 404) {
        if (++attempts <= 5) {
          setButter(loadingMsg + ' [attempt #' + attempts + ']');
          window.setTimeout(function() {
            $.ajax(jQuery.extend({}, ajaxRequest));
          }, 2000);
        } else {
          setButter('Could not find pipeline #' + ROOT_PIPELINE_ID +
              '. For a job list click <a href="list">here</a>.',
              true, null, true);
        }
      } else if (request.status == 449) {
        var root = request.getResponseHeader('root_pipeline_id');
        var newURL = '?root=' + root + '#pipeline-' + ROOT_PIPELINE_ID;
        window.location.replace(newURL);
      } else {
        getResponseDataJson(textStatus);
      }
    },
    success: function(data, textStatus, request) {
      var response = getResponseDataJson(null, data);
      if (response) {
        clearButter();
        STATUS_MAP = response;
        LANG = request.getResponseHeader('Pipeline-Lang');
        initStatusDone();
      }
    }
  };
  setButter(loadingMsg);
  $.ajax(jQuery.extend({}, ajaxRequest));
}


function initStatusDone() {
  jQuery.timeago.settings.allowFuture = true;

  // Update the root pipeline ID to match what the server returns. This handles
  // the case where the ID specified is for a child node. We always want to
  // show status up to the root.
  ROOT_PIPELINE_ID = STATUS_MAP.rootPipelineId;

  // Generate the sidebar.
  generateSidebar(STATUS_MAP, null, $('#sidebar'));

  // Turn the sidebar into a tree.
  $('#pipeline-tree').treeview({
    collapsed: true,
    unique: false,
    cookieId: 'pipeline Id here',
    toggle: handleTreeToggle
  });
  $('#sidebar').show();

  var rootStatus = STATUS_MAP.pipelines[STATUS_MAP.rootPipelineId].status;
  var isFinalState = /^done$|^aborted$|^canceled$/.test(rootStatus);

    // Init the control panel.
  $('#auto-refresh').click(handleAutoRefreshClick);
  if (!AUTO_REFRESH) {
    $('#auto-refresh').attr('checked', '');
  } else {
    if (!isFinalState) {
      // Only do auto-refresh behavior if we're not in a terminal state.
      window.setTimeout(function() {
        var loc = window.location;
        var search = '?root=' + ROOT_PIPELINE_ID;
        loc.replace(loc.protocol + '//' + loc.host + loc.pathname + search);
      }, 30 * 1000);
    }
  }
  $('.refresh-link').click(handleRefreshClick);
  $('.abort-link').click(handleAbortClick);
  $('.delete-link').click(handleDeleteClick);
  if (LANG == 'Java') {
    if (isFinalState) {
      $('.delete-link').show();
    } else {
      $('.abort-link').show();
    }
  }
  $('#control').show();

  // Properly adjust the console iframe to match the window size.
  $(window).resize(adjustStatusConsole);
  window.setTimeout(adjustStatusConsole, 0);

  // Handle ajax-y URL fragment events.
  $(window).hashchange(handleHashChange);
  $(window).hashchange();  // Trigger for initial load.

  // When there's no hash selected, auto-navigate to the most active node.
  if (window.location.hash == '' || window.location.hash == '#') {
    var activePipelineId = findActivePipeline(STATUS_MAP.rootPipelineId, true);
    if (activePipelineId) {
      selectPipeline(activePipelineId);
    } else {
      // If there's nothing active, then select the root.
      selectPipeline(ROOT_PIPELINE_ID);
    }
  }

  // Scroll to the current active node.
  expandTreeToPipeline(getSelectedPipelineId());
}
