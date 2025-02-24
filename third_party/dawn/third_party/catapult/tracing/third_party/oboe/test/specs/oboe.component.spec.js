(function(Platform) {
  describe("oboe component (no http, content fed in externally)", function(){

    var path;
    var givenAnOboeInstance;
    var oboe;
    var asserter;
    var matched;
    var wasGivenTheOboeAsContext;
    var hasRootJson;
    var calledCallbackOnce;
    var gaveFinalCallbackWithRootJson;
    var foundOneMatch;
    var wasPassedAnErrorObject;
    var foundNMatches;
    var foundNoMatches;

    // Used to spy on global functions like setTimeout
    var globalContext;
    if ( !Platform.isNode ) {
      globalContext = window;
      givenAnOboeInstance = window.givenAnOboeInstance;
      oboe = window.oboe;
      matched = window.matched;
      wasGivenTheOboeAsContext = window.wasGivenTheOboeAsContext;
      hasRootJson = window.hasRootJson;
      calledCallbackOnce = window.calledCallbackOnce;
      gaveFinalCallbackWithRootJson = window.gaveFinalCallbackWithRootJson;
      foundOneMatch = window.foundOneMatch;
      wasPassedAnErrorObject = window.wasPassedAnErrorObject;
      foundNMatches = window.foundNMatches;
      foundNoMatches = window.foundNoMatches;
    } else {
      globalContext = GLOBAL;
      oboe = require('../../dist/oboe-node.js');
      asserter = require('../libs/oboeAsserter.js');
      givenAnOboeInstance = asserter.givenAnOboeInstance;
      matched = asserter.matched;
      wasGivenTheOboeAsContext = asserter.wasGivenTheOboeAsContext;
      hasRootJson = asserter.hasRootJson;
      calledCallbackOnce = asserter.calledCallbackOnce;
      gaveFinalCallbackWithRootJson = asserter.gaveFinalCallbackWithRootJson;
      foundOneMatch = asserter.foundOneMatch;
      wasPassedAnErrorObject = asserter.wasPassedAnErrorObject;
      foundNMatches = asserter.foundNMatches;
      foundNoMatches = asserter.foundNoMatches;
    }

    /*
     a more jasmine-y version of the next test might look like this:

     describe('empty object detected with bang',  function() {

     var callback = jasmine.createSpy('callback');
     var oboe = anOboe().node('!', callback).afterInput('{}')
     giveInput(oboe, '{}');

     it( 'should find the empty object at root', function(){

     expect(

     nodesFoundBy(
     anOboe().listeningForNodesAt('!').afterInput('{}')
     )

     ).toIncludeNode( {}, atRoot )
     })

     it( 'should not find anything else', function(){

     expect(

     nodesFoundBy(
     anOboe().listeningForNodes('!').afterInput('{}')
     ).length

     ).toBe(1)
     })

     })*/

    it('handles empty object detected with bang',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!')
        .whenGivenInput('{}')
        .thenTheInstance(
          matched({}).atRootOfJson(),
          foundOneMatch
        );
    })

    it('handles empty object detected with bang when explicitly selected',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('$!')
        .whenGivenInput('{}')
        .thenTheInstance(
          matched({}).atRootOfJson(),
          foundOneMatch
        );

    })

    it('gives the oboe instance as context',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!')
        .whenGivenInput('{}')
        .thenTheInstance( wasGivenTheOboeAsContext() );
    })


    it('find only emits when has whole object',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!')
        .whenGivenInputPart('{')
        .thenTheInstance(
          foundNoMatches
        )
        .whenGivenInput('}')
        .thenTheInstance(
          matched({}).atRootOfJson(),
          foundOneMatch
        );

    })

    it('emits path to listener when root object starts',  function() {

      // clarinet doesn't notify of matches to objects (SAX_OPEN_OBJECT) until the
      // first key is found, that is why we don't just give '{' here as the partial
      // input.

      givenAnOboeInstance()
        .andWeAreListeningForPaths('!')
        .whenGivenInputPart('{"foo":')
        .thenTheInstance(
          foundNMatches(1),
          matched({}).atRootOfJson()
        );
    })

    it('emits path to listener when root array starts',  function() {

      // clarinet doesn't notify of matches to objects (SAX_OPEN_OBJECT) until the
      // first key is found, that is why we don't just give '{' here as the partial
      // input.

      givenAnOboeInstance()
        .andWeAreListeningForPaths('!')
        .whenGivenInputPart('[1') // the minimum string required for clarinet
      // to emit SAX_OPEN_ARRAY. Won't emit with '['.
        .thenTheInstance(
          foundNMatches(1),
          matched([]).atRootOfJson()
        );
    })


    it('emits empty object node detected with single star',  function() {
      // *
      givenAnOboeInstance()
        .andWeAreListeningForNodes('*')
        .whenGivenInput('{}')
        .thenTheInstance(
          matched({}).atRootOfJson(),
          foundOneMatch
        );
    })

    it('doesnt detect spurious path off empty object',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForPaths('!.foo.*')
        .whenGivenInput( {foo:{}} )
        .thenTheInstance(
          foundNoMatches
        );
    })

    it('handles empty object detected with double dot',  function() {
      // *
      givenAnOboeInstance()
        .andWeAreListeningForNodes('*')
        .whenGivenInput('{}')
        .thenTheInstance(
          matched({}).atRootOfJson(),
          foundOneMatch
        );
    })

    it('notifies of strings when listened to',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!.string')
        .whenGivenInput('{"string":"s"}')
        .thenTheInstance(
          matched("s"),
          foundOneMatch
        );
    })

    it('can detect nodes with hyphen in the name',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!.a-string')
        .whenGivenInput({"a-string":"s"})
        .thenTheInstance(
          matched("s"),
          foundOneMatch
        );
    })

    it('can detect nodes with underscore in the name',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!.a_string')
        .whenGivenInput({"a_string":"s"})
        .thenTheInstance(
          matched("s"),
          foundOneMatch
        );
    })

    it('can detect nodes with quoted hyphen in the name',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!["a-string"]')
        .whenGivenInput({"a-string":"s"})
        .thenTheInstance(
          matched("s"),
          foundOneMatch
        );
    })

    it('can detect nodes with quoted underscore in the name',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!["a_string"]')
        .whenGivenInput({"a_string":"s"})
        .thenTheInstance(
          matched("s"),
          foundOneMatch
        );
    })

    it('can detect nodes with quoted unusual ascii chars in the name',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!["£@$%^"]')
        .whenGivenInput({"£@$%^":"s"}) // ridiculous JSON!
        .thenTheInstance(
          matched("s"),
          foundOneMatch
        );
    })

    it('can detect nodes with non-ascii keys',  function() {

      //pinyin: Wǒ tǎoyàn IE liúlǎn qì!

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!["我讨厌IE浏览器！"]')
        .whenGivenInput({"我讨厌IE浏览器！":"indeed!"}) // ridiculous JSON!
        .thenTheInstance(
          matched("indeed!"),
          foundOneMatch
        );
    })

    it('can detect nodes with non-ascii keys and values',  function() {

      // hope you have a good unicode font!

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!["☂"]')
        .whenGivenInput({"☂":"☁"}) // ridiculous JSON!
        .thenTheInstance(
          matched("☁"),
          foundOneMatch
        );
    })

    it('notifies of path before given the json value for a property',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForPaths('!.string')
        .whenGivenInputPart('{"string":')
        .thenTheInstance(
          foundOneMatch
        );
    })

    it('notifies of second property name with incomplete json',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForPaths('!.pencils')
        .whenGivenInputPart('{"pens":4, "pencils":')
        .thenTheInstance(
          // undefined because the parser hasn't been given the value yet.
          // can't be null because that is an allowed value
          matched(undefined).atPath(['pencils']),
          foundOneMatch
        );
    })

    it('is able to notify of null',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!.pencils')
        .whenGivenInput('{"pens":4, "pencils":null}')
        .thenTheInstance(
          // undefined because the parser hasn't been given the value yet.
          // can't be null because that is an allowed value
          matched(null).atPath(['pencils']),
          foundOneMatch
        );
    })

    it('is able to notify of boolean true',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!.pencils')
        .whenGivenInput('{"pens":false, "pencils":true}')
        .thenTheInstance(
          // undefined because the parser hasn't been given the value yet.
          // can't be null because that is an allowed value
          matched(true).atPath(['pencils']),
          foundOneMatch
        );
    })

    it('is able to notify of boolean false',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!.pens')
        .whenGivenInput('{"pens":false, "pencils":true}')
        .thenTheInstance(
          // undefined because the parser hasn't been given the value yet.
          // can't be null because that is an allowed value
          matched(false).atPath(['pens']),
          foundOneMatch
        );
    })

    it('notifies of multiple children of root',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!.*')
        .whenGivenInput('{"a":"A","b":"B","c":"C"}')
        .thenTheInstance(
          matched('A').atPath(['a'])
          ,   matched('B').atPath(['b'])
          ,   matched('C').atPath(['c'])
          ,   foundNMatches(3)
        );
    })

    it('notifies of multiple children of root when selecting the root',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('$!.*')
        .whenGivenInput({"a":"A", "b":"B", "c":"C"})
        .thenTheInstance(
          // rather than getting the fully formed objects, we should now see the root object
          // being grown step by step:
          matched({"a":"A"})
          ,   matched({"a":"A", "b":"B"})
          ,   matched({"a":"A", "b":"B", "c":"C"})
          ,   foundNMatches(3)
        );
    })

    it('does not notify spuriously of descendant of roots when key is actually in another object',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForPaths('!.a')
        .whenGivenInput([{a:'a'}])
        .thenTheInstance(foundNoMatches);
    })

    it('does not notify spuriously of found child of root when ndoe is not child of root',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!.a')
        .whenGivenInput([{a:'a'}])
        .thenTheInstance(foundNoMatches);
    })

    describe('progressive output', function(){

      describe('giving notification of multiple properties of an object without waiting for entire object',  function(){

        it( 'gives no notification on seeing just the key', function(){
          givenAnOboeInstance()
            .andWeAreListeningForNodes('!.*')
            .whenGivenInputPart('{"a":')
            .thenTheInstance(
              foundNoMatches
            )
        });

        it( 'gives one notification on seeing just the first value', function(){
          givenAnOboeInstance()
            .andWeAreListeningForNodes('!.*')
            .whenGivenInputPart('{"a":"A",')
            .thenTheInstance(
              matched('A').atPath(['a'])
              ,   foundOneMatch
            )
        });

        it( 'gives another notification on seeing just the second key/value', function(){
          givenAnOboeInstance()
            .andWeAreListeningForNodes('!.*')
            .whenGivenInput('{"a":"A","b":"B"}')
            .thenTheInstance(
              matched('B').atPath(['b'])
              ,   foundNMatches(2)
            );
        });
      })

      describe('giving root progressively as root json object is built up',  function() {

        it('can supply root after seeing first key with undefined value', function(){
          givenAnOboeInstance()
            .whenGivenInputPart('{"a":')
            .thenTheInstance(
              hasRootJson({a:undefined})
            );
        });

        it('can supply root after seeing first key/value with defined value', function(){
          givenAnOboeInstance()
            .whenGivenInputPart('{"a":"A",')
            .thenTheInstance(
              hasRootJson({a:'A'})
            )
        });

        it('gives second key with undefined value', function(){
          givenAnOboeInstance()
            .whenGivenInputPart('{"a":"A","b":')
            .thenTheInstance(
              hasRootJson({a:'A', b:undefined})
            )
        });

        it('gives second key with defined value', function(){
          givenAnOboeInstance()
            .whenGivenInput('{"a":"A","b":"B"}')
            .thenTheInstance(
              hasRootJson({a:'A', b:'B'})
            )
        });

        it('gives final callback when done', function(){
          givenAnOboeInstance()
            .whenGivenInput('{"a":"A","b":"B"}')
            .whenInputFinishes()
            .thenTheInstance(
              gaveFinalCallbackWithRootJson({a:'A', b:'B'})
            );
        });
      })

      describe('giving root progressively as root json array is built up',  function() {

        // let's feed it the array [11,22] in drips of one or two chars at a time:
        it('has nothing on array open', function(){
          givenAnOboeInstance()
            .whenGivenInputPart('[')
            .thenTheInstance(
              // I would like this to be [] but clarinet doesn't emit array found until it has seen
              // the first element
              hasRootJson(undefined)
            )
        });
        it('has empty array soon afterwards', function(){
          givenAnOboeInstance()
            .whenGivenInputPart('[1')
            .thenTheInstance(
              // since we haven't seen a comma yet, the 1 could be the start of a multi-digit number
              // so nothing can be added to the root json
              hasRootJson([])
            )
        });
        it('has the first element on seeing the comma', function(){
          givenAnOboeInstance()
            .whenGivenInputPart('[11,')
            .thenTheInstance(
              hasRootJson([11])
            )
        });
        it('has no more on seeing the start of the next element', function(){
          givenAnOboeInstance()
            .whenGivenInputPart('[11,2')
            .thenTheInstance(
              hasRootJson([11])
            )
        });
        it('has everything when the array closes', function(){
          givenAnOboeInstance()
            .whenGivenInput('[11,22]')
            .thenTheInstance(
              hasRootJson([11,22])
            )
        });
        it('notified correctly of the final root', function(){
          givenAnOboeInstance()
            .whenGivenInput('[11,22]')
            .whenInputFinishes()
            .thenTheInstance(
              gaveFinalCallbackWithRootJson([11,22])
            )
        });
      })
    });

    it('notifies of named child of root',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!.b')
        .whenGivenInput('{"a":"A","b":"B","c":"C"}')
        .thenTheInstance(
          matched('B').atPath(['b'])
          ,   foundOneMatch
        );
    })
    it('notifies of array elements',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!.testArray.*')
        .whenGivenInput('{"testArray":["a","b","c"]}')
        .thenTheInstance(
          matched('a').atPath(['testArray',0])
          ,   matched('b').atPath(['testArray',1])
          ,   matched('c').atPath(['testArray',2])
          ,   foundNMatches(3)
        );
    })
    it('notifies of path match when array starts',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForPaths('!.testArray')
        .whenGivenInputPart('{"testArray":["a"')
        .thenTheInstance(
          foundNMatches(1)
          ,   matched(undefined) // when path is matched, it is not known yet
          // that it contains an array. Null should not
          // be used here because that is an allowed
          // value in json
        );
    })
    it('notifies of path match when second array starts',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForPaths('!.array2')
        .whenGivenInputPart('{"array1":["a","b"], "array2":["a"')
        .thenTheInstance(
          foundNMatches(1)
          ,  matched(undefined) // when path is matched, it is not known yet
          // that it contains an array. Null should not
          // be used here because that is an allowed
          // value in json
        );
    })
    it('notifies of paths inside arrays',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForPaths('![*]')
        .whenGivenInput( [{}, 'b', 2, []] )
        .thenTheInstance(
          foundNMatches(4)
        );
    })

    describe('correctly give index inside arrays', function(){
      it('when finding objects in array',  function() {

        givenAnOboeInstance()
          .andWeAreListeningForPaths('![2]')
          .whenGivenInput( [{}, {}, 'this_one'] )
          .thenTheInstance(
            foundNMatches(1)
          );
      })
      it('when finding arrays inside array',  function() {

        givenAnOboeInstance()
          .andWeAreListeningForPaths('![2]')
          .whenGivenInput( [[], [], 'this_one'] )
          .thenTheInstance(
            foundNMatches(1)
          );
      })
      it('when finding arrays inside arrays etc',  function() {

        givenAnOboeInstance()
          .andWeAreListeningForPaths('![2][2]')
          .whenGivenInput( [
            [],
            [],
            [
              [],
              [],
              ['this_array']
            ]
          ] )
          .thenTheInstance(
            foundNMatches(1)
          );
      })
      it('when finding strings inside array',  function() {

        givenAnOboeInstance()
          .andWeAreListeningForPaths('![2]')
          .whenGivenInput( ['', '', 'this_one'] )
          .thenTheInstance(
            foundNMatches(1)
          );
      })
      it('when finding numbers inside array',  function() {

        givenAnOboeInstance()
          .andWeAreListeningForPaths('![2]')
          .whenGivenInput( [1, 1, 'this_one'] )
          .thenTheInstance(
            foundNMatches(1)
          );
      })
      it('when finding nulls inside array',  function() {

        givenAnOboeInstance()
          .andWeAreListeningForPaths('![2]')
          .whenGivenInput( [null, null, 'this_one'] )
          .thenTheInstance(
            foundNMatches(1)
          );
      })
    })

    it('notifies of paths inside objects',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForPaths('![*]')
        .whenGivenInput( {a:{}, b:'b', c:2, d:[]} )
        .thenTheInstance(
          foundNMatches(4)
        );
    })

    describe('selecting by index', function(){
      it('notifies of array elements',  function() {

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.testArray[2]')
          .whenGivenInput('{"testArray":["a","b","this_one"]}')
          .thenTheInstance(
            matched('this_one').atPath(['testArray',2])
            ,   foundOneMatch
          );
      })

      it('notifies nested array elements',  function() {

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.testArray[2][2]')
          .whenGivenInput( {"testArray":
                            ["a","b",
                             ["x","y","this_one"]
                            ]
                           }
                         )
          .thenTheInstance(
            matched('this_one')
              .atPath(['testArray',2,2])
              .withParent( ["x","y","this_one"] )
              .withGrandparent( ["a","b", ["x","y","this_one"]] )
            ,   foundOneMatch
          );
      })
      it('can notify nested array elements by passing the root array',  function() {

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.$testArray[2][2]')
          .whenGivenInput( {"testArray":
                            ["a","b",
                             ["x","y","this_one"]
                            ]
                           }
                         )
          .thenTheInstance(
            matched(   ["a","b",
                        ["x","y","this_one"]
                       ])
            ,   foundOneMatch
          );
      })
    });

    describe('deeply nested objects', function(){
      it('notifies with star pattern',  function() {

        givenAnOboeInstance()
          .andWeAreListeningForNodes('*')
          .whenGivenInput({"a":{"b":{"c":{"d":"e"}}}})
          .thenTheInstance(
            matched('e')
              .atPath(['a', 'b', 'c', 'd'])
              .withParent({d:'e'})
            ,   matched({d:"e"})
              .atPath(['a', 'b', 'c'])
            ,   matched({c:{d:"e"}})
              .atPath(['a', 'b'])
            ,   matched({b:{c:{d:"e"}}})
              .atPath(['a'])
            ,   matched({a:{b:{c:{d:"e"}}}})
              .atRootOfJson()
            ,   foundNMatches(5)
          );
      })
      it('notifies of with double dot pattern',  function() {

        givenAnOboeInstance()
          .andWeAreListeningForNodes('..')
          .whenGivenInput({"a":{"b":{"c":{"d":"e"}}}})
          .thenTheInstance(
            matched('e')
              .atPath(['a', 'b', 'c', 'd'])
              .withParent({d:'e'})
            ,   matched({d:"e"})
              .atPath(['a', 'b', 'c'])
            ,   matched({c:{d:"e"}})
              .atPath(['a', 'b'])
            ,   matched({b:{c:{d:"e"}}})
              .atPath(['a'])
            ,   matched({a:{b:{c:{d:"e"}}}})
              .atRootOfJson()
            ,   foundNMatches(5)
          );
      })
      it('notifies of objects with double dot star pattern',  function() {

        givenAnOboeInstance()
          .andWeAreListeningForNodes('..*')
          .whenGivenInput({"a":{"b":{"c":{"d":"e"}}}})
          .thenTheInstance(
            matched('e')
              .atPath(['a', 'b', 'c', 'd'])
              .withParent({d:'e'})
            ,   matched({d:"e"})
              .atPath(['a', 'b', 'c'])
            ,   matched({c:{d:"e"}})
              .atPath(['a', 'b'])
            ,   matched({b:{c:{d:"e"}}})
              .atPath(['a'])
            ,   matched({a:{b:{c:{d:"e"}}}})
              .atRootOfJson()
            ,   foundNMatches(5)
          );
      })
    });

    it('can express all but root as a pattern',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('*..*')
        .whenGivenInput({"a":{"b":{"c":{"d":"e"}}}})
        .thenTheInstance(
          matched('e')
            .atPath(['a', 'b', 'c', 'd'])
            .withParent({d:'e'})
          ,   matched({d:"e"})
            .atPath(['a', 'b', 'c'])
          ,   matched({c:{d:"e"}})
            .atPath(['a', 'b'])
          ,   matched({b:{c:{d:"e"}}})
            .atPath(['a'])

          ,   foundNMatches(4)
        );
    })
    it('can detect similar ancestors',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('foo..foo')

        .whenGivenInput({"foo":{"foo":{"foo":{"foo":"foo"}}}})
        .thenTheInstance(
          matched("foo")
          ,   matched({"foo":"foo"})
          ,   matched({"foo":{"foo":"foo"}})
          ,   matched({"foo":{"foo":{"foo":"foo"}}})
          ,   foundNMatches(4)
        );
    })

    it('can detect inside the second object element of an array',  function() {

      // this fails in incrementalJsonBuilder if we don't set the curKey to the
      // length of the array when we detect an object and and the parent of the
      // object that ended was an array

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!..find')
        .whenGivenInput(
          {
            array:[
              {a:'A'}
              ,  {find:'should_find_this'}
            ]
          }
        )
        .thenTheInstance(
          matched('should_find_this')
            .atPath(['array',1,'find'])
        );
    })
    it('ignores keys if only start matches',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!..a')
        .whenGivenInput({
          ab:'should_not_find_this'
          ,  a0:'nor this'
          ,  a:'but_should_find_this'
        }
                       )
        .thenTheInstance(
          matched('but_should_find_this')
          ,  foundOneMatch
        );
    })
    it('ignores keys if only end of pattern matches',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!..a')
        .whenGivenInput({
          aa:'should_not_find_this'
          ,  ba:'nor this'
          ,  a:'but_should_find_this'
        }
                       )
        .thenTheInstance(
          matched('but_should_find_this')
          ,  foundOneMatch
        );
    })
    it('ignores partial path matches in array indices',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!..[1]')
        .whenGivenInput({
          array : [0,1,2,3,4,5,6,7,8,9,10,11,12]
        }
                       )
        .thenTheInstance(
          matched(1)
            .withParent([0,1])
          ,  foundOneMatch
        );
    })
    it('can give an array back when just partially done',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('$![5]')
        .whenGivenInput([0,1,2,3,4,5,6,7,8,9,10,11,12])
        .thenTheInstance(
          matched([0,1,2,3,4,5])
          ,  foundOneMatch
        );
    })

    describe('json arrays give correct parent and grandparent', function(){
      it('gives parent and grandparent for every item of an array',  function() {

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.array.*')
          .whenGivenInput({
            array : ['a','b','c']
          }
                         )
          .thenTheInstance(
            matched('a')
              .withParent(['a'])
              .withGrandparent({array:['a']})
            ,  matched('b')
              .withParent(['a', 'b'])
              .withGrandparent({array:['a','b']})
            ,  matched('c')
              .withParent(['a', 'b', 'c'])
              .withGrandparent({array:['a','b','c']})
          );
      })
      it('is correct for array of objects',  function() {

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.array.*')
          .whenGivenInput({
            array : [{'a':1},{'b':2},{'c':3}]
          }
                         )
          .thenTheInstance(
            matched({'a':1})
              .withParent([{'a':1}])
            ,  matched({'b':2})
              .withParent([{'a':1},{'b':2}])
            ,  matched({'c':3})
              .withParent([{'a':1},{'b':2},{'c':3}])
          );
      })
      it('is correct for object in a mixed array',  function() {

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.array.*')
          .whenGivenInput({
            array : [{'a':1},'b',{'c':3}, {}, ['d'], 'e']
          }
                         )
          .thenTheInstance(
            matched({'a':1})
              .withParent([{'a':1}])
          );
      })
      it('has correct parent for string in mixed array',  function() {

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.array.*')
          .whenGivenInput({
            array : [{'a':1},'b',{'c':3}, {}, ['d'], 'e']
          }
                         )
          .thenTheInstance(

            matched('b')
              .withParent([{'a':1},'b'])

          );
      })
      it('has correct parent for second object in mixed array',  function() {

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.array.*')
          .whenGivenInput({
            array : [{'a':1},'b',{'c':3}, {}, ['d'], 'e']
          }
                         )
          .thenTheInstance(

            matched({'c':3})
              .withParent([{'a':1},'b',{'c':3}])

          );
      })

      it('has correct parent for empty object in mixed array',  function() {

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.array.*')
          .whenGivenInput({
            array : [{'a':1},'b',{'c':3}, {}, ['d'], 'e']
          }
                         )
          .thenTheInstance(

            matched({})
              .withParent([{'a':1},'b',{'c':3}, {}])

          );

      })
      it('has correct parent for singleton string array in mixed array',  function() {

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.array.*')
          .whenGivenInput({
            array : [{'a':1},'b',{'c':3}, {}, ['d'], 'e']
          }
                         )
          .thenTheInstance(

            matched(['d'])
              .withParent([{'a':1},'b',{'c':3}, {}, ['d']])

          );
      })
      it('gives correct parent for singleton string array in singleton array',  function() {

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.array.*')
          .whenGivenInput({
            array : [['d']]
          }
                         )
          .thenTheInstance(

            matched(['d'])
              .withParent([['d']])

          );
      })
      it('gives correct parent for last string in a mixed array',  function() {

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.array.*')
          .whenGivenInput({
            array : [{'a':1},'b',{'c':3}, {}, ['d'], 'e']
          }
                         )
          .thenTheInstance(

            matched('e')
              .withParent([{'a':1},'b',{'c':3}, {}, ['d'], 'e'])

          );
      })

      it('gives correct parent for opening object in a mixed array at root of json',  function() {
        // same test as above but without the object wrapper around the array:

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.*')
          .whenGivenInput([{'a':1},'b',{'c':3}, {}, ['d'], 'e'])
          .thenTheInstance(

            matched({'a':1})
              .withParent([{'a':1}])

          );
      })

      it('gives correct parent for string in a mixed array at root of json',  function() {
        // same test as above but without the object wrapper around the array:

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.*')
          .whenGivenInput([{'a':1},'b',{'c':3}, {}, ['d'], 'e'])
          .thenTheInstance(

            matched('b')
              .withParent([{'a':1},'b'])

          );
      })

      it('gives correct parent for second object in a mixed array at root of json',  function() {
        // same test as above but without the object wrapper around the array:

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.*')
          .whenGivenInput([{'a':1},'b',{'c':3}, {}, ['d'], 'e'])
          .thenTheInstance(

            matched({'c':3})
              .withParent([{'a':1},'b',{'c':3}])

          );
      })

      it('gives correct parent for empty object in a mixed array at root of json',  function() {

        // same test as above but without the object wrapper around the array:

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.*')
          .whenGivenInput([{'a':1},'b',{'c':3}, {}, ['d'], 'e'])
          .thenTheInstance(

            matched({})
              .withParent([{'a':1},'b',{'c':3}, {}])

          );

      })

      it('gives correct parent for singleton string array in a mixed array at root of json',  function() {
        // same test as above but without the object wrapper around the array:

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.*')
          .whenGivenInput([{'a':1},'b',{'c':3}, {}, ['d'], 'e'])
          .thenTheInstance(

            matched(['d'])
              .withParent([{'a':1},'b',{'c':3}, {}, ['d']])

          );
      })
      it('gives correct parent for singleton string array in a singleton array at root of json',  function() {
        // non-mixed array, easier version:

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.*')
          .whenGivenInput([['d']])
          .thenTheInstance(

            matched(['d'])
              .withParent([['d']])

          );
      })

      it('gives correct parent for final string in a mixed array at root of json',  function() {
        // same test as above but without the object wrapper around the array:

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.*')
          .whenGivenInput([{'a':1},'b',{'c':3}, {}, ['d'], 'e'])
          .thenTheInstance(

            matched('e')
              .withParent([{'a':1},'b',{'c':3}, {}, ['d'], 'e'])
          );
      })
    });

    it('can detect at multiple depths using double dot',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!..find')
        .whenGivenInput({

          array:[
            {find:'first_find'}
            ,  {padding:{find:'second_find'}, find:'third_find'}
          ]
          ,  find: {
            find:'fourth_find'
          }

        })
        .thenTheInstance(
          matched('first_find').atPath(['array',0,'find'])
          ,   matched('second_find').atPath(['array',1,'padding','find'])
          ,   matched('third_find').atPath(['array',1,'find'])
          ,   matched('fourth_find').atPath(['find','find'])
          ,   matched({find:'fourth_find'}).atPath(['find'])

          ,   foundNMatches(5)
        );
    })
    it('passes ancestors of found object correctly',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!..find')
        .whenGivenInput({

          array:[
            {find:'first_find'}
            ,  {padding:{find:'second_find'}, find:'third_find'}
          ]
          ,  find: {
            find:'fourth_find'
          }

        })
        .thenTheInstance(
          matched('first_find')
            .withParent( {find:'first_find'} )
            .withGrandparent( [{find:'first_find'}] )

          ,   matched('second_find')
            .withParent({find:'second_find'})
            .withGrandparent({padding:{find:'second_find'}})

          ,   matched('third_find')
            .withParent({padding:{find:'second_find'}, find:'third_find'})
            .withGrandparent([
              {find:'first_find'}
              ,  {padding:{find:'second_find'}, find:'third_find'}
            ])
        );
    })

    it('can detect at multiple depths using implied ancestor of root relationship',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('find')
        .whenGivenInput({

          array:[
            {find:'first_find'}
            ,  {padding:{find:'second_find'}, find:'third_find'}
          ]
          ,  find: {
            find:'fourth_find'
          }

        })
        .thenTheInstance(
          matched('first_find').atPath(['array',0,'find'])
          ,   matched('second_find').atPath(['array',1,'padding','find'])
          ,   matched('third_find').atPath(['array',1,'find'])
          ,   matched('fourth_find').atPath(['find','find'])
          ,   matched({find:'fourth_find'}).atPath(['find'])

          ,   foundNMatches(5)
        );
    })

    it('matches nested adjacent selector',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!..[0].colour')
        .whenGivenInput({

          foods: [
            {  name:'aubergine',
               colour:'purple' // match this
            },
            {name:'apple', colour:'red'},
            {name:'nuts', colour:'brown'}
          ],
          non_foods: [
            {  name:'brick',
               colour:'red'    // and this
            },
            {name:'poison', colour:'pink'},
            {name:'broken_glass', colour:'green'}
          ]
        })
        .thenTheInstance
      (   matched('purple')
          ,   matched('red')
          ,   foundNMatches(2)
      );
    })
    it('matches nested selector separated by a single star selector',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('!..foods.*.name')
        .whenGivenInput({

          foods: [
            {name:'aubergine', colour:'purple'},
            {name:'apple', colour:'red'},
            {name:'nuts', colour:'brown'}
          ],
          non_foods: [
            {name:'brick', colour:'red'},
            {name:'poison', colour:'pink'},
            {name:'broken_glass', colour:'green'}
          ]
        })
        .thenTheInstance
      (   matched('aubergine')
          ,   matched('apple')
          ,   matched('nuts')
          ,   foundNMatches(3)
      );
    })
    it('gets all simple objects from an array',  function() {

      // this test is similar to the following one, except it does not use ! in the pattern
      givenAnOboeInstance()
        .andWeAreListeningForNodes('foods.*')
        .whenGivenInput({
          foods: [
            {name:'aubergine'},
            {name:'apple'},
            {name:'nuts'}
          ]
        })
        .thenTheInstance
      (   foundNMatches(3)
          ,   matched({name:'aubergine'})
          ,   matched({name:'apple'})
          ,   matched({name:'nuts'})
      );
    })

    it('gets same object repeatedly using css4 syntax',  function() {

      givenAnOboeInstance()
        .andWeAreListeningForNodes('$foods.*')
        .whenGivenInput({
          foods: [
            {name:'aubergine'},
            {name:'apple'},
            {name:'nuts'}
          ]
        })
      // essentially, the parser should have been called three times with the same object, but each time
      // an additional item should have been added
        .thenTheInstance
      (   foundNMatches(3)
          ,   matched([{name:'aubergine'}])
          ,   matched([{name:'aubergine'},{name:'apple'}])
          ,   matched([{name:'aubergine'},{name:'apple'},{name:'nuts'}])
      );
    })

    it('matches nested selector separated by double dot',  function() {

      givenAnOboeInstance()
      // we just want the French names of foods:
        .andWeAreListeningForNodes('!..foods..fr')
        .whenGivenInput({

          foods: [
            {name:{en:'aubergine', fr:'aubergine'}, colour:'purple'},
            {name:{en:'apple', fr:'pomme'}, colour:'red'},
            {name:{en:'nuts', fr:'noix'}, colour:'brown'}
          ],
          non_foods: [
            {name:{en:'brick'}, colour:'red'},
            {name:{en:'poison'}, colour:'pink'},
            {name:{en:'broken_glass'}, colour:'green'}
          ]
        })
        .thenTheInstance
      (   matched('aubergine')
          ,   matched('pomme')
          ,   matched('noix')
          ,   foundNMatches(3)
      );
    })

    describe('duck types', function(){
      // only smoke-testing duck types here, tested thoroughly in jsonpath unit tests

      it('can detect',  function() {

        givenAnOboeInstance()
        // we want the bi-lingual objects
          .andWeAreListeningForNodes('{en fr}')
          .whenGivenInput({

            foods: [
              {name:{en:'aubergine',  fr:'aubergine' }, colour:'purple'},
              {name:{en:'apple',      fr:'pomme'     }, colour:'red'   },
              {name:{en:'nuts',       fr:'noix'      }, colour:'brown' }
            ],
            non_foods: [
              {name:{en:'brick'       }, colour:'red'   },
              {name:{en:'poison'      }, colour:'pink'  },
              {name:{en:'broken_glass'}, colour:'green' }
            ]
          })
          .thenTheInstance
        (   matched({en:'aubergine',  fr:'aubergine' })
            ,   matched({en:'apple',      fr:'pomme'     })
            ,   matched({en:'nuts',       fr:'noix'      })
            ,   foundNMatches(3)
        );
      })
      it('can detect by matches with additional keys',  function() {

        givenAnOboeInstance()
        // we want the bi-lingual English and German words, but we still want the ones that have
        // French as well
          .andWeAreListeningForNodes('{en de}')
          .whenGivenInput({

            foods: [
              {name:{en:'aubergine',  fr:'aubergine',   de: 'aubergine' }, colour:'purple'},
              {name:{en:'apple',      fr:'pomme',       de: 'apfel'     }, colour:'red'   },
              {name:{en:'nuts',                         de: 'eier'      }, colour:'brown' }
            ],
            non_foods: [
              {name:{en:'brick'       }, colour:'red'  },
              {name:{en:'poison'      }, colour:'pink' },
              {name:{en:'broken_glass'}, colour:'green'}
            ]
          })
          .thenTheInstance
        (   matched({en:'aubergine',  fr:'aubergine',   de:'aubergine' })
            ,   matched({en:'apple',      fr:'pomme',       de: 'apfel'    })
            ,   matched({en:'nuts',                         de: 'eier'     })
            ,   foundNMatches(3)
        );
      })

    })


    describe('error cases:', function() {

      describe('errors on unquoted keys', function(){
        var invalidJson = '{invalid:"json"}';  // key not quoted, invalid json

        // there have been bugs where this passes when passed in one char at a time
        // but not when given as a single call

        it('fed in as a lump',  function() {
          givenAnOboeInstance()
            .andWeAreExpectingSomeErrors()
            .whenGivenInputPart(invalidJson)
            .thenTheInstance
          (   calledCallbackOnce
              ,   wasPassedAnErrorObject
          );
        })
        it('fed a char at a time',  function() {

          givenAnOboeInstance()
            .andWeAreExpectingSomeErrors()
            .whenGivenInputOneCharAtATime(invalidJson)
            .thenTheInstance
          (   calledCallbackOnce
              ,   wasPassedAnErrorObject
          );
        })
      });

      describe('errors on malformed json', function(){
        var malformedJson = '{{'; // invalid!

        it('works as a lump',  function() {

          givenAnOboeInstance()
            .andWeAreExpectingSomeErrors()
            .whenGivenInputPart(malformedJson)
            .thenTheInstance
          (   calledCallbackOnce
              ,   wasPassedAnErrorObject
          );
        })
        it('works as chars',  function() {

          givenAnOboeInstance()
            .andWeAreExpectingSomeErrors()
            .whenGivenInputOneCharAtATime(malformedJson) // invalid!
            .thenTheInstance
          (   calledCallbackOnce
              ,   wasPassedAnErrorObject
          );
        })
      });

      it('detects error when stream halts early between children of root',  function() {

        givenAnOboeInstance()
          .andWeAreExpectingSomeErrors()
          .whenGivenInputPart('[[1,2,3],[4,5')
          .whenInputFinishes()
          .thenTheInstance
        (   calledCallbackOnce
            ,   wasPassedAnErrorObject
        );
      })
      it('detects error when stream halts early between children of root fed in a char at a time',  function() {

        givenAnOboeInstance()
          .andWeAreExpectingSomeErrors()
          .whenGivenInputOneCharAtATime('[[1,2,3],[4,5')
          .whenInputFinishes()
          .thenTheInstance
        (   calledCallbackOnce
            ,   wasPassedAnErrorObject
        );
      })
      it('detects error when stream halts early between children of root',  function() {
        // currently failing: clarinet is not detecting the error
        givenAnOboeInstance()
          .andWeAreExpectingSomeErrors()
          .whenGivenInputPart('[[1,2,3],')
          .whenInputFinishes()
          .thenTheInstance
        (   calledCallbackOnce
            ,   wasPassedAnErrorObject
        );
      })
      it('detects error when stream halts early inside mid-tree node',  function() {

        givenAnOboeInstance()
          .andWeAreExpectingSomeErrors()
          .whenGivenInputPart('[[1,2,3')
          .whenInputFinishes()
          .thenTheInstance
        (   calledCallbackOnce
            ,   wasPassedAnErrorObject
        );
      })
      it('calls error listener if an error is thrown in the callback',  function() {

        spyOn(globalContext, 'setTimeout');

        givenAnOboeInstance()
          .andWeHaveAFaultyCallbackListeningFor('!') // just want the root object
          .whenGivenInputPart('{}') // valid json, should provide callback
          .thenTheInstance
        ({
          testAgainst: function() {
            // console.log(globalContext.setTimeout.calls.mostRecent())
            expect(globalContext.setTimeout.calls.argsFor(0)[0]).toThrow()
          }
        });
      })
    });

    describe('aborting a request', function(){

      it('does not throw an error', function(){

        expect( function(){

          givenAnOboeInstance()
            .andWeAreListeningForPaths('*')
            .whenGivenInputPart('[1')
            .andWeAbortTheRequest();

        }).not.toThrow();
      });

      it('can abort once some data has been found in response',  function() {

        // we should be able to abort even when given all the content at once

        var asserter = givenAnOboeInstance();
        asserter.andWeAreListeningForNodes('![5]', function(){
          asserter.andWeAbortTheRequest();
        })
          .whenGivenInput([0,1,2,3,4,5,6,7,8,9])
          .thenTheInstance(
            // because the request was aborted on index array 5, we got 6 numbers (inc zero)
            // not the whole ten.
            hasRootJson([0,1,2,3,4,5])
          );
      })
    });

    describe('dropping nodes', function() {

      it('drops from an array leaving holes', function() {

        function isEven(n) {
          return (n % 2) == 0;
        }

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.*', function( number ) {
            if( isEven(number) ) {
              return oboe.drop;
            }
          })
          .whenGivenInput([1,2,3,4,5,6,7])
          .thenTheInstance(
            hasRootJson([1, ,3, ,5, ,7]) // note holes in array!
          );
      })

      it('can drop from an object', function() {

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.*', function notDrinking( course ) {
            if( course == 'wine' ) {
              return oboe.drop;
            }
          })
          .whenGivenInput({
            starter:'soup',
            main:'fish',
            desert:'fresh cheesecake',
            drink:'wine'
          })
          .thenTheInstance(
            hasRootJson({
              starter:'soup',
              main:'fish',
              desert:'fresh cheesecake'
            })
          );
      });

      it('can drop from an object using short-form', function() {

        givenAnOboeInstance()
          .andWeAreListeningForNodes('starter', oboe.drop)
          .whenGivenInput({
            starter:'soup',
            main:'fish',
            desert:'fresh cheesecake',
            drink:'wine'
          })
          .thenTheInstance(
            hasRootJson({
              main:'fish',
              desert:'fresh cheesecake',
              drink:'wine'
            })
          );
      })

    });

    describe('swapping out nodes', function() {

      it('can selectively drop objects from an array', function() {

        givenAnOboeInstance()
          .andWeAreListeningForNodes('!.*', function( obj ) {
            if( obj.nullfily ) {
              return null;
            }
          })
          .whenGivenInput([
            {nullfily:true},
            {keep:true},
            {nullfily:true}
          ])
          .thenTheInstance(
            // because the request was aborted on index array 5, we got 6 numbers (inc zero)
            // not the whole ten.
            hasRootJson([
              null,
              {keep:true},
              null
            ])
          );
      });

      it('can replace objects found in an object', function() {

        givenAnOboeInstance()
          .andWeAreListeningForNodes('{replace}', function( obj ) {
            return {replaced:true};
          })
          .whenGivenInput({
            a: {replace: true},
            b: {keep: true},
            c: {replace: true}
          })
          .thenTheInstance(
            // because the request was aborted on index array 5, we got 6 numbers (inc zero)
            // not the whole ten.
            hasRootJson({
              a: {replaced: true},
              b: {keep: true},
              c: {replaced: true}
            })
          );
      });

      /*
       it('can replace the root', function() {

       // replacing the root currently isn't supported

       givenAnOboeInstance()
       .andWeAreListeningForNodes('!', function( obj ) {
       return 'different_root';
       })
       .whenGivenInput({
       a: 'a',
       b: 'b'
       })
       .thenTheInstance(
       // because the request was aborted on index array 5, we got 6 numbers (inc zero)
       // not the whole ten.
       hasRootJson('different_root')
       );
       });
       */

      it('can transform scalar values', function() {

        function isEven(n) {
          return (n % 2) == 0;
        }

        givenAnOboeInstance()
          .andWeAreListeningForNodes('*', function( number ) {

            if (isEven(number)) {
              return number * 2;
            }
          })
          .whenGivenInput([1,2,3,4,5])
          .thenTheInstance(
            // because the request was aborted on index array 5, we got 6 numbers (inc zero)
            // not the whole ten.
            hasRootJson([1,4,3,8,5])
          );
      });
    });

  });
})(typeof Platform == 'undefined'? require('../libs/platform.js') : Platform)
