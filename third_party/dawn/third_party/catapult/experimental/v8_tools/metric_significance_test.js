'use strict';
describe('MetricSignificance', function() {
  describe('testing for significance amongst metrics', function() {
    const dataOne = [1, 2, 3, 4, 5];
    const dataTwo = [3479, 25983, 2345, 54654, 3245];
    const setUp = (ms) => {
      ms.add('metric1', 'label1', 'story', dataOne);
      ms.add('metric1', 'label2', 'story', dataTwo);
      ms.referenceColumn = 'label1';
    };
    it('should return nothing for no inputs', function() {
      const ms = new MetricSignificance();
      const results = ms.mostSignificant();
      chai.expect(results).to.eql([]);
    });
    it('should return result for significant entry', function() {
      const ms = new MetricSignificance();
      setUp(ms);
      const results = ms.mostSignificant();
      chai.expect(results.length).to.equal(1);
      chai.expect(results[0].metric).to.equal('metric1');
    });
    it('should return result when data added individually', function() {
      const ms = new MetricSignificance();
      dataOne.forEach(datum => ms.add('metric1', 'label1', 'story', [datum]));
      dataTwo.forEach(datum => ms.add('metric1', 'label2', 'story', [datum]));
      ms.referenceColumn = 'label1';
      const results = ms.mostSignificant();
      chai.expect(results.length).to.equal(1);
      chai.expect(results[0].metric).to.equal('metric1');
    });
    it('should throw an error when label has no pair', function() {
      const ms = new MetricSignificance();
      ms.add('metric1', 'label1', 'story', dataOne);
      ms.add('metric1', 'label1', 'story', dataTwo);
      ms.referenceColumn = 'label1';
      chai.expect(() => ms.mostSignificant()).to.throw(Error);
    });
    it('should throw an error when metric has more that two labels ',
        function() {
          const ms = new MetricSignificance();
          ms.add('metric1', 'label1', 'story', dataOne);
          ms.add('metric1', 'label2', 'story', dataOne);
          ms.add('metric1', 'label3', 'story', dataOne);
          ms.referenceColumn = 'label1';
          chai.expect(() => ms.mostSignificant()).to.throw(Error);
        });
    it('should not throw an error when metric has two labels ', function() {
      const ms = new MetricSignificance();
      setUp(ms);
      chai.expect(() => ms.mostSignificant()).to.not.throw(Error);
    });
    it('should only return significant results', function() {
      const noChange = [1, 1, 1, 1, 1];
      const ms = new MetricSignificance();
      ms.add('metric1', 'label1', 'story', dataOne);
      ms.add('metric1', 'label2', 'story', dataTwo);
      ms.add('metric2', 'label1', 'story', noChange);
      ms.add('metric2', 'label2', 'story', noChange);
      ms.referenceColumn = 'label1';
      const results = ms.mostSignificant();
      chai.expect(results.length).to.equal(1);
      chai.expect(results[0].metric).to.equal('metric1');
    });
    it('should return which stories have regressed', function() {
      const ms = new MetricSignificance();
      setUp(ms);
      const results = ms.mostSignificant();
      const regressedStories = results[0].stories.map(({ story }) => story);
      chai.expect(regressedStories).to.eql(['story']);
    });
    it('should define impact against the reference column', function() {
      const ms = new MetricSignificance();
      setUp(ms);
      let results = ms.mostSignificant();
      let impact = results[0].evidence.type;
      chai.expect(impact).to.equal('regression');
      ms.referenceColumn = 'label2';
      results = ms.mostSignificant();
      impact = results[0].evidence.type;
      chai.expect(impact).to.equal('improvement');
    });
  });
});
