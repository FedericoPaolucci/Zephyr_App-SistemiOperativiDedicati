FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    git cmake ninja-build gperf \
    python3-pip python3-setuptools python3-wheel \
    xz-utils file make gcc g++ \
    dfu-util device-tree-compiler wget \
    libusb-1.0-0-dev \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install west

WORKDIR /work

# Installa toolchain GNU ARM Embedded
RUN wget -q https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2 && \
    tar -xjf gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2 -C /opt && \
    rm gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2 && \
    mv /opt/gcc-arm-none-eabi-10.3-2021.10 /opt/gcc-arm-none-eabi

ENV ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
ENV GNUARMEMB_TOOLCHAIN_PATH=/opt/gcc-arm-none-eabi
