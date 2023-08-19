FROM aveferrum/rg35xx-toolchain as toolchain-source
FROM docker.io/debian:bookworm-slim

# Grab all the goodies including patched cross-compiled SDL 1.2 libs
COPY --from=toolchain-source /opt/ /opt

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get install -y make gcc g++ \
    && apt-get -y autoremove \
    && apt-get -y clean

RUN apt-get install -y git vim

RUN mkdir -p /root/workspace
RUN mkdir -p /root/build
WORKDIR /root/build
COPY build/build-libyaml .
RUN ./build-libyaml


WORKDIR /root/workspace
COPY workspace/docker_env.sh .
RUN cat docker_env.sh >> /root/.bashrc

CMD ["/bin/bash"]
