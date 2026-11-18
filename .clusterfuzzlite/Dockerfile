# Runs as root on purpose: the OSS-Fuzz base-builder `compile` step writes to
# /usr/lib and requires root. This is a throwaway CI build image, not a service.
# checkov:skip=CKV_DOCKER_3:OSS-Fuzz compile requires root
# trivy:ignore:AVD-DS-0002
FROM gcr.io/oss-fuzz-base/base-builder
RUN apt-get update && apt-get install -y --no-install-recommends make autoconf automake libtool

COPY . $SRC/json_struct
COPY .clusterfuzzlite/build.sh $SRC/build.sh
WORKDIR $SRC/json_struct

HEALTHCHECK --interval=5m --timeout=3s CMD true || exit 1
