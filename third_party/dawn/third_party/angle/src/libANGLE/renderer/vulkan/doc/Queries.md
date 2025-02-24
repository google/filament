# Queries

OpenGL queries generally have a straightforward mapping to Vulkan queries, with the exception of
`GL_PRIMITIVES_GENERATED`.  Some Vulkan queries are active only inside a render pass, while others
are affected by both inside and outside render pass commands.

## Outside Render Pass Queries

The following queries are recorded outside a render pass.  If a render pass is active when
`begin()` or `end()` is called for these queries, it will be broken.

- `GL_TIME_ELAPSED_EXT`
- `GL_TIMESTAMP_EXT`

## Inside Render Pass Queries

The rest of the queries are active only inside render passes.  The context (`ContextVk`) keeps track
of currently active "render pass queries" and automatically pauses and resumes them as render passes
are broken and started again.

- On query `begin()`: `ContextVk::beginRenderPassQuery()` is called which:
  * If a render pass is open, immediately starts the query and keeps track of it
  * Otherwise keeps the query tracked to be started in the next render pass
- On query `end()`: `ContextVk::endRenderPassQuery()` is called which:
  * If a render pass is open, stops the query
  * Loses track of the query
- On render pass start: `ContextVk::resumeRenderPassQueriesIfActive()` is called which starts all
  active queries.
- On render pass end: `ContextVk::pauseRenderPassQueriesIfActive()` is called which stops all
  active queries.

In Vulkan, a query cannot be paused or resumed, only begun and ended.  This means that GL queries
that span multiple render passes must use multiple Vulkan queries whose results are accumulated.
This is done on render pass start, where `QueryVk::onRenderPassStart()` would stash the previous
Vulkan query (if any) and create a new one before starting it.  When a query is begun, the
`QueryVk`'s "current" Vulkan query (`mQueryHelper`) is only allocated if there's a render pass
active.

**Invariant rule**: With the above algorithm, `QueryVk::mQueryHelper` is at all times either
`nullptr` or has commands recorded.  This is important when getting query results to be able to
ask `mQueryHelper` for availability of results.

Later on, `QueryVk::getResult()` would take the sum of the current and all stashed Vulkan queries as
the final result.

### Mid-Render-Pass Clears

If a clear is performed while a render pass query is active and if that clear needs to take a
draw-based path, `UtilsVk` ensures that the draw call does not contribute to query results.  This is
done by pausing (`ContextVk::pauseRenderPassQueriesIfActive`) the queries before the draw call and
resuming (`ContextVk::resumeRenderPassQueriesIfActive`) afterwards.

The rest of the `UtilsVk` draw-based functions start a render pass out of the knowledge of
`ContextVk`, so queries will not be activated.  In the future, if any `UtilsVk` functions use the
current render pass the way `UtilsVk::clearFramebuffer` does, they must also ensure that they pause
and resume queries.

### Transform Feedback Queries

OpenGL has two distinct queries `GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN` and
`GL_PRIMITIVES_GENERATED`.  In Vulkan however, these are served by a single query from
`VK_EXT_transform_feedback`.  Additionally, Vulkan requires that only a single query of any type can
be active at a time.  This forces ANGLE to have the two GL queries share their Vulkan queries when
both transform feedback queries are active.

To support the above use case, `QueryVk`s keep ref-counted (`vk::Shared`) Vulkan query
(`vk::QueryHelper`) objects.  When either transform feedback query is begun:

- If the other transform feedback query (`shareQuery`) is active and a render pass is active:
  * `shareQuery`'s current Vulkan query is stopped and stashed, and a new one is allocated
  * `shareQuery`'s new Vulkan query is taken as this query's current one with the ref-count
     incremented
  * The Vulkan query is started as usual
- If the other transform feedback query is active and a render pass is not:
  * The current Vulkan query is kept `nullptr`.  When the next render pass starts, they will share
    their Vulkan queries.
- If the other transform feedback query is not active, proceed as usual

Breaking the `shareQuery`'s Vulkan query on begin ensures that whatever results it may have accrued
before do not contribute to this query.

Similarly, when a transform feedback query is ended, the Vulkan query is ended as usual and then:

- If the other transform feedback query is active and a render pass is active:
  * `shareQuery`'s Vulkan query (which is the same as this query's, as they share it) is stashed
  * `shareQuery` allocates a new Vulkan query and starts it

When a render pass is broken and active queries are paused
(`ContextVk::pauseRenderPassQueriesIfActive`), only one of the queries will close the shared Vulkan
query.

When a render pass is started and active queries are resumed
(`ContextVk::resumeRenderPassQueriesIfActive`), only one of the queries will allocate and start a
new Vulkan query.  The other one will just take that and share it (and increment the ref-count).

The above solution supports the following scenarios:

- Simultaneous begin of the two queries:

```
glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN)
glBeginQuery(GL_PRIMITIVES_GENERATED)
glDraw*()
```

- Simultaneous end of the two queries:

```
glDraw*()
glEndQuery(GL_PRIMITIVES_GENERATED)
glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN)
```

- Draw calls between begin calls:

```
glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN)
glDraw*()
glBeginQuery(GL_PRIMITIVES_GENERATED)
glDraw*()
```

- Draw calls between end calls:

```
glDraw*()
glEndQuery(GL_PRIMITIVES_GENERATED)
glDraw*()
glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN)
```

- Queries getting deleted / rebound when the other is active, for example:

```
glDraw*()
glEndQuery(GL_PRIMITIVES_GENERATED)
glDeleteQueries(primitivesGenerated)
glDraw*()
glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN)
```
