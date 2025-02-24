// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var SourceFilterParser = (function() {
  'use strict';

  /**
   * Parses |filterText|, extracting a sort method, a list of filters, and a
   * copy of |filterText| with all sort parameters removed.
   */
  function SourceFilterParser(filterText) {
    // Final output will be stored here.
    this.filter = null;
    this.sort = {};
    this.filterTextWithoutSort = '';
    var filterList = parseFilter_(filterText);

    // Text filters are stored here as strings and then added as a function at
    // the end, for performance reasons.
    var textFilters = [];

    // Filter functions are first created individually, and then merged.
    var filterFunctions = [];

    for (var i = 0; i < filterList.length; ++i) {
      var filterElement = filterList[i].parsed;
      var negated = filterList[i].negated;

      var sort = parseSortDirective_(filterElement, negated);
      if (sort) {
        this.sort = sort;
        continue;
      }

      this.filterTextWithoutSort += filterList[i].original;

      var filter = parseRestrictDirective_(filterElement, negated);
      if (!filter)
        filter = parseStringDirective_(filterElement, negated);
      if (filter) {
        if (negated) {
          filter = (function(func, sourceEntry) {
                     return !func(sourceEntry);
                   }).bind(null, filter);
        }
        filterFunctions.push(filter);
        continue;
      }
      textFilters.push({text: filterElement, negated: negated});
    }

    // Create a single filter for all text filters, so they can share a
    // TabePrinter.
    filterFunctions.push(textFilter_.bind(null, textFilters));

    // Create function to go through all the filters.
    this.filter = function(sourceEntry) {
      for (var i = 0; i < filterFunctions.length; ++i) {
        if (!filterFunctions[i](sourceEntry))
          return false;
      }
      return true;
    };
  }

  /**
   * Parses a single "sort:" directive, and returns a dictionary containing
   * the sort function and direction.  Returns null on failure, including
   * the case when no such sort function exists.
   */
  function parseSortDirective_(filterElement, backwards) {
    var match = /^sort:(.*)$/.exec(filterElement);
    if (!match)
      return null;
    return {method: match[1], backwards: backwards};
  }

  /**
   * Tries to parses |filterElement| as a single "is:" directive, and returns a
   * new filter function.  Returns null on failure.
   */
  function parseRestrictDirective_(filterElement) {
    var match = /^is:(.*)$/.exec(filterElement);
    if (!match)
      return null;
    if (match[1] == 'active') {
      return function(sourceEntry) {
        return !sourceEntry.isInactive();
      };
    }
    if (match[1] == 'error') {
      return function(sourceEntry) {
        return sourceEntry.isError();
      };
    }
    return null;
  }

  /**
   * Tries to parse |filterElement| as a single filter of a type that takes
   * arbitrary strings as input, and returns a new filter function on success.
   * Returns null on failure.
   */
  function parseStringDirective_(filterElement) {
    var match = RegExp('^([^:]*):(.*)$').exec(filterElement);
    if (!match)
      return null;

    // Split parameters around commas and remove empty elements.
    var parameters = match[2].split(',');
    parameters = parameters.filter(function(string) {
      return string.length > 0;
    });

    if (match[1] == 'type') {
      return function(sourceEntry) {
        var i;
        var sourceType = sourceEntry.getSourceTypeString().toLowerCase();
        for (i = 0; i < parameters.length; ++i) {
          if (sourceType.search(parameters[i]) != -1)
            return true;
        }
        return false;
      };
    }

    if (match[1] == 'id') {
      return function(sourceEntry) {
        return parameters.indexOf(sourceEntry.getSourceId() + '') != -1;
      };
    }

    return null;
  }

  /**
   * Takes in the text of a filter and returns a list of
   * {parsed, original, negated} values that correspond to substrings of the
   * filter before and after filtering, and whether or not it started with a
   * '-'.  Extra whitespace other than a single character after each element is
   * ignored.  Parsed strings are all lowercase.
   */
  function parseFilter_(filterText) {
    // Assemble a list of quoted and unquoted strings in the filter.
    var filterList = [];
    var position = 0;
    while (position < filterText.length) {
      var inQuote = false;
      var filterElement = '';
      var negated = false;
      var startPosition = position;
      while (position < filterText.length) {
        var nextCharacter = filterText[position];
        ++position;
        if (nextCharacter == '\\' && position < filterText.length) {
          // If there's a backslash, skip the backslash and add the next
          // character to the element.
          filterElement += filterText[position];
          ++position;
          continue;
        } else if (nextCharacter == '"') {
          // If there's an unescaped quote character, toggle |inQuote| without
          // modifying the element.
          inQuote = !inQuote;
        } else if (!inQuote && /\s/.test(nextCharacter)) {
          // If not in a quote and have a whitespace character, that's the
          // end of the element.
          break;
        } else if (nextCharacter == '-' && startPosition == position - 1) {
          // If this is the first character, and it's a '-', this entry is
          // negated.
          negated = true;
        } else {
          // Otherwise, add the next character to the element.
          filterElement += nextCharacter;
        }
      }

      if (filterElement.length > 0) {
        var filter = {
          parsed: filterElement.toLowerCase(),
          original: filterText.substring(startPosition, position),
          negated: negated,
        };
        filterList.push(filter);
      }
    }
    return filterList;
  }

  /**
   * Takes in a list of text filters and a SourceEntry.  Each filter has
   * "text" and "negated" fields.  Returns true if the SourceEntry matches all
   * filters in the (possibly empty) list.
   */
  function textFilter_(textFilters, sourceEntry) {
    let tablePrinter = null;
    for (var i = 0; i < textFilters.length; ++i) {
      let text = textFilters[i].text;
      let negated = textFilters[i].negated;
      let match = false;
      // The description, id, and source type are not always contained in the
      // log entries, so search them directly.
      let description = sourceEntry.getDescription().toLowerCase();
      let type = sourceEntry.getSourceTypeString().toLowerCase();
      let id = sourceEntry.getSourceId() + '';
      if (description.indexOf(text) != -1 || type.indexOf(text) != -1 ||
          id.indexOf(text) != -1) {
        match = true;
      } else {
        if (!tablePrinter)
          tablePrinter = sourceEntry.createTablePrinter(true /* forSearch */);
        match = tablePrinter.search(text);
      }
      if (negated)
        match = !match;
      if (!match)
        return false;
    }
    return true;
  }

  return SourceFilterParser;
})();

