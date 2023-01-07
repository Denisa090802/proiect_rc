FROM ubuntu:latest
RUN apt update
RUN apt-get -y install build-essential

WORKDIR /app

COPY ./client .
RUN g++ interface.cpp -o interface

ENTRYPOINT ["tail", "-f", "/dev/null"]