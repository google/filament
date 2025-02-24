# Running Integration Tests

You can use Docker (and `docker-compose`) to run the integration test for the
sheriff-config service:

```
$ docker-compose build
...
$ docker-compose run sheriff-config-test
...
$ docker-compose down
```

Or, you can do it in a single line:

```
$ docker-compose build && docker-compose run sheriff-config-test && \
    docker-compose down
```

This will automatically invoke the unit tests and the quasi-end-to-end tests
along with the service dependencies.

The integration tests currently:

-   Depend on the Google Cloud SDK for running a Datastore Emulator in a
    container.
-   Mocks out the luci-config service responses, until we have a way of
    building/running a luci-config Docker image.
-   Do not test any browser-related functionality (since we don't need to for
    the sheriff-config service).
