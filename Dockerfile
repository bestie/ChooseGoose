FROM aveferrum/rg35xx-toolchain as toolchain-source
FROM docker.io/debian:bookworm-20240701-slim

# Grab all the goodies including patched cross-compiled SDL 1.2 libs
COPY --from=toolchain-source /opt/ /opt

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get install -y \
      make \
      gcc \
      g++ \
      gdb \
      git \
      wget \
      curl \
      lbzip2 \
      xxd \
      libsdl1.2-dev \
      libsdl-ttf2.0-dev \
      libsdl-image1.2-dev \
      vim \
      ripgrep \
    && apt-get -y autoremove \
    && apt-get -y clean

WORKDIR /root/choosegoose
COPY . .

CMD ["/bin/bash"]
