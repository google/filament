describe("incremental content builder", function(){

  function IncrementalContentBuilderAsserter(){

     var eventBus = pubSub();

    sinon.spy(eventBus(NODE_CLOSED), 'emit');
    sinon.spy(eventBus(NODE_CLOSED), 'on');
    sinon.spy(eventBus(NODE_OPENED), 'emit');
    sinon.spy(eventBus(NODE_OPENED), 'on');
    sinon.spy(eventBus(ROOT_PATH_FOUND), 'emit');
    sinon.spy(eventBus(ROOT_PATH_FOUND), 'on');

    this._eventBus = eventBus;

    var builderInstance = incrementalContentBuilder(eventBus);

    ascentManager( this._eventBus, builderInstance);
  }

  IncrementalContentBuilderAsserter.prototype.receivingEvent = function(eventName /* args */){

    var args = Array.prototype.slice.call(arguments, 1);

    this._eventBus(eventName).emit.apply(undefined, args);

    return this;
  };

  describe('when root object opens', function() {

    var builder = aContentBuilder().receivingEvent(SAX_VALUE_OPEN, {});

    it('emits correct event', function(){
      expect( builder)
        .toHaveEmitted(
          NODE_OPENED
          ,  anAscentContaining(
            {key:ROOT_PATH, node:{}}
          )

        )
    });

    it('reports correct root', function () {

      expect(builder).toHaveEmittedRootWhichIsNow({})

    });
  })

  describe('after key is found in root object', function(){
    // above test, plus some extra events from clarinet
    var builder = aContentBuilder()
          .receivingEvent(SAX_VALUE_OPEN, {})
          .receivingEvent(SAX_KEY, 'flavour');

    it('emits correct event', function(){

      expect( builder )
        .toHaveEmitted(
          NODE_OPENED
          ,  anAscentContaining(
            {key:ROOT_PATH, node:{flavour:undefined}}
            ,  {key:'flavour', node:undefined}
          )
        )
    })

    it('reports correct root', function(){

      expect(builder).toHaveEmittedRootWhichIsNow({flavour:undefined});
    });

  })

  describe('if key is found at same time as root object', function() {
    // above test, plus some extra events from clarinet

    var builder = aContentBuilder()
          .receivingEvent(SAX_VALUE_OPEN, {}, undefined)
          .receivingEvent(SAX_KEY,         'flavour');

    it('emits correct event', function(){

      expect(builder).toHaveEmitted(
        NODE_OPENED
        ,  anAscentContaining(
          {key:ROOT_PATH, node:{flavour:undefined}}
          ,  {key:'flavour', node:undefined}
        )
      )
    });

    it('reports correct root', function(){

      expect(builder).toHaveEmittedRootWhichIsNow({flavour:undefined});
    });

  })

  describe('after value is found for that key', function() {

    var builder = aContentBuilder()
          .receivingEvent(SAX_VALUE_OPEN, {})
          .receivingEvent(SAX_KEY    ,  'flavour')
          .receivingEvent(SAX_VALUE_OPEN  ,  'strawberry')
          .receivingEvent(SAX_VALUE_CLOSE);

    it('emits correct event', function(){
      expect(builder).toHaveEmitted(
        NODE_CLOSED
        ,  anAscentContaining(
          {key:ROOT_PATH, node:{flavour:'strawberry'}}
          ,  {key:'flavour', node:'strawberry'}
        )
      )
    });

    it('reports correct root', function(){

      expect(builder).toHaveEmittedRootWhichIsNow({flavour:'strawberry'});
    });

  })

  describe('emits node found after root object closes', function() {

    var builder = aContentBuilder()
          .receivingEvent(SAX_VALUE_OPEN, {})
          .receivingEvent(SAX_KEY, 'flavour')
          .receivingEvent(SAX_VALUE_OPEN, 'strawberry').receivingEvent(SAX_VALUE_CLOSE)
          .receivingEvent(SAX_VALUE_CLOSE);

    it('emits correct event', function(){
      expect(builder).toHaveEmitted(
        NODE_CLOSED
        ,  anAscentContaining(
          {key:ROOT_PATH, node:{flavour:'strawberry'}}
        )
      )
    })

    it('reports correct root', function(){

      expect(builder).toHaveEmittedRootWhichIsNow({flavour:'strawberry'});
    });

  })

  describe('first array element', function() {

    var builder = aContentBuilder()
          .receivingEvent(SAX_VALUE_OPEN, {})
          .receivingEvent(SAX_KEY, 'alphabet')
          .receivingEvent(SAX_VALUE_OPEN, [])
          .receivingEvent(SAX_VALUE_OPEN, 'a').receivingEvent(SAX_VALUE_CLOSE);

    it('emits path event with numeric paths', function(){

      expect(builder).toHaveEmitted(
        NODE_OPENED
        , anAscentContaining(
          {key:ROOT_PATH,  node:{'alphabet':['a']}    }
          ,  {key:'alphabet', node:['a']                 }
          ,  {key:0,          node:'a'                   }
        )
      );
    })

    it('emitted node event', function(){
      expect(builder).toHaveEmitted(
        NODE_CLOSED
        ,  anAscentContaining(
          {key:ROOT_PATH,      node:{'alphabet':['a']} }
          ,  {key:'alphabet',     node:['a']              }
          ,  {key:0,              node:'a'                }
        )
      )
    })

    it('reports correct root', function(){

      expect(builder).toHaveEmittedRootWhichIsNow({'alphabet':['a']});
    });

  })

  describe('second array element', function() {

    var builder = aContentBuilder()
          .receivingEvent(SAX_VALUE_OPEN, {})
          .receivingEvent(SAX_KEY, 'alphabet')
          .receivingEvent(SAX_VALUE_OPEN, [])
          .receivingEvent(SAX_VALUE_OPEN, 'a').receivingEvent(SAX_VALUE_CLOSE)
          .receivingEvent(SAX_VALUE_OPEN, 'b').receivingEvent(SAX_VALUE_CLOSE);

    it('emits events with numeric paths', function(){

      expect(builder).toHaveEmitted(
        NODE_OPENED
        ,  anAscentContaining(
          {key:ROOT_PATH,  node:{'alphabet':['a','b']}   }
          , {key:'alphabet', node:['a','b']                }
          , {key:1,          node:'b'                      }
        )
      )
    })

    it('emitted node event', function(){
      expect(builder).toHaveEmitted(
        NODE_CLOSED
        ,  anAscentContaining(
          {key:ROOT_PATH,      node:{'alphabet':['a', 'b']} }
          ,  {key:'alphabet',     node:['a','b']               }
          ,  {key:1,              node:'b'                     }
        )
      )
    })

    it('reports correct root', function(){

      expect(builder).toHaveEmittedRootWhichIsNow({'alphabet':['a','b']});
    });

  })

  describe('array at root', function() {

    var builder = aContentBuilder()
          .receivingEvent(SAX_VALUE_OPEN, [])
          .receivingEvent(SAX_VALUE_OPEN, 'a').receivingEvent(SAX_VALUE_CLOSE)
          .receivingEvent(SAX_VALUE_OPEN, 'b').receivingEvent(SAX_VALUE_CLOSE);

    it('emits events with numeric paths', function(){

      expect(builder).toHaveEmitted(
        NODE_OPENED
        ,  anAscentContaining(
          {key:ROOT_PATH,  node:['a','b']                }
          , {key:1,          node:'b'                      }
        )
      )
    })

    it('emitted node event', function(){
      expect(builder).toHaveEmitted(
        NODE_CLOSED
        ,  anAscentContaining(
          {key:ROOT_PATH,    node:['a','b']                }
          ,  {key:1,            node:'b'                      }
        )
      )
    })

    it('reports correct root', function(){

      expect(builder).toHaveEmittedRootWhichIsNow(['a','b']);
    });

  })


  function aContentBuilder() {

    return new IncrementalContentBuilderAsserter();
  }


  beforeEach(function(){

    jasmine.addMatchers({
      toHaveEmittedRootWhichIsNow: function() {
        return {
          compare: function(actual, expectedRootObj) {
            var asserter = actual;
            var emit = asserter._eventBus(ROOT_PATH_FOUND).emit;

            return {
              pass: emit.calledWith(expectedRootObj)
            }
          }
        }
      },

      toHaveEmitted: function() {
        return {
          compare: function(actual, eventName, expectedAscent){

            var asserter = actual;
            var emit = asserter._eventBus(eventName).emit;

            var ascentMatch = sinon.match(function ( foundAscent ) {

              function matches( expect, found ) {
                if( !expect && !found ) {
                  return true;
                }

                if( !expect || !found ) {
                  // Both not empty, but one is. Unequal length ascents.
                  return false;
                }

                if( head(expect).key != head(found).key ) {
                  // keys unequal
                  return false;
                }

                if( JSON.stringify( head(expect).node ) != JSON.stringify( head(found).node ) ) {
                  // nodes unequal
                  return false;
                }

                return matches(tail(expect), tail(found));
              }

              return matches(expectedAscent, foundAscent);

            }, 'ascent match');


            this.message = function(){
              if( !emit.called ) {
                return 'no events have been emitted at all';
              }

              function reportCall(eventName, ascentList) {
                var argArray = listAsArray(ascentList);
                var toJson = JSON.stringify.bind(JSON);
                return 'type:' + eventName + ', ascent:[' + argArray.map(toJson).join(',    \t') + ']';
              }

              function reportArgs(args){
                return reportCall(args[0], args[1]);
              }

              return   'expected a call with : \t' + reportCall(eventName, expectedAscent) +
                '\n' +
                'latest call had :      \t' + reportArgs(emit.lastCall.args) +
                '\n' +
                'all calls were :' +
                '\n                     \t' +
                emit.args.map( reportArgs ).join('\n                     \t')
            };

            return {pass: emit.calledWithMatch( ascentMatch )};
          }
        }
      }

    });
  });

  function anAscentContaining ( /* descriptors */ ) {

    var ascentArray = Array.prototype.slice.call(arguments),
        ascentList = emptyList;

    ascentArray.forEach( function(ascentNode){
      ascentList = cons(ascentNode, ascentList);
    });

    return ascentList;
  }

});
