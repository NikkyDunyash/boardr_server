FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive
RUN apt update && \
    apt install -y git gcc g++ cmake \
    libjsoncpp-dev uuid-dev zlib1g-dev openssl libssl-dev postgresql-all \
    wget unzip

WORKDIR /home/cryptopp      
RUN wget https://github.com/weidai11/cryptopp/releases/download/CRYPTOPP_8_8_0/cryptopp880.zip && \
    unzip -a cryptopp880.zip && make static && make install PREFIX=/usr/local
    
WORKDIR /home/app
RUN git clone https://github.com/drogonframework/drogon && \
    cd drogon && \
    git submodule update --init && \
    mkdir build && \
    cd build && \
    cmake .. && make && make install

WORKDIR /home/app/SSL
RUN openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem \
    -sha256 -days 3650 -nodes \
    -subj "/C=RU/ST=/L=MOS/CN=localhost"

WORKDIR /home/app
COPY . .

WORKDIR /home/app/build
RUN  cmake .. && make
EXPOSE 5555
CMD ./boardr_server