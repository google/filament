'use strict';
/**
 * Interface for classes implementing a plotting strategy
 * to be provided to the graph plotter.
 *
 * @interface
 */
class Plotter {
  /**
   * Plots the supplied graph to the chart.
   * @param {GraphData} graph The data to be plotted.
   * @param {Object} chart d3 selection for the chart element to be drawn on.
   * @param {Object} legend d3 selection for the legend element for
   * additional information to be drawn on.
   * @param {Object} chartDimensions The margins, width and height
   * of the chart. This is useful for computing appropriates axis
   * scales and positioning elements.
   */
  plot(graph, chart, legend, chartDimensions) {
    throw new Error(
        'Cannot call functions on an interface.' +
        'Provide a concrete plotter implementation.');
  }
}
