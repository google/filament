# Dashboard Dockerfile

This is an attempt to make the testing and deploying process for dashboard
more consistent and running everything in docker.

There is a `run` script helps you to fetch some information from host and set
it into docker. It's only a simple wrap of `docker-compose run`. And you
can always use this script as the entrypoint.

## Initial setup

### Install Docker and `docker-compose`.

This process requires that you have Docker installed locally. Googlers, see
[go/install-docker](http://go/install-docker).

E.g. `sudo apt install docker-ce docker-compose`.

### Authenicate to gcloud

```
cd dev_dockerfiles
./run auth
```

## Usage

### Build Images

**You need to manually rebuild the images every time Dockerfile have updated currently.** This process can be automated in `run` script later.

```
docker-compose down
docker-compose build --no-cache
```

### Run Python Unit Tests

```
./run python-unittest
```

### Deploy Dashboard

```
./run deploy-dashboard
```

### Deploy Pinpoint

```
./run deploy-pinpoint
```