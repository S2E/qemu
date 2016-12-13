# Compile and install qemu. To get smaller image, use docker/build script.

FROM ubuntu:16.04
MAINTAINER Vitaly Chipounov <vitaly@cyberhaven.io>

RUN apt-get update

RUN DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential git python zlib1g-dev pkg-config libglib2.0-dev libpng12-dev

RUN adduser --disabled-password --gecos "" qemu

RUN mkdir -p /home/qemu/qemu-build/build && \
    mkdir -p /qemu && \
    chown qemu:qemu /home/qemu/qemu-build/build /qemu

# These files stay in a layer and can't be easily removed.
COPY . /home/qemu/qemu-build/qemu

RUN su qemu -c " \
        cd /home/qemu/qemu-build/build && \
        ../qemu/configure --target-list=i386-softmmu,x86_64-softmmu --disable-docs --disable-xen --prefix=/qemu && \
        make -j32 install \
        "
