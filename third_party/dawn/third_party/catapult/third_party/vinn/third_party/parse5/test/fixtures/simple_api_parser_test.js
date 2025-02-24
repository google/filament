var assert = require('assert'),
    path = require('path'),
    SimpleApiParser = require('../../index').SimpleApiParser,
    TestUtils = require('../test_utils');

function getFullTestName(test, idx) {
    return ['SimpleApiParser - ', idx, '.', test.name].join('');
}

function sanitizeForComparison(str) {
    return TestUtils.removeNewLines(str)
        .replace(/\s/g, '')
        .replace(/'/g, '"')
        .toLowerCase();
}


function createTest(html, expected, options) {
    return function () {
        //NOTE: the idea of the test is to serialize back given HTML using SimpleApiParser handlers
        var actual = '',
            parser = new SimpleApiParser({
                doctype: function (name, publicId, systemId) {
                    actual += '<!DOCTYPE ' + name;

                    if (publicId !== null)
                        actual += ' PUBLIC "' + publicId + '"';

                    else if (systemId !== null)
                        actual += ' SYSTEM';

                    if (systemId !== null)
                        actual += ' "' + systemId + '"';


                    actual += '>';
                },

                startTag: function (tagName, attrs, selfClosing) {
                    actual += '<' + tagName;

                    if (attrs.length) {
                        for (var i = 0; i < attrs.length; i++)
                            actual += ' ' + attrs[i].name + '="' + attrs[i].value + '"';
                    }

                    actual += selfClosing ? '/>' : '>';
                },

                endTag: function (tagName) {
                    actual += '</' + tagName + '>';
                },

                text: function (text) {
                    actual += text;
                },

                comment: function (text) {
                    actual += '<!--' + text + '-->';
                }
            }, options);

        parser.parse(html);

        expected = sanitizeForComparison(expected);
        actual = sanitizeForComparison(actual);

        //NOTE: use ok assertion, so output will not be polluted by the whole content of the strings
        assert.ok(actual === expected, TestUtils.getStringDiffMsg(actual, expected));
    };
}

TestUtils.loadSerializationTestData(path.join(__dirname, '../data/simple_api_parsing'))
    .concat([
        {
            name: 'Options - decodeHtmlEntities (text)',
            src: '<div>&amp;&copy;</div>',
            expected: '<div>&amp;&copy;</div>',
            options: {
                decodeHtmlEntities: false
            }
        },

        {
            name: 'Options - decodeHtmlEntities (attributes)',
            src: '<a href = "&amp;test&lt;" &copy;>Yo</a>',
            expected: '<a href = "&amp;test&lt;" &copy;="">Yo</a>',
            options: {
                decodeHtmlEntities: false
            }
        }
    ])
    .forEach(function (test, idx) {
        var testName = getFullTestName(test, idx);

        exports[testName] = createTest(test.src, test.expected, test.options);

        exports['Options - locationInfo - ' + testName] = function () {
            //NOTE: we've already tested the correctness of the location info with the Tokenizer tests.
            //So here we just check that SimpleApiParser provides this info in the handlers.
            var handlers = ['doctype', 'startTag', 'endTag', 'text', 'comment'].reduce(function (handlers, key) {
                    handlers[key] = function () {
                        var locationInfo = arguments[arguments.length - 1];

                        assert.strictEqual(typeof locationInfo.start, 'number');
                        assert.strictEqual(typeof locationInfo.end, 'number');
                    };
                    return handlers;
                }, {}),
                parser = new SimpleApiParser(handlers, {locationInfo: true});

            parser.parse(test.src);
        };
    });



