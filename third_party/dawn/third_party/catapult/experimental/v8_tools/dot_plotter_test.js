'use strict';
const getStackedOffsets = (data, s) => {
  const dotPlotter = new DotPlotter();
  // Needed in the dot stacking method.
  dotPlotter.maxDotStacking_ = Number.POSITIVE_INFINITY;
  // Stubs the x axis scale so that theres a 1:1 mapping between pixel
  // values and memory values.
  const scale = s || (x => x);
  const stackedLocations = dotPlotter.computeDotStacking_(data.source, scale);
  return stackedLocations.map(({x, stackOffset}) => stackOffset);
};
describe('DotPlotter', function() {
  describe('dot stacking', function() {
    it('should not stack far away values', function() {
      const data = {
        source: [5, 40, 90, 100],
      };
      const stackedOffsets = getStackedOffsets(data);
      chai.expect(stackedOffsets).to.eql([0, 0, 0, 0]);
    });
    it('should stack duplicates', function() {
      const data = {
        source: [100, 100, 100],
      };
      const stackedOffsets = getStackedOffsets(data);
      chai.expect(stackedOffsets).to.have.members([-1, 0, 1]);
    });
    it('should allow for multiple stacks', function() {
      const data = {
        source: [992, 994, 555, 556, 15],
      };
      const stackedOffsets = getStackedOffsets(data);
      chai.expect(stackedOffsets).to.have.members([0, -1, 0, -1, 0]);
    });
    it('should do nothing for empty inputs', function() {
      const data = {
        source: [],
      };
      const stackedOffsets = getStackedOffsets(data);
      chai.expect(stackedOffsets).to.have.members([]);
    });
    it('should not stack an individual input', function() {
      const data = {
        source: [1],
      };
      const stackedOffsets = getStackedOffsets(data);
      chai.expect(stackedOffsets).to.have.members([0]);
    });
    it('should correctly stack decimal values', function() {
      const data = {
        source: [0.01, 0.02, 10.1, 10.5],
      };
      const stackedOffsets = getStackedOffsets(data);
      chai.expect(stackedOffsets).to.have.members([-1, 0, -1, 0]);
    });
    it('should correctly stack input domain between 0 and 1', function() {
      const data = {
        source: [0.01, 0.02, 0.09, 0.099],
      };
      const scale = d3.scaleLinear().domain([0, 1]).range([0, 100]);
      const stackedOffsets = getStackedOffsets(data, scale);
      chai.expect(stackedOffsets).to.have.members([-1, 0, -1, 0]);
    });
  });
  describe('plotting', function() {
    const graph = new GraphData();
    it('should plot data despite invalid selection characters', function() {
      const data = {
        'source.with:inv@lid _chars"}~$': [1, 2, 3, 4, 5],
      };
      graph.setData(data);
      chai.expect(() => graph.plotDot()).to.not.throw(Error);
      chai.expect(
          document.querySelectorAll('.dot-0').length)
          .to.equal(5);
    });
    it('should trigger the supplied callback when a dot is clicked',
        function() {
          const data = {
            'label': [1],
          };
          const callback = chai.spy((metric, story, key, index) => {});
          graph.setData(data, callback).plotDot();
          d3.selectAll('circle').dispatch('click');
          chai.expect(callback)
              .to.have.been.called.with
              .exactly('label', 0);
        });
    it('should trigger the supplied callback with the correct arguments',
        function() {
          const data = {
            'labelOne': [5, 8, 10],
            'labelTwo': [4, 1],
          };
          const callback = chai.spy();
          graph.setData(data, callback).plotDot();

          d3.select('.chai-test-dot-0-5').dispatch('click');
          chai.expect(callback)
              .on.nth(1).to.have.been.called.with
              .exactly('labelOne', 0);

          d3.select('.chai-test-dot-0-8').dispatch('click');
          chai.expect(callback)
              .on.nth(2).to.have.been.called.with
              .exactly('labelOne', 1);

          d3.select('.chai-test-dot-0-10').dispatch('click');
          chai.expect(callback)
              .on.nth(3).to.have.been.called.with
              .exactly('labelOne', 2);

          d3.select('.chai-test-dot-1-4').dispatch('click');
          chai.expect(callback)
              .on.nth(4).to.have.been.called.with
              .exactly('labelTwo', 0);

          d3.select('.chai-test-dot-1-1').dispatch('click');
          chai.expect(callback)
              .on.nth(5).to.have.been.called.with
              .exactly('labelTwo', 1);
        });
  });
});
