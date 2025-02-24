# Getting started with the Performance Dashboard

## Prerequisites

1. [Download the Google Cloud SDK.](https://cloud.google.com/sdk/downloads)
2. Update the Cloud SDK and set the default project to your project ID by
   invoking the following commands:
   ```
   gcloud components update
   gcloud components install app-engine-python
   gcloud config set project [PROJECT-ID]
   ```
   Replace `[PROJECT-ID]` with your project ID. For chromeperf.appspot.com,
   it's `chromeperf`.
3. Make sure that gcloud is in your PATH.
4. Make sure that you have installed
[protoc](https://github.com/protocolbuffers/protobuf).

## Running the tests

To run the Python unit tests, use `bin/run_py_tests`. To run the front-end
component tests, use `bin/run_dev_server_tests`.

## Running a local instance

Running a local instance (i.e., a dev server) of the Performance Dashboard is
no longer supported due to a python2 dependency in the appengine toolchain.

To manually test your python server changes, deploy them to chromeperf-stage.

## Deploying to production

See [Docker deploy](/dashboard/dev_dockerfiles/README.md).

## Where to find documentation

- [App Engine](https://developers.google.com/appengine/docs/python/)
- [Polymer](http://www.polymer-project.org/) (web component framework)
- [Flot](http://flotcharts.org/) (JS chart plotting library)
- [App engine stubs](https://developers.google.com/appengine/docs/python/tools/localunittesting)
- [Python mock](http://www.voidspace.org.uk/python/mock/)
