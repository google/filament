'use strict';
describe('GraphPlotter', function() {
  describe('changing plots', function() {
    it('should only display one plot at a time', function() {
      const simplePlotter = {
        plot: () => {},
      };
      const graph = new GraphData();
      const plotter = new GraphPlotter(graph);
      plotter.plot(simplePlotter);
      plotter.plot(simplePlotter);
      plotter.plot(simplePlotter);
      chai.expect(d3.selectAll('svg').size()).to.equal(1);
    });
  });
});
