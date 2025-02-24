# Overview of dev_server testing

## Introduction

Catapult has a simple, optionally asynchronous, JavaScript testing framework.
The framework is located in `/tracing/base/unittest/`.

Test files exist in `<filename>_test.html` files where, typically, filename
will match the name of the file being tested. The tests sit in the same
folder as their respective files.

## Test Creation

The general structure of tests is (assuming a file of ui/foo_test.html):

```
<link rel="import" href="/ui/foo.html">
<script>
'use strict';

tr.b.unittest.testSuite(function() {
  test('instantiate', function() {
    var myFoo = ui.Foo();
    this.addHTMLOutput(myFoo);
  });

  test('somethingElse', function() {
  });
});
```

Generally, there is one test suite per file (there is an assumption inside the
code that this is true).

If you add something to the DOM with `appendChild` you should remove it. The
exception is if you use `this.addHTMLOutput(element)`. If you use that, then
you should be good, the content will get shown if there is an error,
otherwise it's hidden.

The current tests follow the convention that if the test is there just to draw
things, to name them with a prefix of instantiate. So, `instantiate`,
`instantiate_multiRow`, etc.

## Chai

Catapult uses [Chai](http://chaijs.com) for assertions. We are using Chai's
[TDD `assert` style](http://chaijs.com/api/assert/).

## Execution

You'll need to start a dev_server to run the tests
```
$ bin/run_dev_server
```

After you start the dev_server, it'll be available at http://localhost:8003.
You'll see links to run unit tests for all projects. We'll use the `tracing/`
project as an example below.

### Running all tests

```
http://localhost:8003/tracing/tests.html
```

### Running an individual test suite (such as `ui/foo_test.js`)

```
http://localhost:8003/tracing/tests.html?testSuiteName=ui.foo
```

### Running tests named blah

```
http://localhost:8003/tracing/tests.html?testFilterString=blah
```

## Options

If you select the `small format` option on the main test page and reload then
the test output will be condensed to a lot smaller, making it easier to see
errors without having to scroll the screen.
