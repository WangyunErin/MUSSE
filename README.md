# MUSSE

This project contains implementation of MUSSE.

# Pre-requisites

MUSSE implementation needs a compiler supporting C++14. It has been successfully built and tested on Ubuntu 18.04.
It uses Google's [gRPC](http://grpc.io) as its RPC machinery.
The linux build commands are as follows.

## Linux

```sh
$ sudo apt-get update
$ sudo apt-get install build-essential autoconf libtool yasm openssl scons pkg-config libgflags-dev libgtest-dev clang libc++-dev git unzip libgflags-dev libsnappy-dev zlib1g-dev libbz2-dev liblz4-dev  libssl-dev libcrypto++-dev libssl1.0.0 libgmp-dev curl make automake jq cmake
```

### Installing GRPC
```sh
$ git clone -b $(curl -L https://grpc.io/release) https://github.com/grpc/grpc
$ cd grpc/
$ git submodule update --init
add -Wno-unused-variable at the end of line 356 in Makefile
$ make
$ sudo make install
$ cd ..
```

### Installing Relic
```sh
$ git clone https://github.com/relic-toolkit/relic.git
$ cd relic/
$ mkdir build
$ cd build/
$ cmake ..
$ make
$ sudo make install
$ cd ../..
```

### Installing Protobuf
```sh
$ wget https://github.com/google/protobuf/releases/download/v3.5.1/protobuf-cpp-3.5.1.zip
$ unzip protobuf-cpp-3.5.1.zip
$ cd protobuf-3.5.1/
$ ./configure
$ make
$ sudo make install
$ cd ..
```

### Reloading Modules
```sh
$ sudo ldconfig
```

## Go to the musse folder
$ cd MUSSE/

# Building

To build the code itself, and run its tests just enter in your terminal

```sh
 $ ./build.sh
```

# Usage

After compiling the project, several binary files will be generated. 
1- xxx_single is designed to run some tests on a single machine system. It executes client and server operations as a local command.
2- xxx_client and musse_server are designed to execute MUSSE in a real environment. Indeed, the client uses the existing GRPC services provided by the server to run setup, update, and search operations. 
3- mkse is the implementation of Hamlin et al. scheme (we refer the readers to their paper for more information)

