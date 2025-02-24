
describe("oboe loaded using require", function() {

  it('is not on the global namespace by default', function () {

    expect(window.oboe).toBe(undefined);
  });

  it('can be loaded using require', function (done) {

    require(['oboe'], function(oboe){
      expect(oboe).not.toBe(undefined);
      expect(oboe('foo.json')).not.toBe(undefined);
      done();
    });
  });

  it('it not on global after being loaded', function (done) {

    require(['oboe'], function(oboe){
      expect(window.oboe).toBe(undefined);
      done();
    });
  });
});
