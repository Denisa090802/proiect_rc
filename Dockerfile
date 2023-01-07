FROM ubuntu

RUN apt-get update && \
    apt-get -y install gcc
RUN apt-get -y update && apt-get install -y
RUN apt-get -y install clang
RUN apt-get -y install build-essential
RUN apt-get -y install net-tools
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      libncurses-dev \
      sqlite3 \
      libsqlite3-dev \
      libedit-dev

WORKDIR /app
COPY . .
RUN g++ main.cpp -o main -l sqlite3
RUN echo $(hostname -i)
EXPOSE 8452
ENTRYPOINT [ "./main" ]