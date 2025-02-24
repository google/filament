'use strict';
describe('GraphUtils', function() {
  describe('getClassNameSuffix', function() {
    it('should create new entries by default', function() {
      const getClassNameSuffix = GraphUtils.getClassNameSuffixFactory();
      chai.expect(getClassNameSuffix('never seen')).to.equal('0');
    });
    it('should return same class name for same key', function() {
      const getClassNameSuffix = GraphUtils.getClassNameSuffixFactory();
      chai.expect(getClassNameSuffix('key')).to.equal('0');
      chai.expect(getClassNameSuffix('key')).to.equal('0');
    });
    it('should return different class names for different keys', function() {
      const getClassNameSuffix = GraphUtils.getClassNameSuffixFactory();
      chai.expect(getClassNameSuffix('key one'))
          .to.not.equal(getClassNameSuffix('key two'));
    });
  });
});
