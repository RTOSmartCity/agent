FROM gcc:latest AS builder

RUN apt-get update && \
    apt-get install -y cmake 

WORKDIR /app

RUN git clone https://github.com/nats-io/nats.c.git && \
    cd nats.c && \
    mkdir -p build && \
    cd build && \
    cmake .. -DNATS_BUILD_STREAMING=OFF && \
    make && \
    make install && \
    ldconfig /usr/local/lib64/


COPY . .

RUN gcc -o broker src/main.c -lnats

FROM debian:latest

WORKDIR /broker
COPY --from=builder /app/broker /broker/broker
COPY --from=builder /usr/local/lib /usr/local/lib
COPY --from=builder /usr/local/include /usr/local/include

CMD ["/broker/broker"]

