FROM ubuntu:25.04 AS builder

ARG DEBIAN_FRONTEND=noninteractive
ARG SIMULATION=native
ENV SIMULATION=${SIMULATION}
ARG BUILDTYPE=debug
ENV BUILDTYPE=${BUILDTYPE}
ARG OMIT_DEPRECATED=false
ENV OMIT_DEPRECATED=${OMIT_DEPRECATED}

# Rust envs
ARG RUST_TOOLCHAIN=nightly
ENV RUSTUP_HOME=/root/.rustup
ENV CARGO_HOME=/root/.cargo
ENV PATH=/root/.cargo/bin:${PATH}

RUN \
  apt-get update && \
  apt-get -y upgrade && \
  apt-get install -y build-essential git cmake curl && \
  rm -rf /var/lib/apt/lists/*

# Install rustup and a specific toolchain (override with --build-arg RUST_TOOLCHAIN=...)
RUN \
   curl -fsSL https://sh.rustup.rs | sh -s -- -y --no-modify-path --profile minimal --default-toolchain ${RUST_TOOLCHAIN} && \
   rustup toolchain install ${RUST_TOOLCHAIN} && \
   rustup default ${RUST_TOOLCHAIN} && \
   rustc -V && cargo -V

WORKDIR /cfs
COPY . .

RUN git submodule init \
  && git submodule update \
  && cp cfe/cmake/Makefile.sample Makefile \
  && cp -r cfe/cmake/sample_defs .

RUN make prep
RUN make
RUN make install

FROM ubuntu:25.04
COPY --from=builder /cFS/build /cFS/build
WORKDIR /cFS/build/exe/cpu1
ENTRYPOINT [ "./core-cpu1" ]