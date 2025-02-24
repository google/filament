/*
 * Copyright 2012 Google Inc.
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


function initRootList() {
  setButter('Loading root jobs...');
  $.ajax({
    type: 'GET',
    url: 'rpc/list' + window.location.search,
    dataType: 'text',
    error: function(request, textStatus) {
      getResponseDataJson(textStatus);
    },
    success: function(data, textStatus, request) {
      var response = getResponseDataJson(null, data);
      if (response) {
        clearButter();
        initRootListDone(response);
      }
    }
  });
}


function initRootListDone(response) {
  if (response.pipelines && response.pipelines.length > 0) {
    $('#root-list').show();
    if (response.cursor) {
      // Prepend the cursor to the next link. This may have a suffix of
      // the class_path from initRootNamesDone() below.
      var href = $('#next-link').attr('href');
      $('#next-link').attr('href', '?cursor=' + response.cursor + href);
      $('#next-link').show();
    }

    $.each(response.pipelines, function(index, infoMap) {
      var row = $('<tr>');
      $('<td class="class-path">').text(infoMap.classPath).appendTo(row);
      $('<td class="status">').text(infoMap.status).appendTo(row);

      if (infoMap.startTimeMs) {
        var sinceSpan = $('<abbr class="timeago">');
        var isoDate = getIso8601String(infoMap.startTimeMs);
        sinceSpan.attr('title', isoDate);
        sinceSpan.text(isoDate);
        sinceSpan.timeago();
        $('<td class="start-time">').append(sinceSpan).appendTo(row);
      } else {
        $('<td class="start-time">').text('-').appendTo(row);
      }

      if (infoMap.endTimeMs) {
        $('<td class="run-time">').text(getElapsedTimeString(
            infoMap.startTimeMs, infoMap.endTimeMs)).appendTo(row);
      } else {
        $('<td class="run-time">').text('-').appendTo(row);
      }

      $('<td class="links">')
          .append(
            $('<a>')
                .attr('href', 'status?root=' + infoMap.pipelineId)
                .text(infoMap.pipelineId))
          .appendTo(row);
      $('#root-list>tbody').append(row);
    });
  } else {
    $('#empty-list-message').text('No pipelines found.').show();
  }

  initRootNames();
}


function initRootNames() {
  setButter('Loading names...');
  $.ajax({
    type: 'GET',
    url: 'rpc/class_paths',
    dataType: 'text',
    error: function(request, textStatus) {
      getResponseDataJson(textStatus);
    },
    success: function(data, textStatus, request) {
      var response = getResponseDataJson(null, data);
      if (response) {
        clearButter();
        initRootNamesDone(response);
      }
    }
  });
}


function initRootNamesDone(response) {
  if (response.classPaths) {
    var filterMenu = $('#filter_menu');

    $.each(response.classPaths, function(index, path) {
      // Ignore internal pipelines.
      if (path.match(/\.?pipeline\./)) {
        return;
      }

      var option = $('<option>').val(path).text(path);
      if (window.location.search.indexOf(path) != -1) {
        option.attr('selected', 'selected');
        // Append the class name selected to the "next page" link. This
        // may already have a value from initRootListDone() above.
        var href = $('#next-link').attr('href');
        $('#next-link').attr('href', href + '&class_path=' + path);
      }
      option.appendTo(filterMenu);
    });
  }
}
