<!-- Copyright 2017 The Chromium Authors. All rights reserved.
     Use of this source code is governed by a BSD-style license that can be
     found in the LICENSE file.
-->

# CPU Time MultiDimensionalView Explainer

This document explains the MultiDimensionalView returned by `constructMultiDimensionalView` in `cpuTime.html`.

The returned MultiDimensionalView is in TopDownTreeView mode. It is three
dimensional (processType, threadType, and railStage + initiator). Rail stage and
initiator are not separate dimensions because they are not independent - there
is no such thing as CSS Response or Scroll Load.

Each node in the tree view contains two values - cpuUsage and cpuTotal.

When talking about multidimensional tree views, a useful abstration is "path",
which uniquely determines a node in the tree: A path is a 3 element array, and
each of these three elements is a possibly empty array of strings. Here is an
example path:
```
[ ['browser_process'], ['CrBrowserMain'], ['Animation', 'CSS'] ]
    Dimension 1          Dimension 2          Dimension 3
```

We can arrive at the node denoted by this path in many different ways starting
from the root node, so this path is not to be confused with the graph theoretic
notion of path. Here is one of the ways to reach the node (we show the
intermediate paths during the traversal inline):

```javascript
const node = treeRoot  // [[], [], []]
  .children[0]  // access children along first dimension
  .get('browser_process')  // [['browser_process'], [], []]
  .children[2]  // access children along third dimension
  .get('Animation')  // [['browser_process'], [], ['Animation']]
  .children[1]  // Access children along second dimension
  .get('CrBrowserMain')  // [['browser_process'], ['CrBrowserMain'], ['Animation']]
  .children[2]  // Go further down along third dimension
  .get('CSS')  // [['browser_process'], ['CrBrowserMain'], ['Animation', 'CSS']]
```
Now node.values contains the cpu time data for the browser main thread during
the CSS Animation stage:
- `node.values[0]` is `cpuUsage` - cpu time over per unit of wall clock time
- `node.values[1]` is `cpuTotal` - total miliseconds of used cpu time

The path for the node that hold data for all threads of renderer process
during scroll response expectations is `[['renderer_process'], [], ['Response', 'Scroll']]`.

As we can see, we simply have an empty array for the second dimension. This
works similarly if we want to get data for all processes for a particular
thread.

However, if we want to access data for all rail stages and all initiator
types, we have to use the special rail stage `all_stages`, and initiator
type `all_initiators`. For example, to get cpu data during all Response
stages for all processes and threads, we use the node at path
  `[[], [], ['Response', 'all_initiators']]`

To get cpu data for all rail stages for ChildIOThread, we use the path
  `[[], ['ChildIOThread'], ['all_stages', 'all_initiators']]`

This is because the tree view automatically aggregates cpu time
data along each dimension by summing values on the children nodes. For
aggregating rail stages and initiator types, summing is not the right thing
to do since

  1. User Expectations can overlap (for example, one tab can go through a
  Video Animation while another tab is concurrently going through a CSS
  Animation - it's worth noting that user expectations are not scoped to a
  tab.)

  2. Different rail stages have different durations (for example, if we
  have 200ms of Video Animation with 50% cpuUsage, and 500ms of CSS
  Animation with 60% cpuUage, cpuUsage for all Animations is clearly not
  110%.)

We therefore more manually do the appropriate aggregations and store the
data in `all_stages` and `all_initiators` nodes.
