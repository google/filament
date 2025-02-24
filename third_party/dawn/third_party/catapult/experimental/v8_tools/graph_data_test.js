'use strict';
describe('GraphData', function() {
  describe('labels', function() {
    it('should have an empty string for all labels by default', function() {
      const graph = new GraphData();
      chai.expect(graph.xAxis()).to.equal('');
      chai.expect(graph.yAxis()).to.equal('');
      chai.expect(graph.title()).to.equal('');
    });

    it('should set labels to provided values', function() {
      const graph = new GraphData();
      chai.expect(graph.xAxis('x').xAxis()).to.equal('x');
      chai.expect(graph.yAxis('y').yAxis()).to.equal('y');
      chai.expect(graph.title('title').title()).to.equal('title');
    });
  });

  describe('processing', function() {
    it('should throw an error if a function is not provided', function() {
      const graph = new GraphData();
      chai.expect(graph.process).to.throw(TypeError);
      chai.expect(() => graph.process('not a function')).to.throw(TypeError);
    });

    it('should do nothing when there is no data', function() {
      let count = 0;
      const processor = (data) => {
        // Increment whenever a data source is processed.
        count++;
        return data;
      };
      const graph = new GraphData();
      graph.process(processor);
      chai.expect(count).to.equal(0);
    });

    it('should apply the processor to all the data provided', function() {
      const data = {
        sourceOne: [1, 2, 3, 4, 5],
        sourceTwo: [6, 7, 8, 9, 10],
      };
      let count = 0;
      const processor = (data) => {
        // Increment whenever a data source is processed.
        count++;
        return data;
      };
      const graph = new GraphData().addData(data);
      graph.process(processor);
      chai.expect(count).to.equal(Object.keys(data).length);
    });

    it('should correctly compute cumulative frequencies', function() {
      const data = {
        sourceOne: [2, 1, 3],
      };
      const expected = [
        {
          y: 0,
          x: 1,
          id: 1,
        }, {
          y: 1,
          x: 2,
          id: 0,
        }, {
          y: 2,
          x: 3,
          id: 2,
        },
      ];
      const graph = new GraphData().addData(data);
      const cumulativeFreqs = graph.process(
          GraphData.computeCumulativeFrequencies);
      chai.expect(cumulativeFreqs[0].data).to.eql(expected);
    });
  });

  describe('add data', function() {
    it('supply different colors to the first two data sources', function() {
      const data = {
        sourceOne: [1, 2, 3, 4, 5],
        sourceTwo: [6, 7, 8, 9, 10],
      };
      const graph = new GraphData().addData(data);
      const colorOne = graph.dataSources[0].color;
      const colorTwo = graph.dataSources[1].color;
      chai.expect(colorOne).to.not.equal(colorTwo);
    });

    it('should throw an error for incorrect input types', function() {
      const incorrectType = 'not an object';
      const incorrectValueTypes = {
        source: 'not an array or object',
      };
      const graph = new GraphData();
      chai.expect(() => graph.addData(incorrectType)).to.throw(TypeError);
      chai.expect(
          () => graph.addData(incorrectValueTypes)).to.throw(TypeError);
    });

    it('should register the supplied key as the display label', function() {
      const data = {
        source: [1, 2],
      };
      const graph = new GraphData();
      chai.expect(graph.addData(data).dataSources[0].key).to.equal('source');
    });
  });

  describe('set data', function() {
    it('should only contain most recent data', function() {
      const data = {
        oldData: [],
      };
      const graph = new GraphData();
      graph.setData({oldData: []})
          .setData({newData: []});
      chai.expect(graph.keys()).to.eql(['newData']);
    });
  });

  describe('max', function() {
    it('should return the max value of all the data sources', function() {
      const data = {
        sourceOne: [1, 2, 3, 4, 5],
        sourceTwo: [6, 7, 8, 9, 10],
      };
      const graph = new GraphData().addData(data);
      chai.expect(graph.max(x => x)).to.equal(10);
    });
  });
});
