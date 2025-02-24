
describe('jsonPath', function(){

  describe('compiles valid syntax while rejecting invalid', function() {

    it("compiles a basic pattern without throwing", function(){

      expect(compiling('!')).not.toThrow();

    });

    describe("syntactically invalid patterns", function() {

      it("fail on single invalid token", function(){

        expect(compiling('/')).toThrow();
      });

      it("fail on invalid pattern with some valid tokens", function(){

        expect(compiling('foo/')).toThrow();
      });

      it("fail on unclosed duck clause", function(){

        expect(compiling('{foo')).toThrow();
      });

      it("fail on token with capture alone", function(){

        expect(compiling('foo$')).toThrow();
      });
    });

  });

  describe('patterns match correct paths', function() {

    describe('when pattern has only bang', function() {
      it("should match root", function(){

        expect('!').toMatchPath([]);
      });

      it("should miss non-root", function(){

        expect('!').not.toMatchPath(['a']);
        expect('!').not.toMatchPath(['a', 'b']);

      });
    });

    it('should match * universally', function() {
      expect('*').toMatchPath(            []          );
      expect('*').toMatchPath(            ['a']       );
      expect('*').toMatchPath(            ['a', 2]    );
      expect('*').toMatchPath(            ['a','b']   );
    });

    it('should match empty pattern universally', function() {
      expect('').toMatchPath(             []          );
      expect('').toMatchPath(             ['a']       );
      expect('').toMatchPath(             ['a', 2]    );
      expect('').toMatchPath(             ['a','b']   );
    });

    it('should match !.* against any top-level path node', function() {

      expect('!.*').toMatchPath(          ['foo'])
      expect('!.*').toMatchPath(          ['bar'])
      expect('!.*').not.toMatchPath(      [])
      expect('!.*').not.toMatchPath(      ['foo', 'bar'])
    });

    it('should match !..* against anything but the root', function() {

      expect('!..*').not.toMatchPath(     []          );
      expect('!..*').toMatchPath(         ['a']       );
      expect('!..*').toMatchPath(         ['a','b']   );
    });

    it('should match *..* against anything except the root since it requires a decendant ' +
       'which the root will never satisfy because it cannot have an ancestor', function() {

         expect('*..*').not.toMatchPath(     []          );
         expect('*..*').toMatchPath(         ['a']       );
         expect('*..*').toMatchPath(         ['a','b']   );

       });

    it('should match !.foo against foo node at first level only', function(){

      expect('!.foo').toMatchPath(        ['foo']     );
      expect('!.foo').not.toMatchPath(    []          );
      expect('!.foo').not.toMatchPath(    ['foo', 'bar']);
      expect('!.foo').not.toMatchPath(    ['bar']     );
    });

    it('should match !.foo.bar against paths with foo as first node and bar as second', function() {

      expect('!.a.b').toMatchPath(        ['a', 'b'])
      expect('!.a.b').not.toMatchPath(    [])
      expect('!.a.b').not.toMatchPath(    ['a'])
    });

    it('should match !..foo against any path ending in foo', function(){

      expect('!..foo').not.toMatchPath(   []);
      expect('!..foo').toMatchPath(       ['foo']);
      expect('!..foo').toMatchPath(       ['a', 'foo']);
      expect('!..foo').not.toMatchPath(   ['a', 'foo', 'a']);
      expect('!..foo').toMatchPath(       ['a', 'foo', 'foo']);
      expect('!..foo').toMatchPath(       ['a', 'a', 'foo']);
      expect('!..foo').not.toMatchPath(   ['a', 'a', 'foot']);
      expect('!..foo').not.toMatchPath(   ['a', 'foo', 'foo', 'a']);
    });

    it('should match ..foo like !..foo', function() {
      expect('..foo').not.toMatchPath(   []);
      expect('..foo').toMatchPath(       ['foo']);
      expect('..foo').toMatchPath(       ['a', 'foo']);
      expect('..foo').not.toMatchPath(   ['a', 'foo', 'a']);
      expect('..foo').toMatchPath(       ['a', 'foo', 'foo']);
      expect('..foo').toMatchPath(       ['a', 'a', 'foo']);
      expect('..foo').not.toMatchPath(   ['a', 'a', 'foot']);
      expect('..foo').not.toMatchPath(   ['a', 'foo', 'foo', 'a']);
    });

    it('should match foo like !..foo or ..foo', function() {
      expect('foo').not.toMatchPath(   []);
      expect('foo').toMatchPath(       ['foo']);
      expect('foo').toMatchPath(       ['a', 'foo']);
      expect('foo').not.toMatchPath(   ['a', 'foo', 'a']);
      expect('foo').toMatchPath(       ['a', 'foo', 'foo']);
      expect('foo').toMatchPath(       ['a', 'a', 'foo']);
      expect('foo').not.toMatchPath(   ['a', 'a', 'foot']);
      expect('foo').not.toMatchPath(   ['a', 'foo', 'foo', 'a']);
    });

    it('is not fooled by substrings in path nodes', function(){
      expect('!.foo').not.toMatchPath(    ['foot'])
    });

    it('matches !..foo.bar against bars which are direct children of a foo anywhere in the document', function() {

      expect('!..foo.bar').not.toMatchPath(   []);
      expect('!..foo.bar').not.toMatchPath(   ['foo']);
      expect('!..foo.bar').not.toMatchPath(   ['a', 'foo']);
      expect('!..foo.bar').toMatchPath(       ['a', 'foo', 'bar']);
      expect('!..foo.bar').not.toMatchPath(   ['a', 'foo', 'foo']);
      expect('!..foo.bar').toMatchPath(       ['a', 'a', 'a', 'foo', 'bar']);
      expect('!..foo.bar').not.toMatchPath(   ['a', 'a', 'a', 'foo', 'bar', 'a']);
    });

    it('matches foo.bar like !..foo.bar', function() {

      expect('foo.bar').not.toMatchPath(   [])
      expect('foo.bar').not.toMatchPath(   ['foo'])
      expect('foo.bar').not.toMatchPath(   ['a', 'foo'])
      expect('foo.bar').toMatchPath(       ['a', 'foo', 'bar'])
      expect('foo.bar').not.toMatchPath(   ['a', 'foo', 'foo'])
      expect('foo.bar').toMatchPath(       ['a', 'a', 'a', 'foo', 'bar'])
      expect('foo.bar').not.toMatchPath(   ['a', 'a', 'a', 'foo', 'bar', 'a'])
    });

    it('matches !..foo.*.bar only if there is an intermediate node between foo and bar', function(){

      expect('!..foo.*.bar').not.toMatchPath(   [])
      expect('!..foo.*.bar').not.toMatchPath(   ['foo'])
      expect('!..foo.*.bar').not.toMatchPath(   ['a', 'foo'])
      expect('!..foo.*.bar').not.toMatchPath(   ['a', 'foo', 'bar'])
      expect('!..foo.*.bar').toMatchPath(       ['a', 'foo', 'a', 'bar'])
      expect('!..foo.*.bar').not.toMatchPath(   ['a', 'foo', 'foo'])
      expect('!..foo.*.bar').not.toMatchPath(   ['a', 'a', 'a', 'foo', 'bar'])
      expect('!..foo.*.bar').toMatchPath(       ['a', 'a', 'a', 'foo', 'a', 'bar'])
      expect('!..foo.*.bar').not.toMatchPath(   ['a', 'a', 'a', 'foo', 'bar', 'a'])
      expect('!..foo.*.bar').not.toMatchPath(   ['a', 'a', 'a', 'foo', 'a', 'bar', 'a'])
    });

    describe('with numeric path nodes in the pattern', function() {

      it('should be able to handle numeric nodes in object notation', function(){

        expect('!.a.2').toMatchPath(       ['a', 2])
        expect('!.a.2').toMatchPath(       ['a', '2'])
        expect('!.a.2').not.toMatchPath(    [])
        expect('!.a.2').not.toMatchPath(    ['a'])
      });

      it('should be able to handle numberic nodes in array notation', function(){

        expect('!.a[2]').toMatchPath(       ['a', 2])
        expect('!.a[2]').toMatchPath(       ['a', '2'])
        expect('!.a[2]').not.toMatchPath(    [])
        expect('!.a[2]').not.toMatchPath(    ['a'])
      });
    });

    describe('with array notation', function() {

      it('should handle adjacent array notations', function(){

        expect('!["a"][2]').toMatchPath(       ['a', 2])
        expect('!["a"][2]').toMatchPath(       ['a', '2'])
        expect('!["a"][2]').not.toMatchPath(    [])
        expect('!["a"][2]').not.toMatchPath(    ['a'])
      });

      it('should allow to specify child of root', function(){

        expect('![2]').toMatchPath(       [2])
        expect('![2]').toMatchPath(       ['2'])
        expect('![2]').not.toMatchPath(    [])
        expect('![2]').not.toMatchPath(    ['a'])
      });

      it('should be allowed to contain a star', function(){

        expect('![*]').toMatchPath(       [2])
        expect('![*]').toMatchPath(       ['2'])
        expect('![*]').toMatchPath(       ['a'])
        expect('![*]').not.toMatchPath(    [])
      });

    });

    describe('composition of several tokens into complex patterns', function() {

      it('should be able to handle more than one double dot', function() {
        expect('!..foods..fr')
          .toMatchPath( ['foods', 2, 'name', 'fr']);
      });

      it('should be able to match ..* or ..[*] as if it were * because .. matches zero nodes', function(){

        expect('!..*.bar')
          .toMatchPath(['anything', 'bar']);

        expect('!..[*].bar')
          .toMatchPath(['anything', 'bar']);
      });

    });


    describe('using css4-style syntax', function() {


      it('returns deepest node when no css4-style syntax is used', function(){

        expect( matchOf( 'l2.*' ).against(

          ascentFrom({ l1:       {l2:      {l3:'leaf'}}})

        )).toSpecifyNode('leaf');

      });

      it('returns correct named node', function(){

        expect( matchOf( '$l2.*' ).against(

          ascentFrom({ l1:       {l2:      {l3:'leaf'}}})

        )).toSpecifyNode({l3:'leaf'});

      });

      it('returns correct node when css4-style pattern is followed by double dot', function() {

        expect( matchOf( '!..$foo..bar' ).against(

          ascentFrom({ l1:       {foo:      {l3:    {bar:    'leaf'}}}})

        )).toSpecifyNode({l3:    {bar:    'leaf'}});

      });

      it('can match children of root while capturing the root', function() {

        expect( matchOf( '$!.*' ).against(

          ascentFrom({ l1: 'leaf' })

        )).toSpecifyNode({ l1: 'leaf' });

      });

      it('returns captured node with array notation', function() {

        expect( matchOf( '$["l1"].l2' ).against(

          ascentFrom({ l1: {l2:'leaf'} })

        )).toSpecifyNode({ l2: 'leaf' });

      });

      it('returns captured node with array numbered notation', function() {

        expect( matchOf( '$["2"].l2' ).against(

          ascentFrom({ '2': {l2:'leaf'} })

        )).toSpecifyNode({ l2: 'leaf' });

      });

      it('returns captured node with star notation', function() {

        expect( matchOf( '!..$*.l3' ).against(

          ascentFrom({ l1: {l2:{l3:'leaf'}} })

        )).toSpecifyNode({ l3: 'leaf' });

      });

      it('returns captured node with array star notation', function(){

        expect( matchOf( '!..$[*].l3' ).against(

          ascentFrom({ l1: {l2:{l3:'leaf'}} })

        )).toSpecifyNode({ l3: 'leaf' });

      });
    });

    describe('with duck matching', function() {

      it('can do basic ducking', function(){

        var rootJson = {
          people:{
            jack:{
              name:  'Jack'
              ,  email: 'jack@example.com'
            }
          }
        };

        expect( matchOf( '{name email}' ).against(

          asAscent(
            [          'people',          'jack'                 ],
            [rootJson, rootJson.people,   rootJson.people.jack   ]
          )

        )).toSpecifyNode({name:  'Jack',  email: 'jack@example.com'});

      });

      it('can duck on two levels of a path', function(){

        var rootJson = {
          people:{
            jack:{
              name:  'Jack'
              ,  email: 'jack@example.com'
            }
          }
        };

        expect( matchOf( '{people}.{jack}.{name email}' ).against(

          asAscent(
            [          'people',          'jack'                 ],
            [rootJson, rootJson.people,   rootJson.people.jack   ]
          )

        )).toSpecifyNode({name:  'Jack',  email: 'jack@example.com'});
      });

      it('fails if one duck is unsatisfied', function(){

        var rootJson = {
          people:{
            jack:{
              name:  'Jack'
              ,  email: 'jack@example.com'
            }
          }
        };

        expect( matchOf( '{people}.{alberto}.{name email}' ).against(

          asAscent(
            [          'people',          'jack'                 ],
            [rootJson, rootJson.people,   rootJson.people.jack   ]
          )

        )).not.toSpecifyNode({name:  'Jack',  email: 'jack@example.com'});
      });


      it('can construct the root duck type', function(){

        var rootJson = {
          people:{
            jack:{
              name:  'Jack'
              ,  email: 'jack@example.com'
            }
          }
        };

        expect( matchOf( '{}' ).against(

          asAscent(
            [          'people',          'jack'                 ],
            [rootJson, rootJson.people,   rootJson.people.jack   ]
          )

        )).toSpecifyNode({name:  'Jack',  email: 'jack@example.com'});

      });

      it('does not match if not all fields are there', function(){

        var rootJson = {
          people:{
            jack:{
              // no name here!
              email: 'jack@example.com'
            }
          }
        };

        expect( matchOf( '{name email}' ).against(

          asAscent(
            [          'people',          'jack'                 ],
            [rootJson, rootJson.people,   rootJson.people.jack   ]
          )

        )).not.toSpecifyNode({name:  'Jack',  email: 'jack@example.com'});

      });

      it('fails if something upstream fails', function(){

        var rootJson = {
          women:{
            betty:{
              name:'Betty'
              ,  email: 'betty@example.com'
            }
          },
          men:{
            // we don't have no menz!
          }
        };

        expect( matchOf( 'men.{name email}' ).against(

          asAscent(
            [          'women',          'betty'                 ],
            [rootJson, rootJson.women,   rootJson.women.betty    ]
          )

        )).not.toSpecifyNode({name:  'Jack',  email: 'jack@example.com'});

      });

      it('does not crash given ascent starting from non-objects', function(){

        var rootJson = [ 1, 2, 3 ];

        expect( function(){ matchOf( '{spin taste}' ).against(

          asAscent(
            [         '0'           ],
            [rootJson, rootJson[0]  ]
          )

        )}).not.toThrow();

      });

      it('does not match when given non-object', function(){

        var rootJson = [ 1, 2, 3 ];

        expect( matchOf( '{spin taste}' ).against(

          asAscent(
            [         '0'           ],
            [rootJson, rootJson[0]  ]
          )

        )).toBeFalsy();

      });

    });
  });

  beforeEach(function(){

    jasmine.addMatchers({
      toSpecifyNode: function() {
        return {
          compare: function(actual, expectedNode) {
            var result = {};

            function jsonSame(a,b) {
              return JSON.stringify(a) == JSON.stringify(b);
            }

            var match = actual;

            result.message = "Expected node " + JSON.stringify(expectedNode) +
              "but got " + (match.node? JSON.stringify(match.node) : 'no match');

            result.pass = jsonSame( expectedNode, match.node );
            return result;
          }
        };
      },
      toMatchPath: function() {
        return {
          compare: function(actual, pathStack) {
            var result = {};
            var pattern = actual;

            try {
              result.pass = !!matchOf(pattern).against(asAscent(pathStack));
            } catch( e ) {
              result.message = 'Error thrown running pattern "' + pattern +
                '" against path [' + pathStack.join(',') + ']' + "\n" +
                (e.stack || e.message);
              result.pass = false;
            };

            return result;
          }
        };
      }
    });
  });

  function compiling(pattern) {
    return function(){
      jsonPathCompiler(pattern);
    };
  }

  function matchOf(pattern) {
    var compiledPattern = jsonPathCompiler(pattern);

    return {
      against:function(ascent) {

        return compiledPattern(ascent);
      }
    };
  }


  // for the given pattern, return an array of empty objects of the one greater length to
  // stand in for the nodestack in the cases where we only care about match or not match.
  // one greater because the root node doesnt have a name
  function fakeNodeStack(path){

    var rtn = path.map(function(){return {}});

    rtn.unshift({iAm:'root'});
    return rtn;
  }


  function asAscent(pathStack, nodeStack){

    // first, make a defensive copy of the vars so that we can mutate them at will:
    pathStack = pathStack && JSON.parse(JSON.stringify(pathStack));
    nodeStack = nodeStack && JSON.parse(JSON.stringify(nodeStack));

    // change the two parameters into the test from  arrays (which are easy to write as in-line js) to
    // lists (which is what the code under test needs)

    nodeStack = nodeStack || fakeNodeStack(pathStack);

    pathStack.unshift(ROOT_PATH);

    // NB: can't use the more functional Array.prototype.reduce here, IE8 doesn't have it and might not
    // be polyfilled

    var ascent = emptyList;

    for (var i = 0; i < pathStack.length; i++) {

      var mapping = {key: pathStack[i], node:nodeStack[i]};

      ascent = cons( mapping, ascent );
    }

    return ascent;
  }


});
