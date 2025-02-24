// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * TablePrinter is a helper to format a table as ASCII art or an HTML table.
 *
 * Usage: call addRow() and addCell() repeatedly to specify the data.
 *
 * addHeaderCell() can optionally be called to specify header cells for a
 * single header row.  The header row appears at the top of an HTML formatted
 * table, and uses thead and th tags.  In ascii tables, the header is separated
 * from the table body by a partial row of dashes.
 *
 * setTitle() can optionally be used to set a title that is displayed before
 * the header row.  In HTML tables, it uses the title class and in ascii tables
 * it's between two rows of dashes.
 *
 * Once all the fields have been input, call toText() to format it as text or
 * toHTML() to format it as HTML.
 */
var TablePrinter = (function() {
  'use strict';

  /**
   * @constructor
   */
  function TablePrinter() {
    this.rows_ = [];
    this.hasHeaderRow_ = false;
    this.title_ = null;
    // Number of cells automatically added at the start of new rows.
    this.newRowCellIndent_ = 0;
  }

  TablePrinter.prototype = {
    /**
     * Sets the number of blank cells to add after each call to addRow.
     */
    setNewRowCellIndent: function(newRowCellIndent) {
      this.newRowCellIndent_ = newRowCellIndent;
    },

    /**
     * Starts a new row.
     */
    addRow: function() {
      this.rows_.push([]);
      for (var i = 0; i < this.newRowCellIndent_; ++i)
        this.addCell('');
    },

    /**
     * Adds a column to the current row, setting its value to cellText.
     *
     * @return {!TablePrinterCell} the cell that was added.
     */
    addCell: function(cellText) {
      var r = this.rows_[this.rows_.length - 1];
      var cell = new TablePrinterCell(cellText);
      r.push(cell);
      return cell;
    },

    /**
     * Sets the title displayed at the top of a table.  Titles are optional.
     */
    setTitle: function(title) {
      this.title_ = title;
    },

    /**
     * Adds a header row, if not already present, and adds a new column to it,
     * setting its contents to |headerText|.
     *
     * @return {!TablePrinterCell} the cell that was added.
     */
    addHeaderCell: function(headerText) {
      // Insert empty new row at start of |rows_| if currently no header row.
      if (!this.hasHeaderRow_) {
        this.rows_.splice(0, 0, []);
        this.hasHeaderRow_ = true;
      }
      var cell = new TablePrinterCell(headerText);
      this.rows_[0].push(cell);
      return cell;
    },

    /**
     * Returns the maximum number of columns this table contains.
     */
    getNumColumns: function() {
      var numColumns = 0;
      for (var i = 0; i < this.rows_.length; ++i) {
        numColumns = Math.max(numColumns, this.rows_[i].length);
      }
      return numColumns;
    },

    /**
     * Returns the cell at position (rowIndex, columnIndex), or null if there is
     * no such cell.
     */
    getCell_: function(rowIndex, columnIndex) {
      if (rowIndex >= this.rows_.length)
        return null;
      var row = this.rows_[rowIndex];
      if (columnIndex >= row.length)
        return null;
      return row[columnIndex];
    },

    /**
     * Returns true if searchString can be found entirely within a cell.
     * Case insensitive.
     *
     * @param {string} string String to search for, must be lowercase.
     * @return {boolean} True if some cell contains searchString.
     */
    search: function(searchString) {
      var numColumns = this.getNumColumns();
      for (var r = 0; r < this.rows_.length; ++r) {
        for (var c = 0; c < numColumns; ++c) {
          var cell = this.getCell_(r, c);
          if (!cell)
            continue;
          if (cell.text.toLowerCase().indexOf(searchString) != -1)
            return true;
        }
      }
      return false;
    },

    /**
     * Prints a formatted text representation of the table data to the
     * node |parent|.  |spacing| indicates number of extra spaces, if any,
     * to add between columns.
     */
    toText: function(spacing, parent) {
      var pre = addNode(parent, 'pre');
      var numColumns = this.getNumColumns();

      // Figure out the maximum width of each column.
      var columnWidths = [];
      columnWidths.length = numColumns;
      for (var i = 0; i < numColumns; ++i)
        columnWidths[i] = 0;

      // If header row is present, temporarily add a spacer row to |rows_|.
      if (this.hasHeaderRow_) {
        var headerSpacerRow = [];
        for (var c = 0; c < numColumns; ++c) {
          var cell = this.getCell_(0, c);
          if (!cell)
            continue;
          var spacerStr = makeRepeatedString('-', cell.text.length);
          headerSpacerRow.push(new TablePrinterCell(spacerStr));
        }
        this.rows_.splice(1, 0, headerSpacerRow);
      }

      var numRows = this.rows_.length;
      for (var c = 0; c < numColumns; ++c) {
        for (var r = 0; r < numRows; ++r) {
          var cell = this.getCell_(r, c);
          if (cell && !cell.allowOverflow) {
            columnWidths[c] = Math.max(columnWidths[c], cell.text.length);
          }
        }
      }

      var out = [];

      // Print title, if present.
      if (this.title_) {
        var titleSpacerStr = makeRepeatedString('-', this.title_.length);
        out.push(titleSpacerStr);
        out.push('\n');
        out.push(this.title_);
        out.push('\n');
        out.push(titleSpacerStr);
        out.push('\n');
      }

      // Print each row.
      var spacingStr = makeRepeatedString(' ', spacing);
      for (var r = 0; r < numRows; ++r) {
        for (var c = 0; c < numColumns; ++c) {
          var cell = this.getCell_(r, c);
          if (cell) {
            // Pad the cell with spaces to make it fit the maximum column width.
            var padding = columnWidths[c] - cell.text.length;
            var paddingStr = makeRepeatedString(' ', padding);

            if (cell.alignRight)
              out.push(paddingStr);
            if (cell.link) {
              // Output all previous text, and clear |out|.
              addTextNode(pre, out.join(''));
              out = [];

              var linkNode = addNodeWithText(pre, 'a', cell.text);
              linkNode.href = cell.link;
            } else {
              out.push(cell.text);
            }
            if (!cell.alignRight)
              out.push(paddingStr);
            out.push(spacingStr);
          }
        }
        out.push('\n');
      }

      // Remove spacer row under the header row, if one was added.
      if (this.hasHeaderRow_)
        this.rows_.splice(1, 1);

      addTextNode(pre, out.join(''));
    },

    /**
     * Adds a new HTML table to the node |parent| using the specified style.
     */
    toHTML: function(parent, style) {
      var numRows = this.rows_.length;
      var numColumns = this.getNumColumns();

      var table = addNode(parent, 'table');
      table.setAttribute('class', style);

      var thead = addNode(table, 'thead');
      var tbody = addNode(table, 'tbody');

      // Add title, if needed.
      if (this.title_) {
        var tableTitleRow = addNode(thead, 'tr');
        var tableTitle = addNodeWithText(tableTitleRow, 'th', this.title_);
        tableTitle.colSpan = numColumns;
        tableTitle.classList.add('title');
      }

      // Fill table body, adding header row first, if needed.
      for (var r = 0; r < numRows; ++r) {
        var cellType;
        var row;
        if (r == 0 && this.hasHeaderRow_) {
          row = addNode(thead, 'tr');
          cellType = 'th';
        } else {
          row = addNode(tbody, 'tr');
          cellType = 'td';
        }
        for (var c = 0; c < numColumns; ++c) {
          var cell = this.getCell_(r, c);
          if (cell) {
            var tableCell = addNode(row, cellType, cell.text);
            if (cell.alignRight)
              tableCell.alignRight = true;
            // If allowing overflow on the rightmost cell of a row,
            // make the cell span the rest of the columns.  Otherwise,
            // ignore the flag.
            if (cell.allowOverflow && !this.getCell_(r, c + 1))
              tableCell.colSpan = numColumns - c;
            if (cell.link) {
              var linkNode = addNodeWithText(tableCell, 'a', cell.text);
              linkNode.href = cell.link;
            } else {
              addTextNode(tableCell, cell.text);
            }
          }
        }
      }
      return table;
    }
  };

  /**
   * Links are only used in HTML tables.
   */
  function TablePrinterCell(value) {
    this.text = '' + value;
    this.link = null;
    this.alignRight = false;
    this.allowOverflow = false;
  }

  return TablePrinter;
})();

