var assert = require('assert'),
    html = require('../../lib/common/html'),
    FormattingElementList = require('../../lib/tree_construction/formatting_element_list'),
    TestUtils = require('../test_utils');

//Aliases
var $ = html.TAG_NAMES,
    NS = html.NAMESPACES;

TestUtils.generateTestsForEachTreeAdapter(module.exports, function (_test, treeAdapter) {
    _test['Insert marker'] = function () {
        var list = new FormattingElementList(treeAdapter);

        list.insertMarker();
        assert.strictEqual(list.length, 1);
        assert.strictEqual(list.entries[0].type, FormattingElementList.MARKER_ENTRY);

        list.insertMarker();
        assert.strictEqual(list.length, 2);
        assert.strictEqual(list.entries[1].type, FormattingElementList.MARKER_ENTRY);
    };

    _test['Push element'] = function () {
        var list = new FormattingElementList(treeAdapter),
            element1Token = 'token1',
            element2Token = 'token2',
            element1 = treeAdapter.createElement($.DIV, NS.HTML, []),
            element2 = treeAdapter.createElement($.P, NS.HTML, []);

        list.pushElement(element1, element1Token);
        assert.strictEqual(list.length, 1);
        assert.strictEqual(list.entries[0].type, FormattingElementList.ELEMENT_ENTRY);
        assert.strictEqual(list.entries[0].element, element1);
        assert.strictEqual(list.entries[0].token, element1Token);

        list.pushElement(element2, element2Token);
        assert.strictEqual(list.length, 2);
        assert.strictEqual(list.entries[1].type, FormattingElementList.ELEMENT_ENTRY);
        assert.strictEqual(list.entries[1].element, element2);
        assert.strictEqual(list.entries[1].token, element2Token);
    };

    _test['Insert element after bookmark'] = function () {
        var list = new FormattingElementList(treeAdapter),
            token = 'token1',
            element1 = treeAdapter.createElement($.DIV, NS.HTML, []),
            element2 = treeAdapter.createElement($.P, NS.HTML, []),
            element3 = treeAdapter.createElement($.SPAN, NS.HTML, []),
            element4 = treeAdapter.createElement($.TITLE, NS.HTML, []);

        list.pushElement(element1, token);
        list.bookmark = list.entries[0];

        list.pushElement(element2, token);
        list.pushElement(element3, token);

        list.insertElementAfterBookmark(element4, token);

        assert.strictEqual(list.length, 4);
        assert.strictEqual(list.entries[1].element, element4);
    };

    _test['Push element - Noah Ark condition'] = function () {
        var list = new FormattingElementList(treeAdapter),
            token1 = 'token1',
            token2 = 'token2',
            token3 = 'token3',
            token4 = 'token4',
            token5 = 'token5',
            token6 = 'token6',
            element1 = treeAdapter.createElement($.DIV, NS.HTML, [
                {name: 'attr1', value: 'val1'},
                {name: 'attr2', value: 'val2'}
            ]),
            element2 = treeAdapter.createElement($.DIV, NS.HTML, [
                {name: 'attr1', value: 'val1'},
                {name: 'attr2', value: 'someOtherValue'}
            ]);

        list.pushElement(element1, token1);
        list.pushElement(element1, token2);
        list.pushElement(element2, token3);
        list.pushElement(element1, token4);

        assert.strictEqual(list.length, 4);
        assert.strictEqual(list.entries[0].token, token1);
        assert.strictEqual(list.entries[1].token, token2);
        assert.strictEqual(list.entries[2].token, token3);
        assert.strictEqual(list.entries[3].token, token4);

        list.pushElement(element1, token5);

        assert.strictEqual(list.length, 4);
        assert.strictEqual(list.entries[0].token, token2);
        assert.strictEqual(list.entries[1].token, token3);
        assert.strictEqual(list.entries[2].token, token4);
        assert.strictEqual(list.entries[3].token, token5);

        list.insertMarker();
        list.pushElement(element1, token6);

        assert.strictEqual(list.length, 6);
        assert.strictEqual(list.entries[0].token, token2);
        assert.strictEqual(list.entries[1].token, token3);
        assert.strictEqual(list.entries[2].token, token4);
        assert.strictEqual(list.entries[3].token, token5);
        assert.strictEqual(list.entries[4].type, FormattingElementList.MARKER_ENTRY);
        assert.strictEqual(list.entries[5].token, token6);
    };

    _test['Clear to the last marker'] = function () {
        var list = new FormattingElementList(treeAdapter),
            token = 'token',
            element1 = treeAdapter.createElement($.DIV, NS.HTML, [
                {name: 'attr1', value: 'val1'},
                {name: 'attr2', value: 'val2'}
            ]),
            element2 = treeAdapter.createElement($.DIV, NS.HTML, [
                {name: 'attr1', value: 'val1'},
                {name: 'attr2', value: 'someOtherValue'}
            ]);

        list.pushElement(element1, token);
        list.pushElement(element2, token);
        list.insertMarker();
        list.pushElement(element1, token);
        list.pushElement(element1, token);
        list.pushElement(element2, token);

        list.clearToLastMarker();

        assert.strictEqual(list.length, 2);
    };

    _test['Remove entry'] = function () {
        var list = new FormattingElementList(treeAdapter),
            token = 'token',
            element1 = treeAdapter.createElement($.DIV, NS.HTML, [
                {name: 'attr1', value: 'val1'},
                {name: 'attr2', value: 'val2'}
            ]),
            element2 = treeAdapter.createElement($.DIV, NS.HTML, [
                {name: 'attr1', value: 'val1'},
                {name: 'attr2', value: 'someOtherValue'}
            ]);

        list.pushElement(element1, token);
        list.pushElement(element2, token);
        list.pushElement(element2, token);

        list.removeEntry(list.entries[0]);

        assert.strictEqual(list.length, 2);

        for (var i = list.length - 1; i >= 0; i--)
            assert.notStrictEqual(list.entries[i].element, element1);
    };

    _test['Get entry in scope with given tag name'] = function () {
        var list = new FormattingElementList(treeAdapter),
            token = 'token',
            element = treeAdapter.createElement($.DIV, NS.HTML, []);

        assert.ok(!list.getElementEntryInScopeWithTagName($.DIV));

        list.pushElement(element, token);
        list.pushElement(element, token);
        assert.strictEqual(list.getElementEntryInScopeWithTagName($.DIV), list.entries[1]);

        list.insertMarker();
        assert.ok(!list.getElementEntryInScopeWithTagName($.DIV));

        list.pushElement(element, token);
        assert.strictEqual(list.getElementEntryInScopeWithTagName($.DIV), list.entries[3]);
    };

    _test['Get element entry'] = function () {
        var list = new FormattingElementList(treeAdapter),
            token = 'token',
            element1 = treeAdapter.createElement($.DIV, NS.HTML, []),
            element2 = treeAdapter.createElement($.A, NS.HTML, []);

        list.pushElement(element2, token);
        list.pushElement(element1, token);
        list.pushElement(element2, token);
        list.insertMarker();

        var entry = list.getElementEntry(element1);
        assert.strictEqual(entry.type, FormattingElementList.ELEMENT_ENTRY);
        assert.strictEqual(entry.token, token);
        assert.strictEqual(entry.element, element1);
    };
});
