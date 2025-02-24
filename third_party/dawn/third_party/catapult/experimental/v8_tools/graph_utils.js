'use strict';
/**
 * Utility functions common to graph plotters.
 */
class GraphUtils {
  /**
   * Attaches a zoom listener to the chart and rescales the
   * axes based on mouse scroll and click events.
   * @param {boolean} shouldScale.x
   * Set to true if the x axis should be rescaled.
   * @param {boolean} shouldScale.y
   * Set to true if the y axis should be rescaled.
   * @param {Object} chart The chart to draw on.
   * @param {Object} chartDimensions
   * The width, height and margins of the chart.
   * @param {function(Object, Object):} redraw Given the x and y axes scales
   * this should redraw the graph based on the new scales.
   * @param {Object} axes Provides the axis drawings, scales and generators
   * in the format:
   * {
   *   x: {
   *     generator: Object,
   *     scale: Object,
   *     drawing: Object,
   *   },
   *   y: {
   *     generator: Object,
   *     scale: Object,
   *     drawing: Object,
   *   },
   * }
   * If an axis is set to true in shouldScale it's corresponding information
   * must be provided in axes. Otherwise, it may be omitted.
   */
  static createZoom(shouldScale, chart, chartDimensions, redraw, axes) {
    const transformLinePlot = (xAxisScale, yAxisScale) => {
      if (xAxisScale) {
        axes.x.generator.scale(xAxisScale);
        axes.x.drawing.call(axes.x.generator);
      }
      if (yAxisScale) {
        axes.y.generator.scale(yAxisScale);
        axes.y.drawing.call(axes.y.generator);
      }
      redraw(xAxisScale, yAxisScale);
    };

    const setUpZoom = (zoom) => {
      const onZoom = () => {
        const transform = d3.event.transform;
        const transformedScaleForYAxis = shouldScale.y ?
          transform.rescaleY(axes.y.scale) : undefined;
        const transformedScaleForXAxis = shouldScale.x ?
          transform.rescaleX(axes.x.scale) : undefined;
        transformLinePlot(
            transformedScaleForXAxis, transformedScaleForYAxis);
      };
      zoom.on('zoom', onZoom).scaleExtent([1, Infinity]);
      // The following invisible rectangle is there just to catch
      // mouse events for zooming. It's not possible to listen on
      // the chart itself because it is a g element (which does not
      // capture mouse events).
      chart.append('rect')
          .attr('width', chartDimensions.width)
          .attr('height', chartDimensions.height)
          .attr('class', 'zoom-listener')
          .style('opacity', 0)
          .call(zoom);
    };

    const setUpZoomReset = (zoom) => {
      // Gives some padding between the x-axis and the reset button.
      const padding = 10;
      const resetButton = chart.append('svg')
          .attr('width', '10em')
          .attr('height', '2em')
          .attr('x', `${chartDimensions.width + padding}px`)
          .attr('y', `${chartDimensions.height}px`)
          .attr('cursor', 'pointer');
      // Styling for the button.
      resetButton.append('rect')
          .attr('rx', '5px')
          .attr('ry', '5px')
          .attr('width', '100%')
          .attr('height', '100%')
          .attr('fill', '#1b39a8');
      resetButton.append('text')
          .text('RESET CHART')
          .attr('x', '50%')
          .attr('y', '50%')
          .attr('fill', 'white')
          .attr('text-anchor', 'middle')
          .attr('dominant-baseline', 'middle');
      resetButton
          .on('mouseover', () => {
            resetButton.attr('opacity', '0.5');
          })
          .on('mouseout', () => {
            resetButton.attr('opacity', '1');
          })
          .on('click', () => {
            resetButton.attr('opacity', '1');
            chart.select('.zoom-listener')
                .call(zoom.transform, d3.zoomIdentity);
            const originalXAxisScale = shouldScale.x ?
              axes.x.scale : undefined;
            const originalYAxisScale = shouldScale.y ?
              axes.y.scale : undefined;
            transformLinePlot(
                originalXAxisScale, originalYAxisScale);
          });
    };
    const zoom = d3.zoom();
    setUpZoom(zoom);
    setUpZoomReset(zoom
    );
  }

  /**
   * Utility function for converting text which may contain invalid
   * characters for CSS classes/selectors into something safe to
   * use in class names and retrieve using selectors.
   * @return {function(string): string} Retrieves a unique class name
   * mapped to the supplied key.
   */
  static getClassNameSuffixFactory() {
    let nextUniqueClassName = 0;
    const keyToClassName = {};
    return (key) => {
      if (keyToClassName[key] === undefined) {
        keyToClassName[key] = nextUniqueClassName++;
      }
      return keyToClassName[key].toString();
    };
  }

  static useTraceCallback(selection, graph, key, radius, className) {
    selection
        .on('click', datum => {
          graph.interactiveCallbackForSelectedDatum(key, datum.id);
        })
        .on('mouseover', (datum, datumIndex) => {
          const zoomOnHoverScaleFactor = 2;
          d3.selectAll(`.${className}`)
              .filter((d, i) => i === datumIndex)
              .attr('r', radius * zoomOnHoverScaleFactor);
        })
        .on('mouseout', (datum, datumIndex) => {
          d3.selectAll(`.${className}`)
              .filter((d, i) => i === datumIndex)
              .attr('r', radius);
        })
        .append('title')
        .text('click to view trace');
    return selection;
  }
}
