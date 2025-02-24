'use strict';
/**
 * Concrete implementation of Plotter strategy for creating
 * bar charts.
 * @implements {Plotter}
 */
class BarPlotter {
  constructor() {
    /** @private @const {number} x_
     * The x axis column in the graph data table.
     */
    this.x_ = 0;
    /** @private @const {number} x_
     * The y axis column in the graph data table.
     */
    this.y_ = 1;
  }
  /**
   * Initalises the chart by computing the scales for the axes and
   * drawing them. It also applies the labels to the graph (axes and
   * graph titles).
   * @param {GraphData} graph Data to be drawn.
   * @param {Object} chart Chart to draw to.
   * @param {Object} chartDimensions Size of the chart.
   */
  initChart_(graph, chart, chartDimensions) {
    this.outerBandScale_ = this.createXAxisScale_(graph, chartDimensions);
    this.scaleForYAxis_ = this.createYAxisScale_(graph, chartDimensions);
    this.xAxisGenerator_ = d3.axisBottom(this.outerBandScale_);
    this.yAxisGenerator_ = d3.axisLeft(this.scaleForYAxis_);
    // Wrap in untransformed g tag to preserve co ordinate system of chart.
    chart.append('g')
        .attr('class', 'xaxis')
        .append('g')
        .call(this.xAxisGenerator_)
        .attr('transform', `translate(0, ${chartDimensions.height})`);
    this.yAxisDrawing_ = chart.append('g')
        .call(this.yAxisGenerator_);
    // Each story is assigned a band by the createXAxisScale function
    // which maintains the positions in which each category of the chart
    // will be rendered. This further divides these bands into
    // sub-bands for each data source, so that different labels can be
    // grouped within the same category.
    this.innerBandScale_ = this.createInnerBandScale_(graph);
  }

  getBarCategories_(graph) {
    // Process data sources so that their data contains only their categories.
    const dataSources = graph.process(data => data.map(row => row[this.x_]));
    if (dataSources.length > 0) {
      return dataSources[0].data;
    }
    return [];
  }

  createXAxisScale_(graph, chartDimensions) {
    return d3.scaleBand()
        .domain(this.getBarCategories_(graph))
        .range([0, chartDimensions.width])
        .padding(0.2);
  }

  createYAxisScale_(graph, chartDimensions) {
    const maxHeight =
        Math.round(graph.max(row => this.computeAverage_(row[this.y_])));
    const defaultRange = 1;
    return d3.scaleLinear()
        .domain([maxHeight || defaultRange, 0]).nice()
        .range([0, chartDimensions.height]);
  }

  createInnerBandScale_(graph) {
    const keys = graph.keys();
    return d3.scaleBand()
        .domain(keys)
        .range([0, this.outerBandScale_.bandwidth()]);
  }

  /**
   * Attempts to compute the average of the given numbers but returns
   * 0 if the supplied array is empty.
   * @param {Array<number>} data
   * @returns {number}
   */
  computeAverage_(data) {
    if (!data.every(val => typeof val === 'number')) {
      throw new TypeError('Expected an array of numbers.');
    }
    return data.reduce((a, b) => a + b, 0) / data.length || 0;
  }

  /**
   * Calculates the standard error of the mean for the sample.
   * See https://en.wikipedia.org/wiki/Standard_error.
   * @param {Array<number>} sample The input data.
   * @returns {number} The standard error of the mean for sample.
   */
  standardError_(sample) {
    const sampleSize = sample.length;
    if (sampleSize < 2) {
      return 0;
    }
    const mean = this.computeAverage_(sample);
    const sampleStandardDeviation = (sample) => {
      let sum = 0;
      for (const val of sample) {
        sum += Math.pow((val - mean), 2);
      }
      return Math.sqrt(sum / (sampleSize - 1));
    };
    return sampleStandardDeviation(sample) / Math.sqrt(sampleSize);
  }

  statistics_(sample) {
    const mean = this.computeAverage_(sample);
    const se = this.standardError_(sample);
    return {
      mean,
      upper: mean + se,
      lower: mean - se,
    };
  }

  barStart_(category, key) {
    return this.outerBandScale_(category) + this.innerBandScale_(key);
  }

  barWidthQuantile_(category, key, quantile) {
    return this.barStart_(category, key) +
        this.innerBandScale_.bandwidth() * quantile;
  }
  /**
   * Draws a bar chart to the canvas. If there are multiple dataSources it will
   * plot them both and label their colors in the legend. This expects the data
   * in graph to be formatted as a table, with the first column being categories
   * and the second being the corresponding values.
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
    const computeAllAverages = stories =>
      stories.map(
          ([story, rawData]) => [story, this.statistics_(rawData)]);
    const dataInAverages = graph.process(computeAllAverages);
    const getClassNameSuffix = GraphUtils.getClassNameSuffixFactory();
    dataInAverages.forEach(({ data, color, key }, index) => {
      chart.selectAll(`.bar-${getClassNameSuffix(key)}`)
          .data(data)
          .enter()
          .call(
              this.appendBar_.bind(this),
              graph,
              color,
              key,
              chartDimensions,
              getClassNameSuffix)
          .call(this.appendErrorLine_.bind(this), key)
          .call(this.appendErrorLineCaps_.bind(this), key);
      const boxSize = 10;
      const offset = `${index}em`;
      legend.append('rect')
          .attr('fill', color)
          .attr('height', boxSize)
          .attr('width', boxSize)
          .attr('y', offset)
          .attr('x', 0);
      legend.append('text')
          .text(key)
          .attr('x', boxSize)
          .attr('y', offset)
          .attr('dy', boxSize)
          .attr('text-anchor', 'start');
    });
    const tickRotation = -30;
    d3.select('.xaxis')
        .attr('clip-path', 'url(#regionForXAxisTickText)')
        .selectAll('text')
        .attr('text-anchor', 'end')
        .attr('font-size', 12)
        .attr('transform', `rotate(${tickRotation})`)
        .append('title')
        .text(text => text);
  }

  appendBar_(
      selection, graph, color, key, chartDimensions, getClassNameSuffix) {
    const barWidth = this.innerBandScale_.bandwidth();
    const barHeight = value =>
      chartDimensions.height - this.scaleForYAxis_(value);
    selection.append('rect')
        .attr('class', `.bar-${getClassNameSuffix(key)}`)
        .attr('x', d => this.barStart_(d[this.x_], key))
        .attr('y', d => this.scaleForYAxis_(d[this.y_].mean))
        .attr('width', barWidth)
        .attr('height', d => barHeight(d[this.y_].mean))
        .attr('fill', color)
        .on('click', d =>
          graph.interactiveCallbackForCategory(d[this.x_]))
        .on('mouseover', function() {
          d3.select(this).attr('opacity', 0.5);
        })
        .on('mouseout', function() {
          d3.select(this).attr('opacity', 1);
        });
    return selection;
  }

  appendErrorLine_(selection, key) {
    const middle = 0.5;
    const getXPosition =
        ([category]) => this.barWidthQuantile_(category, key, middle);
    selection.append('line')
        .attr('x1', getXPosition)
        .attr('x2', getXPosition)
        .attr('y1', ([, data]) => this.scaleForYAxis_(data.lower))
        .attr('y2', ([, data]) => this.scaleForYAxis_(data.upper))
        .attr('stroke-width', 2)
        .attr('stroke', 'black');
    return selection;
  }

  appendErrorLineCaps_(selection, key) {
    const quarter = 0.25;
    const upperQuarter = 0.75;
    const getXPosition = (category, quantile) =>
      this.barWidthQuantile_(category, key, quantile);
    const appendLine = (selection, error) => {
      selection.append('line')
          .attr('x1', ([category]) => getXPosition(category, quarter))
          .attr('x2', ([category]) => getXPosition(category, upperQuarter))
          .attr('y1', ([, data]) => this.scaleForYAxis_(data[error]))
          .attr('y2', ([, data]) => this.scaleForYAxis_(data[error]))
          .attr('stroke-width', 2)
          .attr('stroke', 'black');
      return selection;
    };
    return selection
        .call(appendLine.bind(this), 'upper')
        .call(appendLine.bind(this), 'lower');
  }
}
