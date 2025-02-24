# Set up an image that just runs the datastore emulator.
FROM google/cloud-sdk:latest
ARG emulator_port=8888
ARG emulator_project=chromeperf
ARG emulator_host=localhost
ENV EMULATOR_HOST $emulator_host
ENV EMULATOR_PORT $emulator_port
ENV EMULATOR_PROJECT $emulator_project

# TODO(dberris): Figure out how to signal this container to shutdown externally.
CMD gcloud beta emulators datastore start \
        --project "$EMULATOR_PROJECT" \
        --host-port="$EMULATOR_HOST:$EMULATOR_PORT" \
        --no-store-on-disk \
        --consistency=1.0
