FROM google/cloud-sdk:326.0.0

ARG PROTOBUF_VERSION=3.6.1.3-2

ENV PATH=/usr/local/depot_tools:${PATH}

# The latest version is in sid repo. So need to specify the
# version of dependency to avoid version conflix if you want
# to use "stable" one.
RUN apt-get --allow-releaseinfo-change update && apt-get install -y \
  libprotoc17=${PROTOBUF_VERSION} \
  protobuf-compiler=${PROTOBUF_VERSION} \
  git \
  && git clone https://chromium.googlesource.com/chromium/tools/depot_tools /usr/local/depot_tools \
  && /usr/local/depot_tools/vpython3 -c ''

ENV PYTHONPATH=/usr/lib/google-cloud-sdk/platform/google_appengine

COPY docker-entrypoint.sh /usr/local/bin/

ENTRYPOINT ["docker-entrypoint.sh"]
