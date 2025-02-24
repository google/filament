# Pinpoint Architecture

## Running stuff

At its core, Pinpoint is a service that can run a list of steps at a particular
commit. In a Pinpoint **Job**, the steps are known as **Quests** and the commit
is known as a **Change**. Usually, there's at least a *Build* Quest and a *Test*
Quest, but a Quest can be arbitrary Python code, as long as it conforms to the
[Quest API](quest/README.md).

When a list of Quests is bound to a Change, an **Attempt** is created. The
Attempt binds each Quest to the Change, creating an **Execution** for each one.
It runs the Executions serially, passing the output of each one to the input of
the following Execution. If any Execution fails, the following Executions are
skipped and the entire Attempt fails. There can be multiple Attempts on the same
Change.

Each individual Attempt handles its own execution, so all Attempts can run in
parallel.

## Bisection

When the Job's `comparison_mode` flag is set, Pinpoint automatically chooses
what Changes to run. It compares the results of each pair of adjacent Changes.
If any two Changes have different results, it finds the midpoint of those
Changes and speculates multiple levels in (currently only two levels) runs the
tests on the additional points as well.

## Data migration

The App Engine documentation provides
[advice](https://cloud.google.com/appengine/articles/update_schema) on when to
update entities in the Datastore.

Most of the Job state is stored in a `PickleProperty`. To update a pickleable
object, commit a change to the object's `__getstate__` method, so that it is
updated when loaded from the Datastore and unpickled. Then use the data
migration page,`/migrate`, which reads and stores every completed Job, causing
them all to update. If there are Jobs in flight, you may need to run multiple
migrations.
