'use strict';
/**
 * Concrete implementation of Plotter strategy for creating
 * box plots.
 * @implements {Plotter}
 */
class BoxPlotter {
  /**
   * Initalises the chart by computing the scales for the axes and
   * drawing them. It also applies the labels to the graph (axes and
   * graph titles).
   * @param {GraphData} graph Data to be drawn.
   * @param {Object} chart Chart to draw to.
   * @param {Object} chartDimensions Size of the chart.
   */
  initChart_(graph, chart, chartDimensions) {
    this.scaleForXAxis_ = this.createXAxisScale_(graph, chartDimensions);
    this.xAxisGenerator_ = d3.axisBottom(this.scaleForXAxis_);
    // Draw the x-axis.
    chart.append('g')
        .call(this.xAxisGenerator_)
        .attr('transform', `translate(0, ${chartDimensions.height})`);
  }

  createXAxisScale_(graph, chartDimensions) {
    return d3.scaleLinear()
        .domain([0, graph.max(x => x) * 1.1])
        .range([0, chartDimensions.width]);
  }

  /**
   * Draws a box plot to the canvas. If there are multiple dataSources it will
   * plot them both and label their colors in the legend.
   * @param {GraphData} graph The data to be plotted.
   * @param {Object} chart d3 selection for the chart element to be drawn on.
   * @param {Object} legend d3 selection for the legend element for
   * additional information to be drawn on.
   * @param {Object} chartDimensions The margins, width and height
   * of the chart. This is useful for computing appropriates axis
   * scales and positioning elements.
   */
  plot(graph, chart, legend, chartDimensions) {
    this.initChart_(graph, chart, chartDimensions);
    const boxHeight = chartDimensions.height / 10;
    const numberOfBoxes = graph.dataSources.length;
    const numberOfGaps = numberOfBoxes + 1;
    const whiteSpace = chartDimensions.height - numberOfBoxes * boxHeight;
    const gapSize = whiteSpace / numberOfGaps;

    let boxPostion = 0;
    const percentiles = graph.process(data => data.sort((a, b) => a - b));
    percentiles.forEach(({data, color, key}, index) => {
      boxPostion += gapSize;
      const length = data.length;
      const percentile0 = data[0];
      const percentile25 = data[Math.round(length * (1 / 4)) - 1];
      const percentile75 = data[Math.round(length * (3 / 4)) - 1];
      const percentile100 = data[Math.round(length - 1)];
      const interquartieRangeInPixels =
        this.scaleForXAxis_(percentile75) - this.scaleForXAxis_(percentile25);

      chart
          .append('rect')
          .attr('x', this.scaleForXAxis_(percentile25))
          .attr('y', boxPostion)
          .attr('width', interquartieRangeInPixels)
          .attr('height', boxHeight)
          .attr('fill', color);
      chart
          .append('line')
          .attr('x1', this.scaleForXAxis_(percentile0))
          .attr('x2', this.scaleForXAxis_(percentile0))
          .attr('y1', boxPostion)
          .attr('y2', boxPostion + boxHeight)
          .style('stroke-width', 1)
          .style('stroke', color);
      chart
          .append('line')
          .attr('x1', this.scaleForXAxis_(percentile100))
          .attr('x2', this.scaleForXAxis_(percentile100))
          .attr('y1', boxPostion)
          .attr('y2', boxPostion + boxHeight)
          .style('stroke-width', 1)
          .style('stroke', color);
      chart
          .append('line')
          .attr('x1', this.scaleForXAxis_(percentile0))
          .attr('x2', this.scaleForXAxis_(percentile100))
          .attr('y1', boxPostion + boxHeight / 2)
          .attr('y2', boxPostion + boxHeight / 2)
          .style('stroke-width', 1)
          .style('stroke', color);
      legend.append('text')
          .text(key)
          .attr('y', index + 'em')
          .attr('fill', color);
    });
  }
}

