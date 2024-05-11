# To run the image and mount the current directory:
#   docker run --rm -it -v `pwd`:/filament -t zbloat

# Then, inside the container:
#   ./tools/zbloat/zbloat.py my_favorite_app.apk
#
# To make this easier, see zbloat.sh

FROM ubuntu:focal
ARG DEBIAN_FRONTEND=noninteractive

WORKDIR filament

RUN apt-get update && \
    apt-get --no-install-recommends install -y \
	apt-transport-https \
	apt-utils \
	build-essential \
	python \
	python3 \
	curl \
	gnupg2

RUN curl -sL https://deb.nodesource.com/setup_15.x | bash - && \
	apt-get install -y nodejs

RUN curl -sS https://dl.yarnpkg.com/debian/pubkey.gpg | apt-key add - && \
	echo "deb https://dl.yarnpkg.com/debian/ stable main" | tee /etc/apt/sources.list.d/yarn.list && \
	apt-get update && \
	apt-get install -y yarn

RUN yarn global add webtreemap typescript
