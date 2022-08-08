FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y ninja-build cmake gcc g++ git python3 python3-distutils python3-dev python3-pip nasm clang-format
COPY . /sold
WORKDIR /sold
# TODO
# RUN ./run-format.sh
RUN rm -rf build
RUN mkdir build
RUN cmake -GNinja -S . -B build
RUN cmake --build build
RUN cd build && ctest --output-on-failure