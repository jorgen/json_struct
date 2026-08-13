FROM gcr.io/oss-fuzz-base/base-builder
RUN apt-get update && apt-get install -y --no-install-recommends make autoconf automake libtool

COPY . $SRC/json_struct
COPY .clusterfuzzlite/build.sh $SRC/build.sh
WORKDIR $SRC/json_struct

RUN groupadd -r user && \
    useradd -r -g user -m -d /home/user -s /bin/bash user

USER user

HEALTHCHECK --interval=5m --timeout=3s \
