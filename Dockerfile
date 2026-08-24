FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /app

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        git \
        pkg-config \
        libssl-dev \
        libpq-dev \
        ca-certificates && \
    rm -rf /var/lib/apt/lists/*

COPY . /app

RUN cmake -S /app -B /app/build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build /app/build --parallel 2

EXPOSE 18080

CMD ["/app/build/tradeflow_app"]
