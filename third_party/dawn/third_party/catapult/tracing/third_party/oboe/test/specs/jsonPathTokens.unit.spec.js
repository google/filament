jsonPathSyntax(function (pathNodeDesc, doubleDotDesc, dotDesc, bangDesc, emptyDesc) {

  describe('json path token parser', function () {

    describe('field list', function () {

      it('parses zero-length list', function () {
        expect(pathNodeDesc('{}')).toContainMatches({fieldList:''});
      });

      it('parses single field', function () {
        expect(pathNodeDesc('{a}')).toContainMatches({fieldList:'a'      });
      });

      it('parses two fields', function () {
        expect(pathNodeDesc('{r2 d2}')).toContainMatches({fieldList:'r2 d2'  });
      });

      it('parses numeric fields', function () {
        expect(pathNodeDesc('{1 2}')).toContainMatches({fieldList:'1 2'    });
      });

      it('ignores whitespace', function () {
        expect(pathNodeDesc('{a  b}')).toContainMatches({fieldList:'a  b'   });
      });

      it('ignores more whitespace', function () {
        expect(pathNodeDesc('{a   b}')).toContainMatches({fieldList:'a   b'  });
      });

      it('parses 3 fields', function () {
        expect(pathNodeDesc('{a  b  c}')).toContainMatches({fieldList:'a  b  c'});
      });

      it('needs a closing brace', function () {
        expect(pathNodeDesc('{a')).toNotMatch();
      });
    });

    describe('object notation', function () {

      it('parses a name', function () {
        expect(pathNodeDesc('aaa')).toContainMatches({name:'aaa'});
      });
      it('parses a name containing a hyphen', function () {
        expect(pathNodeDesc('x-security-token')).toContainMatches({name:'x-security-token'});
      });
      it('parses a name containing an underscore', function () {
        expect(pathNodeDesc('x_security_token')).toContainMatches({name:'x_security_token'});
      });
      it('parses a name and recognises the capturing flag', function () {
        expect(pathNodeDesc('$aaa')).toContainMatches({name:'aaa', capturing:true});
      });
      it('parses a name and field list', function () {
        expect(pathNodeDesc('aaa{a b c}')).toContainMatches({name:'aaa', fieldList:'a b c'});
      });
      it('parses a name with field list and capturing flag', function () {
        expect(pathNodeDesc('$aaa{a b c}')).toContainMatches({name:'aaa', capturing:true, fieldList:'a b c'});
      });
      it('wont parse unless the name is at the start', function () {
        expect(pathNodeDesc('.a')).toNotMatch();
      });
      it('parses only the first name', function () {
        expect(pathNodeDesc('a.b')).toContainMatches({name:'a'});
      });
      it('ignores invalid', function () {
        expect(pathNodeDesc('$$a')).toNotMatch();
      });
      it('needs field list to close', function () {
        expect(pathNodeDesc('.a{')).toNotMatch();
      });
    });

    describe('named array notation', function () {

      it('parses quoted', function () {
        expect(pathNodeDesc('["foo"]')).toContainMatches({name:'foo'});
      });
      it('parses quoted and capturing', function () {
        expect(pathNodeDesc('$["foo"]')).toContainMatches({name:'foo', capturing:true});
      });
      it('parses quoted with field list', function () {
        expect(pathNodeDesc('["foo"]{a b c}')).toContainMatches({name:'foo', fieldList:'a b c'});
      });
      it('parses quoted with field list and capturing', function () {
        expect(pathNodeDesc('$["foo"]{a b c}')).toContainMatches({name:'foo', capturing:true, fieldList:'a b c'});
      });
      it('ignores without a path name', function () {
        expect(pathNodeDesc('[]')).toNotMatch();
      });
      it('fails with too many quotes', function () {
        expect(pathNodeDesc('["""]')).toNotMatch();
      });
      it('parses unquoted', function () {
        expect(pathNodeDesc('[foo]')).toNotMatch();
      });
      it('ignores unnamed because of an empty string', function () {
        expect(pathNodeDesc('[""]')).toNotMatch();
      });
      it('parses first token only', function () {
        expect(pathNodeDesc('["foo"]["bar"]')).toContainMatches({name:'foo'});
      });
      it('allows dot char inside quotes that would otherwise have a special meaning', function () {
        expect(pathNodeDesc('[".foo"]')).toContainMatches({name:'.foo'});
      });
      it('allows star char inside quotes that would otherwise have a special meaning', function () {
        expect(pathNodeDesc('["*"]')).toContainMatches({name:'*'});
      });
      it('allows dollar char inside quotes that would otherwise have a special meaning', function () {
        expect(pathNodeDesc('["$"]')).toContainMatches({name:'$'});
      });
      it('allows underscore in quotes', function () {
        expect(pathNodeDesc('["foo_bar"]')).toContainMatches({name:'foo_bar'});
      });
      it('allows non-ASCII chars in quotes', function () {
        expect(pathNodeDesc('["你好"]')).toContainMatches({name:'你好'});
      });
    });

    describe('numbered array notation', function () {

      it('parses single digit', function () {
        expect(pathNodeDesc('[2]')).toContainMatches({name:'2'});
      });
      it('parses multiple digits', function () {
        expect(pathNodeDesc('[123]')).toContainMatches({name:'123'});
      });
      it('parses with capture flag', function () {
        expect(pathNodeDesc('$[2]')).toContainMatches({name:'2', capturing:true});
      });
      it('parses with field list', function () {
        expect(pathNodeDesc('[2]{a b c}')).toContainMatches({name:'2', fieldList:'a b c'});
      });
      it('parses with field list and capture', function () {
        expect(pathNodeDesc('$[2]{a b c}')).toContainMatches({name:'2', capturing:true, fieldList:'a b c'});
      });
      it('ignores without a name', function () {
        expect(pathNodeDesc('[]')).toNotMatch();
      });
      it('ignores empty string as a name', function () {
        expect(pathNodeDesc('[""]')).toNotMatch();
      });
    });

    beforeEach(function () {
      jasmine.addMatchers({
        toContainMatches: function() {
          return {
            compare: function (actual, expectedResults) {
              var result = {};
              var foundResults = actual;

              if (expectedResults && !foundResults) {
                if (!expectedResults.capturing && !expectedResults.name && !expectedResults.fieldList) {
                  result.pass = true; // wasn't expecting to find anything
                }

                result.message = 'did not find anything';
                result.pass = false;

              } else if ((!!foundResults[1]) != (!!expectedResults.capturing)) {
                result.pass = false;
              } else if ((foundResults[2]) != (expectedResults.name || '')) {
                result.pass = false;
              } else if ((foundResults[3] || '') != (expectedResults.fieldList || '')) {
                result.pass = false;
              } else {
                result.pass = true;
              }

              return result;
            }
          };
        },
        toNotMatch: function () {
          return {
            compare: function(actual) {
              var foundResults = this.actual;

              return {
                pass: !foundResults
              };
            }
          };
        }
      });
    });

  });
});
