'use strict';
describe('BarPlotter', function() {
  describe('Error bars', function() {
    describe('standard error', function() {
      // This should be a reasonable degree of accuracy as the calculation is
      // used for a visual element. Hence, any greater accuracy likely won't
      // make a visible difference.
      const EPSILON = 0.0000001;
      const equalWithTolerance = (a, b) => Math.abs(a - b) < EPSILON;
      chai.use(chai => {
        const Assertion = chai.Assertion;
        Assertion.addMethod('almost', function(b) {
          const a = this._obj;
          this.assert(
              equalWithTolerance(a, b),
              'expected #{act} to be almost equal to #{exp}',
              b,
              a
          );
        });
      });
      it('should return 0 when input values are all the same', function() {
        chai.expect(new BarPlotter().standardError_([1, 1, 1])).to.equal(0);
      });
      it('should return 0 for sample size less than two', function() {
        chai.expect(new BarPlotter().standardError_([1])).to.equal(0);
        chai.expect(new BarPlotter().standardError_([])).to.equal(0);
      });
      it('should have the correct se for far apart numbers', function() {
        const se = new BarPlotter().standardError_([1, 1000000]);
        chai.expect(se).to.almost(499999.5);
      });
      it('should have the correct se for similar numbers', function() {
        const se = new BarPlotter().standardError_(
            [1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2]);
        chai.expect(se).to.almost(0.150755672289);
      });
      it('should have correct se for random numbers', function() {
        const se = new BarPlotter().standardError_(
            [31, 4, 345, 1, 637, 423, 90, 7859, 81, 5783, 482039, 859,
              12, 54, 42, 7, 3, 13, 9, 978, 24, 9, 289]);
        chai.expect(se).to.almost(20927.4803831);
      });
    });
  });
});
