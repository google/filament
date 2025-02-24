
describe('instance api',function() {
  "use strict";

  var oboeBus, oboeInstance,
      sampleUrl = 'http://example.com';

  beforeEach(function () {
    oboeBus = spiedPubSub();
    spyOn(oboeBus, 'emit' );
    spyOn(oboeBus, 'on'   );
    spyOn(oboeBus, 'un'   );

    oboeInstance = instanceApi(oboeBus, sampleUrl);
  });

  function anAscent() {
    return list(namedNode(ROOT_PATH, {}));
  }

  it('puts the url on the oboe instance', function(){
    expect( oboeInstance.source).toBe( sampleUrl );
  });

  describe('header method', function(){

    it('returns undefined if not available', function() {

      expect( oboeInstance.header() ).toBeUndefined();
    });

    it('can provide object once available', function() {

      var headers = {"x-remainingRequests": 100};

      oboeBus(HTTP_START).emit( 200, headers );

      expect( oboeInstance.header() ).toEqual(headers);
    });

    it('can provide single header once available', function() {
      var headers = {"x-remainingRequests": 100};

      oboeBus(HTTP_START).emit( 200, headers );

      expect( oboeInstance.header('x-remainingRequests') ).toEqual(100);
    });

    it('gives undefined for non-existent single header', function() {
      var headers = {"x-remainingRequests": 100};

      oboeBus(HTTP_START).emit( 200, headers );

      expect( oboeInstance.header('x-remainingBathtubs') ).toBeUndefined();
    });
  });

  describe('root method', function(){

    it('returns undefined if not available', function() {

      expect( oboeInstance.root() ).toBeUndefined();
    });

    it('can provide object once available', function() {

      var root = {I:'am', the:'root'};

      oboeBus(ROOT_PATH_FOUND).emit( root);

      expect( oboeInstance.root() ).toEqual(root);
    });
  });

  describe('node and path callbacks', function(){

    it('calls node callback when notified of matching node', function() {

      var callback = jasmine.createSpy('node callback'),
          node = {},
          path = [],
          ancestors = [];

      oboeInstance.on('node', 'a_pattern', callback);

      expect(callback).not.toHaveBeenCalled()

      oboeBus('node:a_pattern').emit( node, path, ancestors );

      expect(callback).toHaveBeenCalledWith( node, path, ancestors );
    });

    it('calls path callback when notified of matching path', function() {

      var callback = jasmine.createSpy('path callback'),
          node = {},
          path = [],
          ancestors = [];

      oboeInstance.on('path', 'a_pattern', callback);

      expect(callback).not.toHaveBeenCalled()

      oboeBus('path:a_pattern').emit( node, path, ancestors );

      expect(callback).toHaveBeenCalledWith( node, path, ancestors );
    });

    it('allows short-cut node matching', function() {

      var pattern1Callback = jasmine.createSpy('pattern 1 callback'),
          pattern2Callback = jasmine.createSpy('pattern 2 callback');

      oboeInstance.on('node', {
        pattern1: pattern1Callback,
        pattern2: pattern2Callback
      });

      expect(pattern1Callback).not.toHaveBeenCalled()
      expect(pattern2Callback).not.toHaveBeenCalled()

      oboeBus('node:pattern1').emit( {}, anAscent())

      expect(pattern1Callback).toHaveBeenCalled()
      expect(pattern2Callback).not.toHaveBeenCalled()

      oboeBus('node:pattern2').emit( {}, anAscent())

      expect(pattern2Callback).toHaveBeenCalled()
    });

    it('calls node callback added using 2-arg mode when notified of match to pattern', function() {

      var callback = jasmine.createSpy('node callback'),
          node = {},
          path = [],
          ancestors = [];

      oboeInstance.on('node:a_pattern', callback)

      expect(callback).not.toHaveBeenCalled()

      oboeBus('node:a_pattern').emit( node, path, ancestors );

      expect(callback).toHaveBeenCalledWith( node, path, ancestors );
    });

    it('allows adding using addListener method', function() {

      var callback = jasmine.createSpy('node callback'),
          node = {},
          path = [],
          ancestors = [];

      oboeInstance.addListener('node:a_pattern', callback)

      expect(callback).not.toHaveBeenCalled()

      oboeBus('node:a_pattern').emit( node, path, ancestors );

      expect(callback).toHaveBeenCalledWith( node, path, ancestors );
    });

    it('calls path callback added using 2-arg mode when notified of match to pattern', function() {

      var callback = jasmine.createSpy('path callback'),
          node = {},
          path = [],
          ancestors = [];

      oboeInstance.on('path:a_pattern', callback);

      expect(callback).not.toHaveBeenCalled()

      oboeBus('path:a_pattern').emit( node, path, ancestors );

      expect(callback).toHaveBeenCalledWith( node, path, ancestors );
    });

    it('doesn\'t call node callback on path found', function() {

      var callback = jasmine.createSpy('node callback');

      oboeInstance.on('node', 'a_pattern', callback);

      expect(callback).not.toHaveBeenCalled()

      oboeBus('path:a_pattern').emit( {}, list(namedNode(ROOT_PATH, {}) ) );

      expect(callback).not.toHaveBeenCalled();
    });

    it('doesn\'t call again after forget called from inside callback', function() {

      var nodeCallback = jasmine.createSpy('node callback').and.callFake(function(){
        this.forget();
      }),
          ascent =   list(namedNode('node', {}));

      oboeInstance.on('node', 'a_pattern', nodeCallback);

      oboeBus('node:a_pattern').emit( {}, ascent);

      expect(nodeCallback.calls.count()).toBe(1)

      oboeBus('node:a_pattern').emit( {}, ascent);

      expect(nodeCallback.calls.count()).toBe(1)
    });

    xit('doesn\'t call node callback after callback is removed', function() {

      var nodeCallback = jasmine.createSpy('node callback'),
          ascent = list(namedNode('node', {}));

      oboeInstance.on('node', 'a_pattern', nodeCallback);
      oboeInstance.removeListener('node', 'a_pattern', nodeCallback);

      oboeBus('node:a_pattern').emit( {}, ascent);

      expect(nodeCallback).not.toHaveBeenCalled()
    });

    it('doesn\'t call node callback after callback is removed using 2-arg form', function() {

      var nodeCallback = jasmine.createSpy('node callback'),
          ascent = list(namedNode('node', {}));

      // oboeInstance.on('node', 'a_pattern', nodeCallback);
      oboeInstance.on('node', 'a_pattern', nodeCallback);
      oboeInstance.removeListener('node:a_pattern', nodeCallback);

      oboeBus('node:a_pattern').emit( {}, ascent);

      expect(nodeCallback).not.toHaveBeenCalled()
    });

    xit('doesn\'t call path callback after callback is removed', function() {

      var pathCallback = jasmine.createSpy('path callback'),
          ascent = list(namedNode('path', {}));

      oboeInstance.on('path', 'a_pattern', pathCallback);
      oboeInstance.removeListener('path', 'a_pattern', pathCallback);

      oboeBus('path:a_pattern').emit( {}, ascent);

      expect(pathCallback).not.toHaveBeenCalled()
    });

    it('doesn\'t call path callback after callback is removed using 2-arg form', function() {

      var pathCallback = jasmine.createSpy('path callback'),
          ascent = list(namedNode('path', {}));

      oboeInstance.on('path', 'a_pattern', pathCallback);
      oboeInstance.removeListener('path:a_pattern', pathCallback);

      oboeBus('path:a_pattern').emit( {}, ascent);

      expect(pathCallback).not.toHaveBeenCalled()
    });

    it('doesn\'t remove callback if wrong pattern is removed', function() {

      var nodeCallback = jasmine.createSpy('node callback'),
          ascent = list(namedNode('node', {}));

      oboeInstance.on('node', 'a_pattern', nodeCallback);

      oboeInstance.removeListener('node', 'wrong_pattern', nodeCallback);

      oboeBus('node:a_pattern').emit( {}, ascent);

      expect(nodeCallback).toHaveBeenCalled()
    });

    it('doesn\'t remove callback if wrong callback is removed', function() {

      var correctCallback = jasmine.createSpy('correct callback'),
          wrongCallback = jasmine.createSpy('wrong callback'),
          ascent = list(namedNode('node', {}));

      oboeInstance.on('node', 'a_pattern', correctCallback);

      oboeInstance.removeListener('node', 'a_pattern', wrongCallback);

      oboeBus('node:a_pattern').emit( {}, ascent);

      expect(correctCallback).toHaveBeenCalled()
    });

    it('allows node listeners to be removed in a different style than they were added', function() {

      var
      callback1 = jasmine.createSpy('callback 1'),
      callback2 = jasmine.createSpy('callback 2'),
      callback3 = jasmine.createSpy('callback 3'),
      ascent = list(namedNode('node', {}));

      oboeInstance.node('pattern1', callback1);
      oboeInstance.on('node', 'pattern2', callback2);
      oboeInstance.on('node', {pattern3: callback3});

      oboeInstance.removeListener('node:pattern1', callback1);
      oboeInstance.removeListener('node:pattern2', callback2);
      oboeInstance.removeListener('node:pattern3', callback3);

      oboeBus('node:pattern1').emit( {}, ascent);
      oboeBus('node:pattern2').emit( {}, ascent);
      oboeBus('node:pattern3').emit( {}, ascent);

      expect(callback1).not.toHaveBeenCalled()
      expect(callback2).not.toHaveBeenCalled()
      expect(callback3).not.toHaveBeenCalled()
    });
  });

  describe('start event', function() {
    it('notifies .on(start) listener when http response starts', function(){
      var startCallback = jasmine.createSpy('start callback');

      oboeInstance.on('start', startCallback);

      expect(startCallback).not.toHaveBeenCalled()

      oboeBus(HTTP_START).emit( 200, {a_header:'foo'} )

      expect(startCallback).toHaveBeenCalledWith( 200, {a_header:'foo'} )
    });

    it('notifies .start listener when http response starts', function(){
      var startCallback = jasmine.createSpy('start callback');

      oboeInstance.start(startCallback);

      expect(startCallback).not.toHaveBeenCalled()

      oboeBus(HTTP_START).emit( 200, {a_header:'foo'} )

      expect(startCallback).toHaveBeenCalledWith( 200, {a_header:'foo'} )
    });

    it('can be de-registered', function() {
      var startCallback = jasmine.createSpy('start callback');

      oboeInstance.on('start', startCallback);
      oboeInstance.removeListener('start', startCallback);

      oboeBus(HTTP_START).emit( 200, {a_header:'foo'} )

      expect(startCallback).not.toHaveBeenCalled()
    });
  });


  describe('done event', function(){

    it('calls listener on end of JSON when added using .on(done)', function() {
      var doneCallback = jasmine.createSpy('done callback');

      oboeInstance.on('done', doneCallback);

      expect(doneCallback).not.toHaveBeenCalled()

      oboeBus(ROOT_NODE_FOUND).emit( {}, anAscent())

      expect(doneCallback).toHaveBeenCalled()
    });

    it('calls listener on end of JSON when added using .done', function() {
      var doneCallback = jasmine.createSpy('done callback');

      oboeInstance.done(doneCallback);

      expect(doneCallback).not.toHaveBeenCalled()

      oboeBus(ROOT_NODE_FOUND).emit( {}, anAscent())

      expect(doneCallback).toHaveBeenCalled()
    });

    it('can be de-registered', function() {
      var doneCallback = jasmine.createSpy('done callback');

      oboeInstance.on('done', doneCallback);
      oboeInstance.removeListener('done', doneCallback);

      oboeBus('node:!').emit( {}, anAscent())

      expect(doneCallback).not.toHaveBeenCalled()
    });
  });


  it('emits ABORTING when .abort() is called', function() {
    oboeInstance.abort();
    expect(oboeBus(ABORTING).emit.calls.count()).toEqual(1)
    expect(oboeBus(ABORTING).emit).toHaveBeenCalled()
  });

  describe('errors cases', function(){

    describe('calling fail listener', function() {

      it('notifies .on(fail) listener when something fails', function(){
        var failCallback = jasmine.createSpy('fail callback');

        oboeInstance.on('fail', failCallback);

        expect(failCallback).not.toHaveBeenCalled()

        oboeBus(FAIL_EVENT).emit( 'something went wrong' )

        expect(failCallback).toHaveBeenCalledWith( 'something went wrong' )
      });

      it('notifies .fail listener when something fails', function(){
        var failCallback = jasmine.createSpy('fail callback');

        oboeInstance.fail(failCallback);

        expect(failCallback).not.toHaveBeenCalled()

        oboeBus(FAIL_EVENT).emit( 'something went wrong' )

        expect(failCallback).toHaveBeenCalledWith( 'something went wrong' )
      });

      it('can be de-registered', function() {
        var failCallback = jasmine.createSpy('fail callback');

        oboeInstance.on('fail', failCallback);
        oboeInstance.removeListener('fail', failCallback);

        oboeBus(FAIL_EVENT).emit( 'something went wrong' )

        expect(failCallback).not.toHaveBeenCalled()
      });
    });


    it('is protected from error in node callback', function() {
      var e = "an error";
      var nodeCallback = jasmine.createSpy('nodeCallback').and.throwError(e);

      oboeInstance.on('node', 'a_pattern', nodeCallback);

      spyOn(window, 'setTimeout');
      expect(function(){
        oboeBus('node:a_pattern').emit( {}, anAscent())
      }).not.toThrow()

      expect(nodeCallback).toHaveBeenCalled()
      expect(window.setTimeout.calls.mostRecent().args[0]).toThrow(new Error(e))
    });

    it('is protected from error in node callback added via shortcut', function() {
      var e = "an error";
      var nodeCallback = jasmine.createSpy('node callback').and.throwError(e);

      oboeInstance.on('node', {'a_pattern': nodeCallback});

      spyOn(window, 'setTimeout');
      expect(function(){
        oboeBus('node:a_pattern').emit( {}, anAscent())
      }).not.toThrow()

      expect(nodeCallback).toHaveBeenCalled()
      expect(window.setTimeout.calls.mostRecent().args[0]).toThrow(new Error(e))
    });

    it('is protected from error in path callback', function() {
      var e = "an error";
      var pathCallback = jasmine.createSpy('path callback').and.throwError(e);

      oboeInstance.on('path', 'a_pattern', pathCallback);

      spyOn(window, 'setTimeout');
      expect(function(){
        oboeBus('path:a_pattern').emit( {}, anAscent())
      }).not.toThrow()

      expect(pathCallback).toHaveBeenCalled()
      expect(window.setTimeout.calls.mostRecent().args[0]).toThrow(new Error(e))
    });

    it('is protected from error in start callback', function() {
      var e = "an error";
      var startCallback = jasmine.createSpy('start callback').and.throwError(e);

      oboeInstance.on('start', startCallback);

      spyOn(window, 'setTimeout');
      expect(function(){
        oboeBus(HTTP_START).emit()
      }).not.toThrow()

      expect(startCallback).toHaveBeenCalled()
      expect(window.setTimeout.calls.mostRecent().args[0]).toThrow(new Error(e))
    });

    it('is protected from error in done callback', function() {
      var e = "an error";
      var doneCallback = jasmine.createSpy('done callback').and.throwError(e);

      oboeInstance.done( doneCallback);

      spyOn(window, 'setTimeout');
      expect(function(){
        oboeBus(ROOT_NODE_FOUND).emit( {}, anAscent())
      }).not.toThrow()

      expect(doneCallback).toHaveBeenCalled()
      expect(window.setTimeout.calls.mostRecent().args[0]).toThrow(new Error(e))
    });

  });

  xdescribe('unknown event types', function() {

    it('can be added and fired', function() {
      var spy1 = jasmine.createSpy('xyzzy callback');
      var spy2 = jasmine.createSpy('end of universe callback');

      var setUp = function(){
        oboeInstance
          .on('xyzzy', spy1)
          .on('end_of_universe', spy2);
      };
      expect(setUp).not.toThrow();

      oboeInstance
        .on('xyzzy', spy1)
        .on('end_of_universe', spy2);

      oboeInstance.emit('xyzzy', 'hello');
      oboeInstance.emit('end_of_universe', 'oh no!');

      expect( spy1 ).toHaveBeenCalledWith('hello');
      expect( spy2 ).toHaveBeenCalledWith('oh no!');
    });

    it('is allows removal', function() {
      var spy1 = jasmine.createSpy('xyzzy callback');
      var spy2 = jasmine.createSpy('end of universe callback');

      oboeInstance
        .on('xyzzy', spy1)
        .on('end_of_universe', spy2);

      oboeInstance.removeListener('xyzzy', spy1);
      oboeInstance.removeListener('end_of_universe', spy2);

      oboeInstance.emit('xyzzy', 'hello');
      oboeInstance.emit('end_of_universe', 'oh no!');

      expect( spy1 ).not.toHaveBeenCalled()
      expect( spy2 ).not.toHaveBeenCalled()
    });
  });


});
