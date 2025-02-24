'use strict';
/**
 * Concrete implementation of Plotter strategy for creating
 * line graphs.
 * @implements {Plotter}
 */
class LinePlotter {
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
    this.scaleForYAxis_ = this.createYAxisScale_(graph, chartDimensions);
    this.xAxisGenerator_ = d3.axisBottom(this.scaleForXAxis_);
    this.yAxisGenerator_ = d3.axisLeft(this.scaleForYAxis_);
    this.xAxisDrawing_ = chart.append('g')
        .call(this.xAxisGenerator_)
        .attr('transform', `translate(0, ${chartDimensions.height})`);
    this.yAxisDrawing_ = chart.append('g')
        .call(this.yAxisGenerator_);
  }

  createXAxisScale_(graph, chartDimensions) {
    const range = Math.round(graph.max(point => +point));
    const defaultRange = 1;
    return d3.scaleLinear()
        .domain([0, range || defaultRange]).nice()
        .range([0, chartDimensions.width]);
  }

  createYAxisScale_(graph, chartDimensions) {
    const numDataPoints =
      Math.max(...graph.dataSources.map(source => source.data.length));
    return d3.scaleLinear()
        .domain([0, numDataPoints])
        .range([chartDimensions.height, 0]);
  }

  /**
   * Draws a line plot to the canvas. If there are multiple dataSources it will
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
    const dotRadius = 4;
    const pathGenerator = d3.line()
        .x(datum => this.scaleForXAxis_(datum.x))
        .y(datum => this.scaleForYAxis_(datum.y))
        .curve(d3.curveMonotoneX);
    const getClassNameSuffix = GraphUtils.getClassNameSuffixFactory();
    this.setUpZoom_(graph, chart, chartDimensions, getClassNameSuffix);
    const data = graph.process(GraphData.computeCumulativeFrequencies);
    data.forEach(({ data, color, key }, index) => {
      const classNameForDots = `dot-${getClassNameSuffix(key)}`;
      chart.selectAll('.dot')
          .data(data)
          .enter()
          .append('circle')
          .attr('cx', datum => this.scaleForXAxis_(datum.x))
          .attr('cy', datum => this.scaleForYAxis_(datum.y))
          .attr('r', dotRadius)
          .attr('fill', color)
          .attr('class', `${classNameForDots}`)
          .attr('clip-path', 'url(#plot-clip)')
          .call(GraphUtils.useTraceCallback,
              graph,
              key,
              dotRadius,
              classNameForDots);
      chart.append('path')
          .datum(data)
          .attr('d', pathGenerator)
          .attr('stroke', color)
          .attr('fill', 'none')
          .attr('stroke-width', 2)
          .attr('data-legend', key)
          .attr('class', 'line-plot')
          .attr('clip-path', 'url(#plot-clip)');
      const lineLength = 10;
      const offset = `${index}em`;
      legend.append('line')
          .attr('stroke', color)
          .attr('stroke-width', 5)
          .attr('y1', offset)
          .attr('x1', 0)
          .attr('x2', lineLength)
          .attr('y2', offset);
      legend.append('text')
          .text(key)
          .attr('x', lineLength)
          .attr('y', 5)
          .attr('dy', offset)
          .attr('text-anchor', 'start');
    });
  }

  setUpZoom_(graph, chart, chartDimensions, getClassNameSuffix) {
    const redraw = xAxisScale => {
      const pathGenerator = d3.line()
          .x(d => xAxisScale(d.x))
          .y(d => this.scaleForYAxis_(d.y))
          .curve(d3.curveMonotoneX);
      for (const key of graph.keys()) {
        chart.selectAll(`.dot-${getClassNameSuffix(key)}`)
            .attr('cx', datum => xAxisScale(datum.x))
            .attr('cy', datum => this.scaleForYAxis_(datum.y));
      }
      chart.selectAll('.line-plot')
          .attr('d', pathGenerator);
    };
    const axes = {
      x: {
        generator: this.xAxisGenerator_,
        drawing: this.xAxisDrawing_,
        scale: this.scaleForXAxis_,
      },
    };
    const shouldScale = {
      y: false,
      x: true,
    };
    GraphUtils.createZoom(shouldScale, chart, chartDimensions, redraw, axes);
  }
}
