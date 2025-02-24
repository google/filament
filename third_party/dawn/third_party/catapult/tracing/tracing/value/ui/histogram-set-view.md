<!-- Copyright 2017 The Chromium Authors. All rights reserved.
     Use of this source code is governed by a BSD-style license that can be
     found in the LICENSE file.
-->

# HistogramSet UI Architecture

Documentation for users of this UI is in [metrics-results-ui](/docs/metrics-results-ui.md).

This document outlines the MVC architecture of the implementation of the UI.
 * Model: [HistogramSetViewState](/tracing/tracing/value/ui/histogram_set_view_state.html)
    * searchQuery: regex filters Histogram names
    * referenceDisplayLabel selects the reference column in the table
    * showAll: when false, only sourceHistograms are shown in the table
    * groupings: array of HistogramGroupings configures how the hierarchy is constructed
    * sortColumnIndex
    * sortDescending
    * constrainNameColumn: whether the Name column in the table is constrained to 300px
    * tableRowStates: Map from row name to HistogramSetTableRowState
       * subRows: Map from row name to HistogramSetTableRowState
       * isExpanded: whether the row is expanded to show its subRows
       * isOverviewed: whether the overview charts are displayed
       * cells: map from column names to HistogramSetTableCellState:
          * isOpen: whether the cell's histogram-span is open and displaying the BarChart and Diagnostics
          * brushedBinRange: which bins are brushed in the BarChart
          * mergeSampleDiagnostics: whether sample diagnostics are merged
    * Setters delegate to the main entry point, update(delta), which dispatches an update event to listeners
 * View-Controllers:
    * [histogram-set-view](/tracing/tracing/value/ui/histogram_set_view.html):
       * Main entry point: build(HistogramSet, progressIndicator):Promise
       * Displays "zero Histograms"
       * Listens for download-csv event from [histogram-set-controls](/tracing/tracing/value/ui/histogram_set_controls.html)
          * gets leafHistograms from the [histogram-set-table](/tracing/tracing/value/ui/histogram_set_table.html)
          * builds a CSV using [CSVBuilder](/tracing/tracing/value/csv_builder.html)
       * Collects possible configurations of the HistogramSet and passes them to the child elements directly (not through the HistogramSetViewState!):
          * Possible groupings
          * displayLabels
          * baseStatisticNames
       * Contains child elements:
          * [histogram-set-controls](/tracing/tracing/value/ui/histogram_set_controls.html)
             * visualizes and controls the top half of HistogramSetViewState:
                * searchQuery
                * toggle display of all isOvervieweds
                * referenceDisplayLabel
                * showAll
                * groupings
             * Displays a button to download a CSV of the leafHistograms
             * Displays a "Help" link to [metrics-results-ui](/docs/metrics-results-ui.md)
          * [histogram-set-table](/tracing/tracing/value/ui/histogram_set_table.html)
             * Visualizes and controls the bottom half of HistogramSetViewState:
                * sortColumnIndex
                * sortDescending
                * constrainNameColumn
                * HistogramSetTableRowStates
             * Builds [HistogramSetTableRow](/tracing/tracing/value/ui/histogram_set_table_row.html)s containing
                * [histogram-set-table-name-cell](/tracing/tracing/value/ui/histogram_set_table_name_cell.html)
                   * Toggles HistogramSetTableRowState.isOverviewed
                   * Overview [NameLineChart](/tracing/tracing/ui/base/name_line_chart.html)
                * [histogram-set-table-cell](/tracing/tracing/value/ui/histogram_set_table_cell.html)
                   * (missing) / (empty) / (unmergeable)
                   * when closed, [scalar-span](/tracing/tracing/value/ui/scalar_span.html) displays a single summary statistic
                   * when open, [histogram-span](/tracing/tracing/value/ui/histogram_span.html) contains:
                      * [NameBarChart](/tracing/tracing/ui/base/name_bar_chart.html) visualizes and controls HistogramSetTableCellState.brushedBinRange
                      * [scalar-map-table](/tracing/tracing/value/ui/scalar_map_table.html) of statistics
                      * Two [diagnostic-map-tables](/tracing/tracing/value/ui/diagnostic_map_table.html): one for Histogram.diagnostics and another for the sample diagnostics
                      * A checkbox to visualize and control HistogramSetTableCellState.mergeSampleDiagnostics
                   * Overview [NameLineChart](/tracing/tracing/ui/base/name_line_chart.html)
             * Main entry points:
                * build(allHistograms, sourceHistograms, displayLabels, progressIndicator):Promise
                * onViewStateUpdate_(delta)
    * The [HistogramSetLocation](/tracing/tracing/value/ui/histogram_set_location.html) synchronizes the HistogramSetViewState with the URL using the HTML5 history API.
