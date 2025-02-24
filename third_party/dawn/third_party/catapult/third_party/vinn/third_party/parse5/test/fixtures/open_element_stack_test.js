var assert = require('assert'),
    HTML = require('../../lib/common/html'),
    OpenElementStack = require('../../lib/tree_construction/open_element_stack'),
    TestUtils = require('../test_utils');

//Aliases
var $ = HTML.TAG_NAMES,
    NS = HTML.NAMESPACES;

TestUtils.generateTestsForEachTreeAdapter(module.exports, function (_test, treeAdapter) {
    _test['Push element'] = function () {
        var document = treeAdapter.createDocument(),
            element1 = treeAdapter.createElement('#element1', 'namespace1', []),
            element2 = treeAdapter.createElement('#element2', 'namespace2', []),
            stack = new OpenElementStack(document, treeAdapter);

        assert.strictEqual(stack.current, document);
        assert.strictEqual(stack.stackTop, -1);

        stack.push(element1);
        assert.strictEqual(stack.current, element1);
        assert.strictEqual(stack.currentTagName, treeAdapter.getTagName(element1));
        assert.strictEqual(stack.stackTop, 0);

        stack.push(element2);
        assert.strictEqual(stack.current, element2);
        assert.strictEqual(stack.currentTagName, treeAdapter.getTagName(element2));
        assert.strictEqual(stack.stackTop, 1);
    };

    _test['Pop element'] = function () {
        var element = treeAdapter.createElement('#element', 'namespace1', []),
            stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(element);
        stack.push('#element2');
        stack.pop();
        assert.strictEqual(stack.current, element);
        assert.strictEqual(stack.currentTagName, treeAdapter.getTagName(element));
        assert.strictEqual(stack.stackTop, 0);

        stack.pop();
        assert.ok(!stack.current);
        assert.ok(!stack.currentTagName);
        assert.strictEqual(stack.stackTop, -1);
    };

    _test['Replace element'] = function () {
        var element = treeAdapter.createElement('#element', 'namespace', []),
            newElement = treeAdapter.createElement('#newElement', 'newElementNamespace', []),
            stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push('#element2');
        stack.push(element);
        stack.replace(element, newElement);
        assert.strictEqual(stack.current, newElement);
        assert.strictEqual(stack.currentTagName, treeAdapter.getTagName(newElement));
        assert.strictEqual(stack.stackTop, 1);
    };

    _test['Insert element after element'] = function () {
        var element1 = treeAdapter.createElement('#element1', 'namespace1', []),
            element2 = treeAdapter.createElement('#element2', 'namespace2', []),
            element3 = treeAdapter.createElement('#element3', 'namespace3', []),
            stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(element1);
        stack.push(element2);
        stack.insertAfter(element1, element3);
        assert.strictEqual(stack.stackTop, 2);
        assert.strictEqual(stack.items[1], element3);

        stack.insertAfter(element2, element1);
        assert.strictEqual(stack.stackTop, 3);
        assert.strictEqual(stack.current, element1);
        assert.strictEqual(stack.currentTagName, treeAdapter.getTagName(element1));
    };

    _test['Pop elements until popped with given tagName'] = function () {
        var element1 = treeAdapter.createElement('#element1', '', []),
            element2 = treeAdapter.createElement('#element2', '', []),
            stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(element2);
        stack.push(element2);
        stack.push(element2);
        stack.push(element2);
        stack.popUntilTagNamePopped(treeAdapter.getTagName(element1));
        assert.ok(!stack.current);
        assert.strictEqual(stack.stackTop, -1);

        stack.push(element2);
        stack.push(element1);
        stack.push(element2);
        stack.popUntilTagNamePopped(treeAdapter.getTagName(element1));
        assert.strictEqual(stack.current, element2);
        assert.strictEqual(stack.stackTop, 0);
    };

    _test['Pop elements until given element popped'] = function () {
        var element1 = treeAdapter.createElement('#element1', '', []),
            element2 = treeAdapter.createElement('#element2', '', []),
            stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(element2);
        stack.push(element2);
        stack.push(element2);
        stack.push(element2);
        stack.popUntilElementPopped(element1);
        assert.ok(!stack.current);
        assert.strictEqual(stack.stackTop, -1);

        stack.push(element2);
        stack.push(element1);
        stack.push(element2);
        stack.popUntilElementPopped(element1);
        assert.strictEqual(stack.current, element2);
        assert.strictEqual(stack.stackTop, 0);
    };

    _test['Pop elements until numbered header popped'] = function () {
        var element1 = treeAdapter.createElement($.H3, '', []),
            element2 = treeAdapter.createElement('#element2', '', []),
            stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(element2);
        stack.push(element2);
        stack.push(element2);
        stack.push(element2);
        stack.popUntilNumberedHeaderPopped();
        assert.ok(!stack.current);
        assert.strictEqual(stack.stackTop, -1);

        stack.push(element2);
        stack.push(element1);
        stack.push(element2);
        stack.popUntilNumberedHeaderPopped();
        assert.strictEqual(stack.current, element2);
        assert.strictEqual(stack.stackTop, 0);
    };

    _test['Pop all up to <html> element'] = function () {
        var htmlElement = treeAdapter.createElement($.HTML, NS.HTML, []),
            stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(htmlElement);
        stack.push('#element1');
        stack.push('#element2');

        stack.popAllUpToHtmlElement();
        assert.strictEqual(stack.current, htmlElement);
    };

    _test['Clear back to a table context'] = function () {
        var htmlElement = treeAdapter.createElement($.HTML, '', []),
            tableElement = treeAdapter.createElement($.TABLE, '', []),
            divElement = treeAdapter.createElement($.DIV, '', []),
            stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(htmlElement);
        stack.push(divElement);
        stack.push(divElement);
        stack.push(divElement);
        stack.clearBackToTableContext();
        assert.strictEqual(stack.current, htmlElement);
        assert.strictEqual(stack.stackTop, 0);

        stack.push(divElement);
        stack.push(tableElement);
        stack.push(divElement);
        stack.push(divElement);
        stack.clearBackToTableContext();
        assert.strictEqual(stack.current, tableElement);
        assert.strictEqual(stack.stackTop, 2);
    };

    _test['Clear back to a table body context'] = function () {
        var htmlElement = treeAdapter.createElement($.HTML, '', []),
            theadElement = treeAdapter.createElement($.THEAD, '', []),
            divElement = treeAdapter.createElement($.DIV, '', []),
            stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(htmlElement);
        stack.push(divElement);
        stack.push(divElement);
        stack.push(divElement);
        stack.clearBackToTableBodyContext();
        assert.strictEqual(stack.current, htmlElement);
        assert.strictEqual(stack.stackTop, 0);

        stack.push(divElement);
        stack.push(theadElement);
        stack.push(divElement);
        stack.push(divElement);
        stack.clearBackToTableBodyContext();
        assert.strictEqual(stack.current, theadElement);
        assert.strictEqual(stack.stackTop, 2);
    };

    _test['Clear back to a table row context'] = function () {
        var htmlElement = treeAdapter.createElement($.HTML, '', []),
            trElement = treeAdapter.createElement($.TR, '', []),
            divElement = treeAdapter.createElement($.DIV, '', []),
            stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(htmlElement);
        stack.push(divElement);
        stack.push(divElement);
        stack.push(divElement);
        stack.clearBackToTableRowContext();
        assert.strictEqual(stack.current, htmlElement);
        assert.strictEqual(stack.stackTop, 0);

        stack.push(divElement);
        stack.push(trElement);
        stack.push(divElement);
        stack.push(divElement);
        stack.clearBackToTableRowContext();
        assert.strictEqual(stack.current, trElement);
        assert.strictEqual(stack.stackTop, 2);
    };

    _test['Remove element'] = function () {
        var element = treeAdapter.createElement('#element', '', []),
            stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(element);
        stack.push(treeAdapter.createElement('element1', '', []));
        stack.push(treeAdapter.createElement('element2', '', []));

        stack.remove(element);

        assert.strictEqual(stack.stackTop, 1);

        for (var i = stack.stackTop; i >= 0; i--)
            assert.notStrictEqual(stack.items[i], element);
    };

    _test['Try peek properly nested <body> element'] = function () {
        var bodyElement = treeAdapter.createElement($.BODY, '', []),
            stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(treeAdapter.createElement($.HTML, '', []));
        stack.push(bodyElement);
        stack.push(treeAdapter.createElement($.DIV, '', []));
        assert.strictEqual(stack.tryPeekProperlyNestedBodyElement(), bodyElement);

        stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);
        stack.push(treeAdapter.createElement($.HTML, '', []));
        assert.ok(!stack.tryPeekProperlyNestedBodyElement());
    };

    _test['Is root <html> element current'] = function () {
        var stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(treeAdapter.createElement($.HTML, '', []));
        assert.ok(stack.isRootHtmlElementCurrent());

        stack.push(treeAdapter.createElement($.DIV, '', []));
        assert.ok(!stack.isRootHtmlElementCurrent());
    };

    _test['Get common ancestor'] = function () {
        var stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter),
            element = treeAdapter.createElement('#element', '', []),
            ancestor = treeAdapter.createElement('#ancestor', '', []);

        stack.push(treeAdapter.createElement('#someElement', '', []));
        assert.ok(!stack.getCommonAncestor(element));

        stack.pop();
        assert.ok(!stack.getCommonAncestor(element));

        stack.push(element);
        assert.ok(!stack.getCommonAncestor(element));

        stack.push(treeAdapter.createElement('#someElement', '', []));
        stack.push(ancestor);
        stack.push(element);
        assert.strictEqual(stack.getCommonAncestor(element), ancestor);
    };

    _test['Contains element'] = function () {
        var stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter),
            element = treeAdapter.createElement('#element', '', []);

        stack.push(treeAdapter.createElement('#someElement', '', []));
        assert.ok(!stack.contains(element));

        stack.push(element);
        assert.ok(stack.contains(element));
    };

    _test['Has element in scope'] = function () {
        var stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(treeAdapter.createElement($.HTML, NS.HTML, []));
        stack.push(treeAdapter.createElement($.DIV, NS.HTML, []));
        assert.ok(!stack.hasInScope($.P));

        stack.push(treeAdapter.createElement($.P, NS.HTML, []));
        stack.push(treeAdapter.createElement($.UL, NS.HTML, []));
        stack.push(treeAdapter.createElement($.BUTTON, NS.HTML, []));
        stack.push(treeAdapter.createElement($.OPTION, NS.HTML, []));
        assert.ok(stack.hasInScope($.P));

        stack.push(treeAdapter.createElement($.TITLE, NS.SVG, []));
        assert.ok(!stack.hasInScope($.P));
    };

    _test['Has numbered header in scope'] = function () {
        var stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(treeAdapter.createElement($.HTML, NS.HTML, []));
        stack.push(treeAdapter.createElement($.DIV, NS.HTML, []));
        assert.ok(!stack.hasNumberedHeaderInScope());

        stack.push(treeAdapter.createElement($.P, NS.HTML, []));
        stack.push(treeAdapter.createElement($.UL, NS.HTML, []));
        stack.push(treeAdapter.createElement($.H3, NS.HTML, []));
        stack.push(treeAdapter.createElement($.OPTION, NS.HTML, []));
        assert.ok(stack.hasNumberedHeaderInScope());

        stack.push(treeAdapter.createElement($.TITLE, NS.SVG, []));
        assert.ok(!stack.hasNumberedHeaderInScope());

        stack.push(treeAdapter.createElement($.H6, NS.HTML, []));
        assert.ok(stack.hasNumberedHeaderInScope());
    };

    _test['Has element in list item scope'] = function () {
        var stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(treeAdapter.createElement($.HTML, NS.HTML, []));
        stack.push(treeAdapter.createElement($.DIV, NS.HTML, []));
        assert.ok(!stack.hasInListItemScope($.P));

        stack.push(treeAdapter.createElement($.P, NS.HTML, []));
        stack.push(treeAdapter.createElement($.BUTTON, NS.HTML, []));
        stack.push(treeAdapter.createElement($.OPTION, NS.HTML, []));
        assert.ok(stack.hasInListItemScope($.P));

        stack.push(treeAdapter.createElement($.UL, NS.HTML, []));
        assert.ok(!stack.hasInListItemScope($.P));
    };

    _test['Has element in button scope'] = function () {
        var stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(treeAdapter.createElement($.HTML, NS.HTML, []));
        stack.push(treeAdapter.createElement($.DIV, NS.HTML, []));
        assert.ok(!stack.hasInButtonScope($.P));

        stack.push(treeAdapter.createElement($.P, NS.HTML, []));
        stack.push(treeAdapter.createElement($.UL, NS.HTML, []));
        stack.push(treeAdapter.createElement($.OPTION, NS.HTML, []));
        assert.ok(stack.hasInButtonScope($.P));

        stack.push(treeAdapter.createElement($.BUTTON, NS.HTML, []));
        assert.ok(!stack.hasInButtonScope($.P));
    };


    _test['Has element in table scope'] = function () {
        var stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(treeAdapter.createElement($.HTML, NS.HTML, []));
        stack.push(treeAdapter.createElement($.DIV, NS.HTML, []));
        assert.ok(!stack.hasInTableScope($.P));

        stack.push(treeAdapter.createElement($.P, NS.HTML, []));
        stack.push(treeAdapter.createElement($.UL, NS.HTML, []));
        stack.push(treeAdapter.createElement($.TD, NS.HTML, []));
        stack.push(treeAdapter.createElement($.OPTION, NS.HTML, []));
        assert.ok(stack.hasInTableScope($.P));

        stack.push(treeAdapter.createElement($.TABLE, NS.HTML, []));
        assert.ok(!stack.hasInTableScope($.P));
    };

    _test['Has table body context in table scope'] = function () {
        var stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(treeAdapter.createElement($.HTML, NS.HTML, []));
        stack.push(treeAdapter.createElement($.DIV, NS.HTML, []));
        assert.ok(!stack.hasTableBodyContextInTableScope());

        stack.push(treeAdapter.createElement($.TABLE, NS.HTML, []));
        stack.push(treeAdapter.createElement($.UL, NS.HTML, []));
        stack.push(treeAdapter.createElement($.TBODY, NS.HTML, []));
        stack.push(treeAdapter.createElement($.OPTION, NS.HTML, []));
        assert.ok(stack.hasTableBodyContextInTableScope());

        stack.push(treeAdapter.createElement($.TABLE, NS.HTML, []));
        assert.ok(!stack.hasTableBodyContextInTableScope());

        stack.push(treeAdapter.createElement($.TFOOT, NS.HTML, []));
        assert.ok(stack.hasTableBodyContextInTableScope());
    };

    _test['Has element in select scope'] = function () {
        var stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(treeAdapter.createElement($.HTML, NS.HTML, []));
        stack.push(treeAdapter.createElement($.DIV, NS.HTML, []));
        assert.ok(!stack.hasInSelectScope($.P));

        stack.push(treeAdapter.createElement($.P, NS.HTML, []));
        stack.push(treeAdapter.createElement($.OPTION, NS.HTML, []));
        assert.ok(stack.hasInSelectScope($.P));

        stack.push(treeAdapter.createElement($.DIV, NS.HTML, []));
        assert.ok(!stack.hasInSelectScope($.P));
    };

    _test['Generate implied end tags'] = function () {
        var stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(treeAdapter.createElement($.HTML, NS.HTML, []));
        stack.push(treeAdapter.createElement($.LI, NS.HTML, []));
        stack.push(treeAdapter.createElement($.DIV, NS.HTML, []));
        stack.push(treeAdapter.createElement($.LI, NS.HTML, []));
        stack.push(treeAdapter.createElement($.OPTION, NS.HTML, []));
        stack.push(treeAdapter.createElement($.P, NS.HTML, []));

        stack.generateImpliedEndTags();

        assert.strictEqual(stack.stackTop, 2);
        assert.strictEqual(stack.currentTagName, $.DIV);
    };

    _test['Generate implied end tags with exclusion'] = function () {
        var stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(treeAdapter.createElement($.HTML, NS.HTML, []));
        stack.push(treeAdapter.createElement($.LI, NS.HTML, []));
        stack.push(treeAdapter.createElement($.DIV, NS.HTML, []));
        stack.push(treeAdapter.createElement($.LI, NS.HTML, []));
        stack.push(treeAdapter.createElement($.OPTION, NS.HTML, []));
        stack.push(treeAdapter.createElement($.P, NS.HTML, []));

        stack.generateImpliedEndTagsWithExclusion($.LI);

        assert.strictEqual(stack.stackTop, 3);
        assert.strictEqual(stack.currentTagName, $.LI);
    };

    _test['Template count'] = function () {
        var stack = new OpenElementStack(treeAdapter.createDocument(), treeAdapter);

        stack.push(treeAdapter.createElement($.HTML, NS.HTML, []));
        stack.push(treeAdapter.createElement($.TEMPLATE, NS.MATHML, []));
        assert.strictEqual(stack.tmplCount, 0);

        stack.push(treeAdapter.createElement($.TEMPLATE, NS.HTML, []));
        stack.push(treeAdapter.createElement($.LI, NS.HTML, []));
        assert.strictEqual(stack.tmplCount, 1);

        stack.push(treeAdapter.createElement($.OPTION, NS.HTML, []));
        stack.push(treeAdapter.createElement($.TEMPLATE, NS.HTML, []));
        assert.strictEqual(stack.tmplCount, 2);

        stack.pop();
        assert.strictEqual(stack.tmplCount, 1);

        stack.pop();
        stack.pop();
        stack.pop();
        assert.strictEqual(stack.tmplCount, 0);
    };
});
