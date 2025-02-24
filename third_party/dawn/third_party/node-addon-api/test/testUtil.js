// Run each test function in sequence,
// with an async delay and GC call between each.

function tick (x) {
  return new Promise((resolve) => {
    setImmediate(function ontick () {
      if (--x === 0) {
        resolve();
      } else {
        setImmediate(ontick);
      }
    });
  });
}

async function runGCTests (tests) {
  // Break up test list into a list of lists of the form
  // [ [ 'test name', function() {}, ... ], ..., ].
  const testList = [];
  let currentTest;
  for (const item of tests) {
    if (typeof item === 'string') {
      currentTest = [];
      testList.push(currentTest);
    }
    currentTest.push(item);
  }

  for (const test of testList) {
    await (async function (test) {
      let title;
      for (let i = 0; i < test.length; i++) {
        if (i === 0) {
          title = test[i];
        } else {
          try {
            test[i]();
          } catch (e) {
            console.error('Test failed: ' + title);
            throw e;
          }
          if (i < tests.length - 1) {
            global.gc();
            await tick(10);
          }
        }
      }
    })(test);
  }
}

module.exports = {
  runGCTests
};
