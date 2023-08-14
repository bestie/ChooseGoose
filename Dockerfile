FROM docker.io/nfriedly/miyoo-toolchain:steward AS toolchain-source
FROM docker.io/debian:bookworm-slim

# Copy docker-gen binary from build stage
COPY --from=toolchain-source /opt/ /opt

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get install -y make gcc g++ \
    file wget cpio zip unzip rsync bc git autoconf\  
    && apt-get -y autoremove \
    && apt-get -y clean

COPY ./build /root/build/
WORKDIR /root/build/
# Build and install SDL
RUN ./build-sdl.sh

RUN cat setup-env.sh >> /root/.bashrc
WORKDIR /root/workspace

CMD ["/bin/bash"]
