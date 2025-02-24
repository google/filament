# Performance Dashboard

The Chrome Performance Dashboard is an App Engine web application for displaying
and monitoring performance test results.

-   [Getting set up to contribute](/dashboard/docs/getting-set-up.md)
-   [Data format for new graph data](/dashboard/docs/data-format.md)
-   [Rolling back from a broken deployment](/dashboard/docs/rollback.md)
-   [Project glossary](/dashboard/docs/glossary.md)
-   [Pages and endpoints](/dashboard/docs/pages-and-endpoints.md)

## Running a local development server

Running a local instance (i.e., a dev server) of the Performance Dashboard is
no longer supported due to a python2 dependency in the appengine toolchain.

To manually test your python server changes, deploy them to chromeperf-stage.

## Code Structure

All dashboard code lives in the `dashboard/` subdirectory, with the endpoints
for individual HTTP API handlers in that directory. There are a number of
subprojects which are also hosted in that directory:

-   `api`: Handlers for API endpoints.
-   `common`: A module collecting various utilities and common types used across
    multiple subprojects in the performance dashboard.
-   `docs`: A collection of documents for users of the dashboard, the API, and
    other subprojects.
-   `elements`: User interface elements used in the performance dashboard web
    user interface. This is deprecated in favor of `spa`.
-   `models`: A collection of types which represent entities in the data store,
    with associated business logic for operations. These models should be
    thought of as models in the Model-View-Controller conceptual framework.
-   `pinpoint`: The performance regression bisection implementation. See more in
    the [pinpoint documentation](/dashboard/dashboard/pinpoint/README.md).
-   `services`: A collection of wrappers which represent external services which
    the dashboard subprojects interact with.
-   `static`: Directory containing all the static assets used in the user
    interface. This is deprecated in favor of `spa`.
-   `templates`: HTML files representing the templates for views served through
    the App Engine user interface. This is deprecated in favor of `spa`.
-   `sheriff_config`: A standalone service for managing sheriff configurations
    hosted in git repositories, accessed through luci-config.

## Dependencies

The dashboard has a few dependencies. Before running dashboard unit tests,
be sure to following all instructions under this section.

### Google Cloud SDK

The dashboard requires Python modules from Google Cloud SDK to run.
An easy way to install it is through cipd, using the following command.
You only need to do this once.
(You can replace `~/google-cloud-sdk` with another location if you prefer.)

```
echo infra/gae_sdk/python/all latest | cipd ensure -root ~/google-cloud-sdk -ensure-file -
```

Then run the following command to set `PYTHONPATH`. It is recommended to add
this to your `.bashrc` or equivalent.

```
export PYTHONPATH=~/google-cloud-sdk
```

If you already have a non-empty `PYTHONPATH`, you can add the Cloud SDK location
to it. However, dashboard does not require any additional Python libraries.
It is recommended that your `PYTHONPATH` only contains the cloud SDK while
testing the dashboard.

(Note: The official source for Google Cloud SDK is https://cloud.google.com/sdk,
and you can install Python modules with
`gcloud components install app-engine-python`.
However, this method of installation has not been verified with the dashboard.)

### Compile Protobuf Definitions

The dashboard uses several protobuf (protocol buffer) definitions, which must be
compiled into Python modules. First you need to install the protobuf compiler,
and then use it to compile the protobuf definition files.

To install the protobuf compiler, use the following command.
You only need to do this once.
(You can replace `~/protoc` with another location if you prefer.)

```
echo infra/tools/protoc/linux-amd64 protobuf_version:v3.6.1 | cipd ensure -root ~/protoc -ensure-file -
```

Afterwards, run the following commands to compile the protobuf definitions.
You need to do this whenever any of the protobuf definition files have changed.
Modify the first line below if your catapult directory is at a different
location.

```
catapult=~/chromium/src/third_party/catapult
~/protoc/protoc --proto_path $catapult/dashboard --python_out $catapult/dashboard $catapult/dashboard/dashboard/protobuf/sheriff.proto $catapult/dashboard/dashboard/protobuf/sheriff_config.proto
cp $catapult/dashboard/dashboard/protobuf/sheriff_pb2.py $catapult/dashboard/dashboard/sheriff_config/
cp $catapult/dashboard/dashboard/protobuf/sheriff_config_pb2.py $catapult/dashboard/dashboard/sheriff_config/
~/protoc/protoc --proto_path $catapult/tracing/tracing/proto --python_out $catapult/tracing/tracing/proto $catapult/tracing/tracing/proto/histogram.proto
```

## Unit Tests

First following the steps given in Dependencies section above.
Then, run dashboard unit tests with:

```
dashboard/bin/run_py_tests
```

## Contact

Bugs can be reported on the Chromium issue tracker using the `Speed>Dashboard`
component:

-   [File a new Dashboard issue](https://bugs.chromium.org/p/chromium/issues/entry?description=Describe+the+problem:&components=Speed%3EDashboard&summary=[chromeperf]+)
-   [List open Dashboard issues](https://bugs.chromium.org/p/chromium/issues/list?q=component%3ASpeed%3EDashboard)

Note that some existing issues can be found in the
[Github issue tracker](https://github.com/catapult-project/catapult/issues), but
this is no longer the preferred location for filing new issues.

For questions and feedback, send an email to
chrome-perf-dashboard-team@google.com.
