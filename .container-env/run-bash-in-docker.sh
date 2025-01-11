#!/bin/bash

set -e
DOCKER_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PRJ_ROOT="$(dirname "$DOCKER_DIR")"
WEST_WORKSPACE_HOST=$PRJ_ROOT/.west_workspace
WEST_WORKSPACE_CONTAINER=/opt/zephyrproject
WORKDIR_HOST=$PRJ_ROOT
WORKDIR_CONTAINER=$WEST_WORKSPACE_CONTAINER/$(basename "$PRJ_ROOT")

mkdir -p $WEST_WORKSPACE_HOST
docker build --network host --tag zephyr-box-hka .

docker run --network host -ti --rm --privileged \
	-v $WEST_WORKSPACE_HOST:$WEST_WORKSPACE_CONTAINER \
	-v $WORKDIR_HOST:$WORKDIR_CONTAINER \
	-v ~/.ssh:/home/user/.ssh \
	-w $WORKDIR_CONTAINER \
	-e WEST_WORKSPACE_CONTAINER=$WEST_WORKSPACE_CONTAINER \
	-e WORKDIR_CONTAINER=$WORKDIR_CONTAINER \
	-e RUN_IN_TERM=true \
	-p 80:80 \
	-v /dev:/dev \
	zephyr-box-hka
