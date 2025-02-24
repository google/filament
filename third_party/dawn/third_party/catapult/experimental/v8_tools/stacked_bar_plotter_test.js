'use strict';
describe('StackedBarPlotter', function() {
  describe('compute stacked y values', function() {
    it('should compute averages and stack them', function() {
      const bar = {
        'stack1': [1, 1, 1],
        'stack2': [2, 2, 2],
        'stack3': [3, 3, 3],
      };
      const stackedAvgs = [
        ['stack1', {start: 0, height: 1}],
        ['stack2', {start: 1, height: 2}],
        ['stack3', {start: 3, height: 3}],
      ];
      const plotter = new StackedBarPlotter();
      const computedStackedAvgs = plotter.stackLocations_(bar);
      chai.expect(computedStackedAvgs).to.eql(stackedAvgs);
    });
  });
});
